#if !defined(_EFFECTTYPES_H_)
#define _EFFECTTYPES_H_


#include <iostream.h>
#include "ltbasetypes.h"
#include "compat\ltcompat.h"

extern BOOL g_bWriteYLock;

class CBaseEffect
{
public:  // Enums

	enum EffectType
	{
		ALTER_ATTRIBUTES = 0,
		ATTACH_DAMAGE_OBJECT,
		SUMMON,
		BLOCK,
		SPAWN_SPELL,
		SHIELD_BREAK,
		SET_STRING,
		SEND_MESSAGE,
		ENV_EFFECT,
		SHAPESHIFT,
		REFLECT,
		INVULNERABILITY,
		SUPRESS,
		REPULSE,
		TELEKINESIS,
		COUNTERACTALL,
		CLONE,
		SHIELDIMPACT,
		RESOLUTION_END,
	};

public:  // Methods

	CBaseEffect(EffectType type) : m_effectType(type)
	{
		m_tmStart	 = 0;
		m_duration	 = 0;
		m_rate		 = 0;
		m_bActivated = FALSE;
	}

	virtual ~CBaseEffect() { }

	EffectType GetType() { return m_effectType; }

	virtual ostream& Write(ostream& os) = 0;

	static int Compare(CBaseEffect* p1, CBaseEffect* p2);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg) = 0;
		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg) = 0;
	#endif


public:  // Vars

	float m_tmStart;
	float m_duration;  // How many seconds...if 0 and rate doesn't equal 0 then run until the end of spell.
	float m_rate;  // Every m_rate seconds...if 0 only do once
	BOOL  m_bActivated;
	enum EffectType m_effectType;
};




class CAlterAttributes : public CBaseEffect
{
public:  // Enums

	enum AttributeType
	{
		SANITY = 0,
		HEALTH,
		MOTION,
		HIDDEN,
		DISPEL,
		ACCELERATE,
		INVISIBILITY,
		LEVITATE
	};

	enum Affectee
	{
		CASTER = 0,
		TARGET,
		SPHERE,
		CYLINDER,
		SQUARE,
		CONE
	};


public:  // Methods

	CAlterAttributes(EffectType type) : CBaseEffect(type), m_float(0.0f), m_float1(0.0f), m_amount(0), m_affectee(TARGET), m_attributeType(HEALTH) { }

	~CAlterAttributes() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg);
		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg);
	#endif

public:  // Vars

	AttributeType m_attributeType;
	int m_amount;
	Affectee m_affectee;
	float m_float;
	float m_float1;

	static char* EnumStringVals[];
};





class CAttachDamageObject : public CBaseEffect
{
public:  // Methods

	CAttachDamageObject(EffectType type) : CBaseEffect(type) { }

	~CAttachDamageObject() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg);
		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg);
	#endif

public:  // Vars

	CString m_sModelName;
	CString m_sSkinName;
	bool m_bVisible;
	float m_scale;
	CString m_sNodeName;
	int m_damage;
	float m_duration;

};




class CSummon : public CBaseEffect
{
public:  // Methods

	CSummon(EffectType type) : CBaseEffect(type) { }

	~CSummon() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg);
		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg);
	#endif


public:  // Vars

	CString m_sObjectName;

};




class CBlock : public CBaseEffect
{
public:  // Methods

	CBlock(EffectType type) : CBaseEffect(type) { }

	~CBlock() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg);
		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg);
	#endif


public:  // Vars


};

class CResolutionEnd : public CBaseEffect
{
public:  // Methods

	CResolutionEnd(EffectType type) : CBaseEffect(type) { }

	~CResolutionEnd() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg);
		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg);
	#endif


public:  // Vars


};


class CSpawnSpell : public CBaseEffect
{
public:  // Methods

	CSpawnSpell(EffectType type) : CBaseEffect(type) { m_bYLock = 0; }

	~CSpawnSpell() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg);
		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg);
	#endif


public:  // Vars


	CString m_sName;
	DVector m_vecOffset;
	DVector m_vecDirection;
	BOOL m_bYLock;
};




class CShieldBreak : public CBaseEffect
{
public:  // Methods

	CShieldBreak(EffectType type) : CBaseEffect(type) { }

	~CShieldBreak() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg);
		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg);
	#endif

};





class CSetString : public CBaseEffect
{
public:  // Methods

	CSetString(EffectType type) : CBaseEffect(type) { }

