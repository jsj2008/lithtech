#ifndef LTA_PACKER_SAVE_TO_LTA
#define LTA_PACKER_SAVE_TO_LTA



#include "ltaScene.h"

class Model ;



// ------------------------------------------------------------------------
// given model and filename writes out an lta file.
// ------------------------------------------------------------------------
bool ltaSave( const char *filename , Model *pExportModel, CLTATranslStatus &status );






#endif

