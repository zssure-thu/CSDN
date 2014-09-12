#include "stdafx.h"
#include "ZSDcmStoreSCP.h"
#include <windows.h>
#include <direct.h>
#include <dcmtk/dcmnet/diutil.h>
#include "global.h"



OFBool             opt_useMetaheader = OFTrue;
static OFBool      opt_secureConnection = OFFalse;    // default: no secure connection
static OFString    opt_outputDirectory = "d:\\DcmScuScp";         // default: output directory equals "."
OFString           opt_fileNameExtension;
OFBool             opt_abortDuringStore = OFFalse;
OFBool             opt_abortAfterStore = OFFalse;
OFCmdUnsignedInt   opt_sleepAfter = 0;
OFCmdUnsignedInt   opt_sleepDuring = 0;
E_SortStudyMode    opt_sortStudyMode = ESM_None;      // default: no sorting
static const char *opt_sortStudyDirPrefix = NULL;     // default: no directory prefix
E_TransferSyntax   opt_writeTransferSyntax = EXS_Unknown;
E_GrpLenEncoding   opt_groupLength = EGL_recalcGL;
E_EncodingType     opt_sequenceType = EET_ExplicitLength;
E_PaddingEncoding  opt_paddingType = EPD_withoutPadding;
OFCmdUnsignedInt   opt_filepad = 0;
OFCmdUnsignedInt   opt_itempad = 0;
OFCmdUnsignedInt   opt_compressionLevel = 0;
OFBool             opt_bitPreserving = OFFalse;
OFBool             opt_ignore = OFFalse;
OFBool             opt_promiscuous = OFFalse;
OFBool             opt_correctUIDPadding = OFFalse;
OFBool             opt_inetd_mode = OFFalse;
int opt_dimse_timeout = 0;
T_DIMSE_BlockingMode opt_blockMode = DIMSE_BLOCKING;
OFList<OFString>   outputFileNameArray;
OFString           lastStudySubdirectoryPathAndName;
OFString           lastStudyInstanceUID;
OFString           subdirectoryPathAndName;
static const char *opt_execOnReception = NULL;        // default: don't execute anything on reception
static const char *opt_execOnEndOfStudy = NULL;       // default: don't execute anything on end of study
static long        opt_endOfStudyTimeout = -1;        // default: no end of study timeout
OFCmdUnsignedInt   opt_maxPDU = ASC_DEFAULTMAXPDU;

static DUL_PRESENTATIONCONTEXT* findPresentationContextID(LST_HEAD *head,
														  T_ASC_PresentationContextID presentationContextID)
{
	DUL_PRESENTATIONCONTEXT *pc;
	LST_HEAD **l;
	OFBool found = OFFalse;

	if (head == NULL)
		return NULL;

	l = &head;
	if (*l == NULL)
		return NULL;

	pc = (DUL_PRESENTATIONCONTEXT*) LST_Head(l);
	(void)LST_Position(l, (LST_NODE*)pc);

	while (pc && !found)
	{
		if (pc->presentationContextID == presentationContextID)
		{
			found = OFTrue;
		} else
		{
			pc = (DUL_PRESENTATIONCONTEXT*) LST_Next(l);
		}
	}
	return pc;
}

static void getPresentationContextInfo(const T_ASC_Association *assoc,
									   const Uint8 presID,
									   DcmPresentationContextInfo &info)
{
	if (assoc == NULL)
		return;

	DUL_PRESENTATIONCONTEXT *pc = findPresentationContextID(assoc->params->DULparams.acceptedPresentationContext, presID);
	if (pc != NULL)
	{
		info.abstractSyntax = pc->abstractSyntax;
		info.acceptedTransferSyntax = pc->acceptedTransferSyntax;
		info.presentationContextID = pc->presentationContextID;
		info.proposedSCRole = pc->proposedSCRole;
		info.acceptedSCRole = pc->acceptedSCRole;
	}
	return;
}

