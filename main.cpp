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

void GravaTesteComoPNG(vtkImageSlabReslice *reslicer){
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
	applyWl->SetWindow(2639);
	applyWl->SetLevel(1319);
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
}


int main(int argc, char** argv) {
	///Carga da imagem
	ObserveLoadProgressCommand::Pointer prog = ObserveLoadProgressCommand::New();
	const std::string txtFile = "C:\\meus dicoms\\Marching Man";
	const std::vector<std::string> lst = GetList(txtFile);
	std::map<std::string, std::string> metadataDaImagem;
	itk::Image<short, 3>::Pointer imagemOriginal = LoadVolume(metadataDaImagem, lst, prog);
	vtkSmartPointer<vtkImageImport> imagemImportadaPraVTK = CreateVTKImage(imagemOriginal);//importa a imagem da itk pra vtk.
	imagemImportadaPraVTK->Update();

	//Cria a tela do reslice cubico
	auto rendererCubeLayer = vtkSmartPointer<vtkOpenGLRenderer>::New();
	rendererCubeLayer->SetLayer(1);
	rendererCubeLayer->GetActiveCamera()->ParallelProjectionOn();
	rendererCubeLayer->SetBackground(1, 0, 0);
	auto rendererImageLayer = vtkSmartPointer<vtkOpenGLRenderer>::New();
	rendererImageLayer->SetLayer(0);
	rendererImageLayer->GetActiveCamera()->ParallelProjectionOn();
	auto renderWindowCubo = vtkSmartPointer<vtkWin32OpenGLRenderWindow>::New();
	renderWindowCubo->SetNumberOfLayers(2);
	renderWindowCubo->AddRenderer(rendererImageLayer);
	renderWindowCubo->AddRenderer(rendererCubeLayer);
	auto interactorCubo = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowCubo->SetInteractor(interactorCubo);
	auto interactorStyleCubo = vtkSmartPointer<ResliceCubicoInteractionStyle>::New();
	interactorCubo->SetInteractorStyle(interactorStyleCubo);
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
	//cubeActor->SetPosition(imagemImportadaPraVTK->GetOutput()->GetCenter());
	rendererCubeLayer->AddActor(cubeActor);
	rendererCubeLayer->ResetCamera();

	////A saída do reslice
	//auto imageActor = vtkSmartPointer<vtkImageActor>::New();
	//imageActor->GetProperty()->SetColorLevel(50);
	//imageActor->GetProperty()->SetColorWindow(350);
	//imageActor->SetPosition(cubeActor->GetCenter());
	//imageActor->PickableOff();
	//rendererImageLayer->AddActor(imageActor);
	//rendererImageLayer->ResetCamera();

	////O reslicer
	//auto reslicer = vtkSmartPointer<vtkImageSlabReslice>::New();
	//imageActor->GetMapper()->SetInputConnection(reslicer->GetOutputPort());
	//reslicer->SetInputConnection(imagemImportadaPraVTK->GetOutputPort());;
	//reslicer->AutoCropOutputOn();
	//reslicer->SetOutputDimensionality(2);
	//reslicer->Update();


	//vtkImageData *resultado = reslicer->GetOutput();
	//if (resultado->GetExtent()[1] == -1)
	//	throw "ta errado";
	//renderWindowCubo->Render();

	//Qual é a posição inicial do reslice? É o centro da imagem.
	std::array<double, 3> posicaoDoReslice = { { imagemImportadaPraVTK->GetOutput()->GetCenter()[0], imagemImportadaPraVTK->GetOutput()->GetCenter()[1], imagemImportadaPraVTK->GetOutput()->GetCenter()[2] } };


	interactorStyleCubo->SetCallbackPan([cubeActor, rendererCubeLayer](vtkCamera *cam, std::array<double, 3> motionVector){
		auto cubeMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
		cubeActor->GetMatrix(cubeMatrix);
		vtkCamera *cubeCam = rendererCubeLayer->GetActiveCamera();
		std::array<double, 3> oldPos; cubeCam->GetPosition(oldPos.data());
		std::array<double, 3> oldFocus; cubeCam->GetFocalPoint(oldFocus.data());
		oldPos = oldPos + motionVector;
		oldFocus = oldFocus + motionVector;
		cubeCam->SetPosition(oldPos.data());
		cubeCam->SetFocalPoint(oldFocus.data());

		cubeMatrix->Print(cout);
	});


	bool hasAlredySetCamera = false;
	//Quando o cubo gira o reslice deve ser refeito com a orientação e posição espacial do cubo.
	interactorStyleCubo->SetCallbackDeRotacao([cubeActor](vtkProp3D* cubo){
		auto cubeMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
		cubeActor->GetMatrix(cubeMatrix);
		cubeMatrix->Print(std::cout);
	});

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

