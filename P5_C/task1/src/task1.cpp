#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkPNGImageIO.h"
#include "itkBMPImageIO.h"
#include "itkGDCMImageIO.h"

#include <iostream>

int main(int argc, char * argv[])
{
    if (argc < 3)
    {
        std::cerr << "Uso: " << argv[0] << " <imagen_entrada> <imagen_salida>" << std::endl;
        return EXIT_FAILURE;
    }

    const char * inputFile = argv[1];
    const char * outputFile = argv[2];

    constexpr unsigned int Dimension = 2;
    using PixelType = unsigned char;
    using ImageType = itk::Image<PixelType, Dimension>;

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFile);

    auto writer = itk::ImageFileWriter<ImageType>::New();
    writer->SetFileName(outputFile);

    writer->SetInput(reader->GetOutput());

    try
    {
        writer->Update();
    }
    catch (itk::ExceptionObject & error)
    {
        std::cerr << "Excepción capturada al escribir la imagen:" << std::endl;
        std::cerr << error << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Conversión completada: " << inputFile << " -> " << outputFile << std::endl;
    return EXIT_SUCCESS;
}
