#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"

#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"

#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char ** argv)
{
  if(argc < 2)
  {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputImageFile" << std::endl;
    return EXIT_FAILURE;
  }

  // Extraer base y extensión del archivo de entrada
  std::string inputFilename(argv[1]);
  size_t lastSlash = inputFilename.find_last_of("/\\");
  std::string filename = (lastSlash == std::string::npos) ? inputFilename : inputFilename.substr(lastSlash + 1);
  size_t lastDot = filename.find_last_of(".");
  std::string baseName = (lastDot == std::string::npos) ? filename : filename.substr(0, lastDot);
  std::string ext = (lastDot == std::string::npos) ? ".jpg" : filename.substr(lastDot);

  // Directorio de salida
  std::string outputDir = "../../../images/images_generated/";

  // Definir tipos de imagen:
  // - Para procesamiento: float
  // - Para guardar: unsigned char (JPEG)
  typedef float                                  PixelType;
  const unsigned int                             Dimension = 2;
  typedef itk::Image<PixelType, Dimension>       InputImageType;
  typedef itk::Image<PixelType, Dimension>       OutputImageType; // filtros generan float

  typedef unsigned char                          WritePixelType;
  typedef itk::Image<WritePixelType, Dimension>  WriteImageType;

  // Lector de imagen
  typedef itk::ImageFileReader<InputImageType>   ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFilename);
  try {
    reader->Update();
  }
  catch(itk::ExceptionObject & error)
  {
    std::cerr << "Error leyendo la imagen: " << error << std::endl;
    return EXIT_FAILURE;
  }

  // 1. Gradient Anisotropic Diffusion Filter (Perona-Malik)
  typedef itk::GradientAnisotropicDiffusionImageFilter<InputImageType, OutputImageType>
    GradientAnisotropicFilterType;
  GradientAnisotropicFilterType::Pointer gradientAnisotropicFilter = GradientAnisotropicFilterType::New();
  gradientAnisotropicFilter->SetInput(reader->GetOutput());
  const unsigned int gradientAnisotropicNumberOfIterations = 15;
  const double gradientAnisotropicTimeStep = 0.01;
  const double gradientAnistropicConductance = 3;
  gradientAnisotropicFilter->SetNumberOfIterations(gradientAnisotropicNumberOfIterations);
  gradientAnisotropicFilter->SetTimeStep(gradientAnisotropicTimeStep);
  gradientAnisotropicFilter->SetConductanceParameter(gradientAnistropicConductance);
  try {
    gradientAnisotropicFilter->Update();
  }
  catch(itk::ExceptionObject & error)
  {
    std::cerr << "Error en GradientAnisotropicDiffusionImageFilter: " << error << std::endl;
    return EXIT_FAILURE;
  }

  // 2. Curvature Anisotropic Diffusion Filter (MCDE)
  typedef itk::CurvatureAnisotropicDiffusionImageFilter<InputImageType, OutputImageType>
    CurvatureAnisotropicFilterType;
  CurvatureAnisotropicFilterType::Pointer curvatureAnisotropicFilter = CurvatureAnisotropicFilterType::New();
  curvatureAnisotropicFilter->SetInput(reader->GetOutput());
  const unsigned int curvatureAnisotropicNumberOfIterations = 5;
  const double curvatureAnisotropicTimeStep = 0.01;
  const double curvatureAnisotropicConductance = 3;
  const bool curvatureAnisotropicUseImageSpacing = true;
  curvatureAnisotropicFilter->SetNumberOfIterations(curvatureAnisotropicNumberOfIterations);
  curvatureAnisotropicFilter->SetTimeStep(curvatureAnisotropicTimeStep);
  curvatureAnisotropicFilter->SetConductanceParameter(curvatureAnisotropicConductance);
  if(curvatureAnisotropicUseImageSpacing)
    curvatureAnisotropicFilter->UseImageSpacingOn();
  else
    curvatureAnisotropicFilter->UseImageSpacingOff();
  try {
    curvatureAnisotropicFilter->Update();
  }
  catch(itk::ExceptionObject & error)
  {
    std::cerr << "Error en CurvatureAnisotropicDiffusionImageFilter: " << error << std::endl;
    return EXIT_FAILURE;
  }

  // 3. Curvature Flow Filter
  typedef itk::CurvatureFlowImageFilter<InputImageType, OutputImageType> CurvatureFlowFilterType;
  CurvatureFlowFilterType::Pointer curvatureFlowFilter = CurvatureFlowFilterType::New();
  curvatureFlowFilter->SetInput(reader->GetOutput());
  const unsigned int curvatureFlowNumberOfIterations = 3;
  const double curvatureFlowTimeStep = 0.0025;
  curvatureFlowFilter->SetNumberOfIterations(curvatureFlowNumberOfIterations);
  curvatureFlowFilter->SetTimeStep(curvatureFlowTimeStep);
  try {
    curvatureFlowFilter->Update();
  }
  catch(itk::ExceptionObject & error)
  {
    std::cerr << "Error en CurvatureFlowImageFilter: " << error << std::endl;
    return EXIT_FAILURE;
  }

  // Función lambda para convertir (rescalar) una imagen float a unsigned char y escribirla
  typedef itk::RescaleIntensityImageFilter<OutputImageType, WriteImageType> RescaleFilterType;
  typedef itk::ImageFileWriter<WriteImageType> WriterType;
  auto writeImage = [&](const std::string & suffix, OutputImageType::Pointer image)
  {
    RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
    rescaler->SetInput(image);
    rescaler->SetOutputMinimum(0);
    rescaler->SetOutputMaximum(255);
    rescaler->Update();

    WriterType::Pointer writer = WriterType::New();
    std::ostringstream oss;
    oss << outputDir << baseName << "_" << suffix << ext;
    writer->SetFileName(oss.str());
    writer->SetInput(rescaler->GetOutput());
    try {
      writer->Update();
      std::cout << "Guardado: " << oss.str() << std::endl;
    }
    catch(itk::ExceptionObject & error)
    {
      std::cerr << "Error escribiendo la imagen (" << oss.str() << "): " << error << std::endl;
    }
  };

  // Guardar la imagen original (convertida a unsigned char)
  {
    RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
    rescaler->SetInput(reader->GetOutput());
    rescaler->SetOutputMinimum(0);
    rescaler->SetOutputMaximum(255);
    rescaler->Update();
    WriterType::Pointer writer = WriterType::New();
    std::ostringstream oss;
    oss << outputDir << baseName << "_original" << ext;
    writer->SetFileName(oss.str());
    writer->SetInput(rescaler->GetOutput());
    try {
      writer->Update();
      std::cout << "Guardado: " << oss.str() << std::endl;
    }
    catch(itk::ExceptionObject & error)
    {
      std::cerr << "Error escribiendo la imagen (" << oss.str() << "): " << error << std::endl;
    }
  }

  // Guardar cada salida con sufijo indicando la técnica
  writeImage("gradientAnisotropicDiffusion", gradientAnisotropicFilter->GetOutput());
  writeImage("curvatureAnisotropicDiffusion", curvatureAnisotropicFilter->GetOutput());
  writeImage("curvatureFlow", curvatureFlowFilter->GetOutput());

  std::cout << "Imágenes guardadas en: " << outputDir << std::endl;
  return EXIT_SUCCESS;
}
