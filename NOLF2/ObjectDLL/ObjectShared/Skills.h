// ----------------------------------------------------------------------- //
//
// MODULE  : Skills.h
//
// PURPOSE : Player skills definition
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SKILLS_H__
#define __SKILLS_H__

#include "SkillsButeMgr.h"
#include "LTObjRef.h"

class CParsedMsg;

class CSkills
{
public:
	CSkills();
	virtual ~CSkills() {}

	void Init();

	//handle message indicating that a skill point reward has been received
	bool HandleRewardMessage(const CParsedMsg &cMsg);

	void HandleSkillUpdate(ILTMessage_Read *pMsg);

	void Save(ILTMessage_Write *pMsg);
	void Load(ILTMessage_Read *pMsg);

	void SetOwner(HOBJECT hOwner) {m_hOwner = hOwner;}
	void UpdateClient();

	eSkillLevel GetLevel(eSkill skl) {return m_nLevel[skl];}

	void SkillsCheat();

	void MultiplayerInit(ILTMessage_Read *pMsg);


	float	GetSkillModifier(eSkill skl, uint8 nMod); 

	void	GainIntelBonus();


private:

	uint32		m_nTotalPoints;
	uint32		m_nAvailPoints;
	eSkillLevel	m_nLevel[kNumSkills];
	LTObjRef	m_hOwner;


};

#endif