	~CSetString() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg);
		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg);
	#endif


public:  // Vars


	CString m_sString;

};

class CTelekinesis : public CBaseEffect
{
public:  // Methods

	CTelekinesis(EffectType type) : CBaseEffect(type) { }

	~CTelekinesis() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg)
		{
			return hMsg;
		}

		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg)
		{
			return hMsg;
		}
	#endif


public:  // Vars


	CString m_sString;

};

class CCounteractAll : public CBaseEffect
{
	public:  // Methods

		CCounteractAll(EffectType type) : CBaseEffect(type) { }

		~CCounteractAll() { } // virtual

		virtual ostream& Write(ostream& os);

		#if defined(_SANITY_)
			virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg)
			{
				return hMsg;
			}

			virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg)
			{
				return hMsg;
			}
		#endif
};

class CShieldImpact : public CBaseEffect
{
	public:  // Methods

		CShieldImpact(EffectType type) : CBaseEffect(type) { }

		~CShieldImpact() { } // virtual

		virtual ostream& Write(ostream& os);

		#if defined(_SANITY_)
			virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg)
			{
				g_pInterface->WriteToMessageString(hMsg, (char *)(LPCSTR)m_sFx);
				return hMsg;
			}

			virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg)
			{
				m_sFx = g_pInterface->ReadFromMessageString(hMsg);
				
				return hMsg;
			}
		#endif

	CString m_sFx;
};

class CClone : public CBaseEffect
{
	public:  // Methods

		CClone(EffectType type) : CBaseEffect(type) { }

		~CClone() { } // virtual

		virtual ostream& Write(ostream& os);

		#if defined(_SANITY_)
			virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg)
			{
				return hMsg;
			}

			virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg)
			{
				return hMsg;
			}
		#endif

	int					m_nClones;
	float				m_fRadius;
	float				m_fLifespan;
};

class CShapeShift : public CBaseEffect
{
public:  // Methods

	CShapeShift(EffectType type) : CBaseEffect(type) { }

	~CShapeShift() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg)
		{
			g_pInterface->WriteToMessageFloat(hMsg, m_tmDuration);

			return hMsg;
		}

		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg)
		{
			m_tmDuration = g_pInterface->ReadFromMessageFloat(hMsg);

			return hMsg;
		}
	#endif


public:  // Vars

	CString			m_sOrigModel;
	CString			m_sOrigSkin;	
	float			m_tmDuration;
};

class CReflect : public CBaseEffect
{
public:  // Methods

	CReflect(EffectType type) : CBaseEffect(type), m_fSpeedUpDuration(0.0f) { }

	~CReflect() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg)
		{
			g_pInterface->WriteToMessageFloat(hMsg, m_duration);
			g_pInterface->WriteToMessageFloat(hMsg, m_fRadius);
			g_pInterface->WriteToMessageFloat(hMsg, m_fMaxSpeedMult);
			g_pInterface->WriteToMessageFloat(hMsg, m_fSpeedUpDuration);
			return hMsg;
		}

		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg)
		{
			m_duration = g_pInterface->ReadFromMessageFloat(hMsg);
			m_fRadius    = g_pInterface->ReadFromMessageFloat(hMsg);
			m_fMaxSpeedMult = g_pInterface->ReadFromMessageFloat(hMsg);
			m_fSpeedUpDuration = g_pInterface->ReadFromMessageFloat(hMsg);
			return hMsg;
		}
	#endif


public:  // Vars

	float			m_fRadius;
	float			m_fMaxSpeedMult;
	float			m_fSpeedUpDuration;
};

class CRepulse : public CBaseEffect
{
public:  // Methods

	CRepulse(EffectType type) : CBaseEffect(type) { }

	~CRepulse() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg)
		{
			g_pInterface->WriteToMessageFloat(hMsg, m_duration);
			g_pInterface->WriteToMessageFloat(hMsg, m_fRadius);
			g_pInterface->WriteToMessageFloat(hMsg, m_fAccel);

			return hMsg;
		}

		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg)
		{
			m_duration = g_pInterface->ReadFromMessageFloat(hMsg);
			m_fRadius  = g_pInterface->ReadFromMessageFloat(hMsg);
			m_fAccel   = g_pInterface->ReadFromMessageFloat(hMsg);

			return hMsg;
		}
	#endif


public:  // Vars

	float			m_fRadius;
	float			m_fAccel;
};

