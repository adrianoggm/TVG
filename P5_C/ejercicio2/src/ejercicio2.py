#!/usr/bin/env python3
import argparse
import os
import SimpleITK as sitk

def find_slice_file(dirpath, prefix, idx, exts):
    """Busca el archivo para un índice dado probando varias extensiones."""
    for ext in exts:
        name = f"{prefix}{idx}{ext}"
        path = os.path.join(dirpath, name)
        if os.path.isfile(path):
            return path
    return None

def read_dicom_series(input_dir, start_idx, end_idx, prefix, exts):
    """Lee archivos numerados de start_idx a end_idx en input_dir con prefijo y ext."""
    filenames = []
    for i in range(start_idx, end_idx + 1):
        f = find_slice_file(input_dir, prefix, i, exts)
        if f is None:
            raise FileNotFoundError(f"No se encontró slice para índice {i} en {input_dir}")
        filenames.append(f)
    reader = sitk.ImageSeriesReader()
    reader.SetFileNames(filenames)
    volume = reader.Execute()
    return volume, filenames

def print_metadata(volume):
    print("=== Metadatos del volumen ===")
    for key in volume.GetMetaDataKeys():
        print(f"{key}: {volume.GetMetaData(key)}")
    print("=============================")

def main():
    parser = argparse.ArgumentParser(
        description="Leer serie DICOM numerada y guardar como .mhd")
    parser.add_argument("input_dir", help="Directorio con las slices")
    parser.add_argument("output_mhd", help="Archivo de salida .mhd")
    parser.add_argument("--prefix", default="IMG", help="Prefijo de nombre (p.ej. IMG)")
    parser.add_argument("--start", type=int, required=True, help="Índice inicial")
    parser.add_argument("--end",   type=int, required=True, help="Índice final")
    parser.add_argument("--ext", nargs="+", default=["", ".dcm"],
                        help="Extensiones a probar, ej: '' '.dcm'")

    args = parser.parse_args()

    vol, files = read_dicom_series(args.input_dir, args.start, args.end,
                                   args.prefix, args.ext)
    print(f"Leídas {len(files)} slices:")
    for f in files:
        print(" ", os.path.basename(f))
    print_metadata(vol)

    sitk.WriteImage(vol, args.output_mhd)
    print(f"Volumen guardado en {args.output_mhd}")

if __name__ == "__main__":
    main()
