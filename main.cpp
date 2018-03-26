#include "stdafx.h"
#include "loadVolume.h"
#include "utils.h"
#include "ResliceCubicoInteractionStyle.h"
#include "RenderLayer.h"

double window = 0; double level = 0;

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
	///Marching man
	applyWl->SetWindow(window);
	applyWl->SetLevel(level);

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
	const std::string txtFile = argv[1];// "C:\\meus dicoms\\Marching Man";
	window = boost::lexical_cast<double>(argv[2]);
	level = boost::lexical_cast<double>(argv[3]);

	const std::vector<std::string> lst = GetList(txtFile);
	std::map<std::string, std::string> metadataDaImagem;
	itk::Image<short, 3>::Pointer imagemOriginal = LoadVolume(metadataDaImagem, lst, prog);
	vtkSmartPointer<vtkImageImport> imagemImportadaPraVTK = CreateVTKImage(imagemOriginal);//importa a imagem da itk pra vtk.
	imagemImportadaPraVTK->Update();

	//Cria as duas camadas
	std::shared_ptr<RenderLayer> cubeLayer, imageLayer;
	cubeLayer = std::make_shared<RenderLayer>(1);
	imageLayer = std::make_shared<RenderLayer>(0);
	auto renderWindowCubo = vtkSmartPointer<vtkWin32OpenGLRenderWindow>::New();
	renderWindowCubo->SetNumberOfLayers(2);
	renderWindowCubo->AddRenderer(imageLayer->GetRenderer());
	renderWindowCubo->AddRenderer(cubeLayer->GetRenderer());
	auto interactorCubo = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowCubo->SetInteractor(interactorCubo);
	auto interactorStyleCubo = vtkSmartPointer<ResliceCubicoInteractionStyle>::New();
	interactorCubo->SetInteractorStyle(interactorStyleCubo);
	renderWindowCubo->Render();


	//Cria o cubo
	auto cubeSource = vtkSmartPointer<vtkCubeSource>::New();
	cubeSource->SetXLength(50);
	cubeSource->SetYLength(50);
	cubeSource->SetZLength(50);
	auto cubeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	cubeMapper->SetInputConnection(cubeSource->GetOutputPort());
	auto cubeActor = vtkSmartPointer<vtkActor>::New();
	cubeActor->SetMapper(cubeMapper);
	cubeActor->GetProperty()->SetRepresentationToWireframe();
	cubeActor->GetProperty()->ShadingOff();
	cubeActor->GetProperty()->SetColor(0, 1, 0);
	cubeActor->GetProperty()->LightingOff();
	//cubeActor->SetPosition(imagemImportadaPraVTK->GetOutput()->GetCenter());
	cubeLayer->GetRenderer()->AddActor(cubeActor);
	cubeLayer->GetRenderer()->ResetCamera();
	cubeLayer->GetRenderer()->GetActiveCamera()->Zoom(0.5);

	////A saída do reslice
	auto imageActor = vtkSmartPointer<vtkImageActor>::New();
	///Marching man
	imageActor->GetProperty()->SetColorLevel(level);
	imageActor->GetProperty()->SetColorWindow(window);

	imageActor->SetPosition(cubeActor->GetCenter());
	imageActor->PickableOff();
	imageLayer->GetRenderer()->AddActor(imageActor);
	imageLayer->GetRenderer()->ResetCamera();

	////O reslicer
	auto reslicer = vtkSmartPointer<vtkImageSlabReslice>::New();
	imageActor->GetMapper()->SetInputConnection(reslicer->GetOutputPort());
	reslicer->SetInputConnection(imagemImportadaPraVTK->GetOutputPort());;
	reslicer->AutoCropOutputOn();
	reslicer->SetOutputDimensionality(2);
	reslicer->Update();


	//vtkImageData *resultado = reslicer->GetOutput();
	//if (resultado->GetExtent()[1] == -1)
	//	throw "ta errado";
	//renderWindowCubo->Render();

	//Qual é a posição inicial do reslice? É o centro da imagem.
	bool hasAlredySetCamera = false;
	std::array<double, 3> posicaoDoReslice = { { imagemImportadaPraVTK->GetOutput()->GetCenter()[0], imagemImportadaPraVTK->GetOutput()->GetCenter()[1], imagemImportadaPraVTK->GetOutput()->GetCenter()[2] } };

	interactorStyleCubo->SetCallbackDolly([reslicer, imageActor, &posicaoDoReslice, &hasAlredySetCamera, imageLayer, cubeActor](double dollyFactor){
		//image actor sempre no centro da tela
		std::array<double, 3> zero = { { 0, 0, 0 } };
		imageActor->SetPosition(zero.data());
		//pega o z da matriz de reslice. O z é a direção do dolly.
		std::array<double, 3> xVector, yVector, zVector;
		reslicer->GetResliceAxesDirectionCosines(xVector.data(), yVector.data(), zVector.data());
		//vê o deslocamento a ser aplicado à posição do reslice
		std::array<double, 3> dZ = zVector *dollyFactor;
		//soma dZ à posição do reslice
		posicaoDoReslice = posicaoDoReslice + dZ;
		//atualiza a tela
		auto cubeMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
		cubeActor->GetMatrix(cubeMatrix);

		cubeMatrix->Invert();
		cubeMatrix->Element[0][3] = posicaoDoReslice[0];
		cubeMatrix->Element[1][3] = posicaoDoReslice[1];
		cubeMatrix->Element[2][3] = posicaoDoReslice[2];

		reslicer->SetResliceAxesDirectionCosines(cubeMatrix->Element[0][0], cubeMatrix->Element[1][0], cubeMatrix->Element[2][0],
			cubeMatrix->Element[0][1], cubeMatrix->Element[1][1], cubeMatrix->Element[2][1],
			cubeMatrix->Element[0][2], cubeMatrix->Element[1][2], cubeMatrix->Element[2][2]);
		reslicer->SetResliceAxesOrigin(posicaoDoReslice.data());

		reslicer->Update();
		if (!hasAlredySetCamera){
			hasAlredySetCamera = true;
			imageLayer->GetRenderer()->ResetCamera();
			imageLayer->GetRenderer()->GetActiveCamera()->Zoom(2.0);
		}
		imageLayer->GetRenderer()->GetRenderWindow()->Render();
		cubeMatrix->Print(std::cout);
		GravaTesteComoPNG(reslicer);

	});

	interactorStyleCubo->SetCallbackDeZoom([imageLayer](double scaleFactor){
		auto cam = imageLayer->GetRenderer()->GetActiveCamera();
		std::cout << "  paralel scale = " << cam->GetParallelScale() << std::endl;
		cam->Zoom(scaleFactor);
	});

	interactorStyleCubo->SetCallbackPan([&posicaoDoReslice,cubeActor, cubeLayer, imageActor](vtkCamera *cam, std::array<double, 3> motionVector){
		///A câmera tem que acompanhar a posição do cubo.
		auto cubeMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
		cubeActor->GetMatrix(cubeMatrix);
		vtkCamera *cubeCam = cubeLayer->GetRenderer()->GetActiveCamera();
		std::array<double, 3> oldPos; cubeCam->GetPosition(oldPos.data());
		std::array<double, 3> oldFocus; cubeCam->GetFocalPoint(oldFocus.data());
		oldPos = oldPos + motionVector;
		oldFocus = oldFocus + motionVector;
		cubeCam->SetPosition(oldPos.data());
		cubeCam->SetFocalPoint(oldFocus.data());
		//Transforma o motion vector pela orientação
		cubeMatrix->Invert();
		cubeMatrix->Element[0][3] = 0;//Pra garantir que a transformação de orientação esteja em 0,0,0, que é onde o mv tb estará
		cubeMatrix->Element[1][3] = 0;
		cubeMatrix->Element[2][3] = 0;
		double __mv[4] = { motionVector[0], motionVector[1], motionVector[2], 1 };
		double* transMv = cubeMatrix->MultiplyDoublePoint(__mv); std::array<double, 3> motionVectorInResliceSpace = { { transMv[0], transMv[1], transMv[2], } };
		posicaoDoReslice = posicaoDoReslice + motionVectorInResliceSpace;
		//Move a imagem usando o motion vector
		std::array<double, 3> antiMV = motionVector * -1.0;
		std::array<double, 3> imagePos; imageActor->GetPosition(imagePos.data());
		imagePos = imagePos + antiMV;
		imageActor->SetPosition(imagePos.data());
		//cubeMatrix->Print(cout);
	});



	//Quando o cubo gira o reslice deve ser refeito com a orientação e posição espacial do cubo.
	interactorStyleCubo->SetCallbackDeRotacao([&hasAlredySetCamera, imageLayer, &posicaoDoReslice, cubeActor, reslicer, imageActor](vtkProp3D* cubo){
		//image actor sempre no centro da tela
		std::array<double, 3> zero = { { 0, 0, 0 } };
		imageActor->SetPosition(zero.data());

		auto cubeMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
		cubeActor->GetMatrix(cubeMatrix);

		cubeMatrix->Invert();
		cubeMatrix->Element[0][3] = posicaoDoReslice[0];
		cubeMatrix->Element[1][3] = posicaoDoReslice[1];
		cubeMatrix->Element[2][3] = posicaoDoReslice[2];

		reslicer->SetResliceAxesDirectionCosines(cubeMatrix->Element[0][0], cubeMatrix->Element[1][0], cubeMatrix->Element[2][0],
			cubeMatrix->Element[0][1], cubeMatrix->Element[1][1], cubeMatrix->Element[2][1],
			cubeMatrix->Element[0][2], cubeMatrix->Element[1][2], cubeMatrix->Element[2][2]);
		reslicer->SetResliceAxesOrigin(posicaoDoReslice.data());

		reslicer->Update();
		if (!hasAlredySetCamera){
			hasAlredySetCamera = true;
			imageLayer->GetRenderer()->ResetCamera();
			imageLayer->GetRenderer()->GetActiveCamera()->Zoom(2.0);
		}
		imageLayer->GetRenderer()->GetRenderWindow()->Render();

		cubeMatrix->Print(std::cout);
		GravaTesteComoPNG(reslicer);
	});

	///////////////////////////////////////////////////
	//A tela dummy PROS PROBLEMAS DO OPENGL
	vtkSmartPointer<vtkRenderer> rendererDummy = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindowDummy = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindowDummy->AddRenderer(rendererDummy);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractorDummy = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowDummy->SetInteractor(renderWindowInteractorDummy);
	renderWindowDummy->MakeCurrent();
	renderWindowDummy->Render();
	renderWindowDummy->Render();
	renderWindowDummy->Render();
	renderWindowDummy->Render();
	renderWindowDummy->Render();
	renderWindowDummy->Render();
	renderWindowInteractorDummy->Start();

	return EXIT_SUCCESS;
}

