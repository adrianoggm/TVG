#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkVTKImageIO.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Uso: " << argv[0] << " <imagen_entrada.mha> <imagen_salida.vtk>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];

    constexpr unsigned int Dimension = 3;
    using PixelType = float;
    using ImageType = itk::Image<PixelType, Dimension>;

    // Lector
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFile);

    // Writer con IO explícito
    auto writer = itk::ImageFileWriter<ImageType>::New();
    writer->SetInput(reader->GetOutput());
    writer->SetFileName(outputFile);

    auto vtkIO = itk::VTKImageIO::New();
    writer->SetImageIO(vtkIO);  // <-- Aquí se fuerza el uso de VTKImageIO

    try
    {
        writer->Update();
    }
    catch (itk::ExceptionObject& err)
    {
        std::cerr << "Excepción al escribir: " << err << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Imagen exportada a VTK correctamente: " << outputFile << std::endl;
    return EXIT_SUCCESS;
}
