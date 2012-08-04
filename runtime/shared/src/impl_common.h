    
// This module defines common routines to ClientDE and ServerDE as well as 
// helper functions for both modules.

#ifndef __IMPL_COMMON_H__
#define __IMPL_COMMON_H__


#ifndef __ILTCOMMON_H__
#include "iltcommon.h"
#endif

#ifndef __COMPRESS_H__
#include "compress.h"
#endif


class ILTMath;
class Node;
class WorldTree;
typedef void* HLTFileTree; //class HLTFileTree;



inline LTObject* HObjToLTObj(HOBJECT hObj) {ASSERT(hObj); return (LTObject*)hObj;}


// ------------------------------------------------------------------ //
// Helpers.
// ------------------------------------------------------------------ //

// Get the position and rotation out of a matrix.   
inline void ic_GetTransform(LTMatrix &mat, LTVector &pos, LTRotation &rot) {
    Mat_GetTranslation(mat, pos);
    quat_ConvertFromMatrix((float*)&rot, mat.m);
}


// Tells if the point is inside or outside the BSP.  (Inside being the 
// front side of the nodes).
bool ci_IsPointInsideBSP(const Node *pRoot, const LTVector &P);

// Returns LTTRUE if the point is not outside any BSPs that it intersects.
bool ic_IsPointInsideWorld(WorldTree *pWorldTree, const LTVector *pPoint);

// ------------------------------------------------------------------ //
// Interface implementation functions.
// ------------------------------------------------------------------ //

void ic_StartCounter(LTCounter *pCounter);
uint32 ic_EndCounter(LTCounter *pCounter);

bool ic_UpperStrcmp(const char *pStr1, const char *pStr2);

// Get an animation index from the name.
HMODELANIM ic_GetAnimIndex(HOBJECT hObj, const char *pAnimName);
// Get the animation name from the index.
const char *ic_GetAnimName (HOBJECT hObj, HMODELANIM hAnim);

void ic_FreeString(HSTRING hString);

FileEntry* ic_GetFileList(HLTFileTree **trees, int nTrees, const char *pDirName);
void ic_FreeFileList(FileEntry *pList);

float ic_Random(LTFLOAT min, LTFLOAT max);


#endif  // __IMPL_COMMON_H__



