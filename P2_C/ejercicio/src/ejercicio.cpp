#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkBinomialBlurImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"

#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char ** argv)
{
  if(argc < 2)
  {
    std::cerr << "Usage: " << std::endl;
    std::cerr << "  " << argv[0] << " inputImageFile" << std::endl;
    return EXIT_FAILURE;
  }

  // Parse input filename
  std::string inputFilename(argv[1]);
  size_t lastSlash = inputFilename.find_last_of("/\\");
  std::string filename = (lastSlash == std::string::npos)
    ? inputFilename
    : inputFilename.substr(lastSlash + 1);
  size_t lastDot = filename.find_last_of(".");
  std::string baseName = (lastDot == std::string::npos)
    ? filename
    : filename.substr(0, lastDot);
  std::string ext = (lastDot == std::string::npos)
    ? ".jpg"
    : filename.substr(lastDot);

  // Output directory (adjust as needed)
  std::string outputDir = "../../../images/images_generated/";

  // Pixel and image types
  typedef float                                  PixelType;
  const unsigned int                            Dimension = 2;
  typedef itk::Image<PixelType, Dimension>       ImageType;
  typedef unsigned char                          WritePixelType;
  typedef itk::Image<WritePixelType, Dimension>  WriteImageType;

  // Reader
  typedef itk::ImageFileReader<ImageType>        ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFilename);
  try { reader->Update(); }
  catch(itk::ExceptionObject & err) {
    std::cerr << "Error reading image: " << err << std::endl;
    return EXIT_FAILURE;
  }

  // 1) Binomial Blur Filter
  typedef itk::BinomialBlurImageFilter<ImageType, ImageType>  BinomialFilterType;
  BinomialFilterType::Pointer binomialFilter = BinomialFilterType::New();
  binomialFilter->SetInput(reader->GetOutput());
  const unsigned int binomialRepetitions = 5;
  binomialFilter->SetRepetitions(binomialRepetitions);
  try { binomialFilter->Update(); }
  catch(itk::ExceptionObject & err) {
    std::cerr << "Error in BinomialBlurImageFilter: " << err << std::endl;
    return EXIT_FAILURE;
  }

  // 2) Curvature Flow Filter
  typedef itk::CurvatureFlowImageFilter<ImageType, ImageType>  CurvatureFlowFilterType;
  CurvatureFlowFilterType::Pointer curvatureFlow = CurvatureFlowFilterType::New();
  curvatureFlow->SetInput(reader->GetOutput());
  const unsigned int cfIterations = 8;
  const double       cfTimeStep   = 0.0025;
  curvatureFlow->SetNumberOfIterations(cfIterations);
  curvatureFlow->SetTimeStep(cfTimeStep);
  try { curvatureFlow->Update(); }
  catch(itk::ExceptionObject & err) {
    std::cerr << "Error in CurvatureFlowImageFilter: " << err << std::endl;
    return EXIT_FAILURE;
  }

  // Rescaler and writer typedefs
  typedef itk::RescaleIntensityImageFilter<ImageType, WriteImageType>  RescaleFilterType;
  typedef itk::ImageFileWriter<WriteImageType>                        WriterType;

  // Lambda to rescale & write
  auto writeImage = [&](const std::string & suffix, ImageType::Pointer img) {
    RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
    rescaler->SetInput(img);
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
      std::cout << "Saved: " << oss.str() << std::endl;
    }
    catch(itk::ExceptionObject & err) {
      std::cerr << "Error writing image (" << oss.str() << "): " << err << std::endl;
    }
  };

  // Write original (rescaled)
  writeImage("original", reader->GetOutput());

  // Write binomial blur result
  {
    std::ostringstream oss;
    oss << "binomialBlur_R" << binomialRepetitions;
    writeImage(oss.str(), binomialFilter->GetOutput());
  }

  // Write curvature flow result
  {
    std::ostringstream oss;
    oss << "curvatureFlow_I" << cfIterations;
    writeImage(oss.str(), curvatureFlow->GetOutput());
  }

  return EXIT_SUCCESS;
}
