// SeriesVolumeAndViewer.cxx
#include "itkImage.h"
#include "itkImageSeriesReader.h"
#include "itkNumericSeriesFileNames.h"
#include "itkImageFileWriter.h"
#include "itkExtractImageFilter.h"
#include "QuickView.h"

#include <iostream>
#include <string>
#include <sstream>

int main(int argc, char* argv[])
{
    if (argc < 5)
    {
        std::cerr << "Uso: " << argv[0]
                  << " <patrón_entrada> <startIndex> <endIndex> <volumen_salida.mhd>\n"
                  << "Ejemplo: " << argv[0] << " \"t%02d.bmp\" 50 60 resultado.mhd\n";
        return EXIT_FAILURE;
    }

    const std::string pattern    = argv[1];         // e.g. "t%02d.bmp"
    const unsigned int startIdx  = std::stoi(argv[2]);
    const unsigned int endIdx    = std::stoi(argv[3]);
    const std::string outVolume  = argv[4];         // e.g. "resultado.mhd"

    constexpr unsigned int Dimension3D = 3;
    constexpr unsigned int Dimension2D = 2;
    using PixelType   = unsigned char;
    using VolumeType  = itk::Image<PixelType, Dimension3D>;
    using SliceType   = itk::Image<PixelType, Dimension2D>;

    // 1) Generar nombres de serie
    using NameGenType = itk::NumericSeriesFileNames;
    auto nameGenerator = NameGenType::New();
    nameGenerator->SetSeriesFormat(pattern);
    nameGenerator->SetStartIndex(startIdx);
    nameGenerator->SetEndIndex(endIdx);
    nameGenerator->SetIncrementIndex(1);

    // 2) Leer la serie de 2D y crear el volumen 3D
    using ReaderType = itk::ImageSeriesReader<VolumeType>;
    auto seriesReader = ReaderType::New();
    seriesReader->SetFileNames(nameGenerator->GetFileNames());

    try
    {
        seriesReader->Update();
    }
    catch (itk::ExceptionObject& err)
    {
        std::cerr << "Error leyendo la serie: " << err << std::endl;
        return EXIT_FAILURE;
    }

    // 3) Escribir el volumen en .mhd
    using WriterType = itk::ImageFileWriter<VolumeType>;
    auto volumeWriter = WriterType::New();
    volumeWriter->SetFileName(outVolume);
    volumeWriter->SetInput(seriesReader->GetOutput());

    try
    {
        volumeWriter->Update();
    }
    catch (itk::ExceptionObject& err)
    {
        std::cerr << "Error escribiendo el volumen: " << err << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Volumen 3D creado: " << outVolume << std::endl;

    // 4) Extraer y visualizar los slices con QuickView
    auto volume = seriesReader->GetOutput();
    auto fullRegion = volume->GetLargestPossibleRegion();
    auto size = fullRegion.GetSize();
    auto start = fullRegion.GetIndex();

    QuickView viewer;

    for (unsigned int z = startIdx; z <= endIdx; ++z)
    {
        // Definir región de extracción para el slice z
        VolumeType::RegionType sliceRegion;
        VolumeType::SizeType sliceSize = size;
        sliceSize[2] = 0;             // colapsar Z

        VolumeType::IndexType sliceStart = start;
        sliceStart[2] = z;

        sliceRegion.SetSize(sliceSize);
        sliceRegion.SetIndex(sliceStart);

        using ExtractFilterType = itk::ExtractImageFilter<VolumeType, SliceType>;
        auto extractFilter = ExtractFilterType::New();
        extractFilter->SetExtractionRegion(sliceRegion);
        extractFilter->SetInput(volume);
        extractFilter->SetDirectionCollapseToSubmatrix();

        try
        {
            extractFilter->Update();
        }
        catch (itk::ExceptionObject& err)
        {
            std::cerr << "Error extrayendo slice Z=" << z << ": " << err << std::endl;
            continue;
        }

        std::ostringstream title;
        title << "Slice Z=" << z;
        viewer.AddImage(extractFilter->GetOutput(), true, title.str());
    }

    viewer.Visualize();

    return EXIT_SUCCESS;
}
