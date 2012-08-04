#include "Stdafx.h"
#include "LightEditor.h"
#include "DebugLineSystem.h"
#include "LTEulerAngles.h"
#include "iltfilemgr.h"
#include "PlayerObj.h"

//----------------------------------------------------------------------------------------------------------
// Constants and globals

//allows disabling of the light editor in builds
#ifdef _FINAL
#define DISABLE_RUNTIME_LIGHT_EDITOR
#endif

//the name we register the console program under
#define LIGHT_EDITOR_CONSOLE_PROGRAM_NAME		"LED"

//the length of the light orientation arrows
#define LIGHT_ORIENTATION_ARROW_LEN				4.0f

//----------------------------------------------------------------------------------------------------------
// Change flags

enum EChangeFlags
{
	eChangeFlag_Position			= (1<<0),
	eChangeFlag_Rotation			= (1<<1),
	eChangeFlag_Color				= (1<<2),
	eChangeFlag_Texture				= (1<<3),
	eChangeFlag_AttenuationTexture	= (1<<4),
	eChangeFlag_Intensity			= (1<<5),
	eChangeFlag_Dims				= (1<<6),
	eChangeFlag_NearClip			= (1<<7),
	eChangeFlag_Radius				= (1<<8),
	eChangeFlag_FOV					= (1<<9),
	eChangeFlag_WorldShadow			= (1<<10),
	eChangeFlag_ObjectShadow		= (1<<11),
	eChangeFlag_LightLOD			= (1<<12),
};

//given a change mask and a flag it will determine if the provided flag is set
static bool IsFlagSet(uint32 nMask, uint32 nFlag)
{
	return (nMask & nFlag) != 0;
}

//given a mask and a flag, this will return the locked string for the hud if the field cannot be
//modified, otherwise it will return an empty string
static const char* GetLockedStr(uint32 nMask, uint32 nFlag)
{
	return (IsFlagSet(nMask, nFlag)) ? " [L]" : "";
}

//----------------------------------------------------------------------------------------------------------
// Utility functions

//given an output file, this will write the provided line to it
static void WriteLine(std::ofstream& OutFile, const char* pszStr, ...)
{
	char pszLineBuffer[512];

	va_list marker;
	va_start(marker, pszStr);
	int32 iReturn = LTVSNPrintF(pszLineBuffer, LTARRAYSIZE(pszLineBuffer), pszStr, marker);
	va_end(marker);

	OutFile << pszLineBuffer << std::endl;
}

//given an engine LOD, this will convert it to a string
static const char* ConvertLODToStr(EEngineLOD eLOD)
{
	switch(eLOD)
	{
	case eEngineLOD_Low:	return "Low";		break;
	case eEngineLOD_Medium:	return "Medium";	break;
	case eEngineLOD_High:	return "High";		break;
	default:				return "Never";		break;
	}
}

//given an argument list and an argument offset, this will attempt to parse out a single string
//constant and will return that string, or NULL if it cannot be parsed out
static const char* GetStringArg(uint32 nArgOff, uint32 nArgC, const char* const * ppszArgV)
{
	if(nArgOff + 1 <= nArgC)
		return ppszArgV[nArgOff];
	return NULL;
}

//given an argument list and an argument offset, this will attempt to parse out a vector from
//the arguments, and will return the success of the operation
static bool GetVectorArg(uint32 nArgOff, uint32 nArgC, const char* const * ppszArgV, LTVector& vResult)
{
	if(nArgOff + 3 <= nArgC)
	{
		vResult.x = (float)atof(ppszArgV[nArgOff + 0]);
		vResult.y = (float)atof(ppszArgV[nArgOff + 1]);
		vResult.z = (float)atof(ppszArgV[nArgOff + 2]);
		return true;
	}

	return false;
}

//given an argument list and an argument offset, this will attempt to parse out a float from
//the arguments, and will return the success of the operation
static bool GetFloatArg(uint32 nArgOff, uint32 nArgC, const char* const * ppszArgV, float& fResult)
{
	if(nArgOff + 1 <= nArgC)
	{
		fResult = (float)atof(ppszArgV[nArgOff]);
		return true;
	}

	return false;
}

//given an argument list and an offset, this will use the specified argument as a string and try to find
//an object with that name. If the object is found it will return the object, NULL otherwise
static HOBJECT GetObjectArg(uint32 nArgOff, uint32 nArgC, const char* const * ppszArgV)
{
	//find the name of the object
	const char* pszObjName = GetStringArg(nArgOff, nArgC, ppszArgV);
	if(!pszObjName)
		return NULL;

	//find that object!
	ObjArray<HOBJECT, 1> FoundObjects;
	g_pLTServer->FindNamedObjects(pszObjName, FoundObjects);

	if(FoundObjects.NumObjects() == 0)
		return NULL;

	return FoundObjects.GetObject(0);
}

//----------------------------------------------------------------------------------------------------------
// CLightEditor

CLightEditor::CLightEditor() :
	m_bDisplayHud(false),
	m_bDisplayLightVisualization(false),
	m_fMoveScale(1.0f),
	m_bModifiedSinceLastSave(false)
{
}

CLightEditor::~CLightEditor()
{
	Term();
}

//singleton support
CLightEditor& CLightEditor::Singleton()
{
	static CLightEditor s_Singleton;
	return s_Singleton;
}

//called to initialize this system
void CLightEditor::Init(const char* pszWorldName)
{
	#ifndef DISABLE_RUNTIME_LIGHT_EDITOR

		//save this world name (for saving)
		m_sActiveLevel = pszWorldName;

		//make sure that our world has the correct extension on it
		CResExtUtil::SetFileExtension(m_sActiveLevel, RESEXT_WORLD_PACKED);

		//we are starting a new level, so initialize the light visualizations if they are enabled
		if(m_bDisplayLightVisualization)
			CreateLightVisualizations();

		//register our console program so we can get info
		g_pLTServer->RegisterConsoleProgram(LIGHT_EDITOR_CONSOLE_PROGRAM_NAME, ConsoleProgramCB);
	#endif
}

//called to clean up all data associated with this system
void CLightEditor::Term()
{
	#ifndef DISABLE_RUNTIME_LIGHT_EDITOR
		//unregister our console program
		g_pLTServer->UnregisterConsoleProgram(LIGHT_EDITOR_CONSOLE_PROGRAM_NAME);

		ClearEditLights();
		ClearModifiedLights();
		DestroyAllLightVisualizations();
	#endif
}

//this should be called once per frame to allow for the light editor to update itself and 
//print out any other displays
void CLightEditor::Update()
{
	#ifndef DISABLE_RUNTIME_LIGHT_EDITOR

		//remove any lights from our edit or modified list that have been removed
		RemoveInvalidEditLights();
		RemoveInvalidModifiedLights();

		//display the HUD to the console if it is active and we are editing a light
		if(m_bDisplayHud)
			DisplayHUD();

		//and update our light visualizations to follow lights
		if(m_bDisplayLightVisualization)
		{
			UpdateAimLight();
			UpdateLightVisualizations();
		}
		else
		{
			m_hAimLight = NULL;
		}

	#endif
}

//----------------------------------------------------------------------------------------------------------
// HUD