ZSDcmStoreSCP::ZSDcmStoreSCP():zs_assoc(NULL),zs_assocConfig(NULL),zs_DIMSE_data(NULL)
{

}
OFCondition ZSDcmStoreSCP::storeSCP(
  T_ASC_Association *assoc,
  T_DIMSE_Message *msg,
  T_ASC_PresentationContextID presID)
    /*
     * This function processes a DIMSE C-STORE-RQ commmand that was
     * received over the network connection.
     *
     * Parameters:
     *   assoc  - [in] The association (network connection to another DICOM application).
     *   msg    - [in] The DIMSE C-STORE-RQ message that was received.
     *   presID - [in] The ID of the presentation context which was specified in the PDV which contained
     *                 the DIMSE command.
     */
{
  OFCondition cond = EC_Normal;
  T_DIMSE_C_StoreRQ *req;
  char imageFileName[2048];

  // assign the actual information of the C-STORE-RQ command to a local variable
  req = &msg->msg.CStoreRQ;

      // create unique filename by generating a temporary UID and using ".X." as an infix
      char buf[70];
      dcmGenerateUniqueIdentifier(buf);
      sprintf(imageFileName, "%s%c%s.X.%s%s", opt_outputDirectory.c_str(), PATH_SEPARATOR, dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "UNKNOWN"),
        buf, opt_fileNameExtension.c_str());
  // intialize some variables
  StoreCallbackData callbackData;
  callbackData.assoc = assoc;
  callbackData.imageFileName = imageFileName;
  DcmFileFormat dcmff;
  callbackData.dcmff = &dcmff;

  // store SourceApplicationEntityTitle in metaheader
  if (assoc && assoc->params)
  {
    const char *aet = assoc->params->DULparams.callingAPTitle;
    if (aet) dcmff.getMetaInfo()->putAndInsertString(DCM_SourceApplicationEntityTitle, aet);
  }

  // define an address where the information which will be received over the network will be stored
  DcmDataset *dset = dcmff.getDataset();

  // if opt_bitPreserving is set, the user requires that the data shall be
  // written exactly as it was received. Depending on this option, function
  // DIMSE_storeProvider must be called with certain parameters.
   cond = DIMSE_storeProvider(assoc, presID, req, imageFileName, opt_useMetaheader, NULL,
      storeSCPCallback, &callbackData, opt_blockMode, opt_dimse_timeout);

  // return return value
  return cond;
}

