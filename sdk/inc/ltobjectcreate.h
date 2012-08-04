//////////////////////////////////////////////////////////////////////////////
// Object creation structure declaration and related constants and macros

#ifndef __LTOBJECTCREATE_H__
#define __LTOBJECTCREATE_H__

#include "ltproperty.h"

/*!
This structure is used when creating objects.  When you want to
create an object, you call ILTServer::CreateObject with one of these.
The structure you pass in is passed to the object's PostPropRead function,
where it can override whatever it wants to.
*/
struct ObjectCreateStruct {
    ObjectCreateStruct() {
        Clear();
    }

    void Clear() 
	{

        m_ObjectType		= 0;
        m_ContainerCode		= 0;
        m_Flags				= 0;
        m_Flags2			= 0;
		m_nRenderGroup		= 0;
		m_GlobalForceOverride.Init();
        m_Pos.Init();
        m_Scale.Init(1.0f, 1.0f, 1.0f);
        m_Rotation.Init();
        m_UserData			= 0;
        m_hClass			= 0;
        m_Filename[0]		= 0;

        uint32 i;
        for (i=0; i < MAX_MODEL_TEXTURES; i++) {
            m_SkinNames[i][0] = 0;
        }
        for (i=0; i < MAX_MODEL_RENDERSTYLES; i++) {
            m_RenderStyleNames[i][0] = 0;
        }
		
		for (i=1; i < MAX_CHILD_MODELS ; i++ )
		{
			m_Filenames[i][0]= 0;
		}

        m_Name[0]			= 0;
        m_NextUpdate		= 0.0f;

		m_cProperties.Reset();
    }

/*!
Main info.
*/
    uint16 m_ObjectType;

/*!
Container code if it's a container.  It's in here because
you can only set it at creation time.
*/
    uint16 m_ContainerCode;

    uint32 m_Flags;
    uint32 m_Flags2;

	// The render group that this object will belong in, if it should belong in the default
	// render group, simply don't change it
	uint8  m_nRenderGroup;

	// Set this in when the object is created to override the global force for this object.
	// If all vectors are 0 the global force will be used.
	LTVector m_GlobalForceOverride;

    LTVector m_Pos;
    LTVector m_Scale;
    LTRotation m_Rotation;


//! User data.
    uint32 m_UserData;

/*!
NOTE: this is ONLY used during OnPreCreate processing, and is filled in by the engine.
*/
    HCLASS m_hClass;

/*!
This is the model, sound, or sprite filename.
It also can be the WorldModel name.
This can be zero-length when it's not needed.
*/
	union {

    char m_Filename[MAX_CS_FILENAME_LEN+1];  //!
	char m_Filenames[MAX_CHILD_MODELS][MAX_CS_FILENAME_LEN +1];

	};

    union {
/*!
This can be zero-length. If you set it for an
OT_MODEL, it's the skin filename.
*/
        char m_SkinName[MAX_CS_FILENAME_LEN+1];  //!
        char m_SkinNames[MAX_MODEL_TEXTURES][MAX_CS_FILENAME_LEN+1];
    };

    union {
/*!
This can be zero-length. If you set it for an
OT_MODEL, it's the skin filename.
*/
        char m_RenderStyleName[MAX_CS_FILENAME_LEN+1];   //!
        char m_RenderStyleNames[MAX_MODEL_RENDERSTYLES][MAX_CS_FILENAME_LEN+1];
    };

/*!
Server only info.
*/

//! This object's name.
    char m_Name[MAX_CS_FILENAME_LEN+1];

//! This will be the object's starting NextUpdate.
    float m_NextUpdate;

//! List of properties
	GenericPropList m_cProperties;
};


/*!
Initialize an ObjectCreateStruct.
*/
#define INIT_OBJECTCREATESTRUCT(theStruct) \
{ \
    (theStruct).Clear();\
}

#endif //__LTOBJECTCREATE_H__