class CInvulnerability : public CBaseEffect
{
public:  // Methods

	CInvulnerability(EffectType type) : CBaseEffect(type) { }

	~CInvulnerability() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg)
		{
			g_pInterface->WriteToMessageFloat(hMsg, m_duration);
			g_pInterface->WriteToMessageByte(hMsg, m_nTotem);

			return hMsg;
		}

		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg)
		{
			m_duration = g_pInterface->ReadFromMessageFloat(hMsg);
			m_nTotem     = g_pInterface->ReadFromMessageByte(hMsg);

			return hMsg;
		}
	#endif


public:  // Vars

	BYTE			m_nTotem;
};

class CSupress : public CBaseEffect
{
public:  // Methods

	CSupress(EffectType type) : CBaseEffect(type) { }

	~CSupress() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg)
		{
			g_pInterface->WriteToMessageFloat(hMsg, m_duration);
			g_pInterface->WriteToMessageByte(hMsg, m_nTotem);
			g_pInterface->WriteToMessageFloat(hMsg, m_fRadius);

			return hMsg;
		}

		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg)
		{
			m_duration = g_pInterface->ReadFromMessageFloat(hMsg);
			m_nTotem   = g_pInterface->ReadFromMessageByte(hMsg);
			m_fRadius  = g_pInterface->ReadFromMessageFloat(hMsg);

			return hMsg;
		}
	#endif


public:  // Vars

	BYTE			m_nTotem;
	float			m_fRadius;
};

class CSendMessage : public CBaseEffect
{
public:  // Methods

	CSendMessage(EffectType type) : CBaseEffect(type) { }

	~CSendMessage() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg);
		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg);
	#endif


public:  // Vars


	CString m_sMessage;

};




class CEnviroEffect : public CBaseEffect
{
public:  // Enums

	enum Shape
	{
		CIRCLE = 0,
		RING,
		LINE
	};

	enum AttributeType
	{
		SANITY = 0,
		HEALTH,
		MOTION,
		SOLID
	};

public:  // Methods

	CEnviroEffect(EffectType type) : 
		 CBaseEffect(type),
		 m_fxId(0),
		 m_shape((Shape)0),
		 m_attributeType((AttributeType)0),
		 m_radius(0),
		 m_growthDuration(0),
		 m_rateOfEffect(0.0f)
	{ }

	~CEnviroEffect() { } // virtual

	virtual ostream& Write(ostream& os);

	#if defined(_SANITY_)
		virtual HMESSAGEWRITE WriteToMessage(HMESSAGEWRITE hMsg);
		virtual HMESSAGEREAD ReadFromMessage(HMESSAGEREAD hMsg);
	#endif


public:  // Vars

	BYTE m_fxId;
	Shape m_shape;
	AttributeType m_attributeType;
	int m_radius;
	float m_growthDuration;
	float m_rateOfEffect;

	static char* ShapeStringVals[];
	static char* AttributeStringVals[];
};






#if defined(_SANITY_)


inline HMESSAGEWRITE CAlterAttributes::WriteToMessage(HMESSAGEWRITE hMsg)
{
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_effectType);
	g_pInterface->WriteToMessageFloat(hMsg, m_tmStart);
	g_pInterface->WriteToMessageFloat(hMsg, m_duration);
	g_pInterface->WriteToMessageFloat(hMsg, m_rate);

	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_attributeType);
	g_pInterface->WriteToMessageDWord(hMsg, (DWORD)m_amount);
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_affectee);

	return hMsg;
}



inline HMESSAGEREAD CAlterAttributes::ReadFromMessage(HMESSAGEREAD hMsg)
{
	m_tmStart = g_pInterface->ReadFromMessageFloat(hMsg);
	m_duration = g_pInterface->ReadFromMessageFloat(hMsg);
	m_rate = g_pInterface->ReadFromMessageFloat(hMsg);
	m_attributeType = (AttributeType)g_pInterface->ReadFromMessageByte(hMsg);
	m_amount = g_pInterface->ReadFromMessageDWord(hMsg);
	m_affectee = (Affectee)g_pInterface->ReadFromMessageByte(hMsg);

	return hMsg;
}