//prints out the HUD for the provided light
void CLightEditor::DisplayLightHUD(HOBJECT hLight, uint32 nMask)
{
	//skip invalid lights
	if(!hLight)
		return;

	//gather all of the base light information
	char pszLightName[256];
	g_pLTServer->GetObjectName(hLight, pszLightName, LTARRAYSIZE(pszLightName));

	LTRigidTransform tLightTrans;
	g_pLTServer->GetObjectTransform(hLight, &tLightTrans);

	float fLightRadius;
	g_pLTServer->GetLightRadius(hLight, fLightRadius);

	float fR, fG, fB, fA;
	g_pLTServer->GetObjectColor(hLight, &fR, &fG, &fB, &fA);

	float fIntensity;
	g_pLTServer->GetLightIntensityScale(hLight, fIntensity);

	char pszAttenuationTexture[MAX_PATH];
	g_pLTServer->GetLightAttenuationTexture(hLight, pszAttenuationTexture, LTARRAYSIZE(pszAttenuationTexture));

	char pszLightTexture[MAX_PATH];
	g_pLTServer->GetLightTexture(hLight, pszLightTexture, LTARRAYSIZE(pszLightTexture));

	LTVector vDirectionalDims;
	g_pLTServer->GetLightDirectionalDims(hLight, vDirectionalDims);

	float fFovX, fFovY, fNearZ;
	g_pLTServer->GetLightSpotInfo(hLight, fFovX, fFovY, fNearZ);

	EEngineLOD eLightLOD, eWorldShadowLOD, eObjectShadowLOD;
	g_pLTServer->GetLightDetailSettings(hLight, eLightLOD, eWorldShadowLOD, eObjectShadowLOD);

	//make sure to adjust our position by the world offset for displaying
	LTVector vSrcWorldOffset;
	g_pLTServer->GetSourceWorldOffset(vSrcWorldOffset);
	tLightTrans.m_vPos += vSrcWorldOffset;

	//handle the fact that our FOV at runtime is half frustum radians, wheras in the tools it
	//is full frustum degrees
	fFovX = MATH_RADIANS_TO_DEGREES(fFovX) * 2.0f;
	fFovY = MATH_RADIANS_TO_DEGREES(fFovY) * 2.0f;

	//now for each light, display the appropriate hud
	EEngineLightType eLightType;
	g_pLTServer->GetLightType(hLight, eLightType);

	switch(eLightType)
	{
	case eEngineLight_Point:
		g_pLTServer->CPrint("--%s (Point)--", pszLightName);
		g_pLTServer->CPrint("      Pos: (%.2f, %.2f, %.2f)%s", VEC_EXPAND(tLightTrans.m_vPos), GetLockedStr(nMask, eChangeFlag_Position));
		g_pLTServer->CPrint("   Radius: %.2f", fLightRadius);
		g_pLTServer->CPrint("    Color: (%.2f, %.2f, %.2f) * %.2f", fR, fG, fB, fIntensity);
		g_pLTServer->CPrint("Light LOD: %s", ConvertLODToStr(eLightLOD));
		g_pLTServer->CPrint("  Shadows: World %s, Objects %s", ConvertLODToStr(eWorldShadowLOD), ConvertLODToStr(eObjectShadowLOD));
		break;
	case eEngineLight_PointFill:
		g_pLTServer->CPrint("--%s (Point Fill)--", pszLightName);
		g_pLTServer->CPrint("      Pos: (%.2f, %.2f, %.2f)%s", VEC_EXPAND(tLightTrans.m_vPos), GetLockedStr(nMask, eChangeFlag_Position));
		g_pLTServer->CPrint("   Radius: %.2f", fLightRadius);
		g_pLTServer->CPrint("    Color: (%.2f, %.2f, %.2f) * %.2f", fR, fG, fB, fIntensity);
		g_pLTServer->CPrint("Light LOD: %s", ConvertLODToStr(eLightLOD));
		break;
	case eEngineLight_Directional:
		g_pLTServer->CPrint("--%s (Directional)--", pszLightName);
		g_pLTServer->CPrint("         Pos: (%.2f, %.2f, %.2f)%s", VEC_EXPAND(tLightTrans.m_vPos), GetLockedStr(nMask, eChangeFlag_Position));
		g_pLTServer->CPrint("        Dims: (%.2f, %.2f, %.2f)", VEC_EXPAND(vDirectionalDims));
		g_pLTServer->CPrint("       Color: (%.2f, %.2f, %.2f) * %.2f", fR, fG, fB, fIntensity);
		g_pLTServer->CPrint("     Texture: %s", pszLightTexture);
		g_pLTServer->CPrint("Att. Texture: %s", pszAttenuationTexture);
		g_pLTServer->CPrint("   Light LOD: %s", ConvertLODToStr(eLightLOD));
		g_pLTServer->CPrint("     Shadows: World %s, Objects %s", ConvertLODToStr(eWorldShadowLOD), ConvertLODToStr(eObjectShadowLOD));
		break;
	case eEngineLight_SpotProjector:
		g_pLTServer->CPrint("--%s (Spot)--", pszLightName);
		g_pLTServer->CPrint("      Pos: (%.2f, %.2f, %.2f)%s", VEC_EXPAND(tLightTrans.m_vPos), GetLockedStr(nMask, eChangeFlag_Position));
		g_pLTServer->CPrint("   Radius: %.2f", fLightRadius);
		g_pLTServer->CPrint("      FOV: (%.2f, %.2f)", fFovX, fFovY);
		g_pLTServer->CPrint("     Near: %.2f", fNearZ);
		g_pLTServer->CPrint("    Color: (%.2f, %.2f, %.2f) * %.2f", fR, fG, fB, fIntensity);
		g_pLTServer->CPrint("  Texture: %s", pszLightTexture);
		g_pLTServer->CPrint("Light LOD: %s", ConvertLODToStr(eLightLOD));
		g_pLTServer->CPrint("  Shadows: World %s, Objects %s", ConvertLODToStr(eWorldShadowLOD), ConvertLODToStr(eObjectShadowLOD));
		break;
	case eEngineLight_BlackLight:
		g_pLTServer->CPrint("--%s (Black Light)--", pszLightName);
		g_pLTServer->CPrint("      Pos: (%.2f, %.2f, %.2f)%s", VEC_EXPAND(tLightTrans.m_vPos), GetLockedStr(nMask, eChangeFlag_Position));
		g_pLTServer->CPrint("   Radius: %.2f", fLightRadius);
		g_pLTServer->CPrint("      FOV: (%.2f, %.2f)", fFovX, fFovY);
		g_pLTServer->CPrint("     Near: %.2f", fNearZ);
		g_pLTServer->CPrint("    Color: (%.2f, %.2f, %.2f) * %.2f", fR, fG, fB, fIntensity);
		g_pLTServer->CPrint("  Texture: %s", pszLightTexture);
		g_pLTServer->CPrint("Light LOD: %s", ConvertLODToStr(eLightLOD));
		g_pLTServer->CPrint("  Shadows: World %s, Objects %s", ConvertLODToStr(eWorldShadowLOD), ConvertLODToStr(eObjectShadowLOD));
		break;
	case eEngineLight_CubeProjector:
		g_pLTServer->CPrint("--%s (Point)--", pszLightName);
		g_pLTServer->CPrint("      Pos: (%.2f, %.2f, %.2f)%s", VEC_EXPAND(tLightTrans.m_vPos), GetLockedStr(nMask, eChangeFlag_Position));
		g_pLTServer->CPrint("   Radius: %.2f", fLightRadius);
		g_pLTServer->CPrint("    Color: (%.2f, %.2f, %.2f) * %.2f", fR, fG, fB, fIntensity);
		g_pLTServer->CPrint("  Texture: %s", pszLightTexture);
		g_pLTServer->CPrint("Light LOD: %s", ConvertLODToStr(eLightLOD));
		g_pLTServer->CPrint("  Shadows: World %s, Objects %s", ConvertLODToStr(eWorldShadowLOD), ConvertLODToStr(eObjectShadowLOD));
		break;
	}
}

//prints out the HUD to the console
void CLightEditor::DisplayHUD()
{
	//display our base information
	g_pLTServer->CPrint("------------Light Editor------------");
	g_pLTServer->CPrint("Move Scale: %.2f", m_fMoveScale);
	g_pLTServer->CPrint("  Modified: %d %s", m_ModifiedLights.size(), m_bModifiedSinceLastSave ? "(Not Saved)" : "(Saved)");
	
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		DisplayLightHUD(EditLight.m_hLight, EditLight.m_nChangeMask);		
	}
}

//----------------------------------------------------------------------------------------------------------
// Console Commands

//the static function that is actually registered with the engine
void CLightEditor::ConsoleProgramCB(int argc, char **argv)
{
	Singleton().HandleConsoleCommand((uint32)argc, argv);
}

