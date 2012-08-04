#include "StdAfx.h"
#include "strstrea.h"

#if defined(_DEBUG)
	#define new DEBUG_NEW
#endif


#include "BaseSpellDef.h"

CHashTable CBaseSpellDef::sm_hashStrings;


//////////////////////////////////////////////////////////////////////////////
// String values of enumerated types.  The only purpose of this is for being able
// to wright the equivalent word out to a file.
static char* EnumStringVals[] = 
{
	"ENUM",
	"INTEGER",
	"FLOAT",
	"STRING",
	"VECTOR"
	"D4VECTOR",
	"CLRKEY"
};



static char* PhasesEnumStringVals[] =
{
	"CAST",
	"ACTIVE",
	"RESOLUTION"
};



char* TotemTypeEnumStringVals[] =
{
	"FIRE",
	"SUN",
	"ILLUSION",
	"SCIENCE",
	"DEMONOLOGY",
	"DEATH",
	"STORM",
	"TRUTH",
	"BLOOD",
	"ICE",
	"ALL_TOTEMS",
	"NO_TOTEM"
};
int g_nNumTotems = sizeof( TotemTypeEnumStringVals ) / ( sizeof( char * ));
int g_nNumRetailTotems = 8;

char* SpellBitStringVals[] =
{
	"Caster Health",
	"Target Health",
	"Caster Sanity",
	"Target Sanity",
	"Caster Melee Health Damage",
	"Target Melee Health Damage",
	"Caster Melee Sanity Damage",
	"Target Melee Sanity Damage",
	"Caster Speed",
	"Target Speed",
	"Summon",
	"Glyph",
	"Environmental Effect",
	"Invisibility",
	"Levitate",
	"Shield",
	"Reflect",
	"Dispel",
	"Projectile",
	"Target",
	"Point",
	"Self",
	"Reveal",
	"Suppression",
	"Push",
	"Pull",
	"Wall",
	"Pit",
	"Shield Break",
	"Protection",
	"Custom 1",
	"Custom 2"
};
int g_nNumSpellBits = sizeof( SpellBitStringVals ) / ( sizeof( char * ));


CBaseSpellDef::CBaseSpellDef() : m_phaseList(zPtrColl::active)
{ 
	m_type = 0;
	m_subType = 0;
	m_isModelVisible = 0;

	m_health = 0;
	m_lifeSpan = 0;

	m_speed = 0.0;
	m_modelScale = 1.0;
	m_castLoopCount = 1;
	m_activationOffset = 0.0;
	m_totemType = 0;
	m_totemLevel = 0;

	m_casterSanityCost = 0;
	m_casterHealthCost = 0;
	m_targetSanityCost = 0;
	m_targetHealthCost = 0;

	m_phaseCount = 0;
	m_spellBits = 0;

	m_fYVelocity = 0.0f;
	m_fYGravity   = 0.0f;

	m_dwKey1 = 0;
	m_dwKey2 = 0;
	m_dwKey3 = 0;
	m_dwKey4 = 0;
	m_dwKey5 = 0;
	m_dwKey6 = 0;
	m_dwKey7 = 0;
	m_dwKey8 = 0;
}



CBaseSpellDef::~CBaseSpellDef()
{ 
}





