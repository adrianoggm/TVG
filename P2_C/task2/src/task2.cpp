#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMeanImageFilter.h"
#include "itkMedianImageFilter.h"

#include <iostream>
#include <sstream>

int main(int argc, char * argv[])
{
  if(argc != 2)
  {
    std::cerr << "USAGE:\n" << argv[0] << " <Image Filename>" << std::endl;
    return EXIT_FAILURE;
  }

  const char * inputFile = argv[1];

  // Definir tipo de píxel e imagen (2D)
  typedef unsigned char                    PixelType;
  const unsigned int                       Dimension = 2;
  typedef itk::Image<PixelType, Dimension> ImageType;

  // Lector de imagen
  typedef itk::ImageFileReader<ImageType>  ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFile);

  try
  {
    reader->Update();
  }
  catch(itk::ExceptionObject & error)
  {
    std::cerr << "Error leyendo la imagen: " << error << std::endl;
    return EXIT_FAILURE;
  }

  // Mostrar dimensiones de la imagen
  ImageType::RegionType region = reader->GetOutput()->GetLargestPossibleRegion();
  ImageType::SizeType size = region.GetSize();
  std::cout << "Dimensiones de la imagen: " << size[0] << " x " << size[1] << std::endl;

  // Filtros de media (mean):
  // Para una vecindad de 3x3 se usa un radio de 1; para 5x5, radio de 2.
  typedef itk::MeanImageFilter<ImageType, ImageType> MeanFilterType;
  MeanFilterType::Pointer meanFilter3x3 = MeanFilterType::New();
  MeanFilterType::Pointer meanFilter5x5 = MeanFilterType::New();

  ImageType::SizeType radius;
  radius.Fill(1); // 3x3
  meanFilter3x3->SetRadius(radius);
  meanFilter3x3->SetInput(reader->GetOutput());

  radius.Fill(2); // 5x5
  meanFilter5x5->SetRadius(radius);
  meanFilter5x5->SetInput(reader->GetOutput());

  // Filtros de mediana:
  typedef itk::MedianImageFilter<ImageType, ImageType> MedianFilterType;
  MedianFilterType::Pointer medianFilter3x3 = MedianFilterType::New();
  MedianFilterType::Pointer medianFilter5x5 = MedianFilterType::New();

  radius.Fill(1); // 3x3
  medianFilter3x3->SetRadius(radius);
  medianFilter3x3->SetInput(reader->GetOutput());

  radius.Fill(2); // 5x5
  medianFilter5x5->SetRadius(radius);
  medianFilter5x5->SetInput(reader->GetOutput());

  try
  {
    meanFilter3x3->Update();
    meanFilter5x5->Update();
    medianFilter3x3->Update();
    medianFilter5x5->Update();
  }
  catch(itk::ExceptionObject & error)
  {
    std::cerr << "Error durante el filtrado: " << error << std::endl;
    return EXIT_FAILURE;
  }

  // Directorio de salida (ajusta la ruta según la estructura)
  std::string outputDir = "../../../images/images_generated/";

  // Escritores para guardar las imágenes filtradas
  typedef itk::ImageFileWriter<ImageType> WriterType;

  WriterType::Pointer writerMean3x3 = WriterType::New();
  writerMean3x3->SetFileName(outputDir + "mean_3x3.jpg");
  writerMean3x3->SetInput(meanFilter3x3->GetOutput());

  WriterType::Pointer writerMean5x5 = WriterType::New();
  writerMean5x5->SetFileName(outputDir + "mean_5x5.jpg");
  writerMean5x5->SetInput(meanFilter5x5->GetOutput());

  WriterType::Pointer writerMedian3x3 = WriterType::New();
  writerMedian3x3->SetFileName(outputDir + "median_3x3.jpg");
  writerMedian3x3->SetInput(medianFilter3x3->GetOutput());

  WriterType::Pointer writerMedian5x5 = WriterType::New();
  writerMedian5x5->SetFileName(outputDir + "median_5x5.jpg");
  writerMedian5x5->SetInput(medianFilter5x5->GetOutput());

  try
  {
    writerMean3x3->Update();
    writerMean5x5->Update();
    writerMedian3x3->Update();
    writerMedian5x5->Update();
  }
  catch(itk::ExceptionObject & error)
  {
    std::cerr << "Error escribiendo la imagen: " << error << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Imágenes filtradas guardadas en: " << outputDir << std::endl;
  return EXIT_SUCCESS;
}
