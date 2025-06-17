cmake_minimum_required(VERSION 3.5)

project(task8)

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

add_executable(task8 task8.cpp)
target_link_libraries(task8 ${Glue} ${VTK_LIBRARIES} ${ITK_LIBRARIES})
