// ----------------------------------------------------------------------- //
//
// MODULE  : WorldModel.h
//
// PURPOSE : The basic WorldModel object
//
// CREATED : 5/9/01
//
// (c) 2001-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WORLD_MODEL_H__
#define __WORLD_MODEL_H__

//
// Includes...
// 

#include "GameBase.h"
#include "DestructibleWorldModel.h"
#include "ltobjref.h"
#include "ActivateTypeHandler.h"

LINKTO_MODULE( WorldModel );

//
// Defines...
//

	// Blend modes...

	#define	WM_BLEND_NONE			0
	#define	WM_BLEND_ADDITIVE		1
	#define	WM_BLEND_TRANSLUCENT	2
	#define	WM_BLEND_MAXMODES		2

//
// Structs...
//

class WorldModel : public GameBase
{
	public: // Methods...
		DEFINE_CAST( WorldModel );

		WorldModel( );
		virtual ~WorldModel( );

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		void	SetActivateParent( HOBJECT hParent );

		bool	IsDestroyed() 
		{
			return (m_DamageWorldModel.IsDead()); 
		}

		// Attach or detach an object from the world model.  These 
		// were exposed as using a message to perform this action 
		// results in a one frame delay.  This delay is problematic if
		// the WorldModel is moving, as the relative offset/transform 
		// may change over this frame.

		HOBJECT	AttachObject( HOBJECT hObj );
		void	DetachObject( HOBJECT hObj );

	protected: // Members...

		CDestructibleWorldModel	m_DamageWorldModel;		// The Destructable aggregate

		enum WorldModelFlags
		{
			kWorldModelFlag_IsKeyframed			= (1<<0),
			kWorldModelFlag_StartHidden			= (1<<1),
			kWorldModelFlag_RemoveAttachments	= (1<<2), // Remove the attachments, or destroy them
			kWorldModelFlag_CanActivate			= (1<<3), // Can we be activated
			kWorldModelFlag_CastShadow			= (1<<4), // Will this object initially cast a shadow.
			kWorldModelFlag_InheritActivationData = (1<<5), // If this WorldModel is attached to another, should it inherit the activation data of the WorldModel it is attached to...
			kWorldModelFlag_Initialized			= (1<<6),
			kWorldModelFlag_PlayerInteract		= (1<<7),
		};
        uint8 m_WorldModelFlags;

		// Vars for handling attachments...

		std::string				m_sAttachments;			// Names of objects to attach
		LTVector				m_vAttachDir;			// Direction to look for attachments
		ObjRefNotifierList		m_AttachmentList;		// List of Objects attached to us
		ObjRefNotifierList		m_AttachMsgObjList;		// List of objects attached to us via the ATTACH message
		LTObjRefNotifier		m_hAttachDirObj;		// Object attached via the AttachDir

		// Activation support...
		CActivateTypeHandler	m_ActivateTypeHandler;	// Sets the activation text for the WorldModel 
														// (only used on ActiveWorldModels with AWM_PROP_PLAYERACTIVATE
														// and WorldModels that are attachments of ActiveWorldModels with AWM_PROP_PLAYERACTIVATE)

		LTObjRef				m_hActivateParent;		// Relay any Activate messages to our parent

		
		float					m_fAlpha;				// Alpha value of the worldmodel for use in transparency
		float					m_fAlphaScale;			// Current scale of the alpha value. This is used to allow the alpha to be scaled through messages

		
	protected: // Methods...

		//called to update the alpha of this object. This will not perform it unless it is a translucent
		//world model
		void			UpdateAlpha();
		

		// Engine methods

		virtual uint32	EngineMessageFn( uint32 messageID, void *pData, float fData );

		// Engine message handelers

		virtual void	OnObjectCreated( );
		virtual	void	OnEveryObjectCreated( );
		virtual void	OnUpdate( const double &fCurTime );
		virtual void	OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void	OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags );

		virtual void	ReadProps( const GenericPropList *pProps );
		virtual void	PostReadProp( ObjectCreateStruct *pOCS );
	
	private: // Methods...

		// Attachment methods...

		void	RemoveAtachments( );
		void	AttachFromMsg( const CParsedMsg &cMsg, const char *szClass );

		// Message Handlers...

		DECLARE_MSG_HANDLER( WorldModel, HandleAttachMsg );
		DECLARE_MSG_HANDLER( WorldModel, HandleDetachMsg );
		DECLARE_MSG_HANDLER( WorldModel, HandleAttachClassMsg );
		DECLARE_MSG_HANDLER( WorldModel, HandleDestroyMsg );
		DECLARE_MSG_HANDLER( WorldModel, HandleActivateMsg );
		DECLARE_MSG_HANDLER( WorldModel, HandleRigidBodyMsg );
		DECLARE_MSG_HANDLER( WorldModel, HandleScaleAlphaMsg );
		DECLARE_MSG_HANDLER( WorldModel, EmptyHandler );
};

// WorldEdit plugin class

class CWorldModelPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList( const char *szRezPath,
												 const char *szPropName,
												 char **aszStrings,
												 uint32 *pcStrings,
												 const uint32 cMaxStrings,
												 const uint32 cMaxStringLength );

		virtual LTRESULT PreHook_PropChanged( const	char		*szObjName,
											  const	char		*szPropName,
											  const	int			nPropType,
											  const	GenericProp	&gpPropValue,
													ILTPreInterface	*pInterface,
											  const char		*szModifiers );

	protected:

		CDestructibleWorldModelPlugin m_DamageWorldModelPlugin;

};

#endif // __WORLD_MODEL_H__