void
ZSDcmStoreSCP::storeSCPCallback(
    void *callbackData,
    T_DIMSE_StoreProgress *progress,
    T_DIMSE_C_StoreRQ *req,
    char * /*imageFileName*/, DcmDataset **imageDataSet,
    T_DIMSE_C_StoreRSP *rsp,
    DcmDataset **statusDetail)
    /*
     * This function.is used to indicate progress when storescp receives instance data over the
     * network. On the final call to this function (identified by progress->state == DIMSE_StoreEnd)
     * this function will store the data set which was received over the network to a file.
     * Earlier calls to this function will simply cause some information to be dumped to stdout.
     *
     * Parameters:
     *   callbackData  - [in] data for this callback function
     *   progress      - [in] The state of progress. (identifies if this is the initial or final call
     *                   to this function, or a call in between these two calls.
     *   req           - [in] The original store request message.
     *   imageFileName - [in] The path to and name of the file the information shall be written to.
     *   imageDataSet  - [in] The data set which shall be stored in the image file
     *   rsp           - [inout] the C-STORE-RSP message (will be sent after the call to this function)
     *   statusDetail  - [inout] This variable can be used to capture detailed information with regard to
     *                   the status information which is captured in the status element (0000,0900). Note
     *                   that this function does specify any such information, the pointer will be set to NULL.
     */
{
  DIC_UI sopClass;
  DIC_UI sopInstance;

  // determine if the association shall be aborted
  if( (opt_abortDuringStore && progress->state != DIMSE_StoreBegin) ||
      (opt_abortAfterStore && progress->state == DIMSE_StoreEnd) )
  {
    ASC_abortAssociation((OFstatic_cast(StoreCallbackData*, callbackData))->assoc);
    rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
    return;
  }

  // if opt_sleepAfter is set, the user requires that the application shall
  // sleep a certain amount of seconds after having received one PDU.
  if (opt_sleepDuring > 0)
  {
    OFStandard::sleep(OFstatic_cast(unsigned int, opt_sleepDuring));
  }

  // if this is the final call of this function, save the data which was received to a file
  // (note that we could also save the image somewhere else, put it in database, etc.)
  if (progress->state == DIMSE_StoreEnd)
  {
    OFString tmpStr;

    // do not send status detail information
    *statusDetail = NULL;

    // remember callback data
    StoreCallbackData *cbdata = OFstatic_cast(StoreCallbackData *, callbackData);

    // Concerning the following line: an appropriate status code is already set in the resp structure,
    // it need not be success. For example, if the caller has already detected an out of resources problem
    // then the status will reflect this.  The callback function is still called to allow cleanup.
    //rsp->DimseStatus = STATUS_Success;

    // we want to write the received information to a file only if this information
    // is present and the options opt_bitPreserving and opt_ignore are not set.
    if ((imageDataSet != NULL) && (*imageDataSet != NULL) && !opt_bitPreserving && !OFFalse)
    {
      OFString fileName;

      // in case one of the --sort-xxx options is set, we need to perform some particular steps to
      // determine the actual name of the output file
      if (opt_sortStudyMode != ESM_None)
      {
        // determine the study instance UID in the (current) DICOM object that has just been received
        OFString currentStudyInstanceUID;
        if ((*imageDataSet)->findAndGetOFString(DCM_StudyInstanceUID, currentStudyInstanceUID).bad() || currentStudyInstanceUID.empty())
        {
          rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
          return;
        }

        // if --sort-on-patientname is active, we need to extract the
        // patient's name (format: last_name^first_name)
        OFString currentPatientName;
        if (opt_sortStudyMode == ESM_PatientName)
        {
          OFString tmpName;
          if ((*imageDataSet)->findAndGetOFString(DCM_PatientName, tmpName).bad() || tmpName.empty())
          {
            // default if patient name is missing or empty
            tmpName = "ANONYMOUS";
          }

          /* substitute non-ASCII characters in patient name to ASCII "equivalent" */
          const size_t length = tmpName.length();
          for (size_t i = 0; i < length; i++)
            mapCharacterAndAppendToString(tmpName[i], currentPatientName);
        }

        // if this is the first DICOM object that was received or if the study instance UID in the
        // current DICOM object does not equal the last object's study instance UID we need to create
        // a new subdirectory in which the current DICOM object will be stored
        if (lastStudyInstanceUID.empty() || (lastStudyInstanceUID != currentStudyInstanceUID))
        {
          // if lastStudyInstanceUID is non-empty, we have just completed receiving all objects for one
          // study. In such a case, we need to set a certain indicator variable (lastStudySubdirectoryPathAndName),
          // so that we know that executeOnEndOfStudy() might have to be executed later. In detail, this indicator
          // variable will contain the path and name of the last study's subdirectory, so that we can still remember
          // this directory, when we execute executeOnEndOfStudy(). The memory that is allocated for this variable
          // here will be freed after the execution of executeOnEndOfStudy().
          if (!lastStudyInstanceUID.empty())
            lastStudySubdirectoryPathAndName = subdirectoryPathAndName;

          // create the new lastStudyInstanceUID value according to the value in the current DICOM object
          lastStudyInstanceUID = currentStudyInstanceUID;

          // get the current time (needed for subdirectory name)
          OFDateTime dateTime;
          dateTime.setCurrentDateTime();

          // create a name for the new subdirectory.
          char timestamp[32];
          sprintf(timestamp, "%04u%02u%02u_%02u%02u%02u%03u",
            dateTime.getDate().getYear(), dateTime.getDate().getMonth(), dateTime.getDate().getDay(),
            dateTime.getTime().getHour(), dateTime.getTime().getMinute(), dateTime.getTime().getIntSecond(), dateTime.getTime().getMilliSecond());

          OFString subdirectoryName;
          switch (opt_sortStudyMode)
          {
            case ESM_Timestamp:
              // pattern: "[prefix]_[YYYYMMDD]_[HHMMSSMMM]"
              subdirectoryName = opt_sortStudyDirPrefix;
              if (!subdirectoryName.empty())
                subdirectoryName += '_';
              subdirectoryName += timestamp;
              break;
            case ESM_StudyInstanceUID:
              // pattern: "[prefix]_[Study Instance UID]"
              subdirectoryName = opt_sortStudyDirPrefix;
              if (!subdirectoryName.empty())
                subdirectoryName += '_';
              subdirectoryName += currentStudyInstanceUID;
              break;
            case ESM_PatientName:
              // pattern: "[Patient's Name]_[YYYYMMDD]_[HHMMSSMMM]"
              subdirectoryName = currentPatientName;
              subdirectoryName += '_';
              subdirectoryName += timestamp;
              break;
            case ESM_None:
              break;
          }

          // create subdirectoryPathAndName (string with full path to new subdirectory)
          OFStandard::combineDirAndFilename(subdirectoryPathAndName, OFStandard::getDirNameFromPath(tmpStr, cbdata->imageFileName), subdirectoryName);

          // check if the subdirectory already exists
          // if it already exists dump a warning
          if( OFStandard::dirExists(subdirectoryPathAndName) )
		  {}
		  else
          {
            // if it does not exist create it
#ifdef HAVE_WINDOWS_H
            if( _mkdir( subdirectoryPathAndName.c_str() ) == -1 )
#else
            if( mkdir( subdirectoryPathAndName.c_str(), S_IRWXU | S_IRWXG | S_IRWXO ) == -1 )
#endif
            {
              rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
              return;
            }
            // all objects of a study have been received, so a new subdirectory is started.
            // ->timename counter can be reset, because the next filename can't cause a duplicate.
            // if no reset would be done, files of a new study (->new directory) would start with a counter in filename
          }
        }

        // integrate subdirectory name into file name (note that cbdata->imageFileName currently contains both
        // path and file name; however, the path refers to the output directory captured in opt_outputDirectory)
        OFStandard::combineDirAndFilename(fileName, subdirectoryPathAndName, OFStandard::getFilenameFromPath(tmpStr, cbdata->imageFileName));

        // update global variable outputFileNameArray
        // (might be used in executeOnReception() and renameOnEndOfStudy)
        outputFileNameArray.push_back(tmpStr);
      }
      // if no --sort-xxx option is set, the determination of the output file name is simple
      else
      {
        fileName = cbdata->imageFileName;

        // update global variables outputFileNameArray
        // (might be used in executeOnReception() and renameOnEndOfStudy)
        outputFileNameArray.push_back(OFStandard::getFilenameFromPath(tmpStr, fileName));
      }

      // determine the transfer syntax which shall be used to write the information to the file
      E_TransferSyntax xfer = opt_writeTransferSyntax;
      if (xfer == EXS_Unknown) xfer = (*imageDataSet)->getOriginalXfer();

      // store file either with meta header or as pure dataset
      if (OFStandard::fileExists(fileName))
      {
      }
      OFCondition cond = cbdata->dcmff->saveFile(fileName.c_str(), xfer, opt_sequenceType, opt_groupLength,
          opt_paddingType, OFstatic_cast(Uint32, opt_filepad), OFstatic_cast(Uint32, opt_itempad),
          (opt_useMetaheader) ? EWM_fileformat : EWM_dataset);
      if (cond.bad())
      {
        rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
      }

      // check the image to make sure it is consistent, i.e. that its sopClass and sopInstance correspond
      // to those mentioned in the request. If not, set the status in the response message variable.
      if ((rsp->DimseStatus == STATUS_Success)&&(!opt_ignore))
      {
        // which SOP class and SOP instance ?
        if (!DU_findSOPClassAndInstanceInDataSet(*imageDataSet, sopClass, sopInstance, opt_correctUIDPadding))
        {
           rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
        }
        else if (strcmp(sopClass, req->AffectedSOPClassUID) != 0)
        {
          rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
        }
        else if (strcmp(sopInstance, req->AffectedSOPInstanceUID) != 0)
        {
          rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
        }
      }
    }

    // in case opt_bitPreserving is set, do some other things
    if( opt_bitPreserving )
    {
      // we need to set outputFileNameArray and outputFileNameArrayCnt to be
      // able to perform the placeholder substitution in executeOnReception()
      outputFileNameArray.push_back(OFStandard::getFilenameFromPath(tmpStr, cbdata->imageFileName));
    }
  }
}

