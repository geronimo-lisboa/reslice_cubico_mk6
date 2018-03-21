#include "stdafx.h"
#include "loadVolume.h"

const std::vector<std::string> GetList(std::string path)
{
	typedef itk::Image<short, 3> ImageType;
	typedef itk::ImageSeriesReader<ImageType> ReaderType;
	typedef itk::GDCMImageIO ImageIOType;
	typedef itk::GDCMSeriesFileNames NamesGeneratorType;
	typedef std::vector< std::string >    SeriesIdContainer;
	typedef std::vector< std::string > FileNamesContainer;
	ReaderType::Pointer reader = ReaderType::New();
	ImageIOType::Pointer imageIOObject = ImageIOType::New();
	reader->SetImageIO(imageIOObject);
	NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
	nameGenerator->SetUseSeriesDetails(true);
	nameGenerator->SetDirectory(path);
	//This list has all the images in the directory
	SeriesIdContainer seriesUID = nameGenerator->GetSeriesUIDs();
	//The files that contains each of the individual slices that define an image.
	FileNamesContainer fileNames = nameGenerator->GetFileNames(seriesUID[0]);
	return fileNames;
}

itk::Image<short, 3>::Pointer LoadVolume(std::map<std::string, std::string> &outputMetadata,
	const std::vector<std::string> filepaths, itk::Command::Pointer cbk = nullptr)
{
	typedef itk::MetaDataDictionary DictionaryType;
	typedef itk::MetaDataObject< std::string > MetaDataStringType;
	typedef short    PixelType;
	const unsigned int      Dimension = 3;
	typedef itk::Image< PixelType, Dimension >         ImageType;
	typedef itk::ImageSeriesReader< ImageType >        ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	typedef itk::GDCMImageIO       ImageIOType;
	ImageIOType::Pointer dicomIO = ImageIOType::New();
	reader->SetImageIO(dicomIO);
	typedef itk::GDCMSeriesFileNames NamesGeneratorType;
	NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
	std::vector<std::string> foo = filepaths;
	//antes de sair abrindo tudo e tomar exceção se algo não for encontrado testar para cada arquivo fornecido
	////se ele existe. Se não existe, tira da lista
	std::vector<int> indices_zuados;
	for (unsigned int i = 0; i < foo.size(); i++)
	{
		std::string path = foo[i];
		WIN32_FIND_DATA FindFileData;
		HANDLE handle = FindFirstFileA(path.c_str(), &FindFileData);
		int found = handle != INVALID_HANDLE_VALUE;
		if (found)
		{
			FindClose(handle);
		}
		else
		{
			indices_zuados.push_back(i);
		}
	}
	//if (indices_zuados.size()>0)
	//	throw "Série corrompida - indices zuados";
	for (unsigned int i = 0; i < indices_zuados.size(); i++)
	{
		foo.erase(foo.begin() + indices_zuados[i]);
	}


	//Lê a série
	reader->SetFileNames(foo);
	try
	{
		if (cbk)
			reader->AddObserver(itk::ProgressEvent(), cbk);
		reader->Update();
	}
	catch (itk::ExceptionObject& ex)
	{
		std::string erro = ex.GetDescription();
		throw ex;
	}
	//Se chegou aqui, aproveita que tem a série e a transforma de uma itk::image pra algo que eu possa entender
	itk::Image<short, 3>::Pointer ptrImg = reader->GetOutput();
	const DictionaryType& dictionary = dicomIO->GetMetaDataDictionary();
	DictionaryType::ConstIterator metadataDictionaryIterator = dictionary.Begin();
	DictionaryType::ConstIterator metadataDictionaryEnd = dictionary.End();
	std::cout << "tags dicom" << std::endl;
	typedef itk::MetaDataObject< std::string > MetaDataStringType;

	std::map<std::string, std::string> metadataVolume;

	while (metadataDictionaryIterator != metadataDictionaryEnd)
	{
		itk::MetaDataObjectBase::Pointer entry = metadataDictionaryIterator->second;
		MetaDataStringType::Pointer entryValue = dynamic_cast<MetaDataStringType*>(entry.GetPointer());
		if (entryValue)
		{
			std::string tagkey = metadataDictionaryIterator->first;
			std::string labelId;
			bool found = itk::GDCMImageIO::GetLabelFromTag(tagkey, labelId);
			std::string tagvalue = entryValue->GetMetaDataObjectValue();
			std::cout << tagkey << " = " << tagvalue << std::endl;
			metadataVolume[tagkey] = tagvalue;
		}
		++metadataDictionaryIterator;
	}
	outputMetadata = metadataVolume;
	return ptrImg;
}

vtkImageImport* CreateVTKImage(itk::Image<short, 3>::Pointer img)
{
	int szX = img->GetLargestPossibleRegion().GetSize()[0];
	int szY = img->GetLargestPossibleRegion().GetSize()[1];
	int szZ = img->GetLargestPossibleRegion().GetSize()[2];
	double sX = img->GetSpacing()[0];
	double sY = img->GetSpacing()[1];
	double sZ = img->GetSpacing()[2];
	double oX = img->GetOrigin()[0];
	double oY = img->GetOrigin()[1];
	double oZ = img->GetOrigin()[2];
	vtkImageImport* vtkImage = vtkImageImport::New();
	vtkImage = vtkImageImport::New();
	vtkImage->SetDataSpacing(sX, sY, sZ);
	vtkImage->SetDataOrigin(oX, oY, oZ);
	vtkImage->SetWholeExtent(0, szX - 1, 0, szY - 1, 0, szZ - 1);
	vtkImage->SetDataExtentToWholeExtent();
	vtkImage->SetDataScalarTypeToShort();
	void* imgPtr = img->GetBufferPointer();
	vtkImage->SetImportVoidPointer(imgPtr, 1);
	vtkImage->Update();
	return vtkImage;
}