//called to handle a console command
void CLightEditor::HandleConsoleCommand(uint32 nArgc, const char* const* ppszArgs)
{
	//make sure that they provided a command
	if(nArgc < 1)
		return;

	//get that command
	const char* pszCommand = ppszArgs[0];
	if(LTStrIEquals(pszCommand, "Set"))
	{
		OnCommandSet(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "Add"))
	{
		OnCommandAdd(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "Move"))
	{
		OnCommandMove(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "Rotate"))
	{
		OnCommandRotate(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "MoveScale"))
	{
		OnCommandMoveScale(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "NextMoveScale"))
	{
		OnCommandNextMoveScale();
	}
	else if(LTStrIEquals(pszCommand, "MoveColor"))
	{
		OnCommandMoveColor(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "SetTexture"))
	{
		OnCommandSetTexture(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "SetAttenuationTexture"))
	{
		OnCommandSetAttenuationTexture(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "MoveIntensity"))
	{
		OnCommandMoveIntensity(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "MoveDims"))
	{
		OnCommandMoveDims(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "MoveNearClip"))
	{
		OnCommandMoveNearClip(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "MoveRadius"))
	{
		OnCommandMoveRadius(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "MoveFov"))
	{
		OnCommandMoveFov(nArgc, ppszArgs);
	}
	else if(LTStrIEquals(pszCommand, "NextWorldShadow"))
	{
		OnCommandNextWorldShadow();
	}
	else if(LTStrIEquals(pszCommand, "NextObjectShadow"))
	{
		OnCommandNextObjectShadow();
	}
	else if(LTStrIEquals(pszCommand, "NextLightLOD"))
	{
		OnCommandNextLightLOD();
	}
	else if(LTStrIEquals(pszCommand, "ToggleHud"))
	{
		OnCommandToggleHud();
	}
	else if(LTStrIEquals(pszCommand, "ToggleLightDisplay"))
	{
		OnCommandToggleLightDisplay();
	}
	else if(LTStrIEquals(pszCommand, "Save"))
	{
		OnCommandSave();
	}	
	else if(LTStrIEquals(pszCommand, "SetAimLight"))
	{
		OnCommandSetAimLight();
	}
	else if(LTStrIEquals(pszCommand, "AddAimLight"))
	{
		OnCommandAddAimLight();
	}
}

void CLightEditor::OnCommandSet(uint32 nArgc, const char* const* ppszArgs)
{
	//find the object we want to edit
	HOBJECT hLight = GetObjectArg(1, nArgc, ppszArgs);
	if(!hLight)
		return;

	//found it, so clear out our list and set it to this light
	ClearEditLights();
	AddEditLight(hLight);
}

void CLightEditor::OnCommandAdd(uint32 nArgc, const char* const* ppszArgs)
{
	//find the object we want to edit
	HOBJECT hLight = GetObjectArg(1, nArgc, ppszArgs);
	if(!hLight)
		return;

	//found it, so add it to our objects to edit
	AddEditLight(hLight);
}

void CLightEditor::OnCommandMove(uint32 nArgc, const char* const* ppszArgs)
{
	//determine the amount to offset each light
	LTVector vOffset;
	if(!GetVectorArg(1, nArgc, ppszArgs, vOffset))
		return;

	//apply the movement scale to this command
	vOffset *= m_fMoveScale;

	//apply the offset to all of the lights we are editing
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_Position))
		{
			HOBJECT hLight = EditLight.m_hLight;

			LTVector vPos;
			g_pLTServer->GetObjectPos(hLight, &vPos);
			g_pLTServer->SetObjectPos(hLight, vPos + vOffset);

			AddEditLightToModified(hLight, eChangeFlag_Position);
		}
	}
}

void CLightEditor::OnCommandRotate(uint32 nArgc, const char* const* ppszArgs)
{
	//determine the amount to offset each light
	LTVector vOffsetAngles;
	if(!GetVectorArg(1, nArgc, ppszArgs, vOffsetAngles))
		return;

	LTRotation rOffset(	MATH_DEGREES_TO_RADIANS(vOffsetAngles.x), 
						MATH_DEGREES_TO_RADIANS(vOffsetAngles.y),
						MATH_DEGREES_TO_RADIANS(vOffsetAngles.z));

	//apply the offset to all of the lights we are editing
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_Rotation))
		{
			HOBJECT hLight = EditLight.m_hLight;

			LTRotation rRot;
			g_pLTServer->GetObjectRotation(hLight, &rRot);
			g_pLTServer->SetObjectRotation(hLight, rRot * rOffset);

			AddEditLightToModified(hLight, eChangeFlag_Rotation);
		}
	}
}

void CLightEditor::OnCommandMoveScale(uint32 nArgc, const char* const* ppszArgs)
{
	float fNewScale;
	if(!GetFloatArg(1, nArgc, ppszArgs, fNewScale))
		return;

	m_fMoveScale = LTMAX(fNewScale, 0.0f);
}

void CLightEditor::OnCommandNextMoveScale()
{
	//the available movement scales we cycle through
	static const float kfMoveScales[] = { 0.1f, 1.0f, 10.0f, 100.0f };

	//run through and find the move scale that is the next largest move scale
	uint32 nListIndex = 0;
	for(; nListIndex < LTARRAYSIZE(kfMoveScales); nListIndex++)
	{
		if(m_fMoveScale < kfMoveScales[nListIndex])
			break;
	}

	//handle the case where we were larger than the largest move scale, meaning that we need to wrap
	if(nListIndex >= LTARRAYSIZE(kfMoveScales))
		nListIndex = 0;

	//and that is our new movement scale
	m_fMoveScale = kfMoveScales[nListIndex];
}

void CLightEditor::OnCommandMoveColor(uint32 nArgc, const char* const* ppszArgs)
{
	//determine the amount to offset each light
	LTVector vOffset;
	if(!GetVectorArg(1, nArgc, ppszArgs, vOffset))
		return;

	//apply the offset to all of the lights we are editing
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_Color))
		{
			HOBJECT hLight = EditLight.m_hLight;

			LTVector vColor;
			float fAlpha;
			g_pLTServer->GetObjectColor(hLight, &vColor.x, &vColor.y, &vColor.z, &fAlpha);

			//offset the color and clamp it to the allowed range of [0..1]
			vColor += vOffset;
			vColor.Max(LTVector(0.0f, 0.0f, 0.0f));
			vColor.Min(LTVector(1.0f, 1.0f, 1.0f));

			g_pLTServer->SetObjectColor(hLight, vColor.x, vColor.y, vColor.z, fAlpha);

			AddEditLightToModified(hLight, eChangeFlag_Color);
		}
	}

	UpdateEditLightVisualizations();
}

void CLightEditor::OnCommandSetTexture(uint32 nArgc, const char* const* ppszArgs)
{
	const char* pszTexture = GetStringArg(1, nArgc, ppszArgs);
	if(!pszTexture)
		return;

	//apply the new texture
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_Texture))
		{
			HOBJECT hLight = EditLight.m_hLight;
			g_pLTServer->SetLightTexture(hLight, pszTexture);

			AddEditLightToModified(hLight, eChangeFlag_Texture);
		}
	}
}

void CLightEditor::OnCommandSetAttenuationTexture(uint32 nArgc, const char* const* ppszArgs)
{
	const char* pszTexture = GetStringArg(1, nArgc, ppszArgs);
	if(!pszTexture)
		return;

	//apply the new texture
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_AttenuationTexture))
		{
			HOBJECT hLight = EditLight.m_hLight;
			g_pLTServer->SetLightAttenuationTexture(hLight, pszTexture);

			AddEditLightToModified(hLight, eChangeFlag_AttenuationTexture);
		}
	}
}

void CLightEditor::OnCommandMoveIntensity(uint32 nArgc, const char* const* ppszArgs)
{
	float fOffset;
	if(!GetFloatArg(1, nArgc, ppszArgs, fOffset))
		return;

	//apply the offset to all of the lights we are editing
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_Intensity))
		{
			HOBJECT hLight = EditLight.m_hLight;

			float fIntensity;
			g_pLTServer->GetLightIntensityScale(hLight, fIntensity);

			//offset the intensity and clamp it to the allowed range of [0..4]
			fIntensity = LTCLAMP(fIntensity + fOffset, 0.0f, 4.0f);
			g_pLTServer->SetLightIntensityScale(hLight, fIntensity);

			AddEditLightToModified(hLight, eChangeFlag_Intensity);
		}
	}
}