OFCondition ZSDcmStoreSCP::handleIncomingCommand(T_DIMSE_Message *msg,
										  const DcmPresentationContextInfo &info,T_ASC_PresentationContextID presID)
{
	OFCondition cond;
	if( msg->CommandField == DIMSE_C_ECHO_RQ )
	{
		// Process C-ECHO request
		cond = handleECHORequest( msg->msg.CEchoRQ, info.presentationContextID );
	} else if(msg->CommandField == DIMSE_C_STORE_RQ)
	{
		//T_DIMSE_Message msg;
		//T_ASC_PresentationContextID presID = 0;
		//DcmDataset *statusDetail = NULL;

		/*if( opt_endOfStudyTimeout == -1 )
			cond = DIMSE_receiveCommand(zs_assoc, DIMSE_BLOCKING, 0, &presID, &msg, &statusDetail,&zs_DIMSE_data);
		else
			cond = DIMSE_receiveCommand(zs_assoc, DIMSE_NONBLOCKING, OFstatic_cast(int, opt_endOfStudyTimeout), &presID, &msg, &statusDetail);
*/
		cond = storeSCP(zs_assoc, msg, /*presID*/info.presentationContextID);

	}
	else
	{
		// We cannot handle this kind of message. Note that the condition will be returned
		// and that the caller is responsible to end the association if desired.
		OFString tempStr;
		DCMNET_ERROR("Cannot handle this kind of DIMSE command (0x"
			<< STD_NAMESPACE hex << STD_NAMESPACE setfill('0') << STD_NAMESPACE setw(4)
			<< OFstatic_cast(unsigned int, msg->CommandField) << ")");
		DCMNET_DEBUG(DIMSE_dumpMessage(tempStr, *msg, DIMSE_INCOMING));
		cond = DIMSE_BADCOMMANDTYPE;
	}

	// return result
	return cond;
}

