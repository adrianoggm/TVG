import itk
import vtk
import matplotlib.pyplot as plt
import sys

if len(sys.argv) != 2:
    print("Uso: python task02_leer_y_mostrar.py <ruta_imagen>")
    sys.exit(1)

input_filename = sys.argv[1]
itk_image = itk.imread(input_filename)

# Convertir la imagen ITK a un objeto vtkImageData
vtk_image = itk.vtk_image_from_image(itk_image)

# Configurar el visualizador de VTK
viewer = vtk.vtkImageViewer2()
viewer.SetInputData(vtk_image)

# Crear un interactor para la ventana de renderizado
render_window_interactor = vtk.vtkRenderWindowInteractor()
viewer.SetupInteractor(render_window_interactor)

# Renderizar la imagen
viewer.Render()
viewer.GetRenderer().ResetCamera()
viewer.Render()

# Iniciar el interactor para visualizar la imagen
render_window_interactor.Start()
