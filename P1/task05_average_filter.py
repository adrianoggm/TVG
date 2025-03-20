#!/usr/bin/env python3
import itk
import sys

if len(sys.argv) != 3:
    print("Uso: python task05_average_filter.py <imagen_entrada> <kernel_size>")
    sys.exit(1)

input_filename = sys.argv[1]
kernel_size = int(sys.argv[2])

# Leer la imagen de entrada
image = itk.imread(input_filename)

# Calcular el radio: se suele usar la mitad del tamaño del kernel
radius = kernel_size // 2

# Aplicar el filtro promedio (mean filter) usando la interfaz funcional de ITK
try:
    filtered = itk.mean_image_filter(image, radius=radius)
except Exception as e:
    print("Error al aplicar el filtro promedio:", e)
    sys.exit(1)

# Intentar visualizar usando itkwidgets
try:
    import itkwidgets
    # Mostrar ambas imágenes (itkwidgets.view soporta varios argumentos)
    itkwidgets.view(image, filtered)
except ImportError:
    print("No se encontró itkwidgets. Se usará matplotlib como alternativa.")
    import matplotlib.pyplot as plt
    # Convertir las imágenes a arreglos NumPy
    image_array = itk.array_view_from_image(image)
    filtered_array = itk.array_view_from_image(filtered)
    
    fig, axes = plt.subplots(1, 2, figsize=(12, 6))
    
    axes[0].imshow(image_array, cmap="gray")
    axes[0].set_title("Imagen Original")
    axes[0].axis("off")
    
    axes[1].imshow(filtered_array, cmap="gray")
    axes[1].set_title("Imagen Filtrada (Promedio)")
    axes[1].axis("off")
    
    plt.show()
