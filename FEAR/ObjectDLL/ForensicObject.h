// ----------------------------------------------------------------------- //
//
// MODULE  : ForensicObject.h
//
// PURPOSE : Game object that defines forensic areas, and properties
//
// CREATED : 11/22/04
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FORENSICOBJECT_H__
#define __FORENSICOBJECT_H__

// ----------------------------------------------------------------------- //

#include "SpecialMove.h"

// ----------------------------------------------------------------------- //

LINKTO_MODULE( ForensicObject );

// ----------------------------------------------------------------------- //

class ForensicObject : public SpecialMove
{
	public:

		ForensicObject();
		virtual ~ForensicObject();

		static uint32& GetForensicTypeMask();

	protected:

		virtual void ReadProp( const GenericPropList *pProps );
		virtual void PostReadProp(ObjectCreateStruct *pStruct);

		virtual uint32 OnAllObjectsCreated();

		virtual void Save( ILTMessage_Write* pMsg, uint32 nFlags );
		virtual void Load( ILTMessage_Read* pMsg, uint32 nFlags );

		virtual void WriteSFXMsg(CAutoMessage& cMsg);
		virtual uint32 GetSFXID() { return SFX_FORENSICOBJECT_ID; }

		virtual void HandleSfxMessage(HOBJECT hSender, ILTMessage_Read *pMsg, uint8 nSfxId);

		virtual void OnReleased();

	private:

		void		OnRequestDist(HOBJECT hObj);
		void		SetLocked( bool bLocked );
		float		CalcDistFrom(const LTVector& vPos, ENUM_NMPolyID eSourcePoly);
		uint32		GetCharTypeMask() const { return m_dwForensicTypeMask; }

		// Message Handlers...
		DECLARE_MSG_HANDLER( ForensicObject, HandleMsgLock );
		DECLARE_MSG_HANDLER( ForensicObject, HandleMsgUnlock );

	protected:

		// Used only for sending event trigger messages.
		char			m_sFarthestPointName[MAX_CS_FILENAME_LEN + 1];
		LTVector		m_vPos;
		LTVector		m_vDir;
		float			m_fMaxDistance;
		float			m_fCoreRadius;
		float			m_fObjectFOV;
		float			m_fCameraFOV;
		bool			m_bLocked;
		bool			m_bPrimary;
		HRECORD			m_rDetectionTool;
		HRECORD			m_rCollectionTool;
		HRECORD			m_rSecondaryInfo;
		std::string		m_sToolSelectCommand;
		ENUM_NMPolyID	m_eDestPoly;
		uint32			m_dwForensicTypeMask;

		CAIPathNavMesh*	m_pNMPath;
};

class CForensicObjectPlugin : public IObjectPlugin
{
public:

	virtual LTRESULT PreHook_EditStringList(const char* szRezPath,
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

// ----------------------------------------------------------------------- //

#endif//__FORENSICOBJECT_H__
