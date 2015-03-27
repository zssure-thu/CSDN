#ifndef _ZS_UTILITIES_H
#define _ZS_UTILITIES_H

#include "dcmtk/ofstd/ofstdinc.h"

#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdicent.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcpath.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/dcmnet/dfindscu.h"

static void progressCallback(
	void *callbackData,
	T_DIMSE_C_FindRQ *request,
	int responseCount,
	T_DIMSE_C_FindRSP *rsp,
	DcmDataset *responseIdentifiers)
{
	DcmFindSCUCallback *callback = OFreinterpret_cast(DcmFindSCUCallback *, callbackData);
	if (callback) callback->callback(request, responseCount, rsp, responseIdentifiers);
}
#endif;