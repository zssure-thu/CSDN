// dumpCFindResponse.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmnet/dfindscu.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/ofstd/ofstdinc.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdicent.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcpath.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcpxitem.h"
#include "dcmtk/dcmdata//dctk.h"
#include "ZSCFindCallback.h"
#include "ZSUtilities.h"

//--------------------硬编码全局变量----------------------------------
UINT32 maxReceivePDULength=ASC_DEFAULTMAXPDU;
const char* ourTitle="ZSSURE";
const char* peerTitle="OFFIS";
const char* peer="127.0.0.1";
unsigned int port=2234;
const char* abstractSyntax=UID_FINDModalityWorklistInformationModel;
const char* transferSyntaxs[]={
	UID_LittleEndianExplicitTransferSyntax,
	UID_BigEndianExplicitTransferSyntax,
	UID_LittleEndianImplicitTransferSyntax
};
int transferSyntaxNum=3;
//-----------------------zssure:end------------------------------------
void InsertQueryItems(DcmDataset*& dataset,const char* patientName=NULL,const char* patientID=NULL)
{
	if(patientName==NULL && patientID==NULL)
		return;
	if(patientName!=NULL)
		dataset->putAndInsertString(DCM_PatientName,patientName);
	else
	{
		dataset->putAndInsertString(DCM_PatientName,"");

	}
	if(patientID!=NULL)
		dataset->putAndInsertString(DCM_PatientID,patientID);
	else
	{
		dataset->putAndInsertString(DCM_PatientID,"");

	}
	//固定查询的属性参数，这里只设置了三级UID查询
	dataset->putAndInsertString(DCM_StudyInstanceUID,"");
	dataset->putAndInsertString(DCM_SeriesInstanceUID,"");
	dataset->putAndInsertString(DCM_SOPInstanceUID,"");
}
int _tmain(int argc, _TCHAR* argv[])
{
	//1)初始化网络环境
	WSAData winSockData;
	/* we need at least version 1.1 */
	WORD winSockVersionNeeded = MAKEWORD( 1, 1 );
	WSAStartup(winSockVersionNeeded, &winSockData);
	//2）DCMTK环境监测
	if(!dcmDataDict.isDictionaryLoaded())
	{
		printf("No data dictionary loaded, check environment variable\n");
	}
	//3）网络层ASC初始化
	T_ASC_Network* cfindNetwork=NULL;
	int timeout=50;
	OFCondition cond=ASC_initializeNetwork(NET_REQUESTOR,0,timeout,&cfindNetwork);
	if(cond.bad())
	{
		printf("DICOM 底层网络初始化失败\n");
		return -1;
	}
	//4）创建底层连接，即TCP层
	T_ASC_Association* assoc=NULL;
	T_ASC_Parameters* params=NULL;
	DIC_NODENAME localHost;
	DIC_NODENAME peerHost;
	OFString temp_str;
	cond=ASC_createAssociationParameters(&params,maxReceivePDULength);
	if(cond.bad())
	{
		printf("DCMTK创建连接失败\n");
		return -2;
	}
	//5）设置DICOM相关属性，Presentation Context
	ASC_setAPTitles(params, ourTitle, peerTitle, NULL);

	cond = ASC_setTransportLayerType(params, false);
	if (cond.bad()) return -3;


	gethostname(localHost, sizeof(localHost) - 1);
	sprintf(peerHost, "%s:%d", peer, OFstatic_cast(int, port));
	ASC_setPresentationAddresses(params, localHost, peerHost);

	cond=ASC_addPresentationContext(params,1,abstractSyntax,transferSyntaxs,transferSyntaxNum);
	if(cond.bad())
		return -4;
	//6）真正创建连接
	cond=ASC_requestAssociation(cfindNetwork,params,&assoc);
	if (cond.bad()) {
		if (cond == DUL_ASSOCIATIONREJECTED) {
			T_ASC_RejectParameters rej;
			ASC_getRejectParameters(params, &rej);

			DCMNET_ERROR("Association Rejected:" << OFendl << ASC_printRejectParameters(temp_str, &rej));
			return -5;
		} else {
			DCMNET_ERROR("Association Request Failed: " << DimseCondition::dump(temp_str, cond));
			return -6;
		}
	}
	//7）判别返回结果
	//7.1)连接检验阶段，验证Presentation Context
	if(ASC_countAcceptedPresentationContexts(params)==0)
	{
		printf("No acceptable Presentation Contexts\n");
		return -7;
	}
	T_ASC_PresentationContextID presID;
	T_DIMSE_C_FindRQ req;
	T_DIMSE_C_FindRSP rsp;
	DcmFileFormat dcmff;
	
	presID=ASC_findAcceptedPresentationContextID(assoc,abstractSyntax);
	if(presID==0)
	{
		printf("No presentation context\n");
		return -8;
	}
	//8）发起C-FIND请求
	//8.1）准备C-FIND-RQ message
	bzero(OFreinterpret_cast(char*,&req),sizeof(req));//内存初始化为空;
	strcpy(req.AffectedSOPClassUID,abstractSyntax);
	req.DataSetType=DIMSE_DATASET_PRESENT;
	req.Priority=DIMSE_PRIORITY_LOW;
	//设置要查询的信息为空时，待会儿查询结果中会返回
	DcmDataset* dataset=new DcmDataset();
	InsertQueryItems(dataset,"A^B^C");
	//赋值自定义的回调函数，这就是该回调函数中可以进行相关信息的操作
	ZSCFindCallback zsCallback;
	DcmFindSCUCallback* callback=&zsCallback;
	callback->setAssociation(assoc);
	callback->setPresentationContextID(presID);
	/* as long as no error occured and the counter does not equal 0 */
	cond = EC_Normal;
	while (cond.good())
	{
		DcmDataset *statusDetail = NULL;

		/* complete preparation of C-FIND-RQ message */
		req.MessageID = assoc->nextMsgID++;

		/* finally conduct transmission of data */
		cond = DIMSE_findUser(assoc, presID, &req, dataset,
			progressCallback, callback, DIMSE_BLOCKING, timeout,
			&rsp, &statusDetail);
		//设置了查询采用阻塞模式，DIMSE_BLOCKING
		//设置连接超时为50



		/*
		 *添加异常判别
		 *
		 */
		cond=EC_EndOfStream;//假设异常，返回
	}

	return 0;
}

