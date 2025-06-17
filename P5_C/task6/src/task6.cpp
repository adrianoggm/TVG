#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkExtractImageFilter.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        std::cerr << "Uso: " << argv[0]
                  << " <volumen3D> <slice2D_salida> <indiceSliceZ>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* inputVolume = argv[1];
    const char* outputSlice  = argv[2];
    const unsigned int sliceIndex = std::stoi(argv[3]);

    constexpr unsigned int InputDimension  = 3;
    constexpr unsigned int OutputDimension = 2;
    using PixelType = float;

    using VolumeType = itk::Image<PixelType, InputDimension>;
    using SliceType  = itk::Image<PixelType, OutputDimension>;

    // 1) Leer el volumen 3D
    auto reader = itk::ImageFileReader<VolumeType>::New();
    reader->SetFileName(inputVolume);
    try {
        reader->Update();
    }
    catch (itk::ExceptionObject& err) {
        std::cerr << "Error al leer volumen: " << err << std::endl;
        return EXIT_FAILURE;
    }

    // 2) Definir la región de extracción: misma anchura/alto, profundidad = 0
    VolumeType::RegionType volRegion =
        reader->GetOutput()->GetLargestPossibleRegion();

    VolumeType::SizeType size = volRegion.GetSize();
    size[2] = 0;  // colapsar la dimensión Z

    VolumeType::IndexType start = volRegion.GetIndex();
    start[2] = sliceIndex; // slice deseado

    VolumeType::RegionType sliceRegion;
    sliceRegion.SetSize(size);
    sliceRegion.SetIndex(start);

    // 3) Filtro de extracción
    using ExtractFilterType = itk::ExtractImageFilter<VolumeType, SliceType>;
    auto extractFilter = ExtractFilterType::New();
    extractFilter->SetExtractionRegion(sliceRegion);
    extractFilter->SetInput(reader->GetOutput());
    extractFilter->SetDirectionCollapseToSubmatrix(); 
    // necesaria para ajustar la matriz de dirección de 3D a 2D

    try {
        extractFilter->Update();
    }
    catch (itk::ExceptionObject& err) {
        std::cerr << "Error en extraccion: " << err << std::endl;
        return EXIT_FAILURE;
    }

    // 4) Escribir el slice 2D resultante
    using WriterType = itk::ImageFileWriter<SliceType>;
    auto writer = WriterType::New();
    writer->SetFileName(outputSlice);
    writer->SetInput(extractFilter->GetOutput());

    try {
        writer->Update();
    }
    catch (itk::ExceptionObject& err) {
        std::cerr << "Error al escribir slice: " << err << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Slice Z=" << sliceIndex
              << " extraído y guardado en: " << outputSlice << std::endl;
    return EXIT_SUCCESS;
}
