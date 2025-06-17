#!/usr/bin/env python3
import argparse
import os
import SimpleITK as sitk
import matplotlib.pyplot as plt

def read_dicom(path):
    if os.path.isdir(path):
        reader = sitk.ImageSeriesReader()
        series_ids = reader.GetGDCMSeriesIDs(path)
        if not series_ids:
            raise ValueError(f"No se encontró ninguna serie DICOM en: {path}")
        files = reader.GetGDCMSeriesFileNames(path, series_ids[0])
        reader.SetFileNames(files)
        return reader.Execute()
    else:
        return sitk.ReadImage(path)

def main():
    parser = argparse.ArgumentParser(
        description="Leer y mostrar DICOM (archivo .dcm o carpeta), opcionalmente un slice Z.")
    parser.add_argument("dicom_path", help="Fichero .dcm o directorio DICOM")
    parser.add_argument("--frame", "-f", type=int, default=0,
                        help="Índice Z/Frame a mostrar (0-based). Si 2D, se ignora pero se muestra.")
    args = parser.parse_args()

    img = read_dicom(args.dicom_path)
    dim = img.GetDimension()
    k = args.frame

    if dim == 3:
        size = list(img.GetSize())
        d = size[2]
        if not (0 <= k < d):
            raise ValueError(f"Frame fuera de rango [0, {d-1}]")
        # Extraer slice Z=k
        extractor = sitk.ExtractImageFilter()
        extractor.SetSize([size[0], size[1], 0])
        extractor.SetIndex([0, 0, k])
        slice2d = extractor.Execute(img)
        title = f"DICOM slice Z={k}"
        to_show = slice2d
    else:
        # Imagen 2D, no extracción
        title = f"DICOM 2D (usando frame={k})"
        to_show = img

    # Mostrar
    arr = sitk.GetArrayViewFromImage(to_show)
    plt.figure(figsize=(6,6))
    plt.imshow(arr, cmap='gray')
    plt.title(title)
    plt.axis('off')
    plt.show()

if __name__ == "__main__":
    main()