void CLightEditor::OnCommandMoveDims(uint32 nArgc, const char* const* ppszArgs)
{
	//determine the amount to offset each light
	LTVector vOffset;
	if(!GetVectorArg(1, nArgc, ppszArgs, vOffset))
		return;

	//apply the movement scale to this command
	vOffset *= m_fMoveScale;

	//apply the offset to all of the lights we are editing
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_Dims))
		{
			HOBJECT hLight = EditLight.m_hLight;

			LTVector vDims;
			g_pLTServer->GetLightDirectionalDims(hLight, vDims);

			//offset the color and clamp it to the allowed range of [0..+inf]
			vDims += vOffset;
			vDims.Max(LTVector(0.0f, 0.0f, 0.0f));
			g_pLTServer->SetLightDirectionalDims(hLight, vDims);

			AddEditLightToModified(hLight, eChangeFlag_Dims);
		}
	}

	UpdateEditLightVisualizations();
}

void CLightEditor::OnCommandMoveNearClip(uint32 nArgc, const char* const* ppszArgs)
{
	//determine the amount to offset each light
	float fNearOffset;
	if(!GetFloatArg(1, nArgc, ppszArgs, fNearOffset))
		return;

	//apply the movement scale to this command
	fNearOffset *= m_fMoveScale;

	//apply the offset to all of the lights we are editing
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_NearClip))
		{
			HOBJECT hLight = EditLight.m_hLight;

			float fFovX, fFovY, fNearClip;
			g_pLTServer->GetLightSpotInfo(hLight, fFovX, fFovY, fNearClip);

			//and then determine the near clip within that range
			fNearClip = LTMAX(fNearClip + fNearOffset, 0.0f);	        
			g_pLTServer->SetLightSpotInfo(hLight, fFovX, fFovY, fNearClip);

			AddEditLightToModified(hLight, eChangeFlag_NearClip);
		}
	}

	UpdateEditLightVisualizations();
}

void CLightEditor::OnCommandMoveRadius(uint32 nArgc, const char* const* ppszArgs)
{
	float fOffset;
	if(!GetFloatArg(1, nArgc, ppszArgs, fOffset))
		return;

	//apply the movement scale to this command
	fOffset *= m_fMoveScale;

	//apply the offset to all of the lights we are editing
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_Radius))
		{
			HOBJECT hLight = EditLight.m_hLight;

			float fRadius;
			g_pLTServer->GetLightRadius(hLight, fRadius);

			//offset the radius and clamp it to the allowed range of [0..+inf]
			fRadius = LTMAX(fRadius + fOffset, 0.0f);
			g_pLTServer->SetLightRadius(hLight, fRadius);

			AddEditLightToModified(hLight, eChangeFlag_Radius);
		}
	}

	UpdateEditLightVisualizations();
}

void CLightEditor::OnCommandMoveFov(uint32 nArgc, const char* const* ppszArgs)
{
	float fFovOffsetX, fFovOffsetY;
	if(!GetFloatArg(1, nArgc, ppszArgs, fFovOffsetX) || !GetFloatArg(2, nArgc, ppszArgs, fFovOffsetY))
		return;

	//convert these to radius
	fFovOffsetX = MATH_DEGREES_TO_RADIANS(fFovOffsetX);
	fFovOffsetY = MATH_DEGREES_TO_RADIANS(fFovOffsetY);

	//apply the offset to all of the lights we are editing
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_FOV))
		{
			HOBJECT hLight = EditLight.m_hLight;

			float fFovX, fFovY, fNearClip;
			g_pLTServer->GetLightSpotInfo(hLight, fFovX, fFovY, fNearClip);

			//offset the fov's and clamp it to the allowed range of [0..pi/2]
			fFovX = LTCLAMP(fFovX + fFovOffsetX, 0.0f, MATH_HALFPI);
			fFovY = LTCLAMP(fFovY + fFovOffsetY, 0.0f, MATH_HALFPI);

			g_pLTServer->SetLightSpotInfo(hLight, fFovX, fFovY, fNearClip);

			AddEditLightToModified(hLight, eChangeFlag_FOV);
		}
	}

	UpdateEditLightVisualizations();
}

void CLightEditor::OnCommandNextWorldShadow()
{
	if(m_EditLights.empty())
		return;

	//get the first light's shadow LOD and increment it
	EEngineLOD eBaseLightLOD, eBaseWorldShadowLOD, eBaseObjectShadowLOD;
	g_pLTServer->GetLightDetailSettings(m_EditLights[0]->m_hLight, eBaseLightLOD, eBaseWorldShadowLOD, eBaseObjectShadowLOD);
	EEngineLOD eNewShadowLOD = (EEngineLOD)((eBaseWorldShadowLOD + 1) % eEngineLOD_NumLODTypes);

	//now update all of the other lights to use this same LOD
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_WorldShadow))
		{
			HOBJECT hLight = EditLight.m_hLight;

			EEngineLOD eLightLOD, eWorldShadowLOD, eObjectShadowLOD;
			g_pLTServer->GetLightDetailSettings(hLight, eLightLOD, eWorldShadowLOD, eObjectShadowLOD);
			g_pLTServer->SetLightDetailSettings(hLight, eLightLOD, eNewShadowLOD, eObjectShadowLOD);

			AddEditLightToModified(hLight, eChangeFlag_WorldShadow);
		}
	}
}

void CLightEditor::OnCommandNextObjectShadow()
{
	if(m_EditLights.empty())
		return;

	//get the first light's shadow LOD and increment it
	EEngineLOD eBaseLightLOD, eBaseWorldShadowLOD, eBaseObjectShadowLOD;
	g_pLTServer->GetLightDetailSettings(m_EditLights[0]->m_hLight, eBaseLightLOD, eBaseWorldShadowLOD, eBaseObjectShadowLOD);
	EEngineLOD eNewShadowLOD = (EEngineLOD)((eBaseObjectShadowLOD + 1) % eEngineLOD_NumLODTypes);

	//now update all of the other lights to use this same LOD
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_ObjectShadow))
		{
			HOBJECT hLight = EditLight.m_hLight;

			EEngineLOD eLightLOD, eWorldShadowLOD, eObjectShadowLOD;
			g_pLTServer->GetLightDetailSettings(hLight, eLightLOD, eWorldShadowLOD, eObjectShadowLOD);
			g_pLTServer->SetLightDetailSettings(hLight, eLightLOD, eWorldShadowLOD, eNewShadowLOD);

			AddEditLightToModified(hLight, eChangeFlag_ObjectShadow);
		}
	}
}

void CLightEditor::OnCommandNextLightLOD()
{
	if(m_EditLights.empty())
		return;

	//get the first light's LOD and increment it
	EEngineLOD eBaseLightLOD, eBaseWorldShadowLOD, eBaseObjectShadowLOD;
	g_pLTServer->GetLightDetailSettings(m_EditLights[0]->m_hLight, eBaseLightLOD, eBaseWorldShadowLOD, eBaseObjectShadowLOD);
	EEngineLOD eNewLightLOD = (EEngineLOD)((eBaseLightLOD + 1) % eEngineLOD_NumLODTypes);

	//now update all of the other lights to use this same LOD
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		const SLightEntry EditLight = **itEdit;
		if(CanModifyLight(EditLight, eChangeFlag_LightLOD))
		{
			HOBJECT hLight = EditLight.m_hLight;

			EEngineLOD eLightLOD, eWorldShadowLOD, eObjectShadowLOD;
			g_pLTServer->GetLightDetailSettings(hLight, eLightLOD, eWorldShadowLOD, eObjectShadowLOD);
			g_pLTServer->SetLightDetailSettings(hLight, eNewLightLOD, eWorldShadowLOD, eObjectShadowLOD);

			AddEditLightToModified(hLight, eChangeFlag_LightLOD);
		}
	}
}

void CLightEditor::OnCommandToggleHud()
{
	m_bDisplayHud = !m_bDisplayHud;
}

void CLightEditor::OnCommandToggleLightDisplay()
{
	if(m_bDisplayLightVisualization)
	{
		m_bDisplayLightVisualization = false;
		DestroyAllLightVisualizations();
	}
	else
	{
		//make sure to set this first since the other functions may early out if it isn't set
		m_bDisplayLightVisualization = true;

		CreateLightVisualizations();
		UpdateEditLightVisualizations();
	}
}

void CLightEditor::OnCommandSave()
{
	SaveModifiedLights();
}

void CLightEditor::OnCommandSetAimLight()
{
	if(!m_hAimLight)
		return;

	ClearEditLights();
	AddEditLight(m_hAimLight);
}

void CLightEditor::OnCommandAddAimLight()
{
	if(!m_hAimLight)
		return;

	AddEditLight(m_hAimLight);
}


