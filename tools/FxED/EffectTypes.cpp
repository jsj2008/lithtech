#include "StdAfx.h"
#include "EffectTypes.h"
#include "BaseSpellDef.h"

BOOL g_bWriteYLock = TRUE;

// [kml] Use the enumerated type in basespelldef.cpp
/*char* Totems[] =
{
	"FIRE",
	"SUN",
	"STORM",
	"ILLUSION",
	"DEMONOLOGY",
	"DEATH",
	"TRUTH",
	"SCIENCE",
	"BLOOD",
	"ICE",
	"NO_TOTEM",
	"ALL",
};*/

char* CAlterAttributes::EnumStringVals[] = 
{
	"SANITY",
	"HEALTH",
	"MOTION",
	"HIDDEN",
	"DISPEL",
	"ACCELERATE",
	"INVISIBILITY",
	"LEVITATE"
};


char* CEnviroEffect::ShapeStringVals[] = 
{
	"CIRCLE",
	"RING",
	"LINE"
};



char* CEnviroEffect::AttributeStringVals[] = 
{
	"SANITY",
	"HEALTH",
	"MOTION",
	"SOLID"
};




//////////////////////////////////////////////////////////////////////////////
//
// Class CBaseEffect
//
//
int CBaseEffect::Compare(CBaseEffect* p1, CBaseEffect* p2) // static
{
	if (p1->m_tmStart < p2->m_tmStart)
		return -1;
	else if (p1->m_tmStart = p2->m_tmStart)
		return 0;
	return 1;
}




//////////////////////////////////////////////////////////////////////////////
//
// Class CAlterAttributes
//
//
ostream& CAlterAttributes::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT ALTER_ATTRIBUTES" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "Duration=" << m_duration << endl;
	os << "\t\t\t" << "Rate=" << m_rate << endl;
	os << "\t\t\t" << "Attribute=" << CAlterAttributes::EnumStringVals[m_attributeType] << endl;
	os << "\t\t\t" << "Amount=" << m_amount << endl;
	os << "\t\t\t" << "Float=" << m_float << endl;
	os << "\t\t\t" << "Float1=" << m_float1 << endl;
	switch (m_affectee)
	{
	case CASTER:
		os << "\t\t\t" << "Affectee=CASTER" << endl;
		break;
	case TARGET:
		os << "\t\t\t" << "Affectee=TARGET" << endl;
		break;
	case SPHERE:
		os << "\t\t\t" << "Affectee=SPHERE" << endl;
		break;
	case CYLINDER:
		os << "\t\t\t" << "Affectee=CYLINDER" << endl;
		break;
	case SQUARE :
		os << "\t\t\t" << "Affectee=SQUARE" << endl;
		break;
	case CONE :
		os << "\t\t\t" << "Affectee=CONE" << endl;
		break;
	}

	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}




//////////////////////////////////////////////////////////////////////////////
//
// Class CAttachDamageObject
//
//
ostream& CAttachDamageObject::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT ATTACH_DAMAGE_OBJECT" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "ModelName=" << '"' << m_sModelName << '"' << endl;
	os << "\t\t\t" << "SkinName=" << '"' << m_sSkinName << '"' << endl;
	if (m_bVisible)
		os << "\t\t\t" << "Visible=TRUE" << endl;
	else
		os << "\t\t\t" << "Visible=FALSE" << endl;
	os << "\t\t\t" << "Scale=" << m_scale << endl;
	os << "\t\t\t" << "NodeName=" << '"' << m_sNodeName << '"' << endl;
	os << "\t\t\t" << "Damage=" << m_damage << endl;
	os << "\t\t\t" << "Duration=" << m_duration << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}



//////////////////////////////////////////////////////////////////////////////
//
// Class CSummon
//
//
ostream& CSummon::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT SUMMON" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "ObjectName=" << '"' << m_sObjectName << '"' << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}




//////////////////////////////////////////////////////////////////////////////
//
// Class CBlock
//
//
ostream& CBlock::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT BLOCK" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "Duration=" << m_duration << endl;
	os << "\t\t\t" << "Rate=" << m_rate << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}

//////////////////////////////////////////////////////////////////////////////
//
// Class CResolutionEnd
//
//
ostream& CResolutionEnd::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT RESOLUTION_END" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "Duration=" << m_duration << endl;
	os << "\t\t\t" << "Rate=" << m_rate << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}




