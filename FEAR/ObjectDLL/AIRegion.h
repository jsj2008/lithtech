// ----------------------------------------------------------------------- //
//
// MODULE  : AIRegion.h
//
// PURPOSE : AI Region class definition
//
// CREATED : 09/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_REGION_H_
#define _AI_REGION_H_

#include "GameBase.h"
#include "AIEnumNavMeshTypes.h"
#include "AISounds.h"
#include "AICharacterTypeRestrictions.h"

LINKTO_MODULE( AIRegion );

// Macros.

#define MAX_VIEW_NODES	16


//-----------------------------------------------------------------

class AIRegion : public GameBase
{
	friend class CAINavMeshGen;
	public:
		typedef GameBase super;

		AIRegion();

		// Setup.

		void	SetupAIRegion( const LTVector& vCenter, float fRadius, uint32 cNMPolys, ENUM_NMPolyID* pNMPolyList );

		void	AddViewNodesToAIRegion();

		// Engine

		virtual uint32	EngineMessageFn(uint32 messageID, void *pvData, float fData);
		virtual void	ReadProp(const GenericPropList *pProps);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Char type mask.

		uint32		GetCharTypeMask() const { return m_CharTypeRestrictions.GetCharTypeMask(); }

		// Name

		const char* GetName() const { return m_strName.c_str(); }

		// Enter / Exit.

		void	EnterAIRegion( CAI* pAI );
		void	ExitAIRegion( CAI* pAI );

		// Debug rendering.

		void	DrawSelf();
		void	HideSelf();

		// Query.

		bool				ContainsNMPoly( ENUM_NMPolyID ePoly );

		// Data access.

		ENUM_AIRegionID		GetAIRegionID() const { return m_eAIRegionID; }
		int					GetNumViewNodes() const { return m_lstViewNodes.size(); }
		HOBJECT				GetViewNode( uint iNode );
		EnumAISoundType		GetLocationAISoundType() const { return m_eLocationAISoundType; }
		LTVector			GetCenter() const { return m_vCenter; }
		float				GetRadius() const { return m_flRadius; }

	protected:

		// Initialization.

		void				AddNamedObject( const char* pszObject );

		// Lit/Unlit.

		void				LightNMPolys( bool bLit );

		// Message Handlers...

		DECLARE_MSG_HANDLER( AIRegion, HandleAllMsgs );
		DECLARE_MSG_HANDLER( AIRegion, HandleLitMsg );
		DECLARE_MSG_HANDLER( AIRegion, HandleUnlitMsg );

	protected:

		std::string				m_strName;

		ENUM_AIRegionID			m_eAIRegionID;

		EnumAISoundType			m_eLocationAISoundType;

		ObjRefVector			m_lstViewNodes;
		std::string				m_strViewNodes;	

		int						m_cNMPolys;
		ENUM_NMPolyID*			m_pNMPolyList;

		LTVector				m_vCenter;
		float					m_flRadius;

		uint32					m_nOccupancy;

		std::string				m_strEnterCommand;
		std::string				m_strExitCommand;

		CAICharacterTypeRestrictions	m_CharTypeRestrictions;
};

class AIRegionPlugin : public IObjectPlugin
{
public:
	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

private: // Members...

	CAICharacterTypeRestrictionsPlugin	m_AICharTypeRestrictionsPlugin;
};

//-----------------------------------------------------------------

#endif // _AI_REGION_H_
