cmake_minimum_required(VERSION 2.8)
include(PrecompiledHeader.cmake) #O script de cmake pra precompiled headers

project( reslice_cubico_mk6 )
#Seta os paths da VTK, ITK e SDL2
set(ITK_DIR "C:/itk_nova/ITK_build")

set(VTK_DIR "C:/VTK-8.1.0-opengl2-32bits")

# Find ITK.
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
# Find VTK
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
#sdl
set(SDL2_INCLUDE_DIR "C:/Program Files (x86)/SDL2/include")
set(SDL2_LIBRARY "C:/Program Files (x86)/SDL2/lib/SDL2.lib")
include_directories(${SDL2_INCLUDE_DIR})
file(GLOB my_source_files "*.h" "*.cpp" "*.cxx")
#boost.org
SET (BOOST_INCLUDE_DIR "C:/boost_1_64_0")
include_directories(${BOOST_INCLUDE_DIR})

add_executable(reslice2 ${my_source_files} "stdafx.h" "stdafx.cpp")

target_link_libraries(reslice2 ${ITK_LIBRARIES} ${VTK_LIBRARIES}  ${SDL2_LIBRARY})
target_link_libraries(reslice2 "C:/boost_1_64_0/stage/win32/lib/libboost_signals-vc120-mt-1_64.lib")
target_link_libraries(reslice2 "C:/boost_1_64_0/stage/win32/lib/libboost_exception-vc120-mt-gd-1_64.lib")
target_link_libraries(reslice2 "C:/boost_1_64_0/stage/win32/lib/libboost_locale-vc120-mt-1_64.lib")
target_link_libraries(reslice2 "C:/boost_1_64_0/stage/win32/lib/libboost_date_time-vc120-mt-gd-1_64.lib")
target_link_libraries(reslice2 "C:/boost_1_64_0/stage/win32/lib/libboost_date_time-vc120-mt-1_64.lib")
target_link_libraries(reslice2 "C:/libs/SDL2_ttf-2.0.14/lib/x86/SDL2_ttf.lib")
set_property(TARGET reslice2 APPEND PROPERTY LINK_FLAGS /LARGEADDRESSAWARE)

add_precompiled_header(reslice2 stdafx.h)#Para adicionar precompiled headers
target_compile_options(reslice2 PRIVATE "/Zm1024")

#sdl_ttf
#include_directories("C:/libs/SDL2_ttf-2.0.14/include")


