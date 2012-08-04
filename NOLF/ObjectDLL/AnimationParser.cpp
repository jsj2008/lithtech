
#include "StdAfx.h"
#include "AnimationParser.h"
#include "AnimationParse.h"
#include "AnimationLex.h"

using namespace Animation;

const uint32 animodDislocates = 0x01;

#define Output Debug

CAnimationParser* g_pParser = NULL;
static LTBOOL s_btError = LTFALSE;

void yyanimationerror(const char* szError)
{
//	char szShit[1024];
//	sprintf(szShit, "Parse error at line %d: %s\n", g_Lex.GetLine(), szError);
	s_btError = LTTRUE;
}

inline void ParserError(const char* szFormat, ...)
{
	static char szBuffer[1024];
	va_list val;
	va_start(val, szFormat);
	vsprintf(szBuffer, szFormat, val);
	va_end(val);
	yyanimationerror(szBuffer);
}

extern "C" int yyanimationwrap(void)
{
	return 1;
}

CAnimationParser::CAnimationParser()
{
	m_pPropertyGroupCurrent = LTNULL;
	m_pAnimationCurrent = LTNULL;
	m_pTransitionCurrent = LTNULL;
}

struct FnForEachPropertyGroupDelete : public FnForEach<CStr, PROPERTYGROUP*>
{
    void operator()(const CStr& sName, PROPERTYGROUP* pPropertyGroup)
	{
		debug_delete(pPropertyGroup);
	}
};

struct FnForEachPropertyDelete : public FnForEach<CStr, PROPERTY*>
{
    void operator()(const CStr& sName, PROPERTY* pProperty)
	{
		debug_delete(pProperty);
	}
};

CAnimationParser::~CAnimationParser()
{
    struct FnForEachPropertyGroupDelete temp = FnForEachPropertyGroupDelete();
    m_mapPropertyGroups.ForEach(temp);
    struct FnForEachPropertyDelete temp2 = FnForEachPropertyDelete();
    m_mapProperties.ForEach(temp2);

	if ( m_pAnimationCurrent )
	{
		ANIMATION* pAnimation = m_pAnimationCurrent;
		while ( pAnimation )
		{
			ANIMATION* pAnimationNext = pAnimation->pNext;
			debug_delete(pAnimation);
			pAnimation = pAnimationNext;
		}

		m_pAnimationCurrent = LTNULL;
	}
	if ( m_pTransitionCurrent )
	{
		TRANSITION* pTransition = m_pTransitionCurrent;
		while ( pTransition )
		{
			TRANSITION* pTransitionNext = pTransition->pNext;
			debug_delete(pTransition);
			pTransition = pTransitionNext;
		}

		m_pTransitionCurrent = LTNULL;
	}
}

LTBOOL CAnimationParser::Parse(const CStr& sFile)
{
	s_btError = LTFALSE;

	g_pParser = this;

	s_btError = !g_Lex.Lex(sFile);
/*
	#ifdef _REZFILE
	{
		BYTE* pbyItem = pItem->Load();
		char* pchBuffer = new char[pItem->GetSize() + 2];
		pchBuffer[pItem->GetSize()] = 0;
		pchBuffer[pItem->GetSize()+1] = 0;
		memcpy(pchBuffer, pbyItem, sizeof(BYTE)*pItem->GetSize());
		pItem->Unload();

		g_pParser = this;

		yyanimation_scan_buffer(pchBuffer);

		delete [] pchBuffer;
	}
	#else // #ifndef _REZFILE
	{
		yyanimationin = fopen(sFile, "rt");
		if ( !yyanimationin )
		{
			ParserError("Couldn't open %s", (const char*)sFile);
			return LTFALSE;
		}

		g_pParser = this;

		yyanimationparse();
	}
	#endif
*/
	return !s_btError;
}

void CAnimationParser::BeginProperties()
{
	m_pPropertyGroupCurrent = NULL;
	m_cPropertyGroups = 0;
}

void CAnimationParser::AddPropertyGroup(const CStr& sName)
{
	if ( m_mapPropertyGroups.Find(sName) )
	{
		ParserError("Duplicate property group: \"%s\"", (const char*)sName);
		return;
	}

	m_pPropertyGroupCurrent = debug_new(PROPERTYGROUP);
	m_pPropertyGroupCurrent->sName = sName;

	m_mapPropertyGroups.Add(sName, m_pPropertyGroupCurrent);

	AddPropertyToCurrentGroup(CStr("Any")+sName);

	m_pPropertyGroupCurrent->nId = m_cPropertyGroups++;
}

