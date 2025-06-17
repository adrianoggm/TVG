import argparse
import os
import tempfile
import SimpleITK as sitk
import matplotlib
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt

def read_volume(path):
    ext = os.path.splitext(path)[1].lower()
    if ext == ".raw":
        # Create a temporary .mhd header for the raw file
        raw_path = os.path.abspath(path)
        header_content = f"""NDims = 3
DimSize = 181 217 3
ElementType = MET_UCHAR
ElementSpacing = 1.0 1.0 1.0
ElementByteOrderMSB = False
ElementDataFile = {raw_path}
"""
        tmp_dir = tempfile.mkdtemp()
        header_path = os.path.join(tmp_dir, "temp_header.mhd")
        with open(header_path, "w") as f:
            f.write(header_content)
        # Read via MetaImage
        vol = sitk.ReadImage(header_path)
        # cleanup temp files
        # (optional: keep if debugging)
        return vol
    else:
        return sitk.ReadImage(path)

def main():
    parser = argparse.ArgumentParser(
        description="Extract a 2D slice from a 3D volume (.mha or .raw), write it to file, and display.")
    parser.add_argument("input_volume", help="Path to the input 3D volume (.mha or .raw)")
    parser.add_argument("output_image", help="Path for the output 2D slice image")
    parser.add_argument("--slice", "-k", type=int,
                        help="Z-index of the slice to extract (0-based). If omitted, uses middle slice.")
    args = parser.parse_args()

    vol = read_volume(args.input_volume)
    if vol.GetDimension() != 3:
        raise ValueError("Input must be a 3D volume.")

    w, h, d = vol.GetSize()
    slice_idx = args.slice if args.slice is not None else d // 2
    if slice_idx < 0 or slice_idx >= d:
        raise ValueError(f"Slice index out of range [0, {d-1}].")

    extractor = sitk.ExtractImageFilter()
    extractor.SetSize([w, h, 0])
    extractor.SetIndex([0, 0, slice_idx])
    slice2d = extractor.Execute(vol)

    # ensure output dir exists
    out_dir = os.path.dirname(args.output_image)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)

    sitk.WriteImage(slice2d, args.output_image)
    print(f"Extracted slice Z={slice_idx} saved to {args.output_image}")

    arr = sitk.GetArrayViewFromImage(slice2d)
    comps = slice2d.GetNumberOfComponentsPerPixel()
    cmap = 'gray' if comps == 1 else None

    plt.figure(figsize=(6, 6))
    if cmap:
        plt.imshow(arr, cmap=cmap)
    else:
        plt.imshow(arr)
    plt.title(f"Slice Z={slice_idx}")
    plt.axis('off')
    plt.show()

if __name__ == "__main__":
    main()
