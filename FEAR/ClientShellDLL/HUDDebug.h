// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDebug.h
//
// PURPOSE : HUDItem to display Debug
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DEBUG_H
#define __HUD_DEBUG_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Debug
//******************************************************************************************
class CHUDDebug : public CHUDItem
{
public:
	CHUDDebug();

	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();		

	virtual void	UpdateLayout();

	void	SetTargetDebugString(const wchar_t* const pszDebugString)
	{
		if (LTStrCmp(pszDebugString,m_Text.GetText()))
			m_Text.SetText(pszDebugString);
	}

	void	SetPerturbDebugString(const wchar_t* const pszDebugString);

	// Turn on or off specific debug functionality.
	void	ShowPlayerPos(bool bShow=true)	{ m_bShowPlayerPos = bShow; }
	void	ShowCamPosRot(bool bShow=true)  { m_bShowCamPosRot = bShow; }


private:

	// Layout.

	// Lookat Debug String.
	uint16			m_nTextWidth;

	// Debug camera and player position/rotation string screen locations...

	enum Constants { kMaxDebugStrings = 3 };
	enum DSSL { eDSBottomLeft, eDSBottomRight };
	
	void	SetCameraInformationDebugString(char* strMessage, DSSL eLoc, uint8 nLine);
	void	RenderCameraInformationDebugStrings();
	void	UpdateCameraInformationDebugInfo();

	CLTGUIString 	m_LeftDebugString[kMaxDebugStrings];
	CLTGUIString 	m_RightDebugString[kMaxDebugStrings];

	bool	m_bShowPlayerPos;
	bool	m_bShowCamPosRot;
};

#endif