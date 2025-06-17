// DicomToPng.cxx
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkGDCMImageIO.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImageFileWriter.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Uso: " << argv[0]
                  << " <input.dcm> <output.png>" << std::endl;
        return EXIT_FAILURE;
    }

    const char*  dicomFile = argv[1];
    const char*  pngFile   = argv[2];

    constexpr unsigned int Dimension = 2;
    using InputPixelType  = signed short;           // tipo típico en DICOM
    using OutputPixelType = unsigned char;          // PNG 8-bit
    using InputImageType  = itk::Image<InputPixelType, Dimension>;
    using MidImageType    = itk::Image<float, Dimension>;
    using OutputImageType = itk::Image<OutputPixelType, Dimension>;

    // 1) Lector DICOM explícito
    auto dicomIO = itk::GDCMImageIO::New();
    auto reader  = itk::ImageFileReader<InputImageType>::New();
    reader->SetImageIO(dicomIO);
    reader->SetFileName(dicomFile);

    try {
        reader->Update();
    }
    catch (itk::ExceptionObject& err) {
        std::cerr << "Error leyendo DICOM: " << err << std::endl;
        return EXIT_FAILURE;
    }

    // 2) Reescalar intensidad a rango [0,255]
    auto rescaler = itk::RescaleIntensityImageFilter<InputImageType, MidImageType>::New();
    rescaler->SetInput(reader->GetOutput());
    rescaler->SetOutputMinimum(   0.0 );
    rescaler->SetOutputMaximum( 255.0 );

    // 3) Castear a unsigned char
    auto caster = itk::CastImageFilter<MidImageType, OutputImageType>::New();
    caster->SetInput(rescaler->GetOutput());

    // 4) Escribir PNG
    auto writer = itk::ImageFileWriter<OutputImageType>::New();
    writer->SetFileName(pngFile);
    writer->SetInput(caster->GetOutput());

    try {
        writer->Update();
    }
    catch (itk::ExceptionObject& err) {
        std::cerr << "Error escribiendo PNG: " << err << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Convertido: " << dicomFile << " → " << pngFile << std::endl;
    return EXIT_SUCCESS;
}
