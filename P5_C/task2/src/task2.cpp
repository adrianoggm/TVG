#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkVTKImageIO.h"
#include "itkRawImageIO.h"

#include <iostream>
#include <string>
#include <algorithm>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Uso: " << argv[0]
                  << " <imagen_entrada(.mha|.raw)> <imagen_salida.vtk>\n";
        return EXIT_FAILURE;
    }

    const std::string inputFile  = argv[1];
    const std::string outputFile = argv[2];

    // Extraer extensión en minúsculas
    auto pos = inputFile.find_last_of('.');
    std::string ext = (pos == std::string::npos ? "" : inputFile.substr(pos));
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    constexpr unsigned int Dimension = 3;
    using PixelType = unsigned char;                  // MET_UCHAR
    using ImageType = itk::Image<PixelType, Dimension>;

    // --- Reader ---
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFile);

    if (ext == ".raw")
    {
        // Parámetros embebidos (sin .mhd externo):
        const unsigned int dimX = 181;
        const unsigned int dimY = 217;
        const unsigned int dimZ = 1;    // RAW file has only one slice

        using RawIOType = itk::RawImageIO<PixelType, Dimension>;
        auto rawIO = RawIOType::New();
        rawIO->SetFileTypeToBinary();
        rawIO->SetDimensions(0, dimX);
        rawIO->SetDimensions(1, dimY);
        rawIO->SetDimensions(2, dimZ);
        rawIO->SetPixelType(itk::ImageIOBase::SCALAR);
        rawIO->SetNumberOfComponents(1);
        rawIO->SetByteOrderToLittleEndian(); // ElementByteOrderMSB = False
        reader->SetImageIO(rawIO);
    }

    try
    {
        reader->Update();
    }
    catch (itk::ExceptionObject & err)
    {
        std::cerr << "Excepción al leer la imagen: " << err << std::endl;
        return EXIT_FAILURE;
    }

    // --- Ajustar el spacing (1.0,1.0,1.0) ---
    auto image = reader->GetOutput();
    itk::ImageBase<Dimension>::SpacingType spacing;
    spacing.Fill(1.0);
    image->SetSpacing(spacing);

    // --- Writer con VTKImageIO explícito ---
    auto writer = itk::ImageFileWriter<ImageType>::New();
    writer->SetInput(image);
    writer->SetFileName(outputFile);

    auto vtkIO = itk::VTKImageIO::New();
    writer->SetImageIO(vtkIO);

    try
    {
        writer->Update();
    }
    catch (itk::ExceptionObject & err)
    {
        std::cerr << "Excepción al escribir la imagen: " << err << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Imagen exportada a VTK correctamente: " << outputFile << std::endl;
    return EXIT_SUCCESS;
}
