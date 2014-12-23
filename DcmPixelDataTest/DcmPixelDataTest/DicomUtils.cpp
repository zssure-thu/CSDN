#include "DicomUtils.h"


DicomUtils::DicomUtils(void)
{
}


DicomUtils::~DicomUtils(void)
{
}
void DicomUtils::AddDicomElements(DcmDataset*& dataset)
{
	//构建测试数据

	/*	添加患者信息	*/
	dataset->putAndInsertUint16(DCM_AccessionNumber,0);
	dataset->putAndInsertString(DCM_PatientName,"zssure",true);
	dataset->putAndInsertString(DCM_PatientID,"2234");
	dataset->putAndInsertString(DCM_PatientBirthDate,"20141221");
	dataset->putAndInsertString(DCM_PatientSex,"M");

	/*	添加Study信息	*/
	dataset->putAndInsertString(DCM_StudyDate,"20141221");
	dataset->putAndInsertString(DCM_StudyTime,"195411");
	char uid[100];
	dcmGenerateUniqueIdentifier(uid,SITE_STUDY_UID_ROOT);
	dataset->putAndInsertString(DCM_StudyInstanceUID,uid);
	dataset->putAndInsertString(DCM_StudyID,"1111");


	/*	添加Series信息	*/
	dataset->putAndInsertString(DCM_SeriesDate,"20141221");
	dataset->putAndInsertString(DCM_SeriesTime,"195411");
	memset(uid,0,sizeof(char)*100);
	dcmGenerateUniqueIdentifier(uid,SITE_SERIES_UID_ROOT);
	dataset->putAndInsertString(DCM_SeriesInstanceUID,uid);
	/*	添加Image信息	*/
	dataset->putAndInsertString(DCM_ImageType,"ORIGINAL\\PRIMARY\\AXIAL");
	dataset->putAndInsertString(DCM_ContentDate,"20141221");
	dataset->putAndInsertString(DCM_ContentTime,"200700");
	dataset->putAndInsertString(DCM_InstanceNumber,"1");
	dataset->putAndInsertString(DCM_SamplesPerPixel,"1");
	dataset->putAndInsertString(DCM_PhotometricInterpretation,"MONOCHROME2");
	dataset->putAndInsertString(DCM_PixelSpacing,"0.3\\0.3");
	dataset->putAndInsertString(DCM_BitsAllocated,"16");
	dataset->putAndInsertString(DCM_BitsStored,"16");
	dataset->putAndInsertString(DCM_HighBit,"15");
	dataset->putAndInsertString(DCM_WindowCenter,"600");
	dataset->putAndInsertString(DCM_WindowWidth,"800");
	dataset->putAndInsertString(DCM_RescaleIntercept,"0");
	dataset->putAndInsertString(DCM_RescaleSlope,"1");


}