OFCondition ZSDcmStoreSCP::listen()
{

	OFCondition cond = EC_Normal;

	// Make sure data dictionary is loaded.
	if( !dcmDataDict.isDictionaryLoaded() )
		DCMNET_WARN("no data dictionary loaded, check environment variable: " << DCM_DICT_ENVIRONMENT_VARIABLE);

	// Initialize network, i.e. create an instance of T_ASC_Network*.
	T_ASC_Network *m_net = NULL;
	cond = ASC_initializeNetwork( NET_ACCEPTOR, (int)getPort(), getACSETimeout(), &m_net );
	if( cond.bad() )
		return cond;

	while( cond.good() && !stopAfterCurrentAssociation() )
	{
		// Wait for an association and handle the requests of
		// the calling applications correspondingly.
		cond = ASC_receiveAssociation(m_net, &zs_assoc, opt_maxPDU, NULL, NULL, opt_secureConnection, DUL_NOBLOCK, OFstatic_cast(int, 8000));
		char buf[BUFSIZ];
		const char* knownAbstractSyntaxes[] =
		{
			UID_VerificationSOPClass,
			UID_CTImageStorage,
			UID_EnhancedCTImageStorage
		};

		const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
		transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
		transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
		transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
		int numTransferSyntaxes = 3;

		cond = ASC_acceptContextsWithPreferredTransferSyntaxes( zs_assoc->params, knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes), transferSyntaxes, numTransferSyntaxes);
		if (cond.bad())
		{
			continue;
		}



		cond = ASC_getApplicationContextName( zs_assoc->params, buf );
		if( cond.bad() || strcmp( buf, DICOM_STDAPPLICATIONCONTEXT ) != 0 )
		{
			refuseAssociation( DCMSCP_BAD_APPLICATION_CONTEXT_NAME );
		}

		/* set our application entity title */
		ASC_setAPTitles(zs_assoc->params, NULL, NULL, zs_assoc->params->DULparams.calledAPTitle);
		
		if (zs_assoc == NULL)
			continue;
		// If the negotiation was successful, accept the association request
		cond = ASC_acknowledgeAssociation( zs_assoc );
		if( cond.bad() )
		{
			continue;
		}
		//handleAssociation();
		//-----------------------------------------
		if (zs_assoc == NULL)
		{
			DCMNET_WARN("DcmSCP::handleAssociation() called but SCP actually has no association running, ignoring");
			return cond;
		}

		// Receive a DIMSE command and perform all the necessary actions. (Note that ReceiveAndHandleCommands()
		// will always return a value 'cond' for which 'cond.bad()' will be true. This value indicates that either
		// some kind of error occurred, or that the peer aborted the association (DUL_PEERABORTEDASSOCIATION),
		// or that the peer requested the release of the association (DUL_PEERREQUESTEDRELEASE).) (Also note
		// that ReceiveAndHandleCommands() will never return EC_Normal.)
		OFCondition cond = EC_Normal;
		T_DIMSE_Message msg;
		T_ASC_PresentationContextID presID;

		// start a loop to be able to receive more than one DIMSE command
		while( cond.good() )
		{
			// receive a DIMSE command over the network
			//DcmDataset* zsDcmDataset;
			cond = DIMSE_receiveCommand( zs_assoc, DIMSE_BLOCKING, 0, &presID, &msg, NULL );

			// check if peer did release or abort, or if we have a valid message
			if( cond.good() )
			{
				DcmPresentationContextInfo pcInfo;
				getPresentationContextInfo(zs_assoc, presID, pcInfo);
				cond = handleIncomingCommand(&msg, pcInfo, presID);
			}
		}
		// Clean up on association termination.
		if( cond == DUL_PEERREQUESTEDRELEASE )
		{
			notifyReleaseRequest();
			ASC_acknowledgeRelease(zs_assoc);
			ASC_dropSCPAssociation(zs_assoc);
		}
		else if( cond == DUL_PEERABORTEDASSOCIATION )
		{
			notifyAbortRequest();
		}
		else
		{
			notifyDIMSEError(cond);
			ASC_abortAssociation( zs_assoc );
		}

		// Drop and destroy the association.
		if (zs_assoc)
		{
			ASC_dropAssociation( zs_assoc );
			ASC_destroyAssociation( &zs_assoc );
		}

		// Dump some information if required.
		DCMNET_DEBUG( "+++++++++++++++++++++++++++++" );

		//-----------------------------------------
	}
	cond = ASC_dropNetwork( &m_net );
	m_net = NULL;
	if( cond.bad() )
		return cond;
	return EC_Normal;
}
