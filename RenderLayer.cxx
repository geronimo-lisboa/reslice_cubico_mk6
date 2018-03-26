#include "stdafx.h"
#include "RenderLayer.h"

RenderLayer::RenderLayer(int _numLayer) :numLayer(_numLayer) {
	renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
	renderer-> SetLayer(numLayer);
	renderer->GetActiveCamera()->ParallelProjectionOn();
}

vtkSmartPointer<vtkOpenGLRenderer> RenderLayer::GetRenderer(){
	return renderer;
}

const int RenderLayer::GetNumLayer(){
	return numLayer;
}