//////////////////////////////////////////////////////////////////////////////
//
// void CBaseSpellDef::WriteToStream(ostream& os)
//
//		Writes this data structure to and ostream
//
void CBaseSpellDef::WriteToStream(ostream& os, BOOL bWriteThing)
{
	os.flags(ios::showpoint);

	os << "BEGIN_SPELL " << '"' << m_sGuid << '"' << endl << endl;
	os << "\tNAME=" << '"' << m_sName << '"' << endl;
	os << "\tDESCRIPTION=" << '"' << m_sDescription << '"' << endl;
	os << "\tSPELL_TYPE=";
	switch (m_type)
	{
		case SPELL_SELF:				os << "SELF"; break;
		case SPELL_POINT:				os << "POINT"; break;
		case SPELL_TARGETING:			os << "TARGETING"; break;
		case SPELL_TARGETACTIVATEABLE:	os << "TARGETACTIVATEABLE"; break;
	}
	os << endl;

	os << "\tSUBTYPE=";
	switch (m_subType)
	{
		case SUB_PROJECTILE:	os << "PROJECTILE"; break;
		case SUB_BLOCKING:	    os << "BLOCKING"; break;
		case SUB_GLYPH:			os << "GLYPH"; break;
		case SUB_NONE:		    os << "NONE"; break;
	}
	os << endl;
	os << "\tMODIFIER=";
	switch (m_modifier)
	{
		case MT_BINDING:		os << "BINDING"; break;
		case MT_NONE:		    os << "NONE"; break;
	}
	os << endl;

	if (bWriteThing)
	{
		os << "\tEDITOR_FLAGS=" << "0x" << hex << m_dwKey3 << dec << endl;
	}
	else
	{
		DWORD dwZero = 0;
		os << "\tEDITOR_FLAGS=" << "0x" << hex << dwZero << dec << endl;
	}

	os << "\tTOTEM_TYPE=" << TotemTypeEnumStringVals[m_totemType] << "," << m_totemLevel << endl;
	os << "\tMODEL_NAME=" << '"' << m_sModelName << '"' << endl;
	os << "\tMODEL_SCALE=" << m_modelScale << endl;
	os << "\tMODEL_VISIBLE=" << m_isModelVisible << endl;

	if (bWriteThing)
	{
		os << "\tLINK_ID=" << "0x" << hex << m_dwKey5 << dec << endl;
	}
	else
	{
		DWORD dwZero = 0;
		os << "\tLINK_ID=" << "0x" << hex << dwZero << dec << endl;
	}

	os << "\tSKIN_NAME=" << '"' << m_sSkinName << '"' << endl;
	os << "\tSPEED=" << m_speed << endl;

	if (bWriteThing)
	{
		os << "\tPROJECT_FLAGS=" << "0x" << hex << m_dwKey4 << dec << endl;
	}
	else
	{
		DWORD dwZero = 0;
		os << "\tPROJECT_FLAGS=" << "0x" << hex << dwZero << dec << endl;
	}

	if (bWriteThing)
	{
		os << "\tCASTTARGETID_FLAGS=" << "0x" << hex << m_dwKey2 << dec << endl;
	}
	else
	{
		DWORD dwZero = 0;
		os << "\tCASTTARGETID_FLAGS=" << "0x" << hex << dwZero << dec << endl;
	}

	os << "\tICON=" << '"' << m_sIcon << '"' << endl;
	os << "\tCAST_ANI=" << '"' << m_sCastAniName << '"' << endl;
	os << "\tAI_CAST_ANI=" << '"' << m_sAiCastAniName << '"' << endl;
	if (bWriteThing)
	{
		os << "\tAIPHASE=" << "0x" << hex << m_dwKey1 << dec << endl;
	}
	else
	{
		DWORD dwZero = 0;
		os << "\tAIPHASE=" << "0x" << hex << dwZero << dec << endl;
	}
	os << "\tACT_OFFSET=" << m_activationOffset << endl;
	os << "\tLOOP_COUNT=" << m_castLoopCount << endl;
	os << "\tHEALTH=" << m_health << endl;
	os << "\tCASTER_SANITY_COST=" << m_casterSanityCost << endl;
	os << "\tCASTER_HEALTH_COST=" << m_casterHealthCost << endl;
	os << "\tTARGET_SANITY_COST=" << m_targetSanityCost << endl;
	os << "\tTARGET_HEALTH_COST=" << m_targetHealthCost << endl;

	if (bWriteThing)
	{
		os << "\tCAST_ID=" << "0x" << hex << m_dwKey6 << dec << endl;
	}
	else
	{
		DWORD dwZero = 0;
		os << "\tCAST_ID=" << "0x" << hex << dwZero << dec << endl;
	}

	if (bWriteThing)
	{
		os << "\tACTIVE_ID=" << "0x" << hex << m_dwKey7 << dec << endl;
	}
	else
	{
		DWORD dwZero = 0;
		os << "\tACTIVE_ID=" << "0x" << hex << dwZero << dec << endl;
	}

	if (bWriteThing)
	{
		os << "\tRESOLUTION_ID=" << "0x" << hex << m_dwKey8 << dec << endl;
	}
	else
	{
		DWORD dwZero = 0;
		os << "\tRESOLUTION_ID=" << "0x" << hex << dwZero << dec << endl;
	}

	os << "\tLIFESPAN=" << m_lifeSpan << endl;
	os << "\tSPELL_BITS=" << "0x" << hex << m_spellBits << dec << endl;
	os << "\tYVELOCITY=" << m_fYVelocity << endl;
	os << "\tGRAVITY=" << m_fYGravity << endl;
	os << endl;

	zGDLCursor<Phase> c = m_phaseList;
	Phase* pPhase;
	for (pPhase = c.setfirst(); pPhase; pPhase = c.next())
		WritePhase(os, pPhase);

	os << "END_SPELL" << endl;
}


