
// This module defines a bunch of operations on Models.
// Note: The C style in here is on its way out.  All these functions will wind
// up in the model classes.

#ifndef __MODEL_OPS_H__
#define __MODEL_OPS_H__

// Set or offset animation translations.
void model_SetAnimTrans(Model *pModel, uint32 iAnim, LTVector *pNewTrans);
void model_OffsetAnimTrans(Model *pModel, uint32 iAnim, LTVector *pNewTrans);

void model_RotateAnim(Model *pModel, uint32 iAnim, LTVector *pRot);

// Set the animation framerate.


void model_GetDimsFromBounding( LTVector *pvBoundMax, LTVector *pvBoundMin, LTVector *pvDims );


#endif  // __MODEL_OPS_H__