//----------------------------------------------------------------------------------------------------------
// Saving Support

//called to save out a specific light to the provided output stream
void CLightEditor::SaveLight(std::ofstream& OutFile, HOBJECT hLight, uint32 nChangeMask)
{
    //skip invalid lights
	if(!hLight || !nChangeMask)
		return;

	//gather all of the base light information
	char pszLightName[256];
	g_pLTServer->GetObjectName(hLight, pszLightName, LTARRAYSIZE(pszLightName));

	LTRigidTransform tLightTrans;
	g_pLTServer->GetObjectTransform(hLight, &tLightTrans);

	float fLightRadius;
	g_pLTServer->GetLightRadius(hLight, fLightRadius);

	float fR, fG, fB, fA;
	g_pLTServer->GetObjectColor(hLight, &fR, &fG, &fB, &fA);

	float fIntensity;
	g_pLTServer->GetLightIntensityScale(hLight, fIntensity);

	char pszAttenuationTexture[MAX_PATH];
	g_pLTServer->GetLightAttenuationTexture(hLight, pszAttenuationTexture, LTARRAYSIZE(pszAttenuationTexture));

	char pszLightTexture[MAX_PATH];
	g_pLTServer->GetLightTexture(hLight, pszLightTexture, LTARRAYSIZE(pszLightTexture));

	LTVector vDirectionalDims;
	g_pLTServer->GetLightDirectionalDims(hLight, vDirectionalDims);

	float fFovX, fFovY, fNearZ;
	g_pLTServer->GetLightSpotInfo(hLight, fFovX, fFovY, fNearZ);

	EEngineLOD eLightLOD, eWorldShadowLOD, eObjectShadowLOD;
	g_pLTServer->GetLightDetailSettings(hLight, eLightLOD, eWorldShadowLOD, eObjectShadowLOD);

	//make sure that our angles are in degrees instead of radians!
	fFovX = MATH_RADIANS_TO_DEGREES(fFovX);
	fFovY = MATH_RADIANS_TO_DEGREES(fFovY);

	//also, our FOV at runtime is half frustum whereas in the tools it is full frustum
	fFovX *= 2.0f;
	fFovY *= 2.0f;

	//determine our euler orientation for the editor
	EulerAngles EA = Eul_FromQuat( tLightTrans.m_rRot, EulOrdYXZr );
	float fYawDeg	= EA.x;
	float fPitchDeg = EA.y;
	float fRollDeg	= EA.z;

	//convert our colors to [0..255]
	fR *= 255.0f;
	fG *= 255.0f;
	fB *= 255.0f;

	//make sure to adjust our position by the world offset for displaying
	LTVector vSrcWorldOffset;
	g_pLTServer->GetSourceWorldOffset(vSrcWorldOffset);
	tLightTrans.m_vPos += vSrcWorldOffset;

	//now for each light, display the appropriate hud
	EEngineLightType eLightType;
	g_pLTServer->GetLightType(hLight, eLightType);

	switch(eLightType)
	{
	case eEngineLight_Point:
		WriteLine(OutFile, "[LightPoint___%s]", pszLightName);
		if(IsFlagSet(nChangeMask, eChangeFlag_Position))
			WriteLine(OutFile, "Pos = <%.3f,%.3f,%.3f>", VEC_EXPAND(tLightTrans.m_vPos));
		if(IsFlagSet(nChangeMask, eChangeFlag_Rotation))
			WriteLine(OutFile, "Rotation = <%.5f,%.5f,%.5f>", fPitchDeg, fYawDeg, fRollDeg);
		if(IsFlagSet(nChangeMask, eChangeFlag_LightLOD))
			WriteLine(OutFile, "LightLOD = %s", ConvertLODToStr(eLightLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_WorldShadow))
			WriteLine(OutFile, "WorldShadowsLOD = %s", ConvertLODToStr(eWorldShadowLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_ObjectShadow))
			WriteLine(OutFile, "ObjectShadowsLOD = %s", ConvertLODToStr(eObjectShadowLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_Radius))
			WriteLine(OutFile, "LightRadius = %.3f", fLightRadius);
		if(IsFlagSet(nChangeMask, eChangeFlag_Color))
			WriteLine(OutFile, "LightColor = <%.3f,%.3f,%.3f>", fR, fG, fB);
		if(IsFlagSet(nChangeMask, eChangeFlag_Intensity))
			WriteLine(OutFile, "IntensityScale = %.3f", fIntensity);
		break;
	case eEngineLight_PointFill:
		WriteLine(OutFile, "[LightPointFill___%s]", pszLightName);
		if(IsFlagSet(nChangeMask, eChangeFlag_Position))
			WriteLine(OutFile, "Pos = <%.3f,%.3f,%.3f>", VEC_EXPAND(tLightTrans.m_vPos));
		if(IsFlagSet(nChangeMask, eChangeFlag_Rotation))
			WriteLine(OutFile, "Rotation = <%.5f,%.5f,%.5f>", fPitchDeg, fYawDeg, fRollDeg);
		if(IsFlagSet(nChangeMask, eChangeFlag_LightLOD))
			WriteLine(OutFile, "LightLOD = %s", ConvertLODToStr(eLightLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_Radius))
			WriteLine(OutFile, "LightRadius = %.3f", fLightRadius);
		if(IsFlagSet(nChangeMask, eChangeFlag_Color))
			WriteLine(OutFile, "LightColor = <%.3f,%.3f,%.3f>", fR, fG, fB);
		if(IsFlagSet(nChangeMask, eChangeFlag_Intensity))
			WriteLine(OutFile, "IntensityScale = %.3f", fIntensity);
		break;
	case eEngineLight_Directional:
		WriteLine(OutFile, "[LightDirectional___%s]", pszLightName);
		if(IsFlagSet(nChangeMask, eChangeFlag_Position))
			WriteLine(OutFile, "Pos = <%.3f,%.3f,%.3f>", VEC_EXPAND(tLightTrans.m_vPos));
		if(IsFlagSet(nChangeMask, eChangeFlag_Rotation))
			WriteLine(OutFile, "Rotation = <%.5f,%.5f,%.5f>", fPitchDeg, fYawDeg, fRollDeg);
		if(IsFlagSet(nChangeMask, eChangeFlag_LightLOD))
			WriteLine(OutFile, "LightLOD = %s", ConvertLODToStr(eLightLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_WorldShadow))
			WriteLine(OutFile, "WorldShadowsLOD = %s", ConvertLODToStr(eWorldShadowLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_ObjectShadow))
			WriteLine(OutFile, "ObjectShadowsLOD = %s", ConvertLODToStr(eObjectShadowLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_Dims))
			WriteLine(OutFile, "Dims = <%.5f,%.5f,%.5f>", VEC_EXPAND(vDirectionalDims));
		if(IsFlagSet(nChangeMask, eChangeFlag_Color))
			WriteLine(OutFile, "LightColor = <%.3f,%.3f,%.3f>", fR, fG, fB);
		if(IsFlagSet(nChangeMask, eChangeFlag_Intensity))
			WriteLine(OutFile, "IntensityScale = %.3f", fIntensity);
		if(IsFlagSet(nChangeMask, eChangeFlag_Texture))
			WriteLine(OutFile, "Texture = %s", pszLightTexture);
		if(IsFlagSet(nChangeMask, eChangeFlag_AttenuationTexture))
			WriteLine(OutFile, "AttenuationTexture = %s", pszAttenuationTexture);
		break;
	case eEngineLight_SpotProjector:
		WriteLine(OutFile, "[LightSpot___%s]", pszLightName);
		if(IsFlagSet(nChangeMask, eChangeFlag_Position))
			WriteLine(OutFile, "Pos = <%.3f,%.3f,%.3f>", VEC_EXPAND(tLightTrans.m_vPos));
		if(IsFlagSet(nChangeMask, eChangeFlag_Rotation))
			WriteLine(OutFile, "Rotation = <%.5f,%.5f,%.5f>", fPitchDeg, fYawDeg, fRollDeg);
		if(IsFlagSet(nChangeMask, eChangeFlag_LightLOD))
			WriteLine(OutFile, "LightLOD = %s", ConvertLODToStr(eLightLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_WorldShadow))
			WriteLine(OutFile, "WorldShadowsLOD = %s", ConvertLODToStr(eWorldShadowLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_ObjectShadow))
			WriteLine(OutFile, "ObjectShadowsLOD = %s", ConvertLODToStr(eObjectShadowLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_Radius))
			WriteLine(OutFile, "LightRadius = %.3f", fLightRadius);
		if(IsFlagSet(nChangeMask, eChangeFlag_NearClip))
			WriteLine(OutFile, "NearZ = %.3f", fNearZ);
		if(IsFlagSet(nChangeMask, eChangeFlag_Color))
			WriteLine(OutFile, "LightColor = <%.3f,%.3f,%.3f>", fR, fG, fB);
		if(IsFlagSet(nChangeMask, eChangeFlag_Intensity))
			WriteLine(OutFile, "IntensityScale = %.3f", fIntensity);
		if(IsFlagSet(nChangeMask, eChangeFlag_Texture))
			WriteLine(OutFile, "Texture = %s", pszLightTexture);
		if(IsFlagSet(nChangeMask, eChangeFlag_FOV))
		{
			WriteLine(OutFile, "FovX = %.3f", fFovX);
			WriteLine(OutFile, "FovY = %.3f", fFovY);
		}
		break;
	case eEngineLight_BlackLight:
		WriteLine(OutFile, "[LightBlackLight___%s]", pszLightName);
		if(IsFlagSet(nChangeMask, eChangeFlag_Position))
			WriteLine(OutFile, "Pos = <%.3f,%.3f,%.3f>", VEC_EXPAND(tLightTrans.m_vPos));
		if(IsFlagSet(nChangeMask, eChangeFlag_Rotation))
			WriteLine(OutFile, "Rotation = <%.5f,%.5f,%.5f>", fPitchDeg, fYawDeg, fRollDeg);
		if(IsFlagSet(nChangeMask, eChangeFlag_LightLOD))
			WriteLine(OutFile, "LightLOD = %s", ConvertLODToStr(eLightLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_WorldShadow))
			WriteLine(OutFile, "WorldShadowsLOD = %s", ConvertLODToStr(eWorldShadowLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_ObjectShadow))
			WriteLine(OutFile, "ObjectShadowsLOD = %s", ConvertLODToStr(eObjectShadowLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_Radius))
			WriteLine(OutFile, "LightRadius = %.3f", fLightRadius);
		if(IsFlagSet(nChangeMask, eChangeFlag_NearClip))
			WriteLine(OutFile, "NearZ = %.3f", fNearZ);
		if(IsFlagSet(nChangeMask, eChangeFlag_Color))
			WriteLine(OutFile, "LightColor = <%.3f,%.3f,%.3f>", fR, fG, fB);
		if(IsFlagSet(nChangeMask, eChangeFlag_Intensity))
			WriteLine(OutFile, "IntensityScale = %.3f", fIntensity);
		if(IsFlagSet(nChangeMask, eChangeFlag_Texture))
			WriteLine(OutFile, "Texture = %s", pszLightTexture);
		if(IsFlagSet(nChangeMask, eChangeFlag_FOV))
		{
			WriteLine(OutFile, "FovX = %.3f", fFovX);
			WriteLine(OutFile, "FovY = %.3f", fFovY);
		}
		break;
	case eEngineLight_CubeProjector:
		WriteLine(OutFile, "[LightCube___%s]", pszLightName);
		if(IsFlagSet(nChangeMask, eChangeFlag_Position))
			WriteLine(OutFile, "Pos = <%.3f,%.3f,%.3f>", VEC_EXPAND(tLightTrans.m_vPos));
		if(IsFlagSet(nChangeMask, eChangeFlag_Rotation))
			WriteLine(OutFile, "Rotation = <%.5f,%.5f,%.5f>", fPitchDeg, fYawDeg, fRollDeg);
		if(IsFlagSet(nChangeMask, eChangeFlag_LightLOD))
			WriteLine(OutFile, "LightLOD = %s", ConvertLODToStr(eLightLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_WorldShadow))
			WriteLine(OutFile, "WorldShadowsLOD = %s", ConvertLODToStr(eWorldShadowLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_ObjectShadow))
			WriteLine(OutFile, "ObjectShadowsLOD = %s", ConvertLODToStr(eObjectShadowLOD));
		if(IsFlagSet(nChangeMask, eChangeFlag_Radius))
			WriteLine(OutFile, "LightRadius = %.3f", fLightRadius);
		if(IsFlagSet(nChangeMask, eChangeFlag_Color))
			WriteLine(OutFile, "LightColor = <%.3f,%.3f,%.3f>", fR, fG, fB);
		if(IsFlagSet(nChangeMask, eChangeFlag_Intensity))
			WriteLine(OutFile, "IntensityScale = %.3f", fIntensity);
		if(IsFlagSet(nChangeMask, eChangeFlag_Texture))
			WriteLine(OutFile, "Texture = %s", pszLightTexture);		
		break;
	}
}