void CBaseSpellDef::WritePhase(ostream& os, Phase* pPhase)
{
	os << "\tBEGIN_PHASE ";
	os << PhasesEnumStringVals[pPhase->m_phase] << endl << endl;
	os << "\t\tPHASE_LENGTH " << pPhase->m_phaseLength << endl << endl;

	zGSLCursor<CBaseEffect> c = pPhase->m_effectList;
	for (CBaseEffect* pEffect = c.reset(); pEffect; pEffect = c.next())
		pEffect->Write(os);

	for (Fx* pFx = pPhase->m_fxList.setfirst(); pFx; pFx = pPhase->m_fxList.next())
		WriteFx(os, pFx);
	os << "\tEND_PHASE" << endl << endl;
}



void CBaseSpellDef::WriteFx(ostream& os, Fx* pFx)
{
	os << "\t\t" << "BEGIN_FX " << '"' << pFx->m_szName << '"' << endl;
	for (Id* pId = pFx->m_idList.setfirst(); pId; pId = pFx->m_idList.next())
		WriteId(os, pId, 3);
	os << "\t\t" << "END_FX" << endl << endl;
}



void CBaseSpellDef::WriteId(ostream& os, Id* pId, const int tabPrefix)
{
	for (int i = 0; i < tabPrefix; i++)
		os << "\t";

	os << ( const char * )pId->m_szName << "=";
	switch (pId->m_type)
	{
	case INTEGER:
		os << pId->m_data.m_int;
		break;
	case HEX:
		os << "0x" << hex << pId->m_data.m_int << dec;
		break;
	case FLOAT:
		os << pId->m_data.m_float;
		break;
	case STRING:
		os << '"' << (( const char * )( pId->m_data.m_string )) << '"';
		break;
	case VECTOR:
		os << '<' << pId->m_data.m_vec.x << ", " << pId->m_data.m_vec.y << ", " << pId->m_data.m_vec.z << '>';
		break;
	case D4VECTOR:
		os << '<' << pId->m_data.m_d4vec[0] << ", " << pId->m_data.m_d4vec[1] << ", " << pId->m_data.m_d4vec[2] << ", " << pId->m_data.m_d4vec[3] << '>';
		break;
	case CLRKEY:
		os << pId->m_data.m_clrKey.m_tmKey << ", " << "0x" << hex << pId->m_data.m_clrKey.m_dwColor << dec;
		break;
	case ENUM:
		os << EnumStringVals[pId->m_data.m_int];
		break;
	}
	os << endl;
}

//------------------------------------------------------------------
//
//   FUNCTION : CalcCRC()
//
//   PURPOSE  : Computes CRC....
//
//------------------------------------------------------------------

void CBaseSpellDef::CalcCRC(DWORD dwKey, int nDefSize)
{
}

//------------------------------------------------------------------
//
//   FUNCTION : CheckCRC()
//
//   PURPOSE  : Makes sure this is a valid DEF....
//
//------------------------------------------------------------------