//////////////////////////////////////////////////////////////////////////////
//
// Class CSpawnSpell
//
//
ostream& CSpawnSpell::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT SPAWN_SPELL" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "Name=" << '"' << m_sName << '"' << endl;
	os << "\t\t\t" << "Offset=" << '<' << m_vecOffset.x << ", " << m_vecOffset.y << ", " << m_vecOffset.z << '>' << endl;
	os << "\t\t\t" << "Direction=" << '<' << m_vecDirection.x << ", " << m_vecDirection.y << ", " << m_vecDirection.z << '>' << endl;
	if (g_bWriteYLock) os << "\t\t\t" << "YLock=" << m_bYLock << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}




//////////////////////////////////////////////////////////////////////////////
//
// Class CShieldBreak
//
//
ostream& CShieldBreak::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT SHIELD_BREAK" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "Duration=" << m_duration << endl;
	os << "\t\t\t" << "Rate=" << m_rate << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}





//////////////////////////////////////////////////////////////////////////////
//
// Class CSetString
//
//
ostream& CSetString::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT SET_STRING" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "String=" << '"' << m_sString << '"' << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}


//////////////////////////////////////////////////////////////////////////////
//
// Class CTelekinesis
//
//
ostream& CTelekinesis::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT TELEKINESIS" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}



//////////////////////////////////////////////////////////////////////////////
//
// Class CCounteractAll
//
//
ostream& CCounteractAll::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT COUNTERACTALL" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}




//////////////////////////////////////////////////////////////////////////////
//
// Class CSendMessage
//
//
ostream& CSendMessage::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT SEND_MESSAGE" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "Message=" << '"' << m_sMessage << '"' << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}




//////////////////////////////////////////////////////////////////////////////
//
// Class CClone
//
//
ostream& CClone::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT CLONE" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "NumClones=" << m_nClones << endl;
	os << "\t\t\t" << "Radius=" << m_fRadius << endl;
	os << "\t\t\t" << "Lifespan=" << m_fLifespan << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}



//////////////////////////////////////////////////////////////////////////////
//
// Class CEnvEffect
//
//
ostream& CEnviroEffect::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT ENV_EFFECT" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "FxId=" << (DWORD)m_fxId << endl;
	os << "\t\t\t" << "Shape=" << CEnviroEffect::ShapeStringVals[m_shape] << endl;
	os << "\t\t\t" << "Attribute=" << CEnviroEffect::AttributeStringVals[m_attributeType] << endl;
	os << "\t\t\t" << "Radius=" << m_radius << endl;
	os << "\t\t\t" << "GrowthDuration=" << m_growthDuration << endl;
	os << "\t\t\t" << "RateOfEffect=" << m_rateOfEffect << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}



//////////////////////////////////////////////////////////////////////////////
//
// Class CShapeShift
//
//
ostream& CShapeShift::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT SHAPESHIFT" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "Duration=" << m_tmDuration << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}


//////////////////////////////////////////////////////////////////////////////
//
// Class CReflect
//
//
ostream& CReflect::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT REFLECT" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "Duration=" << m_duration << endl;
	os << "\t\t\t" << "Radius=" << m_fRadius << endl;
	os << "\t\t\t" << "SpeedMultiplier=" << m_fMaxSpeedMult << endl;
	os << "\t\t\t" << "SpeedUpDuration=" << m_fSpeedUpDuration << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}



//////////////////////////////////////////////////////////////////////////////
//
// Class CInvulnerability
//
//
ostream& CInvulnerability::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT INVULNERABILITY" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "Duration=" << m_duration << endl;
	os << "\t\t\t" << "Totem=" << TotemTypeEnumStringVals[m_nTotem] << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}


//////////////////////////////////////////////////////////////////////////////
//
// Class CShieldImpact
//
//
ostream& CShieldImpact::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT SHIELDIMPACT" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "Fx=" << '"' << m_sFx << '"' << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}


//////////////////////////////////////////////////////////////////////////////
//
// Class CSupress
//
//
ostream& CSupress::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT SUPRESS" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "Duration=" << m_duration << endl;
	os << "\t\t\t" << "Totem=" << TotemTypeEnumStringVals[m_nTotem] << endl;
	os << "\t\t\t" << "Radius=" << m_fRadius << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}



//////////////////////////////////////////////////////////////////////////////
//
// Class CRepulse
//
//
ostream& CRepulse::Write(ostream& os)
{
	os << "\t\tBEGIN_EFFECT REPULSE" << endl;
	os << "\t\t\t" << "tmStart=" << m_tmStart << endl;
	os << "\t\t\t" << "Duration=" << m_duration << endl;
	os << "\t\t\t" << "Radius=" << m_fRadius << endl;
	os << "\t\t\t" << "Accel=" << m_fAccel << endl;
	os << "\t\tEND_EFFECT" << endl << endl;

	return os;
}