void CAnimationParser::AddPropertyToCurrentGroup(const CStr& sName)
{
	if ( !m_pPropertyGroupCurrent )
	{
		ParserError("No property group specified before property was declared");
		return;
	}

	PROPERTY* pProperty = debug_new(PROPERTY);
	pProperty->sName = sName;
	pProperty->pNext = m_pPropertyGroupCurrent->pProperties;
	pProperty->pGroup = m_pPropertyGroupCurrent;

	m_mapProperties.Add(sName, pProperty);

	m_pPropertyGroupCurrent->pProperties = pProperty;
}

struct FnFinalizePropertyGroups : public FnForEach<CStr, PROPERTYGROUP*>
{
    void operator()(const CStr& sName, PROPERTYGROUP* pPropertyGroup)
	{
		uint32 nId = 1;

		PROPERTY* pProperty = pPropertyGroup->pProperties;
		while ( pProperty )
		{
			if ( !pProperty->pNext )
			{
				pProperty->nId = -1;
			}
			else
			{
				pProperty->nId = nId++;
			}

			pProperty = pProperty->pNext;
		}
	}
};

void CAnimationParser::BeginAnimations()
{
    struct FnFinalizePropertyGroups temp = FnFinalizePropertyGroups();
    m_mapPropertyGroups.ForEach(temp);
	m_pAnimationCurrent = NULL;
	m_pTransitionCurrent = NULL;
}

void CAnimationParser::AddAnimation(const CStr& sName)
{
	ANIMATION* pAnimation = debug_new(ANIMATION);
	pAnimation->pNext = m_pAnimationCurrent;
	pAnimation->sName = sName;

	m_pAnimationCurrent = pAnimation;
}

void CAnimationParser::AddPropertyToCurrentAnimation(const CStr& sProperty)
{
	if ( !m_pAnimationCurrent )
	{
		ParserError("No animation specified before property set was declared");
		return;
	}

	PROPERTY** ppProperty;
	if ( !m_mapProperties.Find(sProperty, &ppProperty) )
	{
		ParserError("No such property \"%s\"", (const char*)sProperty);
		return;
	}

	m_pAnimationCurrent->stackProperties.Push(*ppProperty);
}

void CAnimationParser::BeginTransitions()
{
	m_pTransitionCurrent = NULL;
}

void CAnimationParser::AddTransition(const CStr& sName)
{
	TRANSITION* pTransition = debug_new(TRANSITION);
	pTransition->pNext = m_pTransitionCurrent;
	pTransition->sName = sName;

	m_pTransitionCurrent = pTransition;
}

void CAnimationParser::AddPropertyToCurrentTransitionInitialSet(const CStr& sProperty)
{
	if ( !m_pTransitionCurrent )
	{
		ParserError("No transition specified before property set was declared");
		return;
	}

	PROPERTY** ppProperty;
	if ( !m_mapProperties.Find(sProperty, &ppProperty) )
	{
		ParserError("No such property \"%s\"", (const char*)sProperty);
		return;
	}

	m_pTransitionCurrent->stackPropertiesInitialSet.Push(*ppProperty);
}

void CAnimationParser::AddPropertyToCurrentTransitionAddSet(const CStr& sProperty)
{
	if ( !m_pTransitionCurrent )
	{
		ParserError("No transition specified before property set was declared");
		return;
	}

	PROPERTY** ppProperty;
	if ( !m_mapProperties.Find(sProperty, &ppProperty) )
	{
		ParserError("No such property \"%s\"", (const char*)sProperty);
		return;
	}

	m_pTransitionCurrent->stackPropertiesAddSet.Push(*ppProperty);
}

void CAnimationParser::AddPropertyToCurrentTransitionRemoveSet(const CStr& sProperty)
{
	if ( !m_pTransitionCurrent )
	{
		ParserError("No transition specified before property set was declared");
		return;
	}

	PROPERTY** ppProperty;
	if ( !m_mapProperties.Find(sProperty, &ppProperty) )
	{
		ParserError("No such property \"%s\"", (const char*)sProperty);
		return;
	}

	m_pTransitionCurrent->stackPropertiesRemoveSet.Push(*ppProperty);
}

