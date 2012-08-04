
#ifndef __DRAWLIGHT_H__
#define __DRAWLIGHT_H__

//given a light object, this will queue it up into the appropriate lighting lists. It
//will return false if it was not able to insert it into all appropriate lists
void d3d_ProcessLight(LTObject* pObj);

#endif