inline HMESSAGEWRITE CAttachDamageObject::WriteToMessage(HMESSAGEWRITE hMsg)
{
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_effectType);
	g_pInterface->WriteToMessageFloat(hMsg, m_tmStart);
	g_pInterface->WriteToMessageFloat(hMsg, m_duration);
	g_pInterface->WriteToMessageFloat(hMsg, m_rate);
	g_pInterface->WriteToMessageString(hMsg, (char*)(LPCTSTR)m_sModelName);
	g_pInterface->WriteToMessageString(hMsg, (char*)(LPCTSTR)m_sSkinName);
	g_pInterface->WriteToMessageByte(hMsg, m_bVisible);
	g_pInterface->WriteToMessageFloat(hMsg, m_scale);
	g_pInterface->WriteToMessageString(hMsg, (char*)(LPCTSTR)m_sNodeName);
	g_pInterface->WriteToMessageDWord(hMsg, (DWORD)m_damage);
	g_pInterface->WriteToMessageFloat(hMsg, m_duration);

	return hMsg;
}



inline HMESSAGEREAD CAttachDamageObject::ReadFromMessage(HMESSAGEREAD hMsg)
{
	m_tmStart = g_pInterface->ReadFromMessageFloat(hMsg);
	m_duration = g_pInterface->ReadFromMessageFloat(hMsg);
	m_rate = g_pInterface->ReadFromMessageFloat(hMsg);
	m_sModelName = g_pInterface->ReadFromMessageString(hMsg);
	m_sSkinName = g_pInterface->ReadFromMessageString(hMsg);
	m_bVisible = g_pInterface->ReadFromMessageByte(hMsg) == 1;
	m_scale = g_pInterface->ReadFromMessageFloat(hMsg);
	m_sNodeName = g_pInterface->ReadFromMessageString(hMsg);
	m_damage = (int)g_pInterface->ReadFromMessageDWord(hMsg);
	m_duration = g_pInterface->ReadFromMessageFloat(hMsg);

	return hMsg;
}



inline HMESSAGEWRITE CSummon::WriteToMessage(HMESSAGEWRITE hMsg)
{
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_effectType);
	g_pInterface->WriteToMessageFloat(hMsg, m_tmStart);
	g_pInterface->WriteToMessageFloat(hMsg, m_duration);
	g_pInterface->WriteToMessageFloat(hMsg, m_rate);
	g_pInterface->WriteToMessageString(hMsg, (char*)(LPCTSTR)m_sObjectName);

	return hMsg;
}



inline HMESSAGEREAD CSummon::ReadFromMessage(HMESSAGEREAD hMsg)
{
	m_tmStart = g_pInterface->ReadFromMessageFloat(hMsg);
	m_duration = g_pInterface->ReadFromMessageFloat(hMsg);
	m_rate = g_pInterface->ReadFromMessageFloat(hMsg);
	m_sObjectName = g_pInterface->ReadFromMessageString(hMsg);

	return hMsg;
}



inline HMESSAGEWRITE CBlock::WriteToMessage(HMESSAGEWRITE hMsg)
{
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_effectType);
	g_pInterface->WriteToMessageFloat(hMsg, m_tmStart);
	g_pInterface->WriteToMessageFloat(hMsg, m_duration);
	g_pInterface->WriteToMessageFloat(hMsg, m_rate);

	return hMsg;
}



inline HMESSAGEREAD CBlock::ReadFromMessage(HMESSAGEREAD hMsg)
{
	m_tmStart = g_pInterface->ReadFromMessageFloat(hMsg);
	m_duration = g_pInterface->ReadFromMessageFloat(hMsg);
	m_rate = g_pInterface->ReadFromMessageFloat(hMsg);

	return hMsg;
}

inline HMESSAGEWRITE CResolutionEnd::WriteToMessage(HMESSAGEWRITE hMsg)
{
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_effectType);
	g_pInterface->WriteToMessageFloat(hMsg, m_tmStart);
	g_pInterface->WriteToMessageFloat(hMsg, m_duration);
	g_pInterface->WriteToMessageFloat(hMsg, m_rate);

	return hMsg;
}

inline HMESSAGEREAD CResolutionEnd::ReadFromMessage(HMESSAGEREAD hMsg)
{
	m_tmStart = g_pInterface->ReadFromMessageFloat(hMsg);
	m_duration = g_pInterface->ReadFromMessageFloat(hMsg);
	m_rate = g_pInterface->ReadFromMessageFloat(hMsg);

	return hMsg;
}


