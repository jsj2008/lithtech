//-------------------------------------------------------------------
//
//   MODULE    : ILTRENDERSTYLES.H
//
//   PURPOSE   : Defines engine interface ILTRenderStyles. 
//
//   CREATED   : On 8/11/00 At 10:40:21 AM
//
//-------------------------------------------------------------------

#ifndef __ILTRENDERSTYLES_H__
#define __ILTRENDERSTYLES_H__

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#ifndef __LTRENDERSTYLE_H__
#include "ltrenderstyle.h"
#endif


#define MAX_NUM_TEXTURES_PER_PASS 4

/*!  ILTRenderStyles Interface. The RenderStyles interface allows users to create, querry,
modify, and destroy individual render styles. */
class ILTRenderStyles : public IBase {
public:
    interface_version(ILTRenderStyles, 0);

    //******** ILTRenderStyles Core Member Functions *********//

/*! 
Duplicate a render style (maybe you want to change it just for a certain object - you better dup it first, then change it, and set it).
\param pRendStyle A \b CRenderStyle pointer to the render style to be duplicated.
\return  A \b CRenderStyle pointer to the duplicate render style.

Used For: Rendering.    
*/
    virtual CRenderStyle *DuplicateRenderStyle(CRenderStyle* pRendStyle) = 0;

/*! 
Create a render style. You can then set all it's internals yourself.
\param bSetToDefault A Boolean value indicating whether or not to set default values for the returned \b CRenderStyle.
\return A \b CRenderStyle pointer to the newly created render style.

Used For: Rendering.    
*/
    virtual CRenderStyle *CreateRenderStyle(bool bSetToDefault = true) = 0;

/*! 
Load a render style (ltb file) and create a render object for it. 
\param szFilename A pointer to a string containing the path to the render style file.
\return A \b CRenderStyle pointer to the new render style.

Used For: Rendering.
*/
    virtual CRenderStyle *LoadRenderStyle(const char* szFilename) = 0;

/*! 
Free a render style - you no longer need it (may or may not be internally freed - depending on the ref count). 
\param pRendStyle A \bCRenderStyle pointer to the render style to be freed.

Used For: Rendering.
*/
    virtual void FreeRenderStyle(CRenderStyle* pRendStyle) = 0;
};

#endif // __ILTRENDERSTYLES_H_

