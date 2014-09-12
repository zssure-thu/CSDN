// C-STORETest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


//add the include files of Dcmtk library
#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmnet/scp.h"
#include "ZSDcmStoreSCP.h"
#include "global.h"

 extern OFBool g_dimse_save_dimse_data ;



int _tmain(int argc, _TCHAR* argv[])
{
	g_dimse_save_dimse_data = OFTrue;
	//DcmSCP mStoreSCP;
	//mStoreSCP.setAETitle("ZS-TEST");
	//mStoreSCP.setPort(104);
	//mStoreSCP.setVerbosePCMode(true);
	//mStoreSCP.setACSETimeout(-1);
	//mStoreSCP.setDIMSETimeout(10000);
	//mStoreSCP.setDIMSEBlockingMode(DIMSE_NONBLOCKING);
	//OFList< OFString > transferSyntaxes;
	//transferSyntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
	//transferSyntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
	//mStoreSCP.addPresentationContext(UID_CTImageStorage,transferSyntaxes);
	////mStoreSCP.SET
	//mStoreSCP.listen();

	ZSDcmStoreSCP mZSStoreScp;
	mZSStoreScp.setAETitle("ZS-TEST");
	mZSStoreScp.setPort(104);
	mZSStoreScp.setVerbosePCMode(true);
	mZSStoreScp.setACSETimeout(-1);
	mZSStoreScp.setDIMSETimeout(10000);
	mZSStoreScp.setDIMSEBlockingMode(DIMSE_NONBLOCKING);
	OFList< OFString > transferSyntaxes;
	transferSyntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
	transferSyntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
	mZSStoreScp.addPresentationContext(UID_CTImageStorage,transferSyntaxes);
	//mStoreSCP.SET
	mZSStoreScp.listen();

	mZSStoreScp.zs_DIMSE_data;

	return 0;
}

