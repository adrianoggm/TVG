#!/usr/bin/env python3
import itk

# --- Creación de una imagen 2D ---
# Usamos unsigned short para los píxeles
PixelType = itk.US  # US equivale a unsigned short
Dimension2D = 2
ImageType2D = itk.Image[PixelType, Dimension2D]

# Crear la imagen
image2D = ImageType2D.New()

# Definir el índice (origen de la región) y el tamaño (número de píxeles)
index2D = itk.Index[Dimension2D]()
index2D[0] = 0
index2D[1] = 0

size2D = itk.Size[Dimension2D]()
size2D[0] = 720   # píxeles en X
size2D[1] = 348   # píxeles en Y

# Crear la región y asignarla a la imagen
region2D = itk.ImageRegion[Dimension2D]()
region2D.SetIndex(index2D)
region2D.SetSize(size2D)

image2D.SetRegions(region2D)
image2D.Allocate()

print("Imagen 2D creada con tamaño:", list(size2D))


# --- Creación de una imagen 3D ---
Dimension3D = 3
ImageType3D = itk.Image[PixelType, Dimension3D]
image3D = ImageType3D.New()

index3D = itk.Index[Dimension3D]()
index3D[0] = 0
index3D[1] = 0
index3D[2] = 0

size3D = itk.Size[Dimension3D]()
size3D[0] = 256   # píxeles/voxeles en X
size3D[1] = 256   # en Y
size3D[2] = 20    # en Z

region3D = itk.ImageRegion[Dimension3D]()
region3D.SetIndex(index3D)
region3D.SetSize(size3D)

image3D.SetRegions(region3D)
image3D.Allocate()

print("Imagen 3D creada con tamaño:", list(size3D))
