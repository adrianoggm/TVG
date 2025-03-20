#!/usr/bin/env python3
import itk
import vtk
import sys

if len(sys.argv) != 3:
    print("Uso: python task05_average_filter.py <imagen_entrada> <kernel_size>")
    sys.exit(1)

input_filename = sys.argv[1]
kernel_size = int(sys.argv[2])

# Leer la imagen de entrada con ITK
try:
    image = itk.imread(input_filename)
except Exception as e:
    print("Error al leer la imagen:", e)
    sys.exit(1)

# Calcular el radio (la mitad del kernel)
radius = kernel_size // 2

# Aplicar el filtro promedio (mean filter) usando la interfaz funcional de ITK
try:
    filtered = itk.mean_image_filter(image, radius=radius)
except Exception as e:
    print("Error al aplicar el filtro promedio:", e)
    sys.exit(1)

# Convertir las imágenes ITK a objetos vtkImageData
vtk_image = itk.vtk_image_from_image(image)
vtk_filtered = itk.vtk_image_from_image(filtered)

# Crear una ventana de renderizado con dos viewports
renderWindow = vtk.vtkRenderWindow()
renderWindow.SetSize(1000, 500)

# Crear un interactor para la ventana
interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(renderWindow)

# Crear dos renderers para dos viewports: izquierda (imagen original) y derecha (imagen filtrada)
renderer_left = vtk.vtkRenderer()
renderer_right = vtk.vtkRenderer()
renderWindow.AddRenderer(renderer_left)
renderWindow.AddRenderer(renderer_right)

renderer_left.SetViewport(0.0, 0.0, 0.5, 1.0)
renderer_right.SetViewport(0.5, 0.0, 1.0, 1.0)

# Crear actores de imagen para cada imagen
actor_left = vtk.vtkImageActor()
actor_left.GetMapper().SetInputData(vtk_image)
actor_right = vtk.vtkImageActor()
actor_right.GetMapper().SetInputData(vtk_filtered)

renderer_left.AddActor(actor_left)
renderer_right.AddActor(actor_right)

# Ajustar las cámaras para que ambas imágenes se vean correctamente
renderer_left.ResetCamera()
renderer_right.ResetCamera()

# Renderizar la ventana y comenzar la interacción
renderWindow.Render()
interactor.Initialize()
interactor.Start()
