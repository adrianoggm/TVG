#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkCannyEdgeDetectionImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageFileWriter.h"

#include <string>
#include <iostream>
#include <sstream>

int main(int argc, char *argv[])
{
    if (argc < 5)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <inputImage> <variance> <lowerThreshold> <upperThreshold> [outputDir]" << std::endl;
        return EXIT_FAILURE;
    }

    const std::string inputPath       = argv[1];
    const double variance             = std::stod(argv[2]);
    const double lowerThreshold       = std::stod(argv[3]);
    const double upperThreshold       = std::stod(argv[4]);
    std::string outputDir = (argc > 5 ? argv[5] : "./");
    if (outputDir.back() != '/' && outputDir.back() != '\\')
        outputDir += '/';

    // Extract basename
    const size_t pos = inputPath.find_last_of("/\\");
    std::string filename = (pos == std::string::npos ? inputPath : inputPath.substr(pos + 1));
    const size_t dot = filename.find_last_of('.');
    const std::string basename = (dot == std::string::npos ? filename : filename.substr(0, dot));

    constexpr unsigned int Dimension = 2;
    using PixelType = float;
    using ImageType = itk::Image<PixelType, Dimension>;
    using OutputPixelType = unsigned char;
    using OutputImageType = itk::Image<OutputPixelType, Dimension>;

    // Reader
    using ReaderType = itk::ImageFileReader<ImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(inputPath);
    reader->Update();

    // Canny edge detection
    using CannyFilterType = itk::CannyEdgeDetectionImageFilter<ImageType, ImageType>;
    auto canny = CannyFilterType::New();
    canny->SetInput(reader->GetOutput());
    canny->SetVariance(variance);
    canny->SetLowerThreshold(lowerThreshold);
    canny->SetUpperThreshold(upperThreshold);
    canny->Update();

    // Rescale to 0-255 for output
    using RescaleType = itk::RescaleIntensityImageFilter<ImageType, OutputImageType>;
    auto rescaler = RescaleType::New();
    rescaler->SetInput(canny->GetOutput());
    rescaler->SetOutputMinimum(0);
    rescaler->SetOutputMaximum(255);
    rescaler->Update();

    // Writer
    using WriterType = itk::ImageFileWriter<OutputImageType>;
    std::ostringstream oss;
    oss << basename << "_canny_var" << variance
        << "_thr" << lowerThreshold << "-" << upperThreshold << ".png";
    auto writer = WriterType::New();
    writer->SetFileName(outputDir + oss.str());
    writer->SetInput(rescaler->GetOutput());
    writer->Update();

    std::cout << "Saved Canny result to: " << outputDir + oss.str() << std::endl;
    return EXIT_SUCCESS;
}
