// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialMove.h
//
// PURPOSE : Definition of SpecialMove class
//
// CREATED : 02/07/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SPECIALMOVE_H__
#define __SPECIALMOVE_H__

#include "GameBase.h"
#include "ActivateTypeHandler.h"

LINKTO_MODULE( SpecialMove );

class SpecialMove : public GameBase 
{
public:	// Methods...

	SpecialMove();
	virtual ~SpecialMove();

protected:

	uint32 EngineMessageFn(uint32 messageID, void *pData, float lData);
	uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read* pMsg);

	virtual void ReadProp(const GenericPropList *pProps);
	virtual void PostReadProp(ObjectCreateStruct *pStruct);

	virtual void InitialUpdate();
	virtual uint32 OnAllObjectsCreated();

	virtual void Save( ILTMessage_Write* pMsg, uint32 nFlags );
	virtual void Load( ILTMessage_Read* pMsg, uint32 nFlags );

	virtual void WriteSFXMsg(CAutoMessage& cMsg);
	virtual uint32 GetSFXID() { return SFX_SPECIALMOVE_ID; }

	virtual void HandleSfxMessage(HOBJECT hSender, ILTMessage_Read *pMsg, uint8 nSfxId);

	void SetEnabled(bool bOn);

	virtual void OnActivated();
	virtual void OnReleased();
	virtual void OnLookedAt();

protected:

	EnumAnimProp	m_eAnimation;
	float			m_fActivateDist;
	bool			m_bOn;
	bool			m_bRadial;
	std::string		m_sActivateCommand;
	std::string		m_sReleaseCommand;
	std::string		m_sLookedAtCommand;

	// Activation support...
	CActivateTypeHandler	m_ActivateTypeHandler;	// Sets the activation text for the WorldModel 
													// (only used on ActiveWorldModels with AWM_PROP_PLAYERACTIVATE
													// and WorldModels that are attachments of ActiveWorldModels with AWM_PROP_PLAYERACTIVATE)
private:

	DECLARE_MSG_HANDLER( SpecialMove, HandleActivateMsg );
	DECLARE_MSG_HANDLER( SpecialMove, HandleOnMsg );
	DECLARE_MSG_HANDLER( SpecialMove, HandleOffMsg );
};


class SpecialMovePlugin : public IObjectPlugin
{
public:
	SpecialMovePlugin();
	virtual ~SpecialMovePlugin();

	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
		uint32* pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLength);

	virtual LTRESULT PreHook_PropChanged( 
		const	char		*szObjName,
		const	char		*szPropName,
		const	int			nPropType,
		const	GenericProp	&gpPropValue,
		ILTPreInterface		*pInterface,
		const	char		*szModifiers );

private:

	CCommandMgrPlugin			m_CommandMgrPlugin;
};


#endif  // __SPECIALMOVE_H__
