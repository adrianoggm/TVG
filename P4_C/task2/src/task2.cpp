#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCurvatureFlowImageFilter.h"            // Curvature Flow
#include "itkMedianImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkWatershedImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    std::cerr << "Uso: " << argv[0]
              << " <imagenEntrada> <prefijoSalida>" << std::endl;
    return EXIT_FAILURE;
  }

  const std::string inputFile = argv[1];
  const std::string prefix    = argv[2];

  // Extraer baseName sin ruta ni extensión
  std::string fname = inputFile;
  auto posSlash = fname.find_last_of("/\\");
  if (posSlash != std::string::npos) fname = fname.substr(posSlash+1);
  auto posDot = fname.rfind('.');
  std::string baseName = (posDot != std::string::npos)
    ? fname.substr(0,posDot)
    : fname;

  constexpr unsigned int Dimension = 2;
  using FloatImage = itk::Image<float,Dimension>;
  using WatershedFilterType = itk::WatershedImageFilter<FloatImage>;
  using LabelImage         = WatershedFilterType::OutputImageType;
  using UCharImage         = itk::Image<unsigned char,Dimension>;

  // Leer imagen
  auto reader = itk::ImageFileReader<FloatImage>::New();
  reader->SetFileName(inputFile);
  reader->Update();

  // Modos de suavizado: ninguno, flujo de curvatura, mediana
  std::vector<std::string> modos = { "none", "curvature", "median" };

  // Parámetros de watershed
  std::vector<double> thresholds = { 0.001, 0.005, 0.01 };
  std::vector<double> levels     = { 0.1, 0.2, 0.3, 0.4 };

  for (const auto& modo : modos)
  {
    // 1) Imagen de entrada al paso de suavizado
    FloatImage::Pointer proc = reader->GetOutput();

    // 2) Suavizado según el modo
    if (modo == "curvature")
    {
      using CurvatureFlowType = itk::CurvatureFlowImageFilter<FloatImage,FloatImage>;
      auto flow = CurvatureFlowType::New();
      flow->SetInput(proc);
      // **Parámetros exactos del ejemplo**:
      flow->SetNumberOfIterations(14);
      flow->SetTimeStep(0.0025);
      flow->Update();
      proc = flow->GetOutput();
    }
    else if (modo == "median")
    {
      auto med = itk::MedianImageFilter<FloatImage,FloatImage>::New();
      typename FloatImage::SizeType radius; radius.Fill(2);
      med->SetRadius(radius);
      med->SetInput(proc);
      med->Update();
      proc = med->GetOutput();
    }

    // 3) Guardar imagen pre‐watershed
    {
      using RescaleType = itk::RescaleIntensityImageFilter<FloatImage,UCharImage>;
      auto res = RescaleType::New();
      res->SetInput(proc);
      res->SetOutputMinimum(0);
      res->SetOutputMaximum(255);
      res->Update();

      std::ostringstream name;
      name << baseName << "_" << prefix
           << "_" << modo << "_processed.png";
      auto writer = itk::ImageFileWriter<UCharImage>::New();
      writer->SetFileName(name.str());
      writer->SetInput(res->GetOutput());
      writer->Update();
    }

    // 4) Calcular el gradiente de magnitud
    auto grad = itk::GradientMagnitudeImageFilter<FloatImage,FloatImage>::New();
    grad->SetInput(proc);
    grad->Update();

    // 5) Ejecutar Watershed con todas las combinaciones
    for (double t : thresholds)
    {
      for (double l : levels)
      {
        auto ws = WatershedFilterType::New();
        ws->SetInput(grad->GetOutput());
        ws->SetThreshold(t);
        ws->SetLevel(l);
        ws->Update();

        // Castear la salida de etiquetas a float
        using CastType = itk::CastImageFilter<LabelImage,FloatImage>;
        auto caster = CastType::New();
        caster->SetInput(ws->GetOutput());
        caster->Update();

        // Reescalar a unsigned char para visualizar
        using RescaleType = itk::RescaleIntensityImageFilter<FloatImage,UCharImage>;
        auto res = RescaleType::New();
        res->SetInput(caster->GetOutput());
        res->SetOutputMinimum(0);
        res->SetOutputMaximum(255);
        res->Update();

        // Nombre de salida
        std::ostringstream out;
        out << baseName << "_" << prefix
            << "_WS_" << modo
            << "_T" << std::fixed << std::setprecision(3) << t
            << "_L" << std::fixed << std::setprecision(1) << l
            << ".png";

        auto writer = itk::ImageFileWriter<UCharImage>::New();
        writer->SetFileName(out.str());
        writer->SetInput(res->GetOutput());
        writer->Update();

        std::cout << "Guardado: " << out.str() << std::endl;
      }
    }
  }

  std::cout << "Watershed segmentación finalizada." << std::endl;
  return EXIT_SUCCESS;
}