//called to save out all of the modified lights to the appropriate output file
void CLightEditor::SaveModifiedLights()
{
	//don't bother saving if we don't have any modified lights
	if(m_ModifiedLights.empty())
		return;

	//determine the full name of the level
	char pszFullName[MAX_PATH];
	if(g_pLTServer->FileMgr()->GetAbsoluteFilename(m_sActiveLevel.c_str(), pszFullName, LTARRAYSIZE(pszFullName)) != LT_OK)
	{
		g_pLTServer->CPrint("Error obtaining the full path for the file %s!", m_sActiveLevel.c_str());
		return;
	}

	//determine the file name that we need to save as
	std::string sSaveFile = pszFullName;
	CResExtUtil::StripFileExtension(sSaveFile);
	sSaveFile += "_Lights.txt";

	//and open this file for writing
	std::ofstream OutFile(sSaveFile.c_str());
	if(!OutFile.good())
	{
		g_pLTServer->CPrint("Error opening the file %s for writing!", sSaveFile.c_str());
		return;
	}

	//now run through and save each of our modified lights
	for(TLightEntryList::iterator itModify = m_ModifiedLights.begin(); itModify != m_ModifiedLights.end(); itModify++)
	{
		const SLightEntry& Modified = **itModify;
		SaveLight(OutFile, Modified.m_hLight, Modified.m_nChangeMask);

		//and place a blank line to make the file more readable
		OutFile << std::endl;
	}

	//we have saved any modifications, so clear the flag
	m_bModifiedSinceLastSave = false;
}

//----------------------------------------------------------------------------------------------------------
// Edit Light List

//given a light, this will determine if it is in the edit light list
bool CLightEditor::IsEditLight(HOBJECT hLight)
{
	//do nothing if this light is already in our list
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		if((*itEdit)->m_hLight == hLight)
			return true;
	}

	return false;
}

//called to add the specified light to the list of lights to be edited. This will fail
//if the object is invalid or not a light
bool CLightEditor::AddEditLight(HOBJECT hLight)
{
	if(!hLight)
		return false;

	//do nothing if this light is already in our list
	if(IsEditLight(hLight))
		return true;

	//determine if this light is attached or not
	bool bAttached = false;
	g_pLTServer->IsObjectAttached(hLight, bAttached);	

	//setup our edit field to disable position and rotation if we are not attached
	uint32 nDisableMask = 0;
	if(bAttached)
	{
		nDisableMask |= eChangeFlag_Position;
		nDisableMask |= eChangeFlag_Rotation;
	}

	//not on our list to edit, so add it
	SLightEntry* pNewEntry = debug_new(SLightEntry);
	if(!pNewEntry)
		return false;	

	pNewEntry->m_hLight			= hLight;
	pNewEntry->m_nChangeMask	= nDisableMask;
	m_EditLights.push_back(pNewEntry);

	//if we are visualizing the lights, create a full visualization for this light
	if(m_bDisplayLightVisualization)
		CreateFullLightVisualization(hLight);

	return true;
}

//called to handle clearing out the current list of lights that are being edited
void CLightEditor::ClearEditLights()
{
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		//restore the light visualization
		if(m_bDisplayLightVisualization)
			RemoveFullLightVisualization((*itEdit)->m_hLight);

		delete *itEdit;
	}
	m_EditLights.clear();
}

