import argparse
import os
import SimpleITK as sitk
import matplotlib
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt

def main():
    parser = argparse.ArgumentParser(
        description="Lee una imagen float, reescala, castea a uchar, escribe y muestra ambas.")
    parser.add_argument("input_image",  help="Ruta de la imagen de entrada (float)")
    parser.add_argument("output_image", help="Ruta de la imagen de salida (uchar)")
    args = parser.parse_args()

    input_path  = args.input_image
    output_path = args.output_image

    # Asegurar que el directorio de salida existe
    out_dir = os.path.dirname(output_path)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)

    # 1) Leer la imagen float
    image_float = sitk.ReadImage(input_path, sitk.sitkFloat32)

    # 2) Reescalar intensidades a [0,255]
    rescaler = sitk.RescaleIntensityImageFilter()
    rescaler.SetOutputMinimum(0.0)
    rescaler.SetOutputMaximum(255.0)
    image_rescaled = rescaler.Execute(image_float)

    # 3) Castear a unsigned char
    image_uchar = sitk.Cast(image_rescaled, sitk.sitkUInt8)

    # 4) Escribir la imagen casteada
    sitk.WriteImage(image_uchar, output_path)
    print(f"Imagen casteada guardada en {output_path}")

    # 5) Convertir a numpy para visualizar
    arr_float = sitk.GetArrayViewFromImage(image_float)
    arr_uchar = sitk.GetArrayViewFromImage(image_uchar)

    # 6) Mostrar lado a lado
    plt.figure(figsize=(10,5))
    plt.subplot(1,2,1)
    plt.title("Original (float)")
    plt.imshow(arr_float, cmap="gray")
    plt.axis("off")

    plt.subplot(1,2,2)
    plt.title("Convertida (uchar)")
    plt.imshow(arr_uchar, cmap="gray", vmin=0, vmax=255)
    plt.axis("off")

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
