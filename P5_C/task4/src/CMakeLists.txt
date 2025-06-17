cmake_minimum_required(VERSION 3.5)

project(task4)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

if (ITKVtkGlue_LOADED)
  find_package(VTK REQUIRED)
  include(${VTK_USE_FILE})
else()
  find_package(ItkVtkGlue REQUIRED)
  include(${ItkVtkGlue_USE_FILE})
  set(Glue ItkVtkGlue)
endif()

add_executable(task4 task4.cpp)
target_link_libraries(task4 ${Glue} ${VTK_LIBRARIES} ${ITK_LIBRARIES})
