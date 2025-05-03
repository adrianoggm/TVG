#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"

#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <inputImageFile>\n";
    return EXIT_FAILURE;
  }

  // --- Extraer input, baseName y extensión ---
  std::string inputFilename(argv[1]);
  size_t posSlash = inputFilename.find_last_of("/\\");
  std::string filename = (posSlash == std::string::npos)
    ? inputFilename
    : inputFilename.substr(posSlash + 1);
  size_t posDot = filename.find_last_of('.');
  std::string baseName = (posDot == std::string::npos)
    ? filename
    : filename.substr(0, posDot);
  std::string ext = (posDot == std::string::npos)
    ? "" 
    : filename.substr(posDot);

  // --- Directorio de salida (debe existir) ---
  std::string outputDir = "../../../images/images_generated/";

  // --- Definición de tipos ---
  using PixelType       = float;
  constexpr unsigned int Dimension = 2;
  using ImageType       = itk::Image<PixelType, Dimension>;
  using OutputImageType = ImageType;  
  using WritePixelType  = unsigned char;
  using WriteImageType  = itk::Image<WritePixelType, Dimension>;

  // --- Lector ---
  using ReaderType = itk::ImageFileReader<ImageType>;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFilename);
  try
  {
    reader->Update();
  }
  catch (itk::ExceptionObject& err)
  {
    std::cerr << "Error leyendo la imagen: " << err << std::endl;
    return EXIT_FAILURE;
  }

  // --- Filtro: CurvatureFlow ---
  using CurvatureFlowFilterType = itk::CurvatureFlowImageFilter<ImageType, OutputImageType>;
  CurvatureFlowFilterType::Pointer cf = CurvatureFlowFilterType::New();
  cf->SetInput(reader->GetOutput());
  cf->SetNumberOfIterations(8);
  cf->SetTimeStep(0.0025);
  try
  {
    cf->Update();
  }
  catch (itk::ExceptionObject& err)
  {
    std::cerr << "Error en CurvatureFlowImageFilter: " << err << std::endl;
    return EXIT_FAILURE;
  }

  // --- Función lambda para reescalar y escribir ---
  using RescaleFilterType = itk::RescaleIntensityImageFilter<OutputImageType, WriteImageType>;
  using WriterType        = itk::ImageFileWriter<WriteImageType>;
  auto writeImage = [&](const std::string& suffix, ImageType::Pointer img)
  {
    auto rescaler = RescaleFilterType::New();
    rescaler->SetInput(img);
    rescaler->SetOutputMinimum(0);
    rescaler->SetOutputMaximum(255);
    rescaler->Update();

    auto writer = WriterType::New();
    std::ostringstream oss;
    oss << outputDir << baseName << "_" << suffix << ext;
    writer->SetFileName(oss.str());
    writer->SetInput(rescaler->GetOutput());
    try
    {
      writer->Update();
      std::cout << "Guardada: " << oss.str() << std::endl;
    }
    catch (itk::ExceptionObject& err)
    {
      std::cerr << "Error escribiendo la imagen (" << oss.str() << "): " << err << std::endl;
    }
  };

  // --- Guardar original y resultado ---
  writeImage("original", reader->GetOutput());
  writeImage("curvatureFlow", cf->GetOutput());

  std::cout << "Todas las imágenes guardadas en: " << outputDir << std::endl;
  return EXIT_SUCCESS;
}
