
// These routines are used to setup GenericProp structures.
#ifndef __GENERICPROP_SETUP_H__
#define __GENERICPROP_SETUP_H__


#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

struct GenericProp;

// Clear the GenericProp.
void gp_Init(GenericProp *pGeneric);

// Init with each basic type.
void gp_InitString(GenericProp *pGeneric, const char *pString);
void gp_InitVector(GenericProp *pGeneric, LTVector *pVec);
void gp_InitFloat(GenericProp *pGeneric, float val);
void gp_InitRotation(GenericProp *pGeneric, LTVector *pAngles);



#endif




