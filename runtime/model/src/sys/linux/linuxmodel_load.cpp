#include "model.h"

// ----------------------------------------------------------------
//  This is the current version of Linux ltb files we are on.
// if a file is not of this version you may puke at your leisure.
// ----------------------------------------------------------------
const uint32 LTB_LINUX_VERSION = 2;

// ----------------------------------------------------------------
//  static model function that returns to the rest of the system
// the current file version.
// ----------------------------------------------------------------
int Model::GetModelSysVersion()
{
    return LTB_LINUX_VERSION ;
}

LTRESULT ModelPiece::ABC_LOAD( ILTStream & file, CDIModelDrawable *pPiece, Model* pModel )
{
    #ifdef _DEBUG
    printf(" --- ABC FORMAT NOT SUPPORTED -- \n" );
    #endif

    return false;
}
