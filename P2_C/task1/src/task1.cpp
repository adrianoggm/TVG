#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkShiftScaleImageFilter.h"
#include "itkNormalizeImageFilter.h"

#include <iostream>
#include <sstream>

int main( int argc, char* argv[] )
{
  if ( argc != 2 )
  {
    std::cerr << "USAGE:\n" << argv[0] << " <Image Filename>" << std::endl;
    return EXIT_FAILURE;
  }
  
  const char * inputFile = argv[1];
  
  // Tipos de imagen: usaremos unsigned char para entrada (para archivos PNG) y float para procesamiento.
  typedef unsigned char                    PixelType;
  const unsigned int                       Dimension = 2;
  typedef itk::Image<PixelType, Dimension> ImageType;
  typedef itk::Image<float, Dimension>     FloatImageType;
  
  // Lector de imagen
  typedef itk::ImageFileReader<ImageType>  ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFile);
  
  try {
    reader->Update();
  }
  catch( itk::ExceptionObject & error ) {
    std::cerr << "Error leyendo la imagen: " << error << std::endl;
    return EXIT_FAILURE;
  }
  
  // Mostrar dimensiones de la imagen
  ImageType::RegionType region = reader->GetOutput()->GetLargestPossibleRegion();
  ImageType::SizeType size = region.GetSize();
  std::cout << "Dimensiones de la imagen: " << size[0] << " x " << size[1] << std::endl;
  
  // Filtro 1: RescaleIntensityImageFilter
  typedef itk::RescaleIntensityImageFilter<ImageType, ImageType> RescaleFilterType;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(reader->GetOutput());
  rescaleFilter->SetOutputMinimum(10);
  rescaleFilter->SetOutputMaximum(250);
  
  // Filtro 2: ShiftScaleImageFilter
  typedef itk::ShiftScaleImageFilter<ImageType, ImageType> ShiftScaleFilterType;
  ShiftScaleFilterType::Pointer shiftScaleFilter = ShiftScaleFilterType::New();
  shiftScaleFilter->SetInput(reader->GetOutput());
  shiftScaleFilter->SetShift(25);
  shiftScaleFilter->SetScale(1.2);
  
  // Filtro 3: NormalizeImageFilter seguido de rescale para convertir a unsigned char
  typedef itk::NormalizeImageFilter<ImageType, FloatImageType> NormalizeFilterType;
  NormalizeFilterType::Pointer normalizeFilter = NormalizeFilterType::New();
  normalizeFilter->SetInput(reader->GetOutput());
  
  typedef itk::RescaleIntensityImageFilter<FloatImageType, ImageType> CastRescaleFilterType;
  CastRescaleFilterType::Pointer castRescaleFilter = CastRescaleFilterType::New();
  castRescaleFilter->SetInput(normalizeFilter->GetOutput());
  castRescaleFilter->SetOutputMinimum(0);
  castRescaleFilter->SetOutputMaximum(255);
  
  try {
    rescaleFilter->Update();
    shiftScaleFilter->Update();
    castRescaleFilter->Update();
  }
  catch( itk::ExceptionObject & error ) {
    std::cerr << "Error durante el procesamiento: " << error << std::endl;
    return EXIT_FAILURE;
  }
  
  // Definir los escritores para guardar las imágenes
  typedef itk::ImageFileWriter<ImageType> WriterType;
  
  // Directorio de salida (ajusta la ruta según tu estructura)
  std::string outputDir = "../../../images/images_generated/";
  
  WriterType::Pointer writerOriginal = WriterType::New();
  writerOriginal->SetFileName( outputDir + "original.png" );
  writerOriginal->SetInput( reader->GetOutput() );
  
  WriterType::Pointer writerRescale = WriterType::New();
  writerRescale->SetFileName( outputDir + "rescale.png" );
  writerRescale->SetInput( rescaleFilter->GetOutput() );
  
  WriterType::Pointer writerShiftScale = WriterType::New();
  writerShiftScale->SetFileName( outputDir + "shift_scale.png" );
  writerShiftScale->SetInput( shiftScaleFilter->GetOutput() );
  
  WriterType::Pointer writerNormalize = WriterType::New();
  writerNormalize->SetFileName( outputDir + "normalize.png" );
  writerNormalize->SetInput( castRescaleFilter->GetOutput() );
  
  try {
    writerOriginal->Update();
    writerRescale->Update();
    writerShiftScale->Update();
    writerNormalize->Update();
  }
  catch( itk::ExceptionObject & error ) {
    std::cerr << "Error escribiendo la imagen: " << error << std::endl;
    return EXIT_FAILURE;
  }
  
  std::cout << "Imágenes guardadas en: " << outputDir << std::endl;
  
  return EXIT_SUCCESS;
}
