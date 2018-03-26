#include "stdafx.h"
#include "ResliceCubicoInteractionStyle.h"

vtkStandardNewMacro(ResliceCubicoInteractionStyle);

//----------------------------------------------------------------------------
ResliceCubicoInteractionStyle::ResliceCubicoInteractionStyle()
{
  this->MotionFactor    = 10.0;
  this->InteractionProp = nullptr;
  this->InteractionPicker = vtkCellPicker::New();
  this->InteractionPicker->SetTolerance(0.001);
  callbackRotacao = nullptr;
}

//----------------------------------------------------------------------------
ResliceCubicoInteractionStyle::~ResliceCubicoInteractionStyle()
{
  this->InteractionPicker->Delete();
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::OnMouseMove()
{
  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];

  switch (this->State)
  {
    case VTKIS_ROTATE:
      this->FindPokedRenderer(x, y);
      this->Rotate();
      this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
      break;

    case VTKIS_PAN:
      this->FindPokedRenderer(x, y);
      this->Pan();
      this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
      break;

    case VTKIS_DOLLY:
      this->FindPokedRenderer(x, y);
      this->Dolly();
      this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
      break;

    case VTKIS_SPIN:
      this->FindPokedRenderer(x, y);
      this->Spin();
      this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
      break;

    case VTKIS_USCALE:
      this->FindPokedRenderer(x, y);
      this->UniformScale();
      this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
      break;
  }
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::OnLeftButtonDown()
{
  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];

  this->FindPokedRenderer(x, y);
  this->FindPickedActor(x, y);
  if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
  {
    return;
  }

  this->GrabFocus(this->EventCallbackCommand);
  if (this->Interactor->GetShiftKey())
  {
    this->StartPan();
  }
  else if (this->Interactor->GetControlKey())
  {
    this->StartSpin();
  }
  else
  {
    this->StartRotate();
  }
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::OnLeftButtonUp()
{
  switch (this->State)
  {
    case VTKIS_PAN:
      this->EndPan();
      break;

    case VTKIS_SPIN:
      this->EndSpin();
      break;

    case VTKIS_ROTATE:
      this->EndRotate();
      break;
  }

  if ( this->Interactor )
  {
    this->ReleaseFocus();
  }
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::OnMiddleButtonDown()
{
  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];

  this->FindPokedRenderer(x, y);
  this->FindPickedActor(x, y);
  if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
  {
    return;
  }

  this->GrabFocus(this->EventCallbackCommand);
  if (this->Interactor->GetControlKey())
  {
    this->StartDolly();
  }
  else
  {
    this->StartPan();
  }
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::OnMiddleButtonUp()
{
  switch (this->State)
  {
    case VTKIS_DOLLY:
      this->EndDolly();
      break;

    case VTKIS_PAN:
      this->EndPan();
      break;
  }

  if ( this->Interactor )
  {
    this->ReleaseFocus();
  }
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::OnRightButtonDown()
{
  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];

  this->FindPokedRenderer(x, y);
  this->FindPickedActor(x, y);
  if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
  {
    return;
  }

  this->GrabFocus(this->EventCallbackCommand);
  this->StartUniformScale();
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::OnRightButtonUp()
{
  switch (this->State)
  {
    case VTKIS_USCALE:
      this->EndUniformScale();
      break;
  }

  if ( this->Interactor )
  {
    this->ReleaseFocus();
  }
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::Rotate()
{
  if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
  {
    return;
  }

  vtkRenderWindowInteractor *rwi = this->Interactor;
  vtkCamera *cam = this->CurrentRenderer->GetActiveCamera();

  // First get the origin of the assembly
  double *obj_center = this->InteractionProp->GetCenter();

  // GetLength gets the length of the diagonal of the bounding box
  double boundRadius = this->InteractionProp->GetLength() * 0.5;

  // Get the view up and view right vectors
  double view_up[3], view_look[3], view_right[3];

  cam->OrthogonalizeViewUp();
  cam->ComputeViewPlaneNormal();
  cam->GetViewUp(view_up);
  vtkMath::Normalize(view_up);
  cam->GetViewPlaneNormal(view_look);
  vtkMath::Cross(view_up, view_look, view_right);
  vtkMath::Normalize(view_right);

  // Get the furtherest point from object position+origin
  double outsidept[3];

  outsidept[0] = obj_center[0] + view_right[0] * boundRadius;
  outsidept[1] = obj_center[1] + view_right[1] * boundRadius;
  outsidept[2] = obj_center[2] + view_right[2] * boundRadius;

  // Convert them to display coord
  double disp_obj_center[3];

  this->ComputeWorldToDisplay(obj_center[0], obj_center[1], obj_center[2],
                              disp_obj_center);

  this->ComputeWorldToDisplay(outsidept[0], outsidept[1], outsidept[2],
                              outsidept);

  double radius = sqrt(vtkMath::Distance2BetweenPoints(disp_obj_center,
                                                       outsidept));
  double nxf = (rwi->GetEventPosition()[0] - disp_obj_center[0]) / radius;

  double nyf = (rwi->GetEventPosition()[1] - disp_obj_center[1]) / radius;

  double oxf = (rwi->GetLastEventPosition()[0] - disp_obj_center[0]) / radius;

  double oyf = (rwi->GetLastEventPosition()[1] - disp_obj_center[1]) / radius;

  if (((nxf * nxf + nyf * nyf) <= 1.0) &&
      ((oxf * oxf + oyf * oyf) <= 1.0))
  {
    double newXAngle = vtkMath::DegreesFromRadians( asin( nxf ) );
    double newYAngle = vtkMath::DegreesFromRadians( asin( nyf ) );
    double oldXAngle = vtkMath::DegreesFromRadians( asin( oxf ) );
    double oldYAngle = vtkMath::DegreesFromRadians( asin( oyf ) );

	std::cout << __FUNCTION__ << " old x angle = " << oldXAngle << std::endl;
	std::cout << __FUNCTION__ << " old y angle = " << oldYAngle << std::endl;

    double scale[3];
    scale[0] = scale[1] = scale[2] = 1.0;

    double **rotate = new double*[2];

    rotate[0] = new double[4];
    rotate[1] = new double[4];

    rotate[0][0] = newXAngle - oldXAngle;
    rotate[0][1] = view_up[0];
    rotate[0][2] = view_up[1];
    rotate[0][3] = view_up[2];

    rotate[1][0] = oldYAngle - newYAngle;
    rotate[1][1] = view_right[0];
    rotate[1][2] = view_right[1];
    rotate[1][3] = view_right[2];


    this->Prop3DTransform(this->InteractionProp,
                          obj_center,
                          2,
                          rotate,
                          scale);

    delete [] rotate[0];
    delete [] rotate[1];
    delete [] rotate;

    if (this->AutoAdjustCameraClippingRange)
    {
      this->CurrentRenderer->ResetCameraClippingRange();
    }
	if (callbackRotacao)
		callbackRotacao(this->InteractionProp);
    rwi->Render();
  }
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::Spin()
{
  if ( this->CurrentRenderer == nullptr || this->InteractionProp == nullptr )
  {
    return;
  }

  vtkRenderWindowInteractor *rwi = this->Interactor;
  vtkCamera *cam = this->CurrentRenderer->GetActiveCamera();

  // Get the axis to rotate around = vector from eye to origin

  double *obj_center = this->InteractionProp->GetCenter();

  double motion_vector[3];
  double view_point[3];

  if (cam->GetParallelProjection())
  {
    // If parallel projection, want to get the view plane normal...
    cam->ComputeViewPlaneNormal();
    cam->GetViewPlaneNormal( motion_vector );
  }
  else
  {
    // Perspective projection, get vector from eye to center of actor
    cam->GetPosition( view_point );
    motion_vector[0] = view_point[0] - obj_center[0];
    motion_vector[1] = view_point[1] - obj_center[1];
    motion_vector[2] = view_point[2] - obj_center[2];
    vtkMath::Normalize(motion_vector);
  }

  double disp_obj_center[3];

  this->ComputeWorldToDisplay(obj_center[0], obj_center[1], obj_center[2],
                              disp_obj_center);

  double newAngle =
    vtkMath::DegreesFromRadians( atan2( rwi->GetEventPosition()[1] - disp_obj_center[1],
                                        rwi->GetEventPosition()[0] - disp_obj_center[0] ) );

  double oldAngle =
    vtkMath::DegreesFromRadians( atan2( rwi->GetLastEventPosition()[1] - disp_obj_center[1],
                                        rwi->GetLastEventPosition()[0] - disp_obj_center[0] ) );

  double scale[3];
  scale[0] = scale[1] = scale[2] = 1.0;

  double **rotate = new double*[1];
  rotate[0] = new double[4];

  rotate[0][0] = newAngle - oldAngle;
  rotate[0][1] = motion_vector[0];
  rotate[0][2] = motion_vector[1];
  rotate[0][3] = motion_vector[2];

  this->Prop3DTransform( this->InteractionProp,
                         obj_center,
                         1,
                         rotate,
                         scale );

  delete [] rotate[0];
  delete [] rotate;

  if ( this->AutoAdjustCameraClippingRange )
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }
  if (callbackRotacao)
	  callbackRotacao(this->InteractionProp);
  rwi->Render();
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::Pan()
{
	//if (this->CurrentRenderer == nullptr){
	//	return;
	//}
	//vtkRenderWindowInteractor *rwi = this->Interactor;
	//double viewFocus[4], focalDepth, viewPoint[3];
	//double newPickPoint[4], oldPickPoint[4], motionVector[3];
	////calculo do focal depth
	//vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
	//camera->GetFocalPoint(viewFocus);
	//this->ComputeWorldToDisplay(viewFocus[0], viewFocus[1], viewFocus[2], viewFocus);
	//focalDepth = viewFocus[2];
	//this->ComputeDisplayToWorld(rwi->GetEventPosition()[0], rwi->GetEventPosition()[1], focalDepth, newPickPoint);
	////Recalcula o mouse antigo
	//this->ComputeDisplayToWorld(rwi->GetLastEventPosition()[0], rwi->GetLastEventPosition()[1], focalDepth, oldPickPoint);
	//motionVector[0] = oldPickPoint[0] - newPickPoint[0];
	//motionVector[1] = oldPickPoint[1] - newPickPoint[1];
	//motionVector[2] = oldPickPoint[2] - newPickPoint[2];
	//camera->GetFocalPoint(viewFocus);
	//camera->GetPosition(viewPoint);
	//camera->SetFocalPoint(motionVector[0] + viewFocus[0], motionVector[1] + viewFocus[1], motionVector[2] + viewFocus[2]);
	//camera->SetPosition(motionVector[0] + viewPoint[0], motionVector[1] + viewPoint[1], motionVector[2] + viewPoint[2]);
	//if (rwi->GetLightFollowCamera()){
	//	this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
	//}
	//std::array<double, 3> _mv = { { motionVector[0], motionVector[1], motionVector[2] } };
	//callbackPan(camera, _mv);
	//rwi->Render();

///Nessa versão o cubo não anda no z, quebrando o translante no caso da sagital
  if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
  {
    return;
  }
  vtkRenderWindowInteractor *rwi = this->Interactor;
  // Use initial center as the origin from which to pan
  double *obj_center = this->InteractionProp->GetCenter();
  double disp_obj_center[3], new_pick_point[4];
  double old_pick_point[4], motion_vector[3];
  this->ComputeWorldToDisplay(obj_center[0], obj_center[1], obj_center[2],
                              disp_obj_center);
  this->ComputeDisplayToWorld(rwi->GetEventPosition()[0],
                              rwi->GetEventPosition()[1],
                              disp_obj_center[2],
                              new_pick_point);
  this->ComputeDisplayToWorld(rwi->GetLastEventPosition()[0],
                              rwi->GetLastEventPosition()[1],
                              disp_obj_center[2],
                              old_pick_point);
  motion_vector[0] = new_pick_point[0] - old_pick_point[0];
  motion_vector[1] = new_pick_point[1] - old_pick_point[1];
  motion_vector[2] = new_pick_point[2] - old_pick_point[2];



  if (this->InteractionProp->GetUserMatrix() != nullptr)
  {
    vtkTransform *t = vtkTransform::New();
    t->PostMultiply();
    t->SetMatrix(this->InteractionProp->GetUserMatrix());
    //t->Translate(motion_vector[0], motion_vector[1], motion_vector[2]);
	t->Translate(motion_vector[0], motion_vector[1], motion_vector[2]);
    this->InteractionProp->GetUserMatrix()->DeepCopy(t->GetMatrix());
    t->Delete();
  }
  else
  {
    this->InteractionProp->AddPosition(motion_vector[0],
                                       motion_vector[1],
                                       motion_vector[2]);

  }
  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }
  std::array<double, 3> _mv = { { motion_vector[0], motion_vector[1], motion_vector[2] } };
  callbackPan(this->CurrentRenderer->GetActiveCamera(), _mv);
  rwi->Render();
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::Dolly()
{
  if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
  {
    return;
  }

  vtkRenderWindowInteractor *rwi = this->Interactor;
  vtkCamera *cam = this->CurrentRenderer->GetActiveCamera();

  double view_point[3], view_focus[3];
  double motion_vector[3];

  cam->GetPosition(view_point);
  cam->GetFocalPoint(view_focus);

  double *center = this->CurrentRenderer->GetCenter();

  int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];
  double yf = dy / center[1] * this->MotionFactor;
  double dollyFactor = pow(1.1, yf);

  dollyFactor -= 1.0;
  motion_vector[0] = (view_point[0] - view_focus[0]) * dollyFactor;
  motion_vector[1] = (view_point[1] - view_focus[1]) * dollyFactor;
  motion_vector[2] = (view_point[2] - view_focus[2]) * dollyFactor;

  if (this->InteractionProp->GetUserMatrix() != nullptr)
  {
    vtkTransform *t = vtkTransform::New();
    t->PostMultiply();
    t->SetMatrix(this->InteractionProp->GetUserMatrix());
    t->Translate(motion_vector[0], motion_vector[1],
                 motion_vector[2]);
    this->InteractionProp->GetUserMatrix()->DeepCopy(t->GetMatrix());
    t->Delete();
  }
  else
  {
    this->InteractionProp->AddPosition(motion_vector);
  }

  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }

  rwi->Render();
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::UniformScale()
{
  if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
  {
    return;
  }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];

  double *obj_center = this->InteractionProp->GetCenter();
  double *center = this->CurrentRenderer->GetCenter();

  double yf = dy / center[1] * this->MotionFactor;
  double scaleFactor = pow(1.1, yf);

  double **rotate = nullptr;

  double scale[3];
  scale[0] = scale[1] = scale[2] = scaleFactor;

  this->Prop3DTransform(this->InteractionProp,
                        obj_center,
                        0,
                        rotate,
                        scale);

  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }

  rwi->Render();
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::FindPickedActor(int x, int y)
{
  this->InteractionPicker->Pick(x, y, 0.0, this->CurrentRenderer);
  vtkProp *prop = this->InteractionPicker->GetViewProp();
  if (prop != nullptr)
  {
    this->InteractionProp = vtkProp3D::SafeDownCast(prop);
  }
  else
  {
    this->InteractionProp = nullptr;
  }
}

//----------------------------------------------------------------------------
void ResliceCubicoInteractionStyle::Prop3DTransform(vtkProp3D *prop3D,
                                                       double *boxCenter,
                                                       int numRotation,
                                                       double **rotate,
                                                       double *scale)
{
  vtkMatrix4x4 *oldMatrix = vtkMatrix4x4::New();
  prop3D->GetMatrix(oldMatrix);

  double orig[3];
  prop3D->GetOrigin(orig);

  vtkTransform *newTransform = vtkTransform::New();
  newTransform->PostMultiply();
  if (prop3D->GetUserMatrix() != nullptr)
  {
    newTransform->SetMatrix(prop3D->GetUserMatrix());
  }
  else
  {
    newTransform->SetMatrix(oldMatrix);
  }

  newTransform->Translate(-(boxCenter[0]), -(boxCenter[1]), -(boxCenter[2]));

  for (int i = 0; i < numRotation; i++)
  {
    newTransform->RotateWXYZ(rotate[i][0], rotate[i][1],
                             rotate[i][2], rotate[i][3]);
  }

  if ((scale[0] * scale[1] * scale[2]) != 0.0)
  {
    newTransform->Scale(scale[0], scale[1], scale[2]);
  }

  newTransform->Translate(boxCenter[0], boxCenter[1], boxCenter[2]);

  // now try to get the composit of translate, rotate, and scale
  newTransform->Translate(-(orig[0]), -(orig[1]), -(orig[2]));
  newTransform->PreMultiply();
  newTransform->Translate(orig[0], orig[1], orig[2]);

  if (prop3D->GetUserMatrix() != nullptr)
  {
    newTransform->GetMatrix(prop3D->GetUserMatrix());
  }
  else
  {
    prop3D->SetPosition(newTransform->GetPosition());
    prop3D->SetScale(newTransform->GetScale());
    prop3D->SetOrientation(newTransform->GetOrientation());
  }
  oldMatrix->Delete();
  newTransform->Delete();
}

