#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkImageFileWriter.h"

#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <inputImage> [lower] [upper] [outside] [inside] [below] [outputDir]" << std::endl;
        return EXIT_FAILURE;
    }

    // Parse arguments
    std::string inputPath = argv[1];
    int lower = 150;
    int upper = 180;
    int outside = 0;
    int inside = 255;
    int below = 180;
    std::string outputDir = "./";
    
    if (argc >= 3) lower = std::stoi(argv[2]);
    if (argc >= 4) upper = std::stoi(argv[3]);
    if (argc >= 5) outside = std::stoi(argv[4]);
    if (argc >= 6) inside = std::stoi(argv[5]);
    if (argc >= 7) below = std::stoi(argv[6]);
    if (argc >= 8)
    {
        outputDir = argv[7];
        if (outputDir.back() != '/' && outputDir.back() != '\\') outputDir += '/';
    }

    // Extract basename
    size_t pos = inputPath.find_last_of("/\\");
    std::string filename = (pos == std::string::npos ? inputPath : inputPath.substr(pos + 1));
    size_t dot = filename.find_last_of('.');
    std::string basename = (dot == std::string::npos ? filename : filename.substr(0, dot));

    constexpr unsigned int Dimension = 2;
    using PixelType = unsigned char;
    using ImageType = itk::Image<PixelType, Dimension>;

    // Reader
    using ReaderType = itk::ImageFileReader<ImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(inputPath);
    reader->Update();

    // 1) Binary threshold filter
    using BinaryFilterType = itk::BinaryThresholdImageFilter<ImageType, ImageType>;
    auto binaryFilter = BinaryFilterType::New();
    binaryFilter->SetInput(reader->GetOutput());
    binaryFilter->SetLowerThreshold(lower);
    binaryFilter->SetUpperThreshold(upper);
    binaryFilter->SetOutsideValue(outside);
    binaryFilter->SetInsideValue(inside);
    binaryFilter->Update();

    // 2) General threshold below filter
    using ThreshFilterType = itk::ThresholdImageFilter<ImageType>;
    auto threshFilter = ThreshFilterType::New();
    threshFilter->SetInput(reader->GetOutput());
    threshFilter->SetOutsideValue(outside);
    threshFilter->ThresholdBelow(below);
    threshFilter->Update();

    // Writers
    using WriterType = itk::ImageFileWriter<ImageType>;

    // Binary output
    auto writerBin = WriterType::New();
    writerBin->SetFileName(outputDir + basename
                             + "_binary_" + std::to_string(lower)
                             + "-" + std::to_string(upper)
                             + "_in" + std::to_string(inside)
                             + "_out" + std::to_string(outside) + ".png");
    writerBin->SetInput(binaryFilter->GetOutput());

    // Threshold output
    auto writerTh = WriterType::New();
    writerTh->SetFileName(outputDir + basename
                            + "_thresholdBelow_" + std::to_string(below)
                            + "_out" + std::to_string(outside) + ".png");
    writerTh->SetInput(threshFilter->GetOutput());

    try {
        writerBin->Update();
        writerTh->Update();
    }
    catch (itk::ExceptionObject &err) {
        std::cerr << "Error writing images: " << err << std::endl;
        return EXIT_FAILURE;
    }

    // Print paths of saved images
    std::cout << "Saved:\n"
              << "  " << outputDir << basename << "_binary_" << lower << "-" << upper
              << "_in" << inside << "_out" << outside << ".png\n"
              << "  " << outputDir << basename << "_thresholdBelow_" << below
              << "_out" << outside << ".png" << std::endl;

    return EXIT_SUCCESS;
}