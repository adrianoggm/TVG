#include <iostream>
#include <string>
#include <vector>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMedianImageFilter.h"
#include "itkConnectedThresholdImageFilter.h"
#include "itkNeighborhoodConnectedImageFilter.h"
#include "itkConfidenceConnectedImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

int main(int argc, char* argv[])
{
    // Comprobar argumentos de entrada
    if (argc < 3)
    {
        std::cerr << "Uso: " << argv[0]
                  << " <imagenEntrada> <prefijoSalida>" << std::endl;
        return EXIT_FAILURE;
    }
    // Ruta completa de la imagen de entrada
    const std::string rutaEntrada = argv[1];
    const std::string prefijoSalida = argv[2];

    // Extraer nombre base de la imagen (sin ruta ni extensión)
    std::string fichero = rutaEntrada;
    // Obtener sólo el último componente tras '/' o '\'
    auto posSlash = fichero.find_last_of("/\\");
    if (posSlash != std::string::npos)
        fichero = fichero.substr(posSlash + 1);
    // Quitar extensión
    auto posDot = fichero.rfind('.');
    std::string nombreBase = (posDot != std::string::npos) ? fichero.substr(0, posDot) : fichero;

    // Definir tipos de imagen de 2D con píxel float y con píxel unsigned char para salida
    constexpr unsigned int Dimension = 2;
    using PixelType = float;
    using ImageType = itk::Image<PixelType, Dimension>;
    using OutputPixelType = unsigned char;
    using OutputImageType = itk::Image<OutputPixelType, Dimension>;

    // Lector de imagen
    auto lector = itk::ImageFileReader<ImageType>::New();
    lector->SetFileName(rutaEntrada);
    lector->Update();

    // Valores de sigma para decidir si aplicar mediana
    std::vector<double> sigmasSuavizado = { 0.0, 1.0 };

    // Coordenadas de semilla
    ImageType::IndexType semilla;
    semilla[0] = 128;
    semilla[1] = 128;

    // Parámetros para ConnectedThresholdImageFilter
    std::vector<PixelType> ctLimiteInferior = { 15.0f,50.0f, 80.0f };
    std::vector<PixelType> ctLimiteSuperior = { 100.0f,150.0f, 200.0f };

    // Parámetros para NeighborhoodConnectedImageFilter
    std::vector<unsigned int> ncRadio = { 1, 3 };

    // Parámetros para ConfidenceConnectedImageFilter
    std::vector<unsigned int> ccRadio = { 1, 2 };
    std::vector<double> ccMultiplicador = { 1.0, 2.5 };
    std::vector<unsigned int> ccIteraciones = { 1, 3 };

    for (double sigma : sigmasSuavizado)
    {
        // 1. Obtener puntero a la imagen (sin suavizar o suavizada)
        typename ImageType::Pointer procesada = lector->GetOutput();

        // 2. Si sigma>0, aplicar MedianImageFilter 5×5
        if (sigma > 0.0)
        {
            using MedianFilterType = itk::MedianImageFilter<ImageType, ImageType>;
            auto mediana = MedianFilterType::New();
            mediana->SetInput(lector->GetOutput());
            typename MedianFilterType::InputSizeType radius;
            radius.Fill(2); // vecindario 5×5
            mediana->SetRadius(radius);
            mediana->Update();
            procesada = mediana->GetOutput();
        }

        // --- BLOQUE NUEVO: guardar 'procesada' antes de segmentar ---
        {
            using RescaleType = itk::RescaleIntensityImageFilter<ImageType, OutputImageType>;
            auto rescaleProc = RescaleType::New();
            rescaleProc->SetInput(procesada);
            rescaleProc->SetOutputMinimum(0);
            rescaleProc->SetOutputMaximum(255);
            rescaleProc->Update();

            std::ostringstream nombreProc;
            nombreProc << nombreBase << "_"               // <-- Nombre de la imagen original
                       << prefijoSalida << "_processed_"   // <-- tu prefijo
                       << "sigma" << sigma << ".png";      // <-- sufijo
            auto writerProc = itk::ImageFileWriter<OutputImageType>::New();
            writerProc->SetInput(rescaleProc->GetOutput());
            writerProc->SetFileName(nombreProc.str());
            writerProc->Update();
        }
        // --- FIN BLOQUE NUEVO ---

        // 3) ConnectedThresholdImageFilter
        for (size_t i = 0; i < ctLimiteInferior.size(); ++i)
        {
            using CTFilterType = itk::ConnectedThresholdImageFilter<ImageType, ImageType>;
            auto filtroCT = CTFilterType::New();
            filtroCT->SetInput(procesada);
            filtroCT->SetLower(ctLimiteInferior[i]);
            filtroCT->SetUpper(ctLimiteSuperior[i]);
            filtroCT->SetSeed(semilla);
            filtroCT->Update();

            using RescaleType = itk::RescaleIntensityImageFilter<ImageType, OutputImageType>;
            auto rescale = RescaleType::New();
            rescale->SetInput(filtroCT->GetOutput());
            rescale->SetOutputMinimum(0);
            rescale->SetOutputMaximum(255);
            rescale->Update();

            std::ostringstream nombreSalida;
            nombreSalida << nombreBase << "_"
                         << prefijoSalida << "_CT_"
                         << "sigma" << sigma
                         << "_L" << ctLimiteInferior[i]
                         << "_U" << ctLimiteSuperior[i]
                         << ".png";
            auto escritor = itk::ImageFileWriter<OutputImageType>::New();
            escritor->SetInput(rescale->GetOutput());
            escritor->SetFileName(nombreSalida.str());
            escritor->Update();
        }

        // 4) NeighborhoodConnectedImageFilter
        for (auto radio : ncRadio)
        {
            using NCFilterType = itk::NeighborhoodConnectedImageFilter<ImageType, ImageType>;
            auto filtroNC = NCFilterType::New();
            filtroNC->SetInput(procesada);
            filtroNC->SetSeed(semilla);
            typename NCFilterType::InputImageSizeType radiusSize;
            radiusSize.Fill(radio);
            filtroNC->SetRadius(radiusSize);
            filtroNC->SetReplaceValue(255.0f);
            filtroNC->Update();

            using RescaleType = itk::RescaleIntensityImageFilter<ImageType, OutputImageType>;
            auto rescale = RescaleType::New();
            rescale->SetInput(filtroNC->GetOutput());
            rescale->SetOutputMinimum(0);
            rescale->SetOutputMaximum(255);
            rescale->Update();

            std::ostringstream nombreSalida;
            nombreSalida << nombreBase << "_"
                         << prefijoSalida << "_NC_"
                         << "sigma" << sigma
                         << "_R" << radio
                         << ".png";
            auto escritorNC = itk::ImageFileWriter<OutputImageType>::New();
            escritorNC->SetInput(rescale->GetOutput());
            escritorNC->SetFileName(nombreSalida.str());
            escritorNC->Update();
        }

        // 5) ConfidenceConnectedImageFilter
        for (auto radio : ccRadio)
        {
            for (auto mult : ccMultiplicador)
            {
                for (auto iter : ccIteraciones)
                {
                    using CCFilterType = itk::ConfidenceConnectedImageFilter<ImageType, ImageType>;
                    auto filtroCC = CCFilterType::New();
                    filtroCC->SetInput(procesada);
                    filtroCC->SetSeed(semilla);
                    filtroCC->SetInitialNeighborhoodRadius(radio);
                    filtroCC->SetMultiplier(mult);
                    filtroCC->SetNumberOfIterations(iter);
                    filtroCC->SetReplaceValue(255.0f);
                    filtroCC->Update();

                    using RescaleType = itk::RescaleIntensityImageFilter<ImageType, OutputImageType>;
                    auto rescale = RescaleType::New();
                    rescale->SetInput(filtroCC->GetOutput());
                    rescale->SetOutputMinimum(0);
                    rescale->SetOutputMaximum(255);
                    rescale->Update();

                    std::ostringstream nombreSalida;
                    nombreSalida << nombreBase << "_"
                                 << prefijoSalida << "_CC_"
                                 << "sigma" << sigma
                                 << "_R" << radio
                                 << "_M" << mult
                                 << "_It" << iter
                                 << ".png";
                    auto escritorCC = itk::ImageFileWriter<OutputImageType>::New();
                    escritorCC->SetInput(rescale->GetOutput());
                    escritorCC->SetFileName(nombreSalida.str());
                    escritorCC->Update();
                }
            }
        }
    }

    std::cout << "Procesamiento completado." << std::endl;
    return EXIT_SUCCESS;
}