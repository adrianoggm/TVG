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

int main(int argc, char * argv[])
{
  if(argc < 2)
  {
    std::cerr << "Usage:\n" << argv[0] << " inputImageFile" << std::endl;
    return EXIT_FAILURE;
  }
  
  // Extraer la base y extensión del archivo de entrada
  std::string inputFilename(argv[1]);
  size_t lastSlash = inputFilename.find_last_of("/\\");
  std::string filename = (lastSlash == std::string::npos) ? inputFilename : inputFilename.substr(lastSlash + 1);
  size_t lastDot = filename.find_last_of(".");
  std::string baseName = (lastDot == std::string::npos) ? filename : filename.substr(0, lastDot);
  std::string ext = (lastDot == std::string::npos) ? ".jpg" : filename.substr(lastDot);
  
  // Directorio de salida
  std::string outputDir = "../../../images/images_generated/";
  
  // Tipos de imagen:
  // - Imagen interna (para procesamiento): float.
  // - Imagen de escritura: unsigned char (para guardar en JPEG).
  typedef float                                  InternalPixelType;
  const unsigned int                             Dimension = 2;
  typedef itk::Image<InternalPixelType, Dimension> InternalImageType;
  
  typedef unsigned char                          WritePixelType;
  typedef itk::Image<WritePixelType, Dimension>  WriteImageType;
  
  // Lector de imagen (en formato interno)
  typedef itk::ImageFileReader<InternalImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFilename);
  try {
    reader->Update();
  }
  catch(itk::ExceptionObject & error) {
    std::cerr << "Error leyendo la imagen: " << error << std::endl;
    return EXIT_FAILURE;
  }
  
  // 1. Gradient Anisotropic Diffusion Filter (Perona-Malik)
  typedef itk::GradientAnisotropicDiffusionImageFilter<InternalImageType, InternalImageType>
    GradientAnisotropicDiffusionFilterType;
  GradientAnisotropicDiffusionFilterType::Pointer gradientDiffusion =
    GradientAnisotropicDiffusionFilterType::New();
  gradientDiffusion->SetInput(reader->GetOutput());
  gradientDiffusion->SetNumberOfIterations(5);
  gradientDiffusion->SetTimeStep(0.01); // Valor ajustado para estabilidad
  gradientDiffusion->SetConductanceParameter(3.0);
  try {
    gradientDiffusion->Update();
  }
  catch(itk::ExceptionObject & error) {
    std::cerr << "Error en GradientAnisotropicDiffusionImageFilter: " << error << std::endl;
    return EXIT_FAILURE;
  }
  
  // 2. Curvature Anisotropic Diffusion Filter (MCDE)
  typedef itk::CurvatureAnisotropicDiffusionImageFilter<InternalImageType, InternalImageType>
    CurvatureAnisotropicDiffusionFilterType;
  CurvatureAnisotropicDiffusionFilterType::Pointer curvatureDiffusion =
    CurvatureAnisotropicDiffusionFilterType::New();
  curvatureDiffusion->SetInput(reader->GetOutput());
  curvatureDiffusion->SetNumberOfIterations(5);
  curvatureDiffusion->SetTimeStep(0.01); // Valor ajustado para estabilidad
  curvatureDiffusion->SetConductanceParameter(3.0);
  try {
    curvatureDiffusion->Update();
  }
  catch(itk::ExceptionObject & error) {
    std::cerr << "Error en CurvatureAnisotropicDiffusionImageFilter: " << error << std::endl;
    return EXIT_FAILURE;
  }
  
  // 3. Curvature Flow Filter
  typedef itk::CurvatureFlowImageFilter<InternalImageType, InternalImageType> CurvatureFlowFilterType;
  CurvatureFlowFilterType::Pointer curvatureFlow = CurvatureFlowFilterType::New();
  curvatureFlow->SetInput(reader->GetOutput());
  curvatureFlow->SetNumberOfIterations(10);
  curvatureFlow->SetTimeStep(0.01); // Valor ajustado para estabilidad
  try {
    curvatureFlow->Update();
  }
  catch(itk::ExceptionObject & error) {
    std::cerr << "Error en CurvatureFlowImageFilter: " << error << std::endl;
    return EXIT_FAILURE;
  }
  
  // Función lambda para convertir la imagen interna (float) a WriteImageType (unsigned char) y guardarla
  typedef itk::RescaleIntensityImageFilter<InternalImageType, WriteImageType> RescaleFilterType;
  typedef itk::ImageFileWriter<WriteImageType> WriterType;
  
  auto writeImage = [&](const std::string & suffix, InternalImageType::Pointer image)
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
    catch(itk::ExceptionObject & error) {
      std::cerr << "Error escribiendo la imagen (" << oss.str() << "): " << error << std::endl;
    }
  };
  
  // Guardar la imagen original
  writeImage("original", reader->GetOutput());
  
  // Guardar cada una de las salidas con sufijos que indican la técnica utilizada
  writeImage("gradientAnisotropicDiffusion", gradientDiffusion->GetOutput());
  writeImage("curvatureAnisotropicDiffusion", curvatureDiffusion->GetOutput());
  writeImage("curvatureFlow", curvatureFlow->GetOutput());
  
  std::cout << "Imágenes guardadas en: " << outputDir << std::endl;
  
  return EXIT_SUCCESS;
}
