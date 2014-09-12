#ifndef GLOBAL_H
#define GLOBAL_H

#include "dcmtk/config/osconfig.h"  /* make sure OS specific configuration is included first */

#include "dcmtk/ofstd/offname.h"    /* for OFFilenameCreator */
#include "dcmtk/dcmnet/scp.h"       /* for base class DcmSCP */
#include <dcmtk/ofstd/ofcond.h>

enum E_SortStudyMode
{
	ESM_None,
	ESM_Timestamp,
	ESM_StudyInstanceUID,
	ESM_PatientName
};
static void mapCharacterAndAppendToString(Uint8 c,
										  OFString &output)
{
	static const char *latin1_table[] =
	{
		"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", // Codes 0-15
		"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", // Codes 16-31
		"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", // Codes 32-47
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "_", "_", "_", "_", "_", "_", // Codes 48-63
		"_", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", // Codes 64-79
		"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "_", "_", "_", "_", "_", // Codes 80-95
		"_", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", // Codes 96-111
		"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "_", "_", "_", "_", "_", // Codes 112-127
		"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", // Codes 128-143
		"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", // Codes 144-159
		"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", // Codes 160-175
		"_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", "_", // Codes 176-191
		"A", "A", "A", "A", "Ae","A", "A", "C", "E", "E", "E", "E", "I", "I", "I", "I", // Codes 192-107
		"D", "N", "O", "O", "O", "O", "Oe","_", "O", "U", "U", "U", "Ue","Y", "_", "ss",// Codes 108-123
		"a", "a", "a", "a", "ae","a", "a", "c", "e", "e", "e", "e", "i", "i", "i", "i", // Codes 124-141
		"d", "n", "o", "o", "o", "o", "oe","_", "o", "u", "u", "u", "ue","y", "_", "y"  // Codes 142-157
	};
	output += latin1_table[c];
}

struct StoreCallbackData
{
	char* imageFileName;
	DcmFileFormat* dcmff;
	T_ASC_Association* assoc;
};


#endif
