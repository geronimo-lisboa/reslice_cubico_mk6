#ifndef ResliceCubicoInteractionStyle_h
#define ResliceCubicoInteractionStyle_h
#include "stdafx.h"
#include "vtkInteractorStyle.h"

class vtkCellPicker;

class ResliceCubicoInteractionStyle : public vtkInteractorStyle
{
private:
	std::function<void(double scaleFactor)> callbackZoom;

	std::function<void(vtkProp3D *)> callbackRotacao;
	//std::function<void(vtkProp3D *cubo, std::array<double, 3> motionVector)> callbackPan;
	std::function<void(vtkCamera *cam, std::array<double, 3> motionVector)> callbackPan;
public:
	void SetCallbackDeZoom(std::function<void(double scaleFactor)> c){
		callbackZoom = c;
	}

	void SetCallbackDeRotacao(std::function<void(vtkProp3D *)> c){
		callbackRotacao = c;
	}
	//void SetCallbackPan(std::function <void(vtkProp3D *cubo, std::array<double, 3> motionVector)> c){
	//	callbackPan = c;
	//}
	void SetCallbackPan(std::function<void(vtkCamera *cam, std::array<double, 3> motionVector)> c){
		callbackPan = c;
	}

  static ResliceCubicoInteractionStyle *New();
  vtkTypeMacro(ResliceCubicoInteractionStyle,vtkInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /**
   * Event bindings controlling the effects of pressing mouse buttons
   * or moving the mouse.
   */
  void OnMouseMove() override;
  void OnLeftButtonDown() override;
  void OnLeftButtonUp() override;
  void OnMiddleButtonDown() override;
  void OnMiddleButtonUp() override;
  void OnRightButtonDown() override;
  void OnRightButtonUp() override;
  //@}

  // These methods for the different interactions in different modes
  // are overridden in subclasses to perform the correct motion. Since
  // they might be called from OnTimer, they do not have mouse coord parameters
  // (use interactor's GetEventPosition and GetLastEventPosition)
  void Rotate() override;
  void Spin() override;
  void Pan() override;
  void Dolly() override;
  void UniformScale() override;

protected:
  ResliceCubicoInteractionStyle();
  ~ResliceCubicoInteractionStyle() override;

  void FindPickedActor(int x, int y);

  void Prop3DTransform(vtkProp3D *prop3D,
                       double *boxCenter,
                       int NumRotation,
                       double **rotate,
                       double *scale);

  double MotionFactor;

  vtkProp3D *InteractionProp;
  vtkCellPicker *InteractionPicker;

private:
  ResliceCubicoInteractionStyle(const ResliceCubicoInteractionStyle&) = delete;
  void operator=(const ResliceCubicoInteractionStyle&) = delete;
};

#endif
