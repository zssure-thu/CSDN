#ifndef _ZSCFINDCALLBACK_H
#define _ZSCFINDCALLBACK_H
#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/ofcond.h"    /* for class OFCondition */
#include "dcmtk/dcmdata/dcxfer.h"  /* for E_TransferSyntax */
#include "dcmtk/dcmnet/dimse.h"    /* for T_DIMSE_BlockingMode */
#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/dcmnet/dfindscu.h"
class ZSCFindCallback:public DcmFindSCUCallback
{
public:
	ZSCFindCallback()
	{

	}

	~ZSCFindCallback()
	{

	}

	void callback( T_DIMSE_C_FindRQ *request,
		int responseCount,
		T_DIMSE_C_FindRSP *rsp,
		DcmDataset *rspMessage
		);

};
void ZSCFindCallback::callback(
	T_DIMSE_C_FindRQ *request,
	int responseCount,
	T_DIMSE_C_FindRSP *rsp,
	DcmDataset *rspMessage)
{
	//简单的输出查询到的结果，即C-FIND-RSP
	//std::cout<<DcmObject::PrintHelper(*rspMessage);
	//const char* patientID=new char[100];
	//memset((void*)patientID,0,100);
	//const char* patientName=new char[100];
	//const char* birthDay=new char[100];
	//const char* studyUID=new char[100];
	//const char* seriesUID=new char[100];
	//const char* sopUID=new char[100];
	const char* patientID;
	const char* patientName;
	//const char* birthDay;
	const char* studyUID;
	const char* seriesUID;
	const char* sopUID;
	rspMessage->findAndGetString(DCM_PatientID,patientID);
	rspMessage->findAndGetString(DCM_PatientName,patientName);
	rspMessage->findAndGetString(DCM_StudyInstanceUID,studyUID);
	rspMessage->findAndGetString(DCM_SeriesInstanceUID,seriesUID);
	rspMessage->findAndGetString(DCM_SOPInstanceUID,sopUID);
	std::cout<<std::endl<<std::endl;
	std::cout<<"C-FIND 查询结果如下：\n\n\n";
	std::cout<<"--------------------BEGIN--------------------\n";
	if(patientID!=NULL)
		std::cout<<"PatientID:           "<<patientID<<std::endl;
	if(patientName!=NULL)
		std::cout<<"PatientName:         "<<patientName<<std::endl;
	if(studyUID!=NULL)
		std::cout<<"StudyInstanceUID:    "<<studyUID<<std::endl;
	if(seriesUID!=NULL)
		std::cout<<"SeriesInstanceUID:   "<<seriesUID<<std::endl;
	if(sopUID!=NULL)
		std::cout<<"SopInstanceUID:      "<<sopUID<<std::endl;
	std::cout<<"---------------------END---------------------\n";

	//-----------------To Do yourself-----------------------

}
#endif
