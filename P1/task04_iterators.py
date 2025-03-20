#!/usr/bin/env python3
import itk
import numpy as np

# -------------------------------------------------------------
# Definición del tipo de píxel y de imagen (float, 2D)
PixelType = itk.F   # float
Dimension = 2
ImageType2D = itk.Image[PixelType, Dimension]

# -------------------------------------------------------------
# Crear una imagen aleatoria (20x20) usando RandomImageSource
random_source = itk.RandomImageSource[ImageType2D].New()
random_source.SetMin(0.0)
random_source.SetMax(1.0)

# Definir el tamaño de la imagen
size = itk.Size[Dimension]()
size[0] = 20
size[1] = 20
random_source.SetSize(size)

# Ejecutar la generación de la imagen
random_source.Update()
inputImage2D = random_source.GetOutput()

# -------------------------------------------------------------
# Crear una imagen de salida con la misma región que la de entrada
outputImage2D = ImageType2D.New()
outputImage2D.SetRegions(inputImage2D.GetLargestPossibleRegion())
outputImage2D.Allocate()

# -------------------------------------------------------------
# Acceder a los datos de píxeles utilizando una vista de arreglo (sin copiar datos)
input_array = itk.array_view_from_image(inputImage2D)
# Crear un arreglo para la imagen de salida (inicialmente en ceros) con la misma forma
output_array = np.zeros_like(input_array)

print("Procesando la imagen píxel a píxel (valor de salida = valor de entrada * valor de entrada):")
# Iterar sobre los píxeles usando np.ndenumerate (permite acceder al índice y al valor)
for idx, value in np.ndenumerate(input_array):
    result = value * value  # Operación: elevar al cuadrado
    output_array[idx] = result
    # Imprimir solo las primeras 10 iteraciones para no saturar la salida
    if idx[0] < 1 and idx[1] < 10:
        print(f"Índice: {idx}, Valor de entrada: {value:.4f}, Valor de salida: {result:.4f}")

# -------------------------------------------------------------
# Convertir el arreglo de salida de vuelta a una imagen ITK
outputImage2D = itk.image_from_array(output_array)

# (Opcional) Guardar la imagen de salida en disco, por ejemplo en formato .mha
# itk.imwrite(outputImage2D, "output_image.mha")
