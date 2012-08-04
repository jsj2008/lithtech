// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTracker.cpp
//
// PURPOSE : Track damage received by characters
//
// CREATED : 07/28/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "DamageTracker.h"
#include "Character.h"

void DamageRecord::Clear()
{
	fTotal = 0.0;
	memset(fLocation, 0, sizeof(fLocation) );
}

// ----------------------------------------------------------------------- //
//	Constructor/Destructor
// ----------------------------------------------------------------------- //
DamageTracker::DamageTracker() :
	m_pCharacter (NULL)
{
}

DamageTracker::~DamageTracker()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTracker::Init()
//
//	PURPOSE:	associate the tracker with a particular character
//
// ----------------------------------------------------------------------- //
void DamageTracker::Init(CCharacter* pChar)
{
	m_pCharacter = pChar;
	Clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTracker::Clear()
//
//	PURPOSE:	clear out accumulated damage
//
// ----------------------------------------------------------------------- //
void DamageTracker::Clear()
{
	for (uint32 n = 0; n < kNumDamageTypes; ++n)
	{
		m_Records[n].Clear();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTracker::ProcessDamage()
//
//	PURPOSE:	record incoming damage
//
// ----------------------------------------------------------------------- //
void DamageTracker::ProcessDamage( DamageStruct const& rDamage )
{
	if (rDamage.eType == DT_INVALID)
	{
		return;
	}

	m_Records[rDamage.eType].fTotal += rDamage.fDamage;

	if (!m_pCharacter) return;

	HitLocation eLoc = HL_UNKNOWN;
	if (!IsRandomSeverType(rDamage.eType))
	{
		ModelsDB::HNODE hModelNode = m_pCharacter->GetModelNodeLastHit();
		if( hModelNode )
		{
			eLoc = g_pModelsDB->GetNodeLocation( hModelNode );
		}
	}
	m_Records[rDamage.eType].fLocation[eLoc] += rDamage.fDamage;


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTracker::CheckForGib()
//
//	PURPOSE:	test accumulated damage for gibs
//
// ----------------------------------------------------------------------- //
bool DamageTracker::CheckForGib()
{
	for (uint32 n = 0; n < kNumDamageTypes; ++n)
	{
		DamageType eDT = static_cast<DamageType>(n);
		float fChance = GetGibChance(eDT,m_Records[eDT].fTotal);
		if (GetRandom(0.0f,1.0f) < fChance) 
		{
			return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTracker::PrepareForSeverChecks()
//
//	PURPOSE:	distribute random damage to limbs
//
// ----------------------------------------------------------------------- //
void DamageTracker::PrepareForSeverChecks(ModelsDB::HSEVERBODY hBody)
{
	if (!hBody) return;

	for (uint32 n = 0; n < kNumDamageTypes; ++n)
	{
		DamageType eDT = static_cast<DamageType>(n);

		//if it's a random type we have to allocate damage to random limbs
		if (IsRandomSeverType(eDT))
		{
			//while we've got some undistributed damage
			while (m_Records[eDT].fLocation[HL_UNKNOWN] > 0.0f)
			{
				ModelsDB::HSEVERPIECE hPiece = g_pModelsDB->GetRandomPiece(hBody);
				if (!hPiece) return;

				HitLocation eLoc = g_pModelsDB->GetSPLocation(hPiece);

				ModelsDB::HSEVERDAMAGE hSD = g_pModelsDB->GetSPDamageRecord(hPiece);
				if (!hSD) return;

				float fAmount = LTMIN(m_Records[eDT].fLocation[HL_UNKNOWN],g_pModelsDB->GetSeverDamageMax(hSD,eDT));
				if (fAmount <= 0.0f)
					return;

				m_Records[eDT].fLocation[HL_UNKNOWN] -= fAmount;
				m_Records[eDT].fLocation[eLoc] += fAmount;

			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTracker::CheckForSever()
//
//	PURPOSE:	test accumulated damage for severs
//
// ----------------------------------------------------------------------- //
bool DamageTracker::CheckForSever(ModelsDB::HSEVERBODY hBody, bool bIsCrouched, ModelsDB::HPieceArray& hSeveredPieces)
{
	uint32 nNumPieces = g_pModelsDB->GetBodyNumPieces(hBody);
	if (nNumPieces == 0)
		return false;

	bool bSever = false;

	//iterate through all of our pieces to see if any were severed...
	for (uint32 nPiece = 0; nPiece < nNumPieces; ++nPiece)
	{
		//get the piece
		ModelsDB::HSEVERPIECE hPiece = g_pModelsDB->GetBodyPiece(hBody,nPiece);
		if (hPiece)
		{
			//check for already severed pieces that might prevent us from severing this piece
			ModelsDB::HPieceArray::iterator iter = hSeveredPieces.begin();
			bool bFoundExclusion = false;
			while (iter != hSeveredPieces.end() && !bFoundExclusion)
			{
				bFoundExclusion = (g_pModelsDB->IsExcludedPiece(hPiece,(*iter)));
				iter++;
			}
			if (bFoundExclusion)
			{
				continue;
			}

			//get the associated location
			HitLocation eLoc = g_pModelsDB->GetSPLocation(hPiece);

			//can't sever torso or legs while crouched because of animation and physics issues
			if (bIsCrouched && (eLoc == HL_LEG_LEFT || eLoc == HL_LEG_LEFT || eLoc == HL_TORSO))
			{
				continue;
			}

			// get the damage properties
			ModelsDB::HSEVERDAMAGE hSD = g_pModelsDB->GetSPDamageRecord(hPiece);
			if (hSD)
			{
				//now step through the damage types 
				for (uint32 n = 0; n < kNumDamageTypes; ++n)
				{
					DamageType eDT = static_cast<DamageType>(n);
					//only check for sever if we've got damage here
					float fDamage = m_Records[eDT].fLocation[eLoc];
					if (fDamage > 0.0f)
					{
						float fChance = g_pModelsDB->GetSeverChance(hSD,eDT,fDamage);
						if (GetRandom(0.0f,1.0f) < fChance)
						{
							bSever = true;
							hSeveredPieces.push_back(hPiece);
							break;
						}
					}
				}
				
			}
		}
	
	}

	return bSever;
	
}
