#pragma once
#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmdata/dcistrmf.h"
#include "dcmtk/dcmdata/dcuid.h"

class DicomUtils
{
public:
	DicomUtils(void);
	~DicomUtils(void);

public:
	/*	static functions	*/
	static void AddDicomElements(DcmDataset*& dataset);

};

