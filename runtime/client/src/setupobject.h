#ifndef __SETUPOBJECT_H__
#define __SETUPOBJECT_H__



#ifndef __CLIENT_FILEMGR_H__
#include "client_filemgr.h"
#endif

class CClientShell;
struct Sprite;
struct ObjectCreateStruct;


// Any requests from outside to create an object go thru these.
// m_pFilename and m_pSkinName from the ClientObjectCreateStruct are ignored.
class InternalObjectSetup
{
public:

					InternalObjectSetup()
					{
						m_pSetup = LTNULL;
					}


public:

	// Filename and skin names converted into FileRefs.
	FileRef		m_Filename[MAX_CHILD_MODELS];
	FileRef		m_SkinNames[MAX_MODEL_TEXTURES];
	FileRef		m_RenderStyleNames[MAX_MODEL_RENDERSTYLES];
	
	ObjectCreateStruct	*m_pSetup;
};


// Loads the sprite.
LTRESULT LoadSprite(FileRef *pFilename, Sprite **ppSprite);

// Performs extra data initialization on the object (like model and skin loading).
LTRESULT so_ExtraInit(LTObject *pObject, InternalObjectSetup *pSetup, bool bFromLocalServer);

// Performs extra data termination on the object
LTRESULT so_ExtraTerm(LTObject *pObject);

#endif  // __SETUPOBJECT_H__