BOOL CBaseSpellDef::CheckCRC(int nLen,const CString& csGuid)
{
	if (!nLen) return FALSE;
	BYTE *pArray = new BYTE [nLen];
	
	ostrstream os((char *)pArray, nLen);


	if (!m_sGuid.Compare("MeteorShower") || !m_sGuid.Compare("GrenadeVolley"))
	{
		g_bWriteYLock = FALSE;
	}

	WriteToStream(os);	

	g_bWriteYLock = TRUE;

	int nRealLen = os.tellp();

	// Check the CRC on the array

	DWORD dwCRC = 0;
	
	// Run the CRC on this particular spelldef

	for (int j = 0; j < 20; j ++)
	{
		for (int i = 0; i < nRealLen; i ++)
		{
			dwCRC += ((pArray[i] * i) * j);
		}
	}
	
	os.rdbuf()->freeze(false);
	
	delete [] pArray;

	CString szCrc;
	szCrc.Format("%08x",dwCRC);

	DWORD dwGotcha = 0xF6EA791B;

	if(!m_sGuid.Compare("GreaterFireball") || 
		!m_sGuid.Compare("GrenadeVolley") || 
		!m_sGuid.Compare("GrenadeVolleyProjectile") || 
		!m_sGuid.Compare("HandsOfTheDeadGlyph") || 
		!m_sGuid.Compare("HandsOfTheDeadGlyphHit") || 
		!m_sGuid.Compare("SanityDrain") || 
		!m_sGuid.Compare("WaveOfPain") || 
		!m_sGuid.Compare("MeteorShower") || 
		!m_sGuid.Compare("MeteorShowerProjectile") || 
		!m_sGuid.Compare("NullGround") || 
		!m_sGuid.Compare("Sawblade") || 
		!m_sGuid.Compare("SolarFlare") || 
		!m_sGuid.Compare("SummonStormEfreet"))
	{
		TRACE("RealPackTalent (%s) checking CRC...\n",m_sGuid);
		szCrc.SetAt(6, szCrc[6] * 'F' + 2);
		j = szCrc[6];
		if (j == 1)
		{
			dwGotcha += j * 5;
		}
		szCrc.SetAt(2, szCrc[2] * 'd' + 6);
		if ((BYTE)szCrc[6] != m_dwKey7) return false;

		szCrc.SetAt(0, szCrc[0] * '8' + 8);
		szCrc.SetAt(4, szCrc[4] * '5' + 4);
		szCrc.SetAt(3, szCrc[3] * '2' + 5);

		if ((BYTE)szCrc[2] != m_dwKey3) return false;
		if ((BYTE)szCrc[4] != m_dwKey5) return false;
		if ((BYTE)szCrc[0] != m_dwKey1) return false;

		szCrc.SetAt(1, szCrc[1] * '3' + 7);
		szCrc.SetAt(7, szCrc[7] * '7' + 1);
		szCrc.SetAt(5, szCrc[5] * 'J' + 3);

		if ((BYTE)szCrc[1] != m_dwKey2) return false;
		if ((BYTE)szCrc[3] != m_dwKey4) return false;
		if ((BYTE)szCrc[5] != m_dwKey6) return false;
		if ((BYTE)szCrc[7] != m_dwKey8) return false;
		TRACE("RealPackTalent (%s) CRC OK!\n",m_sGuid);
	}
	else
	{
		TRACE("Non-Real Pack Talent (%s) checking CRC...\n",m_sGuid);
		szCrc.SetAt(6, szCrc[6] * '7' + 2);
		j = szCrc[6];
		if (j == 1)
		{
			dwGotcha += j * 5;
		}
		szCrc.SetAt(2, szCrc[2] * 'F' + 6);
		if ((BYTE)szCrc[6] != m_dwKey7) return false;

		szCrc.SetAt(0, szCrc[0] * '4' + 8);
		szCrc.SetAt(4, szCrc[4] * 'J' + 4);
		szCrc.SetAt(3, szCrc[3] * 'C' + 5);

		if ((BYTE)szCrc[2] != m_dwKey3) return false;
		if ((BYTE)szCrc[4] != m_dwKey5) return false;
		if ((BYTE)szCrc[0] != m_dwKey1) return false;

		szCrc.SetAt(1, szCrc[1] * '8' + 7);
		szCrc.SetAt(7, szCrc[7] * 'b' + 1);
		szCrc.SetAt(5, szCrc[5] * 'n' + 3);

		if ((BYTE)szCrc[1] != m_dwKey2) return false;
		if ((BYTE)szCrc[3] != m_dwKey4) return false;
		if ((BYTE)szCrc[5] != m_dwKey6) return false;
		if ((BYTE)szCrc[7] != m_dwKey8) return false;
		TRACE("Non-Real Pack Talent (%s) CRC OK!\n",m_sGuid);
	}

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : CalcSize()
//
//   PURPOSE  : Calculates the size of a spelldef
//
//------------------------------------------------------------------

DWORD CBaseSpellDef::CalcSize()
{
	DWORD dwSize = 0;

	// Add in the basic size...
	
	dwSize += sizeof(CBaseSpellDef);

	dwSize += m_sGuid.GetLength();
	dwSize += m_sName.GetLength();
	dwSize += m_sDescription.GetLength();
	dwSize += m_sModelName.GetLength();
	dwSize += m_sSkinName.GetLength();
	dwSize += m_sIcon.GetLength();
	dwSize += m_sCastAniName.GetLength();
	dwSize += m_sAiCastAniName.GetLength();

	zGDLCursor<Phase> cPhase = m_phaseList;
	Phase* pPhase;

	for (pPhase = cPhase.setfirst(); pPhase; pPhase = cPhase.next())
	{
		// Add in the size of the phase...

		dwSize += sizeof(Phase);

		// Add in the size of the fx....

		zGDLCursor<Fx> cFx = pPhase->m_fxList;
		Fx *pFx;

		for (pFx = cFx.setfirst(); pFx; pFx = cFx.next())
		{
			// Add in the fx....

			dwSize += sizeof(Fx);

			// Add in the props...

			zGDLCursor<Id> cId = pFx->m_idList;
			Id *pId;

			for (pId = cId.setfirst(); pId; pId = cId.next())
			{
				// Add in the Id....

				dwSize += sizeof(Id);
				dwSize += strlen(pId->m_szName);

				if (pId->m_type == CBaseSpellDef::STRING)
				{
					dwSize += strlen(pId->m_data.m_string);
				}
			}
		}

		// Add in the effects....

		zGSLCursor<CBaseEffect> cEffect = pPhase->m_effectList;
		CBaseEffect *pEffect;

		for (pEffect = cEffect.reset(); pEffect; pEffect = cEffect.next())
		{
			dwSize += 512;
		}
	}

	return dwSize;
}
