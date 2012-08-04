// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisFuncs.h
//
// PURPOSE : Misc functions for creating debris
//
// CREATED : 6/29/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DEBRIS_FUNCS_H__
#define __DEBRIS_FUNCS_H__

// Includes...

#include "ltbasetypes.h"
#include "DebrisMgr.h"
#include "SurfaceFunctions.h"
#include "iobjectplugin.h"

#ifndef __PSX2
#include <memory.h>  // for memset
#endif

// Use ADD_DEBRISTYPE_PROPERTY() in your class definition to enable
// the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_DEBRISTYPE_PROPERTY()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_DEBRISTYPE_PROPERTY(flags) \
	ADD_STRINGPROP_FLAG(DebrisType, "", PF_STATICLIST | (flags))

#ifndef __PSX2
class CDebrisPlugin : public IObjectPlugin
{

  public:

    virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

};
#endif

struct CLIENTDEBRIS
{
    CLIENTDEBRIS();

    LTRotation   rRot;
    LTVector     vPos;
    uint8       nDebrisId;
};

inline CLIENTDEBRIS::CLIENTDEBRIS()
{
	vPos.Init();
	rRot.Init();
	nDebrisId = DEBRISMGR_INVALID_ID;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisProperties()
//
//	PURPOSE:	Determine the debris properties (This should only be
//				called during an object's ReadProp function if the object
//				added the ADD_DEBRISTYPE_PROPERTY macro).
//
// ----------------------------------------------------------------------- //

void GetDebrisProperties(uint8 & nDebrisId);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreatePropDebris()
//
//	PURPOSE:	Create client-side debris for props...
//
// ----------------------------------------------------------------------- //

void CreatePropDebris( const LTVector &vPos, const LTVector &vDir, uint8 nDebrisId );


#endif // __DEBRIS_FUNCS_H__