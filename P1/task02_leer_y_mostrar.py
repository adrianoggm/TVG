import itk
import matplotlib.pyplot as plt
import sys

if len(sys.argv) != 2:
    print("Uso: python task02_leer_y_mostrar.py <ruta_imagen>")
    sys.exit(1)

input_filename = sys.argv[1]

# Leer la imagen
itk_image = itk.imread(input_filename)

# Convertir la imagen a un arreglo NumPy (vista, sin copiar datos)
array_view = itk.array_view_from_image(itk_image)

# Mostrar la imagen (suponiendo que es en escala de grises)
plt.imshow(array_view, cmap='gray')
plt.title("Imagen de Entrada")
plt.axis("off")
plt.show()
