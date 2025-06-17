#!/usr/bin/env python3
import argparse
import os
import SimpleITK as sitk
import numpy as np
import matplotlib
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt

def main():
    parser = argparse.ArgumentParser(
        description="Read a series of 2D BMP images (t50.bmp–t60.bmp), write as a 3D .mhd volume, then display all slices.")
    parser.add_argument("input_dir", help="Directory containing t50.bmp ... t60.bmp")
    parser.add_argument("output_mhd", help="Output MetaImage header file (e.g. volume.mhd)")
    parser.add_argument("--start", type=int, default=50, help="Start index (default: 50)")
    parser.add_argument("--end",   type=int, default=60, help="End index inclusive (default: 60)")
    args = parser.parse_args()

    # Build list of slice filenames
    filenames = []
    for i in range(args.start, args.end + 1):
        fname = os.path.join(args.input_dir, f"t{i:02d}.bmp")
        if not os.path.isfile(fname):
            parser.error(f"Expected file not found: {fname}")
        filenames.append(fname)
    print(f"Found {len(filenames)} images: {filenames[0]} ... {filenames[-1]}")

    # Read the series into a 3D volume
    reader = sitk.ImageSeriesReader()
    reader.SetFileNames(filenames)
    volume = reader.Execute()
    print(f"Volume size: {volume.GetSize()}")

    # Ensure output directory exists
    out_dir = os.path.dirname(args.output_mhd)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)

    # Write out the volume as .mhd/.raw
    sitk.WriteImage(volume, args.output_mhd)
    print(f"Wrote volume to {args.output_mhd}")

    # Convert volume to numpy array (depth, height, width)
    arr = sitk.GetArrayFromImage(volume)
    depth = arr.shape[0]

    # Display all slices in a grid
    cols = 4
    rows = int(np.ceil(depth / cols))
    fig, axes = plt.subplots(rows, cols, figsize=(4*cols, 4*rows))
    axes = axes.flatten()

    for k in range(depth):
        axes[k].imshow(arr[k], cmap='gray')
        axes[k].set_title(f"Slice {k + args.start}")
        axes[k].axis('off')
    # Hide unused axes
    for ax in axes[depth:]:
        ax.axis('off')

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
