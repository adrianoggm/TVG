#include "itkImage.h"
#include "itkRGBPixel.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkNumericTraits.h"
#include "itkIndex.h"

// Visualización con QuickView
#include "QuickView.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Uso: " << argv[0] << " <imagen_entrada_RGB> <imagen_salida_RGB>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];

    constexpr unsigned int Dimension = 2;
    using ComponentType = unsigned char;
    using RGBPixelType = itk::RGBPixel<ComponentType>;
    using RGBImageType = itk::Image<RGBPixelType, Dimension>;

    // Lector
    auto reader = itk::ImageFileReader<RGBImageType>::New();
    reader->SetFileName(inputFile);

    try {
        reader->Update();
    } catch (itk::ExceptionObject& err) {
        std::cerr << "Excepción al leer la imagen: " << err << std::endl;
        return EXIT_FAILURE;
    }

    // Acceso a un píxel RGB (por ejemplo, [25, 35])
    RGBImageType::IndexType pixelIndex = {25, 35};
    RGBPixelType pixelValue = reader->GetOutput()->GetPixel(pixelIndex);

    std::cout << "Valores del píxel (25, 35):\n";
    std::cout << "  Rojo   = " << static_cast<int>(pixelValue.GetRed()) << "\n";
    std::cout << "  Verde  = " << static_cast<int>(pixelValue.GetGreen()) << "\n";
    std::cout << "  Azul   = " << static_cast<int>(pixelValue.GetBlue()) << "\n";

    // Escritor
    auto writer = itk::ImageFileWriter<RGBImageType>::New();
    writer->SetInput(reader->GetOutput());
    writer->SetFileName(outputFile);

    try {
        writer->Update();
    } catch (itk::ExceptionObject& err) {
        std::cerr << "Excepción al escribir la imagen: " << err << std::endl;
        return EXIT_FAILURE;
    }

    // Mostrar la imagen con QuickView
    QuickView viewer;
    viewer.AddRGBImage(reader->GetOutput(), false, "Imagen RGB");
    viewer.Visualize();

    return EXIT_SUCCESS;
}