void CAnimationParser::AddPropertyToCurrentTransitionConstantSet(const CStr& sProperty)
{
	if ( !m_pTransitionCurrent )
	{
		ParserError("No transition specified before property set was declared");
		return;
	}

	PROPERTY** ppProperty;
	if ( !m_mapProperties.Find(sProperty, &ppProperty) )
	{
		ParserError("No such property \"%s\"", (const char*)sProperty);
		return;
	}

	m_pTransitionCurrent->stackPropertiesConstantSet.Push(*ppProperty);
}

void CAnimationParser::AddPropertyToCurrentTransitionNotSet(const CStr& sProperty)
{
	if ( !m_pTransitionCurrent )
	{
		ParserError("No transition specified before property set was declared");
		return;
	}

	PROPERTY** ppProperty;
	if ( !m_mapProperties.Find(sProperty, &ppProperty) )
	{
		ParserError("No such property \"%s\"", (const char*)sProperty);
		return;
	}

	m_pTransitionCurrent->stackPropertiesNotSet.Push(*ppProperty);
}

struct FnEnumerateProperties : public FnForEach<CStr, PROPERTYGROUP*>
{
	FnEnumerateProperties(const char** aszNames, uint32* aiIndices, int32* anValues, uint32* pcProperties)
	{
		m_aszNames = aszNames;
		m_anValues = anValues;
		m_aiIndices = aiIndices;
		m_pcProperties = pcProperties;
	}

    void operator()(const CStr& sName, PROPERTYGROUP* pPropertyGroup)
	{
		uint32 nId = 1;

		PROPERTY* pProperty = pPropertyGroup->pProperties;
		while ( pProperty )
		{
			m_aszNames[*m_pcProperties] = pProperty->sName;
			m_aiIndices[*m_pcProperties] = pProperty->pGroup->nId;
			m_anValues[*m_pcProperties] = pProperty->nId;
			(*m_pcProperties)++;

			pProperty = pProperty->pNext;
		}
	}

	const char**	m_aszNames;
	uint32*			m_aiIndices;
	int32*			m_anValues;
	uint32*			m_pcProperties;
};

void CAnimationParser::EnumerateProperties(const char** aszNames, uint32* aiIndices, int32* anValues, uint32* pcProperties)
{
	*pcProperties = 0;
    FnEnumerateProperties temp(aszNames, aiIndices, anValues, pcProperties);
    m_mapPropertyGroups.ForEach(temp);
}

void CAnimationParser::EnumerateAnimations(const char** aszNames, const char* aaszProperties[512][32], uint32 acProperties[512], uint32* pcAnimations)
{
	*pcAnimations = 0;

	ANIMATION* pAnimation = m_pAnimationCurrent;
	while ( pAnimation )
	{
		aszNames[*pcAnimations] = pAnimation->sName;
		acProperties[*pcAnimations] = pAnimation->stackProperties.GetSize();
		for ( uint32 iProperty = 0 ; iProperty < acProperties[*pcAnimations] ; iProperty++ )
		{
			aaszProperties[*pcAnimations][iProperty] = pAnimation->stackProperties.Pop()->sName;
		}

		(*pcAnimations)++;
		pAnimation = pAnimation->pNext;
	}
}

void CAnimationParser::EnumerateTransitions(const char** aszNames, const char* aaszProperties[512][32][5], uint32 acProperties[512][5], uint32* pcTransitions)
{
	*pcTransitions = 0;

	TRANSITION* pTransition = m_pTransitionCurrent;
	while ( pTransition )
	{
		aszNames[*pcTransitions] = pTransition->sName;

		for ( uint32 iSet = 0 ; iSet < 5 ; iSet++ )
		{
			CStack<PROPERTY, 32>* apStacks[5] = { &pTransition->stackPropertiesInitialSet,
												 &pTransition->stackPropertiesAddSet,
												 &pTransition->stackPropertiesRemoveSet,
												 &pTransition->stackPropertiesConstantSet,
												 &pTransition->stackPropertiesNotSet };

			acProperties[*pcTransitions][iSet] = apStacks[iSet]->GetSize();
			for ( uint32 iProperty = 0 ; iProperty < acProperties[*pcTransitions][iSet] ; iProperty++ )
			{
				aaszProperties[*pcTransitions][iProperty][iSet] = apStacks[iSet]->Pop()->sName;
			}
		}

		(*pcTransitions)++;
		pTransition = pTransition->pNext;
	}
}