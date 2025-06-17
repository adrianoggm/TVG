// DicomReadView.cxx
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkGDCMImageIO.h"
#include "QuickView.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cerr << "Uso: " << argv[0] << " <dicom_file>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* dicomFile = argv[1];

    constexpr unsigned int Dimension = 2;
    using PixelType = signed short;  // típico en DICOM
    using ImageType = itk::Image<PixelType, Dimension>;

    // Configurar GDCM explícito
    auto dicomIO = itk::GDCMImageIO::New();

    // Lector DICOM
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetImageIO(dicomIO);
    reader->SetFileName(dicomFile);

    try
    {
        reader->Update();
    }
    catch(itk::ExceptionObject & err)
    {
        std::cerr << "Error al leer DICOM: " << err << std::endl;
        return EXIT_FAILURE;
    }

    // Visualizar con QuickView
    QuickView viewer;
    viewer.AddImage(reader->GetOutput(), true, "DICOM Image");
    viewer.Visualize();

    return EXIT_SUCCESS;
}
