
#ifndef ZSDCMSTORESCP_H
#define ZSDCMSTORESCP_H

#include "dcmtk/config/osconfig.h"  /* make sure OS specific configuration is included first */

#include "dcmtk/ofstd/offname.h"    /* for OFFilenameCreator */
#include "dcmtk/dcmnet/scp.h"       /* for base class DcmSCP */
#include <dcmtk/ofstd/ofcond.h>

class ZSDcmStoreSCP:public DcmSCP
{
public:
	T_ASC_Association* zs_assoc;
	DcmAssociationConfiguration *zs_assocConfig;
	DcmDataset* zs_DIMSE_data;
	OFCondition listen();
	ZSDcmStoreSCP();
	static OFCondition storeSCP(
		T_ASC_Association *assoc,
		T_DIMSE_Message *msg,
		T_ASC_PresentationContextID presID);
	static void
		storeSCPCallback(
		void *callbackData,
		T_DIMSE_StoreProgress *progress,
		T_DIMSE_C_StoreRQ *req,
		char * /*imageFileName*/, DcmDataset **imageDataSet,
		T_DIMSE_C_StoreRSP *rsp,
		DcmDataset **statusDetail);
	OFCondition handleIncomingCommand(T_DIMSE_Message *msg,
		const DcmPresentationContextInfo &info,T_ASC_PresentationContextID presID);

};


#endif