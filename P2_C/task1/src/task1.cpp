#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCurvatureFlowImageFilter.h"
#include "itkMedianImageFilter.h"
#include "itkConnectedThresholdImageFilter.h"
#include "itkNeighborhoodConnectedImageFilter.h"
#include "itkConfidenceConnectedImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Uso: " << argv[0] << " <imagenEntrada> <prefijoSalida>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* nombreArchivoEntrada = argv[1];
    const std::string prefijoSalida = argv[2];

    constexpr unsigned int Dimension = 2;
    using PixelType = float;
    using ImageType = itk::Image<PixelType, Dimension>;
    using OutputPixelType = unsigned char;
    using OutputImageType = itk::Image<OutputPixelType, Dimension>;

    // Lector
    auto lector = itk::ImageFileReader<ImageType>::New();
    lector->SetFileName(nombreArchivoEntrada);
    lector->Update();

    // Coordenadas semilla
    ImageType::IndexType semilla = {128, 128};

    // Suavizados disponibles: 0 = sin filtro, 1 = CurvatureFlow, 2 = Mediana 5x5
    std::vector<int> modosSuavizado = {0, 1, 2};

    // Segmentación - parámetros
    std::vector<PixelType> ctInferior = {50.0f, 80.0f};
    std::vector<PixelType> ctSuperior = {150.0f, 200.0f};

    std::vector<unsigned int> ncRadios = {1, 3};

    std::vector<unsigned int> ccRadios = {1, 2};
    std::vector<double> ccMults = {1.0, 2.5};
    std::vector<unsigned int> ccIter = {1, 3};

    for (int modo : modosSuavizado)
    {
        ImageType::Pointer imagenFiltrada = lector->GetOutput();
        std::string tagSuavizado;

        // Aplicar suavizado
        if (modo == 1) // CurvatureFlow
        {
            auto suavizado = itk::CurvatureFlowImageFilter<ImageType, ImageType>::New();
            suavizado->SetInput(imagenFiltrada);
            suavizado->SetTimeStep(0.125);
            suavizado->SetNumberOfIterations(5);
            suavizado->Update();
            imagenFiltrada = suavizado->GetOutput();
            tagSuavizado = "_curvflow";
        }
        else if (modo == 2) // Mediana 5x5
        {
            auto mediana = itk::MedianImageFilter<ImageType, ImageType>::New();
            ImageType::SizeType radio;
            radio.Fill(2); // 5x5
            mediana->SetRadius(radio);
            mediana->SetInput(imagenFiltrada);
            mediana->Update();
            imagenFiltrada = mediana->GetOutput();
            tagSuavizado = "_median5x5";
        }
        else
        {
            tagSuavizado = "_nosmooth";
        }

        // --- ConnectedThreshold ---
        for (size_t i = 0; i < ctInferior.size(); ++i)
        {
            auto filtro = itk::ConnectedThresholdImageFilter<ImageType, ImageType>::New();
            filtro->SetInput(imagenFiltrada);
            filtro->SetSeed(semilla);
            filtro->SetLower(ctInferior[i]);
            filtro->SetUpper(ctSuperior[i]);
            filtro->Update();

            auto rescale = itk::RescaleIntensityImageFilter<ImageType, OutputImageType>::New();
            rescale->SetInput(filtro->GetOutput());
            rescale->SetOutputMinimum(0);
            rescale->SetOutputMaximum(255);
            rescale->Update();

            std::ostringstream outName;
            outName << prefijoSalida << "_CT" << tagSuavizado
                    << "_L" << ctInferior[i] << "_U" << ctSuperior[i] << ".png";

            auto writer = itk::ImageFileWriter<OutputImageType>::New();
            writer->SetInput(rescale->GetOutput());
            writer->SetFileName(outName.str());
            writer->Update();
        }

        // --- NeighborhoodConnected ---
        for (auto radio : ncRadios)
        {
            auto filtro = itk::NeighborhoodConnectedImageFilter<ImageType, ImageType>::New();
            filtro->SetInput(imagenFiltrada);
            filtro->SetSeed(semilla);
            ImageType::SizeType rad; rad.Fill(radio);
            filtro->SetRadius(rad);
            filtro->SetReplaceValue(255.0f);
            filtro->Update();

            auto rescale = itk::RescaleIntensityImageFilter<ImageType, OutputImageType>::New();
            rescale->SetInput(filtro->GetOutput());
            rescale->SetOutputMinimum(0);
            rescale->SetOutputMaximum(255);
            rescale->Update();

            std::ostringstream outName;
            outName << prefijoSalida << "_NC" << tagSuavizado << "_R" << radio << ".png";

            auto writer = itk::ImageFileWriter<OutputImageType>::New();
            writer->SetInput(rescale->GetOutput());
            writer->SetFileName(outName.str());
            writer->Update();
        }

        // --- ConfidenceConnected ---
        for (auto radio : ccRadios)
        {
            for (auto mult : ccMults)
            {
                for (auto it : ccIter)
                {
                    auto filtro = itk::ConfidenceConnectedImageFilter<ImageType, ImageType>::New();
                    filtro->SetInput(imagenFiltrada);
                    filtro->SetSeed(semilla);
                    filtro->SetInitialNeighborhoodRadius(radio);
                    filtro->SetMultiplier(mult);
                    filtro->SetNumberOfIterations(it);
                    filtro->SetReplaceValue(255.0f);
                    filtro->Update();

                    auto rescale = itk::RescaleIntensityImageFilter<ImageType, OutputImageType>::New();
                    rescale->SetInput(filtro->GetOutput());
                    rescale->SetOutputMinimum(0);
                    rescale->SetOutputMaximum(255);
                    rescale->Update();

                    std::ostringstream outName;
                    outName << prefijoSalida << "_CC" << tagSuavizado
                            << "_R" << radio << "_M" << mult << "_It" << it << ".png";

                    auto writer = itk::ImageFileWriter<OutputImageType>::New();
                    writer->SetInput(rescale->GetOutput());
                    writer->SetFileName(outName.str());
                    writer->Update();
                }
            }
        }
    }

    std::cout << "Procesamiento completado." << std::endl;
    return EXIT_SUCCESS;
}
