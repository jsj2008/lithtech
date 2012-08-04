// ----------------------------------------------------------------------- //
//
// MODULE  : AIConfig.h
//
// PURPOSE : AIConfig aggregate object - Definition
//           AIConfig wraps a WorldEdit User-Interface around commands
//           that are frequently set as INitialCommands on AI.
//
// CREATED : 01/16/04
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_CONFIG_H__
#define __AI_CONFIG_H__

//
// Includes...
//

	#include "iaggregate.h"


//
// Defines...
//
	
#define ADD_AICONFIG_AGGREGATE( Group ) \
	PROP_DEFINEGROUP( ConfigureAI, Group, "A group of properties used to setup an AI." ) \
		ADD_STRINGPROP_FLAG( Team, "NoTeam", Group|PF_STATICLIST, "This is a dropdown that allows you to set which team this AI will belong.") \
		ADD_STRINGPROP_FLAG( Alignment, "Default", Group|PF_STATICLIST, "The alignment of the AI.") \
		ADD_STRINGPROP_FLAG( Awareness, "Relaxed", Group|PF_STATICLIST, "The awareness of the AI when entering the level.") \
		ADD_STRINGPROP_FLAG( AwarenessMod, "None", Group|PF_STATICLIST, "The awareness modifier of the AI when entering the level.") \
		ADD_BOOLPROP_FLAG( SeekEnemy, false, Group, "If true the AI will immediately seek out an enemy.") \
		ADD_STRINGPROP_FLAG( InitialNode, "", Group|PF_OBJECTLINK, "The node that defines the AIs initial behavior.") \
		ADD_STRINGPROP_FLAG( BehaviorNode, "", Group|PF_OBJECTLINK, "The node that defines the AIs behavior.") \
		ADD_STRINGPROP_FLAG( VehicleObject, "", Group|PF_OBJECTLINK, "The vehicle object that the AI should be attached to when entering the level.") \
		ADD_STRINGPROP_FLAG( VehicleType, "None", Group|PF_STATICLIST, "The vehicle type that the AI should be attached to when entering the level.") \
		ADD_REALPROP_FLAG( VehicleYOffset, 0.f, Group, "The Y-Offset to use when attaching the AI to the vehicle.") \
		ADD_STRINGPROP_FLAG( VehicleKeyframeToRigidBody, "", Group|PF_OBJECTLINK, "The vehicle's keyframer to rigid body object.") \
		ADD_STRINGPROP_FLAG( EntranceAnim, "", Group, "The name of the animation to play when the AI enters the level.") \
		ADD_LONGINTPROP_FLAG( DamagedPlayerNumActivations, 1, Group, "This value sets how many times the DamagedPlayer command can be sent. Setting this value to 0 allows the command to be sent an infinite number of times." ) \
		ADD_COMMANDPROP_FLAG( DamagedPlayerCommand, "", Group|PF_NOTIFYCHANGE, "A command that is sent when the AI damages the player." )


//
// Forwards...
//

class	CAI;


// Class Definition

class CAIConfig : public IAggregate
{
	public :

		CAIConfig();
		~CAIConfig();
		
		void			ConfigureAI( CAI* pAI );

		// Engine calls to the aggregate...

		uint32			EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float lData);

		bool			ReadProp(LPBASECLASS pObject, const GenericPropList *pProps);

		void			Save(ILTMessage_Write *pMsg);
		void			Load(ILTMessage_Read *pMsg);

		void			EnableAIConfig( bool bEnable ) { m_bEnabled = bEnable; }

	private :

		// Private Member functions

		bool			Init( HOBJECT hObject );
		void			QueueNodeMessage( CAI* pAI, const char* pszNodeName );

		// Member Variables

		LTObjRef			m_hObject;		// The owning object of this aggregate

		int					m_nTeamID;
		std::string			m_strAlignment;
		std::string			m_strAwareness;
		std::string			m_strAwarenessMod;
		bool				m_bSeekEnemy;
		std::string			m_strInitialNode;
		std::string			m_strBehaviorNode;
		std::string			m_strVehicleObject;
		std::string			m_strVehicleType;
		float				m_fVehicleYOffset;
		std::string			m_strVehicleKeyframeToRigidBody;
		std::string			m_strEntranceAnim;

		bool				m_bEnabled;
		bool				m_bSensesOn;
		std::string			m_strCmdDamagedPlayer;				// Command to be dispatched.
		int					m_nDamagedPlayerActivationCount;	// How many times the DamagedPlayer message can be fired (<= 0 is infinite)

		DECLARE_MSG_HANDLER( CAIConfig, HandleTeamMsg );
};

class CAIConfigPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList(const char* szRezPath,
												const char* szPropName,
												char** aszStrings,
												uint32* pcStrings,
												const uint32 cMaxStrings,
												const uint32 cMaxStringLength);


		virtual LTRESULT PreHook_PropChanged(const char *szObjName,
											const char *szPropName, 
											const int  nPropType,
											const GenericProp &gpPropValue,
											ILTPreInterface *pInterface,
											const char *szModifiers);

	private:
		CCommandMgrPlugin	m_CommandMgrPlugin;
};

#endif //__AI_CONFIG_H__
