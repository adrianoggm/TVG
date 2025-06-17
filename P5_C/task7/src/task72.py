#!/usr/bin/env python3
import argparse
import os
import sys
import SimpleITK as sitk
import numpy as np
import matplotlib
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt

def find_header_for_raw(raw_path):
    """
    Dado un .raw, busca raw_path.replace('.raw','.mhd') o '.nhdr'.
    """
    base = os.path.splitext(raw_path)[0]
    for ext in (".mhd", ".nhdr", ".mha"):
        candidate = base + ext
        if os.path.isfile(candidate):
            return candidate
    return None

def load_volume(path):
    """
    Carga un volumen desde .mhd/.mha/.nhdr o, si path termina en .raw,
    busca la cabecera asociada y la carga.
    """
    ext = os.path.splitext(path)[1].lower()
    if ext == ".raw":
        hdr = find_header_for_raw(path)
        if hdr is None:
            sys.exit(f"ERROR: No encontré un .mhd/.nhdr asociado a {path}")
        path = hdr
    try:
        vol = sitk.ReadImage(path)
    except Exception as e:
        sys.exit(f"ERROR leyendo volumen {path}: {e}")
    return vol

def visualize_slices(vol):
    """
    Muestra todas las slices del volumen 3D en un grid con matplotlib.
    """
    arr = sitk.GetArrayFromImage(vol)  # shape = (depth, height, width) o (d,h,w,c)
    # si es multicanal, despliega solo el primer canal
    if arr.ndim == 4:
        arr = arr[...,0]
    depth = arr.shape[0]
    cols = 4
    rows = int(np.ceil(depth/cols))
    fig, axes = plt.subplots(rows, cols, figsize=(4*cols, 4*rows))
    axes = axes.flatten()
    for k in range(depth):
        axes[k].imshow(arr[k], cmap='gray')
        axes[k].set_title(f"Slice {k}")
        axes[k].axis('off')
    for ax in axes[depth:]:
        ax.axis('off')
    plt.tight_layout()
    plt.show()

def main():
    parser = argparse.ArgumentParser(
        description="Visualiza todas las slices de un volumen 3D (.mhd/.mha/.nhdr con .raw asociado).")
    parser.add_argument("volume", help="Ruta al volumen o al .raw (busca cabecera .mhd/.nhdr/.mha)")
    args = parser.parse_args()

    vol = load_volume(args.volume)
    size = vol.GetSize()
    if len(size) != 3:
        sys.exit(f"ERROR: El volumen no es 3D (size={size})")
    print(f"Volumen cargado: size={size}, spacing={vol.GetSpacing()}, origin={vol.GetOrigin()}")
    visualize_slices(vol)

if __name__ == "__main__":
    main()
