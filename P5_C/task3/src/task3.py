import argparse
import os
import SimpleITK as sitk
import matplotlib
# Forcing a GUI backend on Windows
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt

def main():
    # 1) Parse command-line arguments
    parser = argparse.ArgumentParser(
        description="Read an RGB image, show a sample pixel, write a copy, and generate a preview.")
    parser.add_argument("input_image",  help="Path to the input RGB image")
    parser.add_argument("output_image", help="Path for the output RGB image")
    args = parser.parse_args()

    input_path  = args.input_image
    output_path = args.output_image

    # Derive preview filename from the output name
    base, ext = os.path.splitext(output_path)
    preview_path = f"{base}_preview.png"

    # 2) Read the RGB image
    img = sitk.ReadImage(input_path)

    # 3) Sample a pixel at (25, 35)
    # Coordinates in SimpleITK are (x, y)
    try:
        r, g, b = img.GetPixel(25, 35)
        print(f"Sample pixel at (25,35): Red={r}, Green={g}, Blue={b}")
    except Exception as e:
        print(f"Warning: could not read pixel (25,35): {e}")

    # 4) Write the image copy
    sitk.WriteImage(img, output_path)
    print(f"Image written to {output_path}")

    # 5) Generate preview with Matplotlib
    arr = sitk.GetArrayViewFromImage(img)  # shape: (height, width, 3)
    plt.figure(figsize=(6, 6))
    plt.imshow(arr)
    plt.axis('off')
    plt.tight_layout()
    plt.savefig(preview_path, dpi=150)
    print(f"Preview saved to {preview_path}")

    # 6) Show the window interactively
    plt.show()

    # 7) On Windows, open the preview file in the default image viewer
    try:
        os.startfile(preview_path)
    except AttributeError:
        # os.startfile is only on Windows
        pass

if __name__ == "__main__":
    main()