inline HMESSAGEWRITE CSpawnSpell::WriteToMessage(HMESSAGEWRITE hMsg)
{
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_effectType);
	g_pInterface->WriteToMessageFloat(hMsg, m_tmStart);
	g_pInterface->WriteToMessageFloat(hMsg, m_duration);
	g_pInterface->WriteToMessageFloat(hMsg, m_rate);
	g_pInterface->WriteToMessageString(hMsg, (char*)(LPCTSTR)m_sName);
	g_pInterface->WriteToMessageVector(hMsg, &m_vecOffset);
	g_pInterface->WriteToMessageVector(hMsg, &m_vecDirection);
	g_pInterface->WriteToMessageByte(hMsg, m_bYLock);

	return hMsg;
}



inline HMESSAGEREAD CSpawnSpell::ReadFromMessage(HMESSAGEREAD hMsg)
{
	m_tmStart = g_pInterface->ReadFromMessageFloat(hMsg);
	m_duration = g_pInterface->ReadFromMessageFloat(hMsg);
	m_rate = g_pInterface->ReadFromMessageFloat(hMsg);
	m_sName = g_pInterface->ReadFromMessageString(hMsg);
	g_pInterface->ReadFromMessageVector(hMsg, &m_vecOffset);
	g_pInterface->ReadFromMessageVector(hMsg, &m_vecDirection);
	m_bYLock = g_pInterface->ReadFromMessageByte(hMsg);

	return hMsg;
}



inline HMESSAGEWRITE CShieldBreak::WriteToMessage(HMESSAGEWRITE hMsg)
{
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_effectType);
	g_pInterface->WriteToMessageFloat(hMsg, m_tmStart);

	return hMsg;
}



inline HMESSAGEREAD CShieldBreak::ReadFromMessage(HMESSAGEREAD hMsg)
{
	m_tmStart = g_pInterface->ReadFromMessageFloat(hMsg);

	return hMsg;
}



inline HMESSAGEWRITE CSetString::WriteToMessage(HMESSAGEWRITE hMsg)
{
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_effectType);
	g_pInterface->WriteToMessageFloat(hMsg, m_tmStart);
	g_pInterface->WriteToMessageString(hMsg, (char*)(LPCTSTR)m_sString);

	return hMsg;
}



inline HMESSAGEREAD CSetString::ReadFromMessage(HMESSAGEREAD hMsg)
{
	m_tmStart = g_pInterface->ReadFromMessageFloat(hMsg);
	m_sString = g_pInterface->ReadFromMessageString(hMsg);

	return hMsg;
}



inline HMESSAGEWRITE CSendMessage::WriteToMessage(HMESSAGEWRITE hMsg)
{
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_effectType);
	g_pInterface->WriteToMessageFloat(hMsg, m_tmStart);
	g_pInterface->WriteToMessageString(hMsg, (char*)(LPCTSTR)m_sMessage);

	return hMsg;
}



inline HMESSAGEREAD CSendMessage::ReadFromMessage(HMESSAGEREAD hMsg)
{
	m_tmStart = g_pInterface->ReadFromMessageFloat(hMsg);
	m_sMessage = g_pInterface->ReadFromMessageString(hMsg);

	return hMsg;
}



inline HMESSAGEWRITE CEnviroEffect::WriteToMessage(HMESSAGEWRITE hMsg)
{
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_effectType);
	g_pInterface->WriteToMessageFloat(hMsg, m_tmStart);
	g_pInterface->WriteToMessageByte(hMsg, m_fxId);
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_shape);
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_attributeType);
	g_pInterface->WriteToMessageByte(hMsg, (BYTE)m_radius);
	g_pInterface->WriteToMessageFloat(hMsg, m_growthDuration);
	g_pInterface->WriteToMessageFloat(hMsg, m_rateOfEffect);

	return hMsg;
}



inline HMESSAGEREAD CEnviroEffect::ReadFromMessage(HMESSAGEREAD hMsg)
{
	m_tmStart = g_pInterface->ReadFromMessageFloat(hMsg);
	m_fxId = g_pInterface->ReadFromMessageByte(hMsg);
	m_shape = (Shape)g_pInterface->ReadFromMessageByte(hMsg);
	m_attributeType = (AttributeType)g_pInterface->ReadFromMessageByte(hMsg);
	m_radius = (int)g_pInterface->ReadFromMessageByte(hMsg);
	m_growthDuration = g_pInterface->ReadFromMessageFloat(hMsg);
	m_rateOfEffect = g_pInterface->ReadFromMessageFloat(hMsg);

	return hMsg;
}


#endif






#endif