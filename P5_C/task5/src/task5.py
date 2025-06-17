import argparse
import os
import SimpleITK as sitk
import matplotlib
# Ensure GUI backend for Windows
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt

def main():
    parser = argparse.ArgumentParser(
        description="Read an image (RGB or grayscale), extract a region of interest (ROI), write it to file, and display both.")
    parser.add_argument("input_image",  help="Path to the input image")
    parser.add_argument("output_image", help="Path for the output ROI image")
    parser.add_argument("start_x", type=int, help="ROI starting X coordinate")
    parser.add_argument("start_y", type=int, help="ROI starting Y coordinate")
    parser.add_argument("size_x",  type=int, help="ROI width")
    parser.add_argument("size_y",  type=int, help="ROI height")
    args = parser.parse_args()

    input_path  = args.input_image
    output_path = args.output_image
    start_x, start_y = args.start_x, args.start_y
    size_x, size_y   = args.size_x,  args.size_y

    # Ensure output directory exists
    out_dir = os.path.dirname(output_path)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)

    # Read the image (SimpleITK auto-detects pixel type)
    image = sitk.ReadImage(input_path)

    # Determine number of components: 1 = grayscale, >1 = vector (RGB)
    ncomp = image.GetNumberOfComponentsPerPixel()
    mode = 'RGB' if ncomp == 3 else 'GRAY'
    print(f"Image components: {ncomp} -> mode={mode}")

    # Image dimensions
    width, height = image.GetSize()[0], image.GetSize()[1]
    print(f"Input size: width={width}, height={height}")

    # Validate and clamp ROI
    if start_x < 0 or start_y < 0 or start_x >= width or start_y >= height:
        raise ValueError("Start coordinates out of image bounds.")
    clamped_x = min(size_x, width - start_x)
    clamped_y = min(size_y, height - start_y)
    if clamped_x != size_x or clamped_y != size_y:
        print(f"Clamped ROI size from ({size_x},{size_y}) to ({clamped_x},{clamped_y})")

    # Extract ROI
    roi_filter = sitk.RegionOfInterestImageFilter()
    roi_filter.SetIndex((start_x, start_y))
    roi_filter.SetSize((clamped_x, clamped_y))
    image_roi = roi_filter.Execute(image)

    # Write ROI image
    sitk.WriteImage(image_roi, output_path)
    print(f"ROI image written to {output_path}")

    # Convert to numpy arrays
    arr_orig = sitk.GetArrayViewFromImage(image)
    arr_roi  = sitk.GetArrayViewFromImage(image_roi)

    # Display
    plt.figure(figsize=(12, 6))

    # Original
    plt.subplot(1, 2, 1)
    plt.title("Original")
    if ncomp == 1:
        plt.imshow(arr_orig, cmap='gray')
    else:
        plt.imshow(arr_orig)
    plt.axis('off')

    # ROI
    plt.subplot(1, 2, 2)
    plt.title(f"ROI start=({start_x},{start_y}), size=({clamped_x},{clamped_y})")
    if ncomp == 1:
        plt.imshow(arr_roi, cmap='gray')
    else:
        plt.imshow(arr_roi)
    plt.axis('off')

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
