// ----------------------------------------------------------------------- //
//
// MODULE  : TargetMgr.h
//
// PURPOSE : Definition of class to handle tracking whjat the player is aimed at.
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TARGET_MGR_H__
#define __TARGET_MGR_H__

#include "ActivationData.h"
#include "DamageTypes.h"


const uint8	kMaxTargetStringLen = 64;
const uint16 kMaxDebugStringLen = 1024;
const float kMaxDistance = 100000.0f;

class CTargetMgr 
{
public:
	CTargetMgr();
	virtual ~CTargetMgr();

	virtual void Update();

	virtual HOBJECT			GetTargetObject()	const	{return m_hTarget;}
	virtual float			GetTargetRange()	const	{return m_fTargetRange;}
	virtual const char*		GetTargetStringID()	const	{return m_szStringID;}
	virtual	const wchar_t*	GetTargetString()	const	{return &m_wszString[0];}
	uint8					GetTargetTeam()		const	{return m_nTargetTeam;}
	virtual HOBJECT			GetEnemyTarget()	const;

	//instersect info to be sent to server
	virtual CActivationData GetActivationData() const {return m_ActivationData;}
	virtual bool			IsActivationType( uint8 nType ) const { return ( m_ActivationData.m_nType == nType ); }

	virtual bool	CanActivateTarget()				{return m_bCanActivate;}
	virtual bool	IsTargetInRange();

	//make sure specified object is considered a valid target regardless of it's current flags
	virtual void LockTarget(HOBJECT hTarget) { m_hLockedTarget = hTarget; }
	virtual HOBJECT GetLockedTarget()	const	{return m_hLockedTarget;}

protected:
	
	LTObjRef	m_hTarget;
	wchar_t		m_wszString[kMaxTargetStringLen];

	void		CheckForIntersect(float &fDistAway);

	void		SetTargetStringID(const char* szID);
		
	LTObjRef	m_hLockedTarget;
	float		m_fTargetRange;

	uint8		m_nTargetTeam;

	const char*	m_szStringID;

	bool		m_bCanActivate;
	bool		m_bFirstUpdate;

	//intersect info
	CActivationData m_ActivationData;

	//check for character FX within an are around a point
	bool CheckForCharacters(LTVector vObjPos,LTVector vDims, uint8 nId);

	void FirstUpdate();
	void ClearTargetInfo();
};

#endif