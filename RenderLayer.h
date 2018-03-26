#pragma once
#include "stdafx.h"
//Uma camada de renderiza��o em uma renderWindow. As camadas tem o renderer, a layer do renderer
//e no futuro ter� os render passes.
class RenderLayer{
private:
	vtkSmartPointer<vtkOpenGLRenderer> renderer;
	const int numLayer;
public:
	RenderLayer(int _numLayer);
	vtkSmartPointer<vtkOpenGLRenderer> GetRenderer();
	const int GetNumLayer();
	bool operator<(const RenderLayer& other){
		return numLayer < other.numLayer;
	}
};