//called to clear out any invalid lights from the edit list
void CLightEditor::RemoveInvalidEditLights()
{
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); )
	{
		//look for invalid edit lights and remove them from the list
		if(!(*itEdit)->m_hLight)
		{
			delete *itEdit;
			itEdit = m_EditLights.erase(itEdit);
		}
		else
		{
			itEdit++;
		}
	}
}

//called to update the visualization for the edit lights (should be called when a property that
//influences visualization changes)
void CLightEditor::UpdateEditLightVisualizations()
{
	//don't bother to update anything if we aren't displaying visualizations
	if(!m_bDisplayLightVisualization)
		return;

	//setup full visualizations for all lights we are currently editing
	for(TLightEntryList::iterator itEdit = m_EditLights.begin(); itEdit != m_EditLights.end(); itEdit++)
	{
		CreateFullLightVisualization((*itEdit)->m_hLight);
	}
}

//----------------------------------------------------------------------------------------------------------
// Modified Light List

//given a light entry, this determines whether or not it can be modified
bool CLightEditor::CanModifyLight(const SLightEntry& Entry, uint32 nModifyMask) const
{
	return ((Entry.m_nChangeMask & nModifyMask) == 0);
}

//called to clear out the list of modified lights
void CLightEditor::ClearModifiedLights()
{
	for(TLightEntryList::iterator itModify = m_ModifiedLights.begin(); itModify != m_ModifiedLights.end(); itModify++)
	{
		delete *itModify;
	}
	m_ModifiedLights.clear();

	//we no longer have any modified lights, so clear our modified flag
	m_bModifiedSinceLastSave = false;
}

//called to handle adding all of the lights on the edit list to the modified list
void CLightEditor::AddEditLightToModified(HOBJECT hLight, uint32 nModifyMask)
{
	//try to see if this is already on the modified list, if so just update our mask
	for(TLightEntryList::iterator itModify = m_ModifiedLights.begin(); itModify != m_ModifiedLights.end(); itModify++)
	{
		SLightEntry& ModifiedEntry = **itModify;
		if(hLight == ModifiedEntry.m_hLight)
		{
			//this light is already on the list, so add our mask to it
			ModifiedEntry.m_nChangeMask |= nModifyMask;
			return;
		}
	}

	//if we didn't find the light, add it onto our list
	SLightEntry* pNewEntry = debug_new(SLightEntry);
	if(pNewEntry)
	{
		pNewEntry->m_hLight			= hLight;
		pNewEntry->m_nChangeMask	= nModifyMask;
		m_ModifiedLights.push_back(pNewEntry);
	}
	
	//we have modified our lights, so set the modified flag
	m_bModifiedSinceLastSave = true;
}

//called to clear out any invalid lights from the modified list
void CLightEditor::RemoveInvalidModifiedLights()
{
	for(TLightEntryList::iterator itModify = m_ModifiedLights.begin(); itModify != m_ModifiedLights.end(); )
	{
		//look for invalid edit lights and remove them from the list
		if(!(*itModify)->m_hLight)
		{
			delete *itModify;
			itModify = m_EditLights.erase(itModify);
		}
		else
		{
			itModify++;
		}
	}
}

//----------------------------------------------------------------------------------------------------------
// Visualization of lights

//called to update the visualization of all of the lights based upon the current light positions
void CLightEditor::UpdateLightVisualizations()
{
	//run through, and update the position of all of the light visualizations, and remove those
	//that are referring to invalid lights
	for(TLightVisList::iterator itLightVis = m_LightVisualization.begin(); itLightVis != m_LightVisualization.end(); )
	{
		SLightVisualization* pLightVis = *itLightVis;

		//see if this is an invalid light at this point
		if(!pLightVis->m_hLight)
		{
			//the light is gone, clean up this object
			g_pLTServer->RemoveObject(pLightVis->m_hDebugLineSystem);
			delete pLightVis;

			//erase this item (which moves us on to the next light)
			itLightVis = m_LightVisualization.erase(itLightVis);
		}
		else
		{
			//update the position of our debug line system to match our light
			LTRigidTransform tLightTransform;
			g_pLTServer->GetObjectTransform(pLightVis->m_hLight, &tLightTransform);
			g_pLTServer->SetObjectTransform(pLightVis->m_hDebugLineSystem, tLightTransform);

			//and move onto the next light
			itLightVis++;
		}
	}
}

//called to create a base light visualization for the provided light, this will return
//the debug line system for that light, or NULL if there was a failure
DebugLineSystem* CLightEditor::CreateLightVisualization(HOBJECT hLight)
{
	//we have a light, create our debug line system for it
	char pszLightName[256];
	g_pLTServer->GetObjectName(hLight, pszLightName, LTARRAYSIZE(pszLightName));

	//add on the line system extension
	char pszLineSystemName[256];
	LTSNPrintF(pszLineSystemName, LTARRAYSIZE(pszLineSystemName), "%s_dbls", pszLightName);

	//create our system
	DebugLineSystem* pNewSystem = DebugLineSystem::Spawn(pszLineSystemName, true);

	//move the object to the current transform
	LTRigidTransform tLightTransform;
	g_pLTServer->GetObjectTransform(hLight, &tLightTransform);
	g_pLTServer->SetObjectTransform(pNewSystem->m_hObject, tLightTransform);

	//setup the name on the debug line system
	pNewSystem->SetDebugString(pszLightName);
	pNewSystem->SetDebugStringPos(LTVector::GetIdentity());

	//and add this to our list!
	SLightVisualization* pNewVis = debug_new(SLightVisualization);
	if(!pNewVis)
	{
		g_pLTServer->RemoveObject(pNewSystem->m_hObject);
		return NULL;
	}
	else
	{
		pNewVis->m_hDebugLineSystem = pNewSystem->m_hObject;
		pNewVis->m_hLight = hLight;
		m_LightVisualization.push_back(pNewVis);
	}

	return pNewSystem;
}

//given a light object this will attempt to find the debug line system that provides the visualization
//for that light, and return NULL if none can be found
DebugLineSystem* CLightEditor::FindLightVisualization(HOBJECT hLight)
{
	//find this light in our listing
	for(TLightVisList::iterator itLightVis = m_LightVisualization.begin(); itLightVis != m_LightVisualization.end(); itLightVis++)
	{
		SLightVisualization* pLightVis = *itLightVis;
		if(pLightVis->m_hLight == hLight)
		{
			return (DebugLineSystem*)g_pLTServer->HandleToObject(pLightVis->m_hDebugLineSystem);
		}
	}

	return NULL;
}

//called to create all of the initial visualizations of lights
void CLightEditor::CreateLightVisualizations()
{
	//remove existing light visualizations
	DestroyAllLightVisualizations();

	//run through all of the game objects, and for each light we come across, create a debug line system
	HOBJECT hCurrObj = g_pLTServer->GetNextObject(NULL);
	while(hCurrObj)
	{
		//see if this object is a light
		uint32 nType = 0;
		g_pLTServer->Common()->GetObjectType(hCurrObj, &nType);
		if(nType == OT_LIGHT)
		{
			DebugLineSystem* pLineSystem = CreateLightVisualization(hCurrObj);
			if(pLineSystem)
			{
				//and setup our initial geometry
				pLineSystem->AddOrientation(LTVector::GetIdentity(), LTRotation::GetIdentity(), LIGHT_ORIENTATION_ARROW_LEN);
			}			
		}

		//onto the next object
		hCurrObj = g_pLTServer->GetNextObject(hCurrObj);
	}

	//now do the same for inactive objects
	hCurrObj = g_pLTServer->GetNextInactiveObject(NULL);
	while(hCurrObj)
	{
		//see if this object is a light
		uint32 nType = 0;
		g_pLTServer->Common()->GetObjectType(hCurrObj, &nType);
		if(nType == OT_LIGHT)
		{
			DebugLineSystem* pLineSystem = CreateLightVisualization(hCurrObj);
			if(pLineSystem)
			{
				//and setup our initial geometry
				pLineSystem->AddOrientation(LTVector::GetIdentity(), LTRotation::GetIdentity(), LIGHT_ORIENTATION_ARROW_LEN);
			}			
		}

		//onto the next object
		hCurrObj = g_pLTServer->GetNextInactiveObject(hCurrObj);
	}
}

