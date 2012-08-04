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

class CTargetMgr 
{
public:
	CTargetMgr();
	virtual ~CTargetMgr();

	virtual void Update();

	virtual HOBJECT GetTargetObject()		const	{return m_hTarget;}
	virtual float GetTargetRange()			const	{return m_fTargetRange;}
	virtual uint16	GetTargetStringID()		const	{return m_nString;}
	virtual	const char*	GetTargetString()	const	{return &m_szString[0];}
	virtual	const char*	GetDebugString()	const	{return &m_szDebugString[0];}
	uint8			GetTargetTeam()			const	{return m_nTargetTeam;}

	//instersect info to be sent to server
	virtual CActivationData GetActivationData() const {return m_ActivationData;}

	virtual	bool	IsSearchTarget()				{return m_bSearchTarget;}
	virtual	bool	IsMoveTarget()					{return m_bMoveTarget;}
	virtual bool	CanActivateTarget()				{return m_bCanActivate;}
	virtual bool	IsTargetInRange();

	


	virtual bool IsGadgetActivatable(HOBJECT hObj);
	virtual bool IsTargetGadgetActivatable();

	//returns the damage type of the gadget required to disable the current target
	DamageType	 RequiredGadgetDamageType();

	//make sure specified object is considered a valid target regardless of it's current flags
	virtual void LockTarget(HOBJECT hTarget) { m_hLockedTarget = hTarget; }
	virtual HOBJECT GetLockedTarget()	const	{return m_hLockedTarget;}

protected:
	
	HOBJECT		m_hTarget;
	char		m_szString[kMaxTargetStringLen];

	void		CheckForIntersect(float &fDistAway);

	void		SetTargetStringID(uint16 nID);
	void		SetGadgetTarget( bool bDisabling );
		
	
	HOBJECT		m_hLockedTarget;
	float		m_fTargetRange;

	uint8		m_nTargetTeam;

	char		m_szDebugString[kMaxDebugStringLen];
	uint16		m_nString;

	bool		m_bSearchTarget;
	bool		m_bMoveTarget;
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