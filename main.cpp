#include "stdafx.h"
#include "loadVolume.h"
#include "utils.h"
#include "ResliceCubicoInteractionStyle.h"


class ObserveLoadProgressCommand : public itk::Command
{
public:
	itkSimpleNewMacro(ObserveLoadProgressCommand);
	void Execute(itk::Object * caller, const itk::EventObject & event)
	{
		Execute((const itk::Object *)caller, event);
	}

	void Execute(const itk::Object * caller, const itk::EventObject & event)
	{
		if (!itk::ProgressEvent().CheckEvent(&event))
		{
			return;
		}
		const itk::ProcessObject * processObject =
			dynamic_cast< const itk::ProcessObject * >(caller);
		if (!processObject)
		{
			return;
		}
		std::cout << processObject->GetProgress() << std::endl;
	}
};




int main(int argc, char** argv) {
	///Carga da imagem
	ObserveLoadProgressCommand::Pointer prog = ObserveLoadProgressCommand::New();
	const std::string txtFile = "c:\\meus dicoms\\abdomem-feet-first";
	const std::vector<std::string> lst = GetList(txtFile);
	std::map<std::string, std::string> metadataDaImagem;
	itk::Image<short, 3>::Pointer imagemOriginal = LoadVolume(metadataDaImagem, lst, prog);
	vtkSmartPointer<vtkImageImport> imagemImportadaPraVTK = CreateVTKImage(imagemOriginal);//importa a imagem da itk pra vtk.
	imagemImportadaPraVTK->Update();

	//Cria a tela do reslice cubico
	auto rendererCubo = vtkSmartPointer<vtkRenderer>::New();
	rendererCubo->GetActiveCamera()->ParallelProjectionOn();
	auto renderWindowCubo = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindowCubo->AddRenderer(rendererCubo);
	auto interactorCubo = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowCubo->SetInteractor(interactorCubo);
	auto interactorStyleCubo = vtkSmartPointer<ResliceCubicoInteractionStyle>::New();
	interactorCubo->SetInteractorStyle(interactorStyleCubo);
	rendererCubo->SetBackground(0.3, 0, 0);
	renderWindowCubo->Render();


	//Cria o cubo
	auto cubeSource = vtkSmartPointer<vtkCubeSource>::New();
	cubeSource->SetXLength(100);
	cubeSource->SetYLength(100);
	cubeSource->SetZLength(100);
	auto cubeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	cubeMapper->SetInputConnection(cubeSource->GetOutputPort());
	auto cubeActor = vtkSmartPointer<vtkActor>::New();
	cubeActor->SetMapper(cubeMapper);
	cubeActor->GetProperty()->SetRepresentationToWireframe();
	cubeActor->GetProperty()->ShadingOff();
	cubeActor->GetProperty()->SetColor(0, 1, 0);
	cubeActor->GetProperty()->LightingOff();
	cubeActor->SetPosition(imagemImportadaPraVTK->GetOutput()->GetCenter());
	rendererCubo->AddActor(cubeActor);
	rendererCubo->ResetCamera();
	rendererCubo->GetActiveCamera()->Zoom(0.5);

	//A saída do reslice
	auto imageActor = vtkSmartPointer<vtkImageActor>::New();
	imageActor->GetProperty()->SetColorLevel(50);
	imageActor->GetProperty()->SetColorWindow(350);
	imageActor->SetPosition(cubeActor->GetCenter());
	imageActor->PickableOff();
	rendererCubo->AddActor(imageActor);

	//O reslicer
	auto reslicer = vtkSmartPointer<vtkImageSlabReslice>::New();
	imageActor->GetMapper()->SetInputConnection(reslicer->GetOutputPort());
	reslicer->SetInputConnection(imagemImportadaPraVTK->GetOutputPort());;
	reslicer->AutoCropOutputOn();
	reslicer->SetOutputDimensionality(2);
	reslicer->Update();


	vtkImageData *resultado = reslicer->GetOutput();
	if (resultado->GetExtent()[1] == -1)
		throw "ta errado";

	//pega o output e grava
	boost::posix_time::ptime current_date_microseconds = boost::posix_time::microsec_clock::local_time();
	long milliseconds = current_date_microseconds.time_of_day().total_milliseconds();
	std::string filename = "C:\\reslice_cubico\\mk6\\dump\\" + boost::lexical_cast<std::string>(milliseconds)+".vti";
	vtkSmartPointer<vtkXMLImageDataWriter> debugsave = vtkSmartPointer<vtkXMLImageDataWriter>::New();
	debugsave->SetFileName(filename.c_str());
	debugsave->SetInputConnection(reslicer->GetOutputPort());
	debugsave->BreakOnError();
	debugsave->Write();

	//pega o output e exibe
	//Ajeita a camera pra ela apontar pro centro do cubo
	vtkCamera *cam = rendererCubo->GetActiveCamera();
	double directionOfProjection[3];
	cam->GetDirectionOfProjection(directionOfProjection);
	double dist = cam->GetDistance();
	cam->SetFocalPoint(cubeActor->GetCenter());
	directionOfProjection[0] = cam->GetFocalPoint()[0] + -dist *directionOfProjection[0];
	directionOfProjection[1] = cam->GetFocalPoint()[1] + -dist *directionOfProjection[1];
	directionOfProjection[2] = cam->GetFocalPoint()[2] + -dist *directionOfProjection[2];
	cam->SetPosition(directionOfProjection);

	renderWindowCubo->Render();
	std::array<double, 3> resliceCenter = { { cubeActor->GetCenter()[0], cubeActor->GetCenter()[1], cubeActor->GetCenter()[2] } };//{ { -78, -55, -245 } };
	////O callback do pan
	//interactorStyleCubo->SetCallbackPan([rendererCubo](vtkProp3D *cubo, std::array<double,3> mv){
	//	vtkCamera *cam = rendererCubo->GetActiveCamera();
	//	std::array<double, 3> pos;
	//	cam->GetPosition(pos.data());
	//	std::array<double, 3>focus;
	//	cam->GetFocalPoint(focus.data());
	//	pos = pos + mv;
	//	focus = focus + mv;
	//	cam->SetPosition(pos.data());
	//	cam->SetFocalPoint(focus.data());
	//});
	interactorStyleCubo->SetCallbackPan([cubeActor](vtkCamera *cam, std::array<double, 3> motionVector){
		//Centro do cubo = focus
		cout << "  pos anterior do cubo = " << cubeActor->GetPosition()[0] << ", " << cubeActor->GetPosition()[1] << ", " << cubeActor->GetPosition()[2] << std::endl;
		cubeActor->SetPosition(cam->GetFocalPoint());
		cout << "  pos atual do cubo = " << cubeActor->GetPosition()[0] << ", " << cubeActor->GetPosition()[1] << ", " << cubeActor->GetPosition()[2] << std::endl;
	});

	//Quando o cubo gira o reslice deve ser refeito com a orientação e posição espacial do cubo.
	interactorStyleCubo->SetCallbackDeRotacao([reslicer, renderWindowCubo, resliceCenter, imageActor](vtkProp3D* cubo){
		std::cout << "current cube pos " << cubo->GetCenter()[0] << ","
			<< cubo->GetCenter()[1] << ","
			<< cubo->GetCenter()[2] << std::endl;


		auto resliceTransform = vtkSmartPointer<vtkTransform>::New();
		resliceTransform->RotateWXYZ(cubo->GetOrientationWXYZ()[0], cubo->GetOrientationWXYZ()[1], cubo->GetOrientationWXYZ()[2], cubo->GetOrientationWXYZ()[3] );
		resliceTransform->Update();

		imageActor->SetPosition(cubo->GetCenter());
		vtkMatrix4x4 *mat = resliceTransform->GetMatrix();
		reslicer->SetResliceAxesDirectionCosines(mat->Element[0][0], mat->Element[1][0], mat->Element[2][0],
			mat->Element[0][1], mat->Element[1][1], mat->Element[2][1],
			mat->Element[0][2], mat->Element[1][2], mat->Element[2][2]
			);
		reslicer->SetResliceAxesOrigin(cubo->GetCenter());
		//reslicer->SetResliceTransform(resliceTransform);
		reslicer->Update();

		//Solução do amassado
		double spacingX = reslicer->GetOutput()->GetSpacing()[0];
		double spacingY = reslicer->GetOutput()->GetSpacing()[1];
		double ratioX = spacingX / spacingY;
		double ratioY = spacingY / spacingX;
		auto scaler = vtkSmartPointer<vtkImageResample>::New();
		if (spacingX > spacingY){
			scaler->SetAxisMagnificationFactor(0, ratioX);
			scaler->SetAxisMagnificationFactor(1, 1);
		}
		else if (spacingX < spacingY){
			scaler->SetAxisMagnificationFactor(0, 1);
			scaler->SetAxisMagnificationFactor(1, ratioY);
		}
		else{
			scaler->SetAxisMagnificationFactor(0, 1);
			scaler->SetAxisMagnificationFactor(1, 1);
		}
		scaler->SetInputConnection(reslicer->GetOutputPort());
		scaler->Update();
		//Pra gravação em disco - aplica window
		auto applyWl = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
		applyWl->SetWindow(350);
		applyWl->SetLevel(50);
		applyWl->SetOutputFormatToRGBA();
		applyWl->SetInputConnection(scaler->GetOutputPort());
		//Grava o png
		auto pngWriter = vtkSmartPointer<vtkPNGWriter>::New();
		pngWriter->SetInputConnection(applyWl->GetOutputPort());
		boost::posix_time::ptime current_date_microseconds = boost::posix_time::microsec_clock::local_time();
		long milliseconds = current_date_microseconds.time_of_day().total_milliseconds();
		std::string filename = "C:\\reslice_cubico\\mk6\\dump\\" + boost::lexical_cast<std::string>(milliseconds)+".png";
		pngWriter->SetFileName(filename.c_str());
		pngWriter->Write();
		renderWindowCubo->Render();	});

	///////////////////////////////////////////////////
	//A tela dummy PROS PROBLEMAS DO OPENGL
	vtkSmartPointer<vtkRenderer> rendererDummy = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindowDummy = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindowDummy->AddRenderer(rendererDummy);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractorDummy = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowDummy->SetInteractor(renderWindowInteractorDummy);
	renderWindowDummy->Render();
	renderWindowDummy->MakeCurrent();
	renderWindowInteractorDummy->Start();

	return EXIT_SUCCESS;
}

