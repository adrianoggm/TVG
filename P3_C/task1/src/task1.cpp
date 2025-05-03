#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkMeanImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageFileWriter.h"

#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <inputImage> [sigma] [outputDir]" << std::endl;
        return EXIT_FAILURE;
    }

    std::string inputPath = argv[1];
    double sigma = 1.0;
    if (argc >= 3)
    {
        sigma = std::stod(argv[2]);
    }

    std::string outputDir = "./";
    if (argc >= 4)
    {
        outputDir = argv[3];
        if (outputDir.back() != '/' && outputDir.back() != '\\')
        {
            outputDir += '/';
        }
    }

    // Extract basename from input path
    std::string inputFilename;
    size_t pos = inputPath.find_last_of("/\\");
    if (pos == std::string::npos)
        inputFilename = inputPath;
    else
        inputFilename = inputPath.substr(pos + 1);
    std::string basename = inputFilename;
    size_t dotPos = inputFilename.find_last_of('.');
    if (dotPos != std::string::npos)
        basename = inputFilename.substr(0, dotPos);

    constexpr unsigned int Dimension = 2;
    using PixelType = float;
    using InputImageType = itk::Image<PixelType, Dimension>;
    using OutputPixelType = unsigned char;
    using OutputImageType = itk::Image<OutputPixelType, Dimension>;

    // Reader
    using ReaderType = itk::ImageFileReader<InputImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(inputPath);
    reader->Update();



    // 1b) Mean filter (5x5 neighborhood)
    using MeanFilterType = itk::MeanImageFilter<InputImageType, InputImageType>;
    auto meanFilter1 = MeanFilterType::New();
    InputImageType::SizeType meanRadius;
    meanRadius.Fill(2); // 5x5 window
    meanFilter1->SetInput(reader->GetOutput());
    meanFilter1->SetRadius(meanRadius);
    meanFilter1->Update();

    // 1c) Gradient magnitude (finite differences)
    using GradMagFilterType = itk::GradientMagnitudeImageFilter<InputImageType, InputImageType>;
    auto gradMagFilter = GradMagFilterType::New();
    gradMagFilter->SetInput(meanFilter1->GetOutput());
    gradMagFilter->Update();

    // ---------- Branch 2: GradientMagnitudeRecursiveGaussian with Mean (5x5) ----------
    // 2a) Mean filter (5x5)
    auto meanFilter2 = MeanFilterType::New();
    meanFilter2->SetInput(reader->GetOutput());
    meanFilter2->SetRadius(meanRadius);
    meanFilter2->Update();

    // 2b) Recursive Gaussian gradient magnitude
    using GradMagRecGaussFilterType = itk::GradientMagnitudeRecursiveGaussianImageFilter<InputImageType, InputImageType>;
    auto gradMagRecFilter = GradMagRecGaussFilterType::New();
    gradMagRecFilter->SetInput(meanFilter2->GetOutput());
    gradMagRecFilter->SetSigma(sigma);
    gradMagRecFilter->Update();

    // Rescale all outputs to 0-255 for writing
    using RescaleFilterType = itk::RescaleIntensityImageFilter<InputImageType, OutputImageType>;
    auto rescaler1 = RescaleFilterType::New();
    rescaler1->SetInput(gradMagFilter->GetOutput());
    rescaler1->SetOutputMinimum(0);
    rescaler1->SetOutputMaximum(255);
    rescaler1->Update();

    auto rescaler2 = RescaleFilterType::New();
    rescaler2->SetInput(gradMagRecFilter->GetOutput());
    rescaler2->SetOutputMinimum(0);
    rescaler2->SetOutputMaximum(255);
    rescaler2->Update();

    // Writers
    using WriterType = itk::ImageFileWriter<OutputImageType>;

    auto writer1 = WriterType::New();
    writer1->SetFileName(outputDir + basename + "_gradmag_gauss_mean5x5_sigma" + std::to_string(sigma) + ".png");
    writer1->SetInput(rescaler1->GetOutput());

    auto writer2 = WriterType::New();
    writer2->SetFileName(outputDir + basename + "_gradmagRec_mean5x5_sigma" + std::to_string(sigma) + ".png");
    writer2->SetInput(rescaler2->GetOutput());

    try
    {
        writer1->Update();
        writer2->Update();
    }
    catch (itk::ExceptionObject &error)
    {
        std::cerr << "Error writing images: " << error << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Generated images:\n"
              << "  " << outputDir + basename + "_gradmag_gauss_mean5x5_sigma" << sigma << ".png\n"
              << "  " << outputDir + basename + "_gradmagRec_mean5x5_sigma" << sigma << ".png" << std::endl;

    return EXIT_SUCCESS;
}
