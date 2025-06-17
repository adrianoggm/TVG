#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCastImageFilter.h"
#include "QuickView.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Uso: " << argv[0] << " <imagen_entrada> <imagen_salida_cast>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];

    constexpr unsigned int Dimension = 2;
    using InputPixelType = float;
    using OutputPixelType = unsigned char;

    using InputImageType = itk::Image<InputPixelType, Dimension>;
    using OutputImageType = itk::Image<OutputPixelType, Dimension>;

    // Lector
    auto reader = itk::ImageFileReader<InputImageType>::New();
    reader->SetFileName(inputFile);

    // Filtro de rescalado: float [min,max] → float [0,255]
    auto rescaler = itk::RescaleIntensityImageFilter<InputImageType, InputImageType>::New();
    rescaler->SetInput(reader->GetOutput());
    rescaler->SetOutputMinimum(0.0);
    rescaler->SetOutputMaximum(255.0);

    // Filtro de casteo: float [0,255] → uchar
    auto caster = itk::CastImageFilter<InputImageType, OutputImageType>::New();
    caster->SetInput(rescaler->GetOutput());

    // Escritura de la imagen convertida
    auto writer = itk::ImageFileWriter<OutputImageType>::New();
    writer->SetFileName(outputFile);
    writer->SetInput(caster->GetOutput());

    try {
        writer->Update();
    } catch (itk::ExceptionObject& err) {
        std::cerr << "Excepción al escribir: " << err << std::endl;
        return EXIT_FAILURE;
    }

    // Visualización de original y convertida
    QuickView viewer;
    viewer.AddImage(reader->GetOutput(), false, "Imagen original (float)");
    viewer.AddImage(caster->GetOutput(), true, "Imagen convertida (uchar)");
    viewer.Visualize();

    return EXIT_SUCCESS;
}
