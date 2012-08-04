// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTracker.h
//
// PURPOSE : Track damage received by characters
//
// CREATED : 07/28/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DAMAGETRACKER_H__
#define __DAMAGETRACKER_H__

#include "ClientServerShared.h"
#include "Destructible.h"
#include "ModelsDB.h"

class CCharacter;

typedef struct DamageRecord_t
{
	float fTotal;
	float fLocation[HL_NUM_LOCS];

	void Clear();
} DamageRecord;



class DamageTracker
{
public:
	DamageTracker();
	virtual ~DamageTracker();

	void	Init(CCharacter* pChar);

	void	ProcessDamage(DamageStruct const& rDamage );
	void	Clear();

	bool	CheckForGib();

	void	PrepareForSeverChecks(ModelsDB::HSEVERBODY hBody);
	bool	CheckForSever(ModelsDB::HSEVERBODY hBody, bool bIsCrouched, ModelsDB::HPieceArray& hSeveredPieces);


private:
	CCharacter*		m_pCharacter;

	DamageRecord	m_Records[kNumDamageTypes];

};


#endif  // __DAMAGETRACKER_H__
