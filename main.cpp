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
	const std::string txtFile = "c:\\meus dicoms\\marching man";
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
	reslicer->AutoCropOutputOn();//Não sei se é on ou off o certo, mas sei que isso aqui controla se vai cortar a imagem ou se vai pegar ela toda.
	auto resliceTransform = vtkSmartPointer<vtkTransform>::New();
	resliceTransform->Translate(imagemImportadaPraVTK->GetOutput()->GetCenter());
	reslicer->SetResliceTransform(resliceTransform);
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