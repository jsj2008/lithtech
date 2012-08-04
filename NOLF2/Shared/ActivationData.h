
// ----------------------------------------------------------------------- //
//
// MODULE  : ActivationData.h
//
// PURPOSE : Structure for handling activation data
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_ACTIVATION_DATA_H_)
#define _ACTIVATION_DATA_H_

#include "MsgIDs.h"
#include "LTObjRef.h"

class CActivationData
{
public:
	CActivationData() { Init(); }

	void Init()
	{
		m_vPos.Init();
		m_rRot.Init();
		m_nType = MID_ACTIVATE_NORMAL;
		m_hTarget = NULL;
		m_vIntersect.Init();
		m_nSurfaceType = NULL;
	}

	void Write(ILTMessage_Write *pMsg)
	{
		pMsg->WriteLTVector(m_vPos);
		pMsg->WriteCompLTRotation(m_rRot);
		pMsg->Writeuint8(m_nType);
		pMsg->WriteObject(m_hTarget);
		pMsg->WriteLTVector(m_vIntersect);
		pMsg->Writeuint8(m_nSurfaceType);
	}

	void Read(ILTMessage_Read *pMsg)
	{
		m_vPos			= pMsg->ReadLTVector();
		m_rRot			= pMsg->ReadCompLTRotation();
		m_nType			= pMsg->Readuint8();
		m_hTarget		= pMsg->ReadObject();
		m_vIntersect	= pMsg->ReadLTVector();
		m_nSurfaceType	= pMsg->Readuint8();
	}

	LTVector	m_vPos;
	LTRotation	m_rRot;
	uint8		m_nType;
	LTObjRef	m_hTarget;
	LTVector	m_vIntersect;
	uint8		m_nSurfaceType;

};


#endif