#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkBinomialBlurImageFilter.h"
#include "itkRecursiveGaussianImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char * argv[])
{
  if(argc < 2)
  {
    std::cerr << "Usage:" << std::endl;
    std::cerr << argv[0] << " inputImageFile" << std::endl;
    return EXIT_FAILURE;
  }
  
  // Parsear el nombre de archivo de entrada para obtener la base y extensión
  std::string inputFilename(argv[1]);
  size_t lastSlash = inputFilename.find_last_of("/\\");
  std::string filename = (lastSlash == std::string::npos) ? inputFilename : inputFilename.substr(lastSlash + 1);
  size_t lastDot = filename.find_last_of(".");
  std::string baseName = (lastDot == std::string::npos) ? filename : filename.substr(0, lastDot);
  std::string ext = (lastDot == std::string::npos) ? ".jpg" : filename.substr(lastDot);
  
  // Definir directorio de salida (ajusta la ruta según tu estructura)
  std::string outputDir = "../../../images/images_generated/";
  
  // Tipos de imagen
  typedef float                                  InternalPixelType;
  const unsigned int                             Dimension = 2;
  typedef itk::Image<InternalPixelType, Dimension>  InternalImageType;
  
  // Tipo de imagen para escribir (unsigned char, requerido por JPEG)
  typedef unsigned char                          WritePixelType;
  typedef itk::Image<WritePixelType, Dimension>  WriteImageType;
  
  // Lector de imagen (lectura en formato internal)
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
  
  // Filtro 1: Discrete Gaussian
  typedef itk::DiscreteGaussianImageFilter<InternalImageType, InternalImageType> DiscreteGaussianFilterType;
  DiscreteGaussianFilterType::Pointer discreteGaussian = DiscreteGaussianFilterType::New();
  discreteGaussian->SetInput(reader->GetOutput());
  int discreteGaussianVariance = 4;
  int discreteGaussianMaximumKernelWidth = 3;
  discreteGaussian->SetVariance(discreteGaussianVariance);
  discreteGaussian->SetMaximumKernelWidth(discreteGaussianMaximumKernelWidth);
  discreteGaussian->Update();
  
  // Filtro 2: Binomial Blur
  typedef itk::BinomialBlurImageFilter<InternalImageType, InternalImageType> BinomialBlurFilterType;
  BinomialBlurFilterType::Pointer binomialBlur = BinomialBlurFilterType::New();
  binomialBlur->SetInput(reader->GetOutput());
  int binomialBlurRepetitions = 5;
  binomialBlur->SetRepetitions(binomialBlurRepetitions);
  binomialBlur->Update();
  
  // Filtro 3: Recursive Gaussian (se aplica en dos direcciones para aproximar 2D)
  typedef itk::RecursiveGaussianImageFilter<InternalImageType, InternalImageType> RecursiveGaussianFilterType;
  RecursiveGaussianFilterType::Pointer filterX = RecursiveGaussianFilterType::New();
  RecursiveGaussianFilterType::Pointer filterY = RecursiveGaussianFilterType::New();
  filterX->SetDirection(0); // dirección X
  filterY->SetDirection(1); // dirección Y
  filterX->SetOrder(RecursiveGaussianFilterType::ZeroOrder);
  filterY->SetOrder(RecursiveGaussianFilterType::ZeroOrder);
  filterX->SetNormalizeAcrossScale(false);
  filterY->SetNormalizeAcrossScale(false);
  filterX->SetInput(reader->GetOutput());
  filterY->SetInput(filterX->GetOutput());
  int sigma = 3;
  filterX->SetSigma(sigma);
  filterY->SetSigma(sigma);
  filterY->Update();
  
  // Función lambda para convertir la imagen de float a unsigned char y escribirla
  typedef itk::RescaleIntensityImageFilter<InternalImageType, WriteImageType> RescaleFilterType;
  typedef itk::ImageFileWriter<WriteImageType> WriterType;
  
  auto writeImage = [&](const std::string & suffix, InternalImageType::Pointer image) {
    // Convertir la imagen de float a unsigned char (0-255)
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
  
  // Guardar imágenes con sufijos que identifiquen cada filtro
  writeImage("original", reader->GetOutput());
  
  {
    std::ostringstream oss;
    oss << "discreteGaussian_V" << discreteGaussianVariance << "_K" << discreteGaussianMaximumKernelWidth;
    writeImage(oss.str(), discreteGaussian->GetOutput());
  }
  
  {
    std::ostringstream oss;
    oss << "binomialBlur_R" << binomialBlurRepetitions;
    writeImage(oss.str(), binomialBlur->GetOutput());
  }
  
  writeImage("recursiveGaussianX_S" + std::to_string(sigma), filterX->GetOutput());
  writeImage("recursiveGaussianXY_S" + std::to_string(sigma), filterY->GetOutput());
  
  std::cout << "Imágenes guardadas en: " << outputDir << std::endl;
  
  return EXIT_SUCCESS;
}
