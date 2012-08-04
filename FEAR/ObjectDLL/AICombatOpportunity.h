// ----------------------------------------------------------------------- //
//
// MODULE  : AICombatOpportunity.h
//
// PURPOSE : 
//
// CREATED : 6/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AICOMBATOPPORTUNITY_H_
#define _AICOMBATOPPORTUNITY_H_

#include "GameBase.h"
#include "AIStimulusMgr.h"
#include "NamedObjectList.h"

LINKTO_MODULE( AICombatOpportunity );


// ----------------------------------------------------------------------- //
//
//	CLASS:		AICombatOpportunity
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AICombatOpportunity : public GameBase
{
	typedef GameBase super;

	enum Const
	{
		kConst_EnemyAreaCount					= 10,
		kConst_AIAreaCount						= 10,
		kConst_ActionNodeCount					= 10,
		kConst_CombatOpportunityViewNodeCount	= 10
	};

public:
	static AICombatOpportunity* HandleToObject(HOBJECT);

	enum kStatusFlags
	{
		kStatusFlag_None					= 0,
		kStatusFlag_AIPosition				= (1<<0),
		kStatusFlag_ThreatPosition			= (1<<1),
		kStatusFlag_QueryObjectInEnemyArea	= (1<<2),
		kStatusFlag_All						= 0xFFFFFFFF,
	};

	// Ctor/Dtor

	AICombatOpportunity();
	virtual ~AICombatOpportunity();

	// Name

	const char* GetName() const { return m_strName.c_str(); }

	// Save/Load

	void Load(ILTMessage_Read *pMsg);
	void Save(ILTMessage_Write *pMsg);

	// Engine

	uint32 EngineMessageFn(uint32 messageID, void *pData, float fData);
	void ReadProp(const GenericPropList *pProps);
	void ConvertObjectNamesToObjectHandles();

	// Lock/Unlock

	void LockCombatOpportunity(HOBJECT hAI);
	void UnlockCombatOpportunity(HOBJECT hAI);
	bool IsCombatOpportunityLocked();
	HOBJECT GetLockingAI();

	// Ally speaker

	void	SetAllySpeaker( HOBJECT hAlly ) { m_hAllySpeaker = hAlly; }
	HOBJECT GetAllySpeaker() const { return m_hAllySpeaker; }

	// Queries

	bool IsValid(HOBJECT hQueryObject, HOBJECT hThreat, uint32 dwStatusFlags);
	HOBJECT GetRangedTargetObject();
	EnumAISoundType GetAllySound() const { return m_eAllySound; }

private:
	PREVENT_OBJECT_COPYING(AICombatOpportunity);

	// Calculates the stimulus radius.  AIs must be inside this radius to 
	// initially sense a AICombatOpportunity.  This is an optimization to
	// prevent AIs from attempting to use AICombatOpportunity objects on
	// the other side of the level.
	float	CalculateStimulusRadius();

	// Handles toggling this object between enabled and disabled.  
	void	SetEnable(bool bEnable);

	// Name of the CombatOpportunity object in WorldEdit.
	std::string		m_strName;

	// This object is the object an AI attempts to shoot at when using this
	// CombatOpportunity.  If this handle is NULL, the AICombatOpportunity 
	// may only be activated through ActionNodes.
	std::string			m_strRangedTargetObject;
	LTObjRef			m_hRangedTargetObject;

	// Contains a list of all EnemyAreas, used defined by this 
	// CombatOpportunity.  The enemy must be 'inside' at least
	// one.  All objects contained should be AIRegion and 
	// AICombatOpportunityRadius objects. If none are listed, 
	// the enemies position is unconstrained.
	CNamedObjectList	 m_EnemyAreaObjects;

	// Contains a list of all AIAreas, used to define where the AI
	// may be to use this CombatOpportunity via ranged attack.  If
	// none are listed, the AIs position is unconstrained and he may
	// attempt to shoot the RangedTargetObject from anywhere.
	CNamedObjectList	 m_AIAreaObjects;

	// Contains a list of AINodeCombatOpportunity instances an AI may use
	// to trigger this CombatOpportunity.
	CNamedObjectList	 m_ActionNodeObjects;

	// Contains a list of AINodeCombatOpportunityView instances an AI may 
	// use gain line of sight on the RangedTargetObject.  The AI will only 
	// consider using this nodes if the AIs' position is valid given the 
	// AIArea list.
	CNamedObjectList	 m_CombatOpportunityViewNodeObjects;

	// Contains used to register a stimulus.  This radius is a fit around the
	// objects that define the EnemyArea, AIArea, ActionNodes' radius and
	// the CombatOpportunityView nodes radius.
	float				m_flStimulusRadius;

	// Contains the command dispatched when the AICombatOpportunity object is 
	// activated.  
	std::string			m_sActivateCmd;

	// Handle to the AI who is currently locking the CombatOpportunity.
	LTObjRef			m_hLockingAI;

	// m_bActivated is true if the CombatOpportunity has been activated, false 
	// if it has not.
	bool				m_bActivated;

	// True if enabled, false if disabled.
	bool				m_bEnabled;

	// Registration of the CombatStimulus ID of the stimulus AIs uses to sense
	// this object.  When disabled, this stimulus ID should be 
	EnumAIStimulusID	m_eStimulusID;

	// AISound played by ally, ordering AI to shoot a combat opportunity target object.
	EnumAISoundType		m_eAllySound;

	// Ally announcing the combat opportunity.
	// AI may not target it until ally is finished speaking.
	LTObjRef			m_hAllySpeaker;

	// flag indicating whether or not we were created from a save game. 
	// This property is not serialized.
	bool				m_bCreatedFromSave;

	// Message Handlers...

	DECLARE_MSG_HANDLER( AICombatOpportunity, HandleDisableMsg );
	DECLARE_MSG_HANDLER( AICombatOpportunity, HandleEnableMsg );
	DECLARE_MSG_HANDLER( AICombatOpportunity, HandleActivateMsg );
};


class AICombatOpportunityPlugin : public IObjectPlugin
{
public:
	LTRESULT PreHook_PropChanged( const char *szObjName,
												const char *szPropName, 
												const int  nPropType, 
												const GenericProp &gpPropValue,
												ILTPreInterface *pInterface,
												const char *szModifiers );
	
	LTRESULT PreHook_EditStringList( const char* szRezPath, 
												const char* szPropName, 
												char** aszStrings, 
												uint32* pcStrings, 
												const uint32 cMaxStrings, 
												const uint32 cMaxStringLength );

private:
	CCommandMgrPlugin	m_CommandMgrPlugin;
};

#endif // _AICOMBATOPPORTUNITY_H_