//called to destroy all currently created visualizations of lights
void CLightEditor::DestroyAllLightVisualizations()
{
	for(TLightVisList::iterator itLightVis = m_LightVisualization.begin(); itLightVis != m_LightVisualization.end(); itLightVis++)
	{
		SLightVisualization* pLightVis = *itLightVis;

		//the light is gone, clean up this object
		g_pLTServer->RemoveObject(pLightVis->m_hDebugLineSystem);
		delete pLightVis;
	}

	m_LightVisualization.resize(0);
}

//called to create a full light visualization for point, cube, and point fill
void CLightEditor::CreateFullSphereLightVisualization(HOBJECT hLight, DebugLineSystem* pLineSystem, const DebugLine::Color& LightColor)
{
	//get the light radius
	float fRadius = 0.0f;
	g_pLTServer->GetLightRadius(hLight, fRadius);

	//add our sphere
	static const uint32 knHorzSubdiv = 5;
	static const uint32 knVertSubdiv = 5;
	pLineSystem->AddSphere(LTVector::GetIdentity(), fRadius, knHorzSubdiv, knVertSubdiv, LightColor);
}

//called to create a full light visualization for spot and black lights
void CLightEditor::CreateFullSpotLightVisualization(HOBJECT hLight, DebugLineSystem* pLineSystem, const DebugLine::Color& LightColor)
{
	//get the properties for our spot projector
	float fFovX, fFovY, fNearZ;
	g_pLTServer->GetLightSpotInfo(hLight, fFovX, fFovY, fNearZ);

	//get the light radius
	float fRadius = 0.0f;
	g_pLTServer->GetLightRadius(hLight, fRadius);

	//determine the relative position and axis for our frustum
	LTVector vFrustumForward(0.0f, 0.0f, 1.0f);
	LTVector vFrustumUp(0.0f, 1.0f, 0.0f);
	LTVector vFrustumRight(1.0f, 0.0f, 0.0f);

	LTVector vFrustumOrigin(LTVector::GetIdentity());
	LTVector vFrustumFarOrigin = vFrustumOrigin + vFrustumForward * fRadius;

	//determine our scales at the end of the frustum
	float fFarX = fRadius * LTTan(fFovX);
	float fFarY = fRadius * LTTan(fFovY);

	//now generate our corner positions for our frustum
	LTVector vUL = vFrustumFarOrigin - vFrustumRight * fFarX + vFrustumUp * fFarY;
	LTVector vUR = vFrustumFarOrigin + vFrustumRight * fFarX + vFrustumUp * fFarY;
	LTVector vLR = vFrustumFarOrigin + vFrustumRight * fFarX - vFrustumUp * fFarY;
	LTVector vLL = vFrustumFarOrigin - vFrustumRight * fFarX - vFrustumUp * fFarY;

	//now construct our frustum

	//origin to plane
	pLineSystem->AddLine(vFrustumOrigin, vUL, LightColor);
	pLineSystem->AddLine(vFrustumOrigin, vUR, LightColor);
	pLineSystem->AddLine(vFrustumOrigin, vLR, LightColor);
	pLineSystem->AddLine(vFrustumOrigin, vLL, LightColor);

	//now our far plane
	pLineSystem->AddLine(vUL, vUR, LightColor);
	pLineSystem->AddLine(vUR, vLR, LightColor);
	pLineSystem->AddLine(vLR, vLL, LightColor);
	pLineSystem->AddLine(vLL, vUL, LightColor);
}

//called to create a full light visualization for directional lights
void CLightEditor::CreateFullDirectionalLightVisualization(HOBJECT hLight, DebugLineSystem* pLineSystem, const DebugLine::Color& LightColor)
{
	//get the light radius
	LTVector vDirectionalDims;
	g_pLTServer->GetLightDirectionalDims(hLight, vDirectionalDims);

	//add our sphere
	pLineSystem->AddOBB(LTOBB(LTVector::GetIdentity(), vDirectionalDims, LTRotation::GetIdentity()), LightColor);
}



//called to setup a full visualization (dimensions and all) of the specified light
void CLightEditor::CreateFullLightVisualization(HOBJECT hLight)
{
	//we can't create a full vis for an invalid light
	if(!hLight)
		return;

	//find our debug line system
	DebugLineSystem* pLineSystem = FindLightVisualization(hLight);

	//if we were unable to find the light, create a visualization for this light
	if(!pLineSystem)
	{
		pLineSystem = CreateLightVisualization(hLight);

		//if we still don't have a line system, we cannot do anything
		if(!pLineSystem)
			return;
	}

	//get our light color
	float r, g, b, a;
	g_pLTServer->GetObjectColor(hLight, &r, &g, &b, &a);
	DebugLine::Color LightColor((uint8)LTCLAMP(r * 255.0f, 0.0f, 255.0f), 
								(uint8)LTCLAMP(g * 255.0f, 0.0f, 255.0f),
								(uint8)LTCLAMP(b * 255.0f, 0.0f, 255.0f));

	//clear out any previous lines in our system
	pLineSystem->Clear();

	//add our orientation arrows
	pLineSystem->AddOrientation(LTVector::GetIdentity(), LTRotation::GetIdentity(), LIGHT_ORIENTATION_ARROW_LEN);

	//we have our line system, so update the geometry based upon the type of light that this is
	EEngineLightType eLightType;
	g_pLTServer->GetLightType(hLight, eLightType);
	switch(eLightType)
	{
	case eEngineLight_Point:
	case eEngineLight_PointFill:
	case eEngineLight_CubeProjector:
		CreateFullSphereLightVisualization(hLight, pLineSystem, LightColor);
		break;
	case eEngineLight_SpotProjector:
	case eEngineLight_BlackLight:
		CreateFullSpotLightVisualization(hLight, pLineSystem, LightColor);
		break;
	case eEngineLight_Directional:
		CreateFullDirectionalLightVisualization(hLight, pLineSystem, LightColor);
		break;
	}
}

//called to restore the default light visualization for a light, removing any full light visualization
void CLightEditor::RemoveFullLightVisualization(HOBJECT hLight)
{
	//find the object
	DebugLineSystem* pLineSystem = FindLightVisualization(hLight);
	if(!pLineSystem)
		return;

	//clear out any previous lines in our system
	pLineSystem->Clear();

	//add our orientation arrows
	pLineSystem->AddOrientation(LTVector::GetIdentity(), LTRotation::GetIdentity(), LIGHT_ORIENTATION_ARROW_LEN);
}

//----------------------------------------------------------------------------------------------------------
// Aim light

//called to update the light that is currently being aimed at
void CLightEditor::UpdateAimLight()
{
	//extract out the player view information
	const CPlayerObj::PlayerObjList& PlayerList = CPlayerObj::GetPlayerObjList();
	if(PlayerList.empty())
	{
		m_hAimLight = NULL;
		return;
	}

	LTRigidTransform tPlayerView;
	PlayerList[0]->GetViewTransform( tPlayerView );

	//the maximum distance that we allow for light selection
	static const float kfMaxLightSelDist = 10000.0f;

	//the minimum angle that the object must be within to be considered
	static const float kfMinAngleRad = MATH_DEGREES_TO_RADIANS(10.0f);

	//determine the plane information for our considered range
	LTPlane ClipPlane(tPlayerView.m_rRot.Forward(), tPlayerView.m_vPos);

	//keep track of the closest information
	HOBJECT hClosest = NULL;
	float fClosestCos = LTCos(kfMinAngleRad);

	//run through our light visualizations and find the best fit
	for(TLightVisList::iterator itLight = m_LightVisualization.begin(); itLight != m_LightVisualization.end(); itLight++)
	{
		HOBJECT hLight = (*itLight)->m_hLight;

		//skip empty lights
		if(!hLight)
			continue;		

		//get the transform of this light
		LTVector vLightPos;
		g_pLTServer->GetObjectPos(hLight, &vLightPos);

		//now see if this light is in viewing range
		float fProjDist = ClipPlane.DistTo(vLightPos);
		if((fProjDist < 0.0f) || (fProjDist > kfMaxLightSelDist))
			continue;

		//it is, now find angle to the line around the cone
		LTVector vToLight = vLightPos - tPlayerView.m_vPos;
		vToLight.Normalize();

		float fCosAngle = vToLight.Dot(ClipPlane.Normal());

		//see if we are closer
		if(fCosAngle > fClosestCos)
		{
			fClosestCos = fCosAngle;
			hClosest = hLight;
		}
	}

	m_hAimLight = hClosest;
}


