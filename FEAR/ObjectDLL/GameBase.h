// ----------------------------------------------------------------------- //
//
// MODULE  : GameBase.h
//
// PURPOSE : Game base object class definition
//
// CREATED : 10/8/99
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_BASE_H__
#define __GAME_BASE_H__

#include "ltengineobjects.h"
#include "iobjectplugin.h"
#include "ltobjref.h"
#include "ServerUtilities.h"

class CTransitionAggregate;
class CParsedMsg;

LINKTO_MODULE( GameBase );

class GameBase : public BaseClass, public ILTObjRefReceiver
{
	public :

		GameBase(uint8 nType=OT_NORMAL);
		virtual ~GameBase();

		bool	CanTransition() const { return bool(!!m_pTransAgg); }

		virtual void AddToObjectList( ObjectList *pObjList, eObjListControl eControl = eObjListNODuplicates );

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier * /*pRef*/, HOBJECT /*hObj*/ ) { }

		// These functions should be overriden if the class recieves messages that may be blocked...

		// Tests to see if the object is still being scripted from a message...
		virtual bool	IsScripted( ) { return false; }

		// Lets the object know its about to be scripted...
		virtual void	SetScripted( ) { }

		// Do some cleanup once an object as finished scripting...
		virtual void	ScriptCleanup( ) { }

		// The object was interrupted while scripting so make sure it can exit gracefully...
		virtual void	InterruptScript( ) { }


	protected :

		virtual uint32 EngineMessageFn(uint32 messageID, void *pData, float fData);

		// Cache the original flags so they can be controlled through messages...
		uint32  m_dwOriginalFlags;

		// Cache the shadow LOD so it can be controlled through messages...
		EEngineLOD	m_eOriginalShadowLOD;


		enum eUpdateControl
		{
			eControlDeactivation=0,
			eControlUpdateOnly
		};

		virtual void SetNextUpdate(float fDelta, eUpdateControl eControl=eControlDeactivation);

		virtual void MakeTransitionable( );
		virtual void DestroyTransitionAggregate( );

		
	private :

		bool	ReadProp(ObjectCreateStruct *pStruct);

		// Handle processing properties after the object has been created...
		void	ObjectCreated( const GenericPropList *pPropList );

		void	HandleModelString( ArgList* pArgList );

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

		CTransitionAggregate	*m_pTransAgg;


		// Message Handlers...

		DECLARE_MSG_HANDLER( GameBase, HandleVisibleMsg );
		DECLARE_MSG_HANDLER( GameBase, HandleSolidMsg );
		DECLARE_MSG_HANDLER( GameBase, HandleHiddenMsg );
		DECLARE_MSG_HANDLER( GameBase, HandleRemoveMsg );
		DECLARE_MSG_HANDLER( GameBase, HandleCastShadowMsg );
		DECLARE_MSG_HANDLER( GameBase, HandleMoveMsg );
		DECLARE_MSG_HANDLER( GameBase, HandleSetRotationMsg );
		DECLARE_MSG_HANDLER( GameBase, HandleCopyXFormMsg );
};

class CGameBasePlugin : public IObjectPlugin
{
public:

	virtual LTRESULT PreHook_EditStringList( const char *szRezPath,
		const char *szPropName,
		char **aszStrings,
		uint32 *pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLength );
};

#endif  // __GAME_BASE_H__

// EOF
