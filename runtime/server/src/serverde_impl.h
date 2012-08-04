
// Implements the ILTServer functions.

#ifndef __SERVERDE_IMPL_H__
#define __SERVERDE_IMPL_H__

struct ObjectLink;

void CreateLTServerDE();

ObjectList *si_CreateObjectList();
ObjectLink *si_AddObjectToList(ObjectList *pList, HOBJECT hObj);
void si_RelinquishList(ObjectList *pList);

bool si_GetPointShade(const LTVector *pPoint, LTVector *pColor);

#endif  // __SERVERDE_IMPL_H__

