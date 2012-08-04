// ----------------------------------------------------------------------- //
//
// MODULE  : WorldModel.h
//
// PURPOSE : The basic WorldModel object
//
// CREATED : 5/9/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WORLD_MODEL_H__
#define __WORLD_MODEL_H__

//
// Includes...
// 

#include "GameBase.h"
#include "DestructibleModel.h"
#include "LTObjRef.h"
#include "ActivateTypeMgr.h"

LINKTO_MODULE( WorldModel );

//
// Defines...
//
class CServerMark;
struct CLIENTWEAPONFX;

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

		WorldModel( );
		virtual ~WorldModel( );

		LTRotation	GetWorldModelRotation( );

		static	LTBOOL RelayTestMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
		{
			return LTTRUE;
		}

		bool CreateServerMark(CLIENTWEAPONFX & theStruct);

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		CActivateTypeHandler* GetActivateTypeHandler() { return &m_ActivateTypeHandler; }

		void	SetActivateParent( HOBJECT hParent );

	protected: // Members...
		
		CDestructibleModel		m_DamageModel;			// The Destructable aggregate

		ObjRefNotifierList		m_MarkList;				// List of server marks... (bulletholes)
		bool					m_bIsKeyframed;			// Is the WorldModel keyframed?
		bool					m_bStartHidden;
		
		// Vars for handling attachments...

		HSTRING					m_hstrAttachments;		// Names of objects to attach
		LTBOOL					m_bRemoveAttachments;	// Remove the attachments, or destroy them
		LTVector				m_vAttachDir;			// Direction to look for attachments
		ObjRefNotifierList		m_AttachmentList;		// List of Objects attached to us
		ObjRefNotifierList		m_AttachMsgObjList;		// List of objects attached to us via the ATTACH message
		LTObjRefNotifier		m_hAttachDirObj;		// Object attached via the AttachDir

		// Disturbance stimulus vars...

		uint32					m_nDestroyAlarmLevel;	// How alarming is destroying.
		LTFLOAT					m_fStimRadius;			// How far away is the stimulus noticeable.

		// [RP] HACK - Deals with the problem of double rotation on OT_WORLMODELS!!
		LTRotation				m_hackInitialRot;
		
		CActivateTypeHandler	m_ActivateTypeHandler;	// Sets the activation text for the WorldModel 
														// (only used on ActiveWorldModels with AWM_PROP_PLAYERACTIVATE
														// and WorldModels that are attachments of ActiveWorldModels with AWM_PROP_PLAYERACTIVATE)

		LTObjRef				m_hActivateParent;		// Relay any Activate messages to our parent

		bool					m_bCanActivate;			// Can we be activated


	protected: // Methods...

		// Engine methods

		virtual uint32	EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );

		// Engine message handelers

		virtual void	OnObjectCreated( );
		virtual	void	OnEveryObjectCreated( );
		virtual void	OnUpdate( const LTFLOAT &fCurTime );
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);
		virtual void	OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void	OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags );

		virtual void	ReadProps( ObjectCreateStruct *pOCS );
		virtual void	PostReadProp( ObjectCreateStruct *pOCS );
	
	private: // Methods...

		// Attachment methods...

		HOBJECT	AttachObject( HOBJECT hObj );
		void	DetachObject( HOBJECT hObj );
		void	RemoveAtachments( );
		void	HandleAttachMsg( const CParsedMsg &cMsg, const char *szClass );
		void	HandleDetachMsg( const CParsedMsg &cMsg );

		void	SetWorldModelRotation( LTRotation &rNewRot );
		bool	AttachServerMark( CServerMark& mark, CLIENTWEAPONFX & theStruct);

		void	AddMarkToList( HOBJECT hObj );
		void	RemoveMarkFromList( HOBJECT hObj );

		void	HandleActivateMsg( HOBJECT hSender, const CParsedMsg &cMsg );
};

// DEdit plugin class

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

		CDestructibleModelPlugin m_DamageModelPlugin;

};

#endif // __WORLD_MODEL_H__