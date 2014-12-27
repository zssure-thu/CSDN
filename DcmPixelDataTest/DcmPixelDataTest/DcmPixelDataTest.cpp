// DcmPixelDataTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmdata/dcistrmf.h"
#include "dcmtk/dcmdata/libi2d/i2dbmps.h"
#include "DicomUtils.h"
#include <direct.h>

int _tmain(int argc, _TCHAR* argv[])
{
	OFCondition status;
	//DcmMetaInfo metainfo;
	//status=metainfo.loadFile("c:\\test4.dcm");
	//if(status.good())
	//{
	//	OFString sopClassUID,xferUID;
	//	if(metainfo.findAndGetOFString(DCM_MediaStorageSOPClassUID, sopClassUID).good())
	//	{
	//		std::cout<<"SOP Class UID:"<<sopClassUID<<OFendl;
	//	}
	//	if(metainfo.findAndGetOFString(DCM_TransferSyntaxUID, xferUID).good())
	//	{
	//		std::cout<<"Transfer SyntaxUID is :"<<xferUID<<OFendl;
	//	}
	//}
	DcmFileFormat fileformat;
	DcmDataset* mydatasete=fileformat.getDataset();
	DicomUtils::AddDicomElements((DcmDataset*&)mydatasete);
	Uint16 rows,cols,samplePerPixel,bitsAlloc,bitsStored,highBit,pixelRpr,planConf,pixAspectH,pixAspectV;
	OFString photoMetrInt;
	Uint32 length;
	E_TransferSyntax ts;
	char* mydata=new char[1024*1024*10];
	memset(mydata,0,sizeof(char)*1024*1024*10);
	char* tmpData=mydata;
	char curDir[100];
	getcwd(curDir,100);
	//循环添加4张图片
	for(int i=0;i<4;++i)
	{
		OFString num;
		char numtmp[100];
		memset(numtmp,0,sizeof(char)*100);
		sprintf(numtmp,"%s\\test\\%d.bmp",curDir,i+1);
		OFString filename=OFString(numtmp);
		I2DBmpSource* bmpSource=new I2DBmpSource();
		bmpSource->setImageFile(filename);
		//bmpSource->openFile("c:\\test.bmp");
		//Uint16 rows,cols,samplePerPixel,bitsAlloc,bitsStored,highBit,pixelRpr,planConf,pixAspectH,pixAspectV;
		//OFString photoMetrInt;
		//Uint32 length;
		char* pixData=NULL;
		//E_TransferSyntax ts;
		bmpSource->readPixelData(rows,cols,samplePerPixel,photoMetrInt,bitsAlloc,bitsStored,highBit,pixelRpr,planConf,pixAspectH,pixAspectV,pixData,length,ts);
		memcpy(tmpData,pixData,length);
		tmpData+=length;
		delete bmpSource;
	};
	//I2DBmpSource* bmpSource=new I2DBmpSource();
	//bmpSource->setImageFile("c:\\test_anon.bmp");
	////bmpSource->openFile("c:\\test.bmp");
	//Uint16 rows,cols,samplePerPixel,bitsAlloc,bitsStored,highBit,pixelRpr,planConf,pixAspectH,pixAspectV;
	//OFString photoMetrInt;
	//Uint32 length;
	//char* pixData=NULL;
	//E_TransferSyntax ts;
	//bmpSource->readPixelData(rows,cols,samplePerPixel,photoMetrInt,bitsAlloc,bitsStored,highBit,pixelRpr,planConf,pixAspectH,pixAspectV,pixData,length,ts);
	mydatasete->putAndInsertUint16(DCM_SamplesPerPixel,samplePerPixel);
	mydatasete->putAndInsertString(DCM_NumberOfFrames,"4");
	mydatasete->putAndInsertUint16(DCM_Rows,rows);
	mydatasete->putAndInsertUint16(DCM_Columns,cols);
	mydatasete->putAndInsertUint16(DCM_BitsAllocated,bitsAlloc);
	mydatasete->putAndInsertUint16(DCM_BitsStored,bitsStored);
	mydatasete->putAndInsertUint16(DCM_HighBit,highBit);
	mydatasete->putAndInsertUint8Array(DCM_PixelData,(Uint8*)mydata,4*length);
	//mydatasete->putAndInsertUint8Array(DCM_PixelData,(Uint8*)pixData,length);
	mydatasete->putAndInsertOFStringArray(DCM_PhotometricInterpretation,photoMetrInt);
	//mydatasete->putAndInsertString(DCM_PlanarConfiguration,"1");
	status=fileformat.saveFile("c:\\Multibmp2dcmtest.dcm",ts);
	if(status.bad())
	{
		std::cout<<"Error:("<<status.text()<<")\n";
	}
	return 0;
}
/*!-------博文：DICOM医学图像处理：DICOM存储操作之 “多幅JPG图像数据存入DCM文件”核心代码--------!*/
int _tmain(int argc, _TCHAR* argv[])
{
	OFCondition status;

	DcmFileFormat fileformat;
	DcmDataset* mydatasete=fileformat.getDataset();
	DicomUtils::AddDicomElements((DcmDataset*&)mydatasete);
	Uint16 rows,cols,samplePerPixel,bitsAlloc,bitsStored,highBit,pixelRpr,planConf,pixAspectH,pixAspectV;
	OFString photoMetrInt;
	Uint32 length;
	E_TransferSyntax ts;
	char curDir[255];
	getcwd(curDir,255);
	DcmPixelSequence *seq=new DcmPixelSequence(DcmTag(DCM_PixelData,EVR_OB));
	/*!------zssure:begin,添加一个空的Dicom Pixel Item充当Offset Fragment------!*/
	seq->insert(new DcmPixelItem(DcmTag(DCM_Item,EVR_OB)));
	/*-------zssure:end,可顺利解决多幅JPEG存入DCM的问题-------------------------*/
	//循环添加4张图片
	for(int i=0;i<4;++i)
	{
		OFString num;
		char numtmp[255];
		memset(numtmp,0,sizeof(char)*255);
		sprintf(numtmp,"%s\\jpeg-test\\%d.jpg",curDir,i+1);
		OFString filename=OFString(numtmp);
		I2DJpegSource* bmpSource=new I2DJpegSource();
		bmpSource->setImageFile(filename);

		char* pixData=NULL;
		bmpSource->readPixelData(rows,cols,samplePerPixel,photoMetrInt,bitsAlloc,bitsStored,highBit,pixelRpr,planConf,pixAspectH,pixAspectV,pixData,length,ts);

		DcmPixelItem *newItem=new DcmPixelItem(DcmTag(DCM_Item,EVR_OB));
		if(newItem!=NULL)
		{
			seq->insert(newItem);
			OFCondition result=newItem->putUint8Array((Uint8*)pixData,length);

		}
		delete bmpSource;
	};

	mydatasete->putAndInsertUint16(DCM_SamplesPerPixel,samplePerPixel);
	mydatasete->putAndInsertString(DCM_NumberOfFrames,"4");
	mydatasete->putAndInsertUint16(DCM_Rows,rows);
	mydatasete->putAndInsertUint16(DCM_Columns,cols);
	mydatasete->putAndInsertUint16(DCM_BitsAllocated,bitsAlloc);
	mydatasete->putAndInsertUint16(DCM_BitsStored,bitsStored);
	mydatasete->putAndInsertUint16(DCM_HighBit,highBit);
	mydatasete->putAndInsertOFStringArray(DCM_PhotometricInterpretation,photoMetrInt);
	mydatasete->insert(seq,OFFalse,OFFalse);
	status=fileformat.saveFile("c:\\MultiJpeg2Multi-frameDCMtest-error.dcm",ts);
	if(status.bad())
	{
		std::cout<<"Error:("<<status.text()<<")\n";
	}
	return 0;
}
/*!--------------------------------------------------------------------------------------------!*/
