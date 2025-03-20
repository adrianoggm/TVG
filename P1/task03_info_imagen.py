#!/usr/bin/env python3
import sys
import itk
import numpy as np

if len(sys.argv) != 2:
    print("Uso: python task03_info_imagen.py <ruta_imagen>")
    sys.exit(1)

input_filename = sys.argv[1]

# Leer la imagen usando ITK
try:
    image = itk.imread(input_filename)
except Exception as e:
    print("Error al leer la imagen:", e)
    sys.exit(1)

# Extraer metadatos de la imagen
region = image.GetLargestPossibleRegion()
size = region.GetSize()
origin = image.GetOrigin()
spacing = image.GetSpacing()
direction = image.GetDirection()

# Imprimir metadatos
print("Tamaño de la imagen (por dimensión):")
print("  ", list(size))

print("\nOrigen:")
print("  ", list(origin))

print("\nEspaciado:")
print("  ", list(spacing))

print("\nMatriz de dirección:")
print("  ", direction)

# --- Acceso a píxeles mediante iteración usando NumPy ---
# Se obtiene una vista del arreglo de píxeles sin copiar datos
array_view = itk.array_view_from_image(image)

print("\nEjemplo de acceso a píxeles (primeros 10 píxeles usando np.ndenumerate):")
counter = 0
for index, value in np.ndenumerate(array_view):
    print("Índice:", index, "Valor:", value)
    counter += 1
    if counter >= 10:
        break
