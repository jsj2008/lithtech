// ----------------------------------------------------------------------- //
//
// MODULE  : GameRuleCtrls.h
//
// PURPOSE : Set of custom UI controls for GameRules
//
// CREATED : 09/07/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef GAMERULECTRLS_H
#define GAMERULECTRLS_H

#include "GameModeMgr.h"
#include "CustomCtrls.h"

class GameRuleFloatSliderCtrl : public FloatSliderCtrl
{
public:

	GameRuleFloatSliderCtrl( )
	{
		m_pGameRuleFloat = NULL;
	}

	GameRuleFloat* GetGameRuleFloat( ) { return m_pGameRuleFloat; }

public:

	bool Create( CBaseScreen& parent, GameRuleFloat& gameRuleFloat, FloatSliderCtrl::CreateStruct& cs, bool bAddToParent );

private:

	GameRuleFloat* m_pGameRuleFloat;

private:

	PREVENT_OBJECT_COPYING( GameRuleFloatSliderCtrl );
};

class GameRuleUint32SliderCtrl : public CLTGUISlider
{
public:

	GameRuleUint32SliderCtrl( )
	{
		m_pGameRuleUint32 = NULL;
	}

	GameRuleUint32* GetGameRuleUint32( ) { return m_pGameRuleUint32; }

public:

	bool Create( CBaseScreen& parent, GameRuleUint32& gameRuleUint32, CLTGUISlider_create& cs, bool bAddToParent );

private:

	GameRuleUint32* m_pGameRuleUint32;

private:

	PREVENT_OBJECT_COPYING( GameRuleUint32SliderCtrl );
};


class GameRuleWStringLabeledEditCtrl : public LabeledEditCtrl
{
public:

	GameRuleWStringLabeledEditCtrl( )
	{
		m_pGameRuleWString = NULL;
	}

	GameRuleWString* GetGameRuleWString( ) { return m_pGameRuleWString; }

public:

	bool Create( CBaseScreen& parent, GameRuleWString& gameRuleWString, LabeledEditCtrl::CreateStruct& cs, bool bAddToParent );

private:

	GameRuleWString* m_pGameRuleWString;

private:

	PREVENT_OBJECT_COPYING( GameRuleWStringLabeledEditCtrl );
};

class GameRuleBoolCtrl : public CLTGUIToggle
{
public:

	GameRuleBoolCtrl( )
	{
		m_pGameRuleBool = NULL;
	}

	GameRuleBool* GetGameRuleBool( ) { return m_pGameRuleBool; }

public:

	bool Create( CBaseScreen& parent, GameRuleBool& gameRuleBool, CLTGUIToggle_create& cs, bool bAddToParent );

private:

	GameRuleBool* m_pGameRuleBool;

private:

	PREVENT_OBJECT_COPYING( GameRuleBoolCtrl );
};


class GameRuleEnumCtrl : public CLTGUICycleCtrl
{
public:

	GameRuleEnumCtrl( )
	{
		m_pGameRuleEnum = NULL;
		m_nValue = 0;
	}

	GameRuleEnum* GetGameRuleEnum( ) { return m_pGameRuleEnum; }

public:

	bool Create( CBaseScreen& parent, GameRuleEnum& gameRuleEnum, CLTGUICycleCtrl_create& cs, bool bAddToParent );
	void UpdateData( bool bSaveAndValidate );

private:

	GameRuleEnum* m_pGameRuleEnum;
	uint8 m_nValue;

private:

	PREVENT_OBJECT_COPYING( GameRuleEnumCtrl );
};


class TeamReflectDamageCtrl : public GameRuleFloatSliderCtrl
{
public:

	TeamReflectDamageCtrl( ) { }

	bool Create( CBaseScreen& parent, GameRuleFloat& gameRuleFloat, FloatSliderCtrl::CreateStruct& cs, bool bAddToParent );

protected:

	static const wchar_t* TeamReflectDamageTextCallback(int nPos, void* pUserData);

private:

	PREVENT_OBJECT_COPYING( TeamReflectDamageCtrl );
};


#endif // GAMERULECTRLS_H
