#pragma once
#define _SCL_SECURE_NO_WARNINGS
#include <vtkAbstractPropPicker.h>
#include <vtkPNGWriter.h>
#include <vtkWindowToImageFilter.h>
#include <itkMacro.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkInteractorStyleImage.h>
#include <vtkCallbackCommand.h>
#include <vtkOpenGLActor.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkWin32RenderWindowInteractor.h>
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <memory>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkCommand.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <array>
#include <vtkProperty.h>
#include <vtkAxesActor.h>
#include <vtkTextProperty.h>
#include <vtkCaptionActor2D.h>
#include <vtkPropCollection.h>
#include <vector>
#include <string>
#include <itkImage.h>
#include <vtkImageImport.h>
#include <itkCommand.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <fstream>
#include <itkMacro.h>
#include <itkOrientImageFilter.h>
#include <vtkOpenGLRenderer.h>
#include <vtkWin32OpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageSlabReslice.h>
#include "boost/date_time/posix_time/posix_time.hpp" //include all types plus i/o
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <vtkXMLImageDataWriter.h>
#include <vtkMatrix4x4.h>
#include <assert.h>
#include <vtkImageActor.h>
#include <vtkImageProperty.h>
#include <vtkImageMapper3d.h>
#include <vtkObjectFactory.h>
#include <vtkLightsPass.h>
#include <vtkDefaultPass.h>
#include <vtkRenderPassCollection.h>
#include <vtkSequencePass.h>
#include <vtkCameraPass.h>
#include <vtkRenderState.h>
#include <SDL2\SDL_ttf.h>
#undef main
#include <SDL2/SDL.h>
#undef main
#include <vtkGeneralTransform.h>
#include <vtkPlane.h>
#include <vtkPlaneSource.h>

#include <vtkTexture.h>
#include <vtkClipPolyData.h>
#include <vtkAlgorithmOutput.h>
#include <vtkPolyData.h>
#include <vtkAssemblyPath.h>
#include <vtkAssemblyPaths.h>
#include <vtkAssemblyNode.h>
#include <vtkAssembly.h>
#include <vtkSphereSource.h>
#include <vtkImageFlip.h>
#include <vtkCameraPass.h>
#include <vtkClearZPass.h>
#include <vtkLightsPass.h>
#include <vtkOpaquePass.h>
#include <vtkTranslucentPass.h>
#include <vtkVolumetricPass.h>
#include <vtkOverlayPass.h>
#include <vtkImageResliceToColors.h>
typedef void(_stdcall*FNCallbackDeCarga)(float);

struct ImageDataToDelphi {
	double spacing[3];
	double physicalOrigin[3];
	double uVector[3];
	double vVector[3];
	int imageSize[2];
	unsigned int bufferSize;
	short *bufferData;
};

typedef void(_stdcall *FNCallbackDoDicomReslice)(ImageDataToDelphi outData);
//Equivale à vtkSmartPointer<X>::New().
#define NewVTK(X) \
vtkSmartPointer<X>::New()
//Definições de operações com vetores de 3 elementos
static inline std::array<double, 3> operator+(const std::array<double, 3> v1, const std::array<double, 3> v2) {
	std::array<double, 3> r;
	r[0] = v1[0] + v2[0];
	r[1] = v1[1] + v2[1];
	r[2] = v1[2] + v2[2];
	return r;
}
static inline std::array<double, 3> operator-(const std::array<double, 3> v1, const std::array<double, 3> v2) {
	std::array<double, 3> r;
	r[0] = v1[0] - v2[0];
	r[1] = v1[1] - v2[1];
	r[2] = v1[2] - v2[2];
	return r;
}

static inline std::array<double, 3> operator*(const std::array<double, 3> v, const double s) {
	std::array<double, 3> r;
	r[0] = v[0] * s;
	r[1] = v[1] * s;
	r[2] = v[2] * s;
	return r;
}

static inline std::array<double, 3> operator*( const double s, const std::array<double, 3> v) {
	return v * s;
}