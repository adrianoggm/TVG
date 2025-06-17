#!/usr/bin/env python3
import argparse
import os
import SimpleITK as sitk

def dicom_to_png(input_dcm: str, output_png: str):
    # Leer el DICOM (puede ser multi-frame, toma el primer frame si es 3D)
    img = sitk.ReadImage(input_dcm)
    if img.GetDimension() == 3:
        # extrae la slice 0 si es volumen
        extract = sitk.ExtractImageFilter()
        size = list(img.GetSize())
        extract.SetSize([size[0], size[1], 0])
        extract.SetIndex([0, 0, 0])
        img = extract.Execute(img)
    # Reescalar intensidades al rango [0,255] si no es entero de 8-bits
    if img.GetPixelID() != sitk.sitkUInt8:
        img = sitk.RescaleIntensity(img, 0, 255)
        img = sitk.Cast(img, sitk.sitkUInt8)
    # Asegurar carpeta de salida
    out_dir = os.path.dirname(output_png)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir)
    # Escribir PNG
    sitk.WriteImage(img, output_png)
    print(f"{input_dcm} → {output_png}")

def main():
    parser = argparse.ArgumentParser(
        description="Convert DICOM (single file) to PNG.")
    parser.add_argument("input_dcm", help="Ruta a imagen DICOM (.dcm)")
    parser.add_argument("output_png", help="Ruta de salida .png")
    args = parser.parse_args()

    if not args.output_png.lower().endswith(".png"):
        parser.error("El archivo de salida debe terminar en .png")
    dicom_to_png(args.input_dcm, args.output_png)

if __name__ == "__main__":
    main()
