#ifndef __LIGHTEDITOR_H__
#define __LIGHTEDITOR_H__

//for DebugLine::Color
#ifndef __DEBUG_LINE_H__
#include "DebugLine.h"
#endif

#include <fstream>

//forward declarations
class DebugLineSystem;

class CLightEditor
{
public:

	~CLightEditor();

	//singleton support
	static CLightEditor& Singleton();

	//called to initialize this system
	void	Init(const char* pszWorldName);

	//called to clean up all data associated with this system
	void	Term();

	//this should be called once per frame to allow for the light editor to update itself and 
	//print out any other displays
	void	Update();

private:

	CLightEditor();
	PREVENT_OBJECT_COPYING(CLightEditor);

	//a structure represenging a light visualization
	struct SLightVisualization
	{
		//the light that we are following
		LTObjRef	m_hLight;

		//the debug line system for this light
		LTObjRef	m_hDebugLineSystem;
	};

	//a structure used to reference a light and also to keep track of either
	//which fields cannot be modified or which fields have changed
	struct SLightEntry
	{
		SLightEntry() : m_nChangeMask(0) {}

		//the actual light
		LTObjRef	m_hLight;

		//the change mask (what has changed or what cannot change)
		uint32		m_nChangeMask;
	};

	//-----------------------------------------------------
	// HUD

	//prints out the HUD for the provided light
	void	DisplayLightHUD(HOBJECT hLight, uint32 nMask);

	//prints out the HUD to the console
	void	DisplayHUD();

	//-----------------------------------------------------
	// Console Commands

	//the static function that is actually registered with the engine
	static void ConsoleProgramCB(int argc, char **argv);

	//called to handle a console command
	void	HandleConsoleCommand(uint32 nArgc, const char* const* ppszArgs);

	void	OnCommandSet(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandAdd(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandMove(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandRotate(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandMoveScale(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandNextMoveScale();
	void	OnCommandMoveColor(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandSetTexture(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandSetAttenuationTexture(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandMoveIntensity(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandMoveDims(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandMoveNearClip(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandMoveRadius(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandMoveFov(uint32 nArgc, const char* const* ppszArgs);
	void	OnCommandNextWorldShadow();
	void	OnCommandNextObjectShadow();
	void	OnCommandNextLightLOD();
	void	OnCommandToggleHud();
	void	OnCommandToggleLightDisplay();
	void	OnCommandSave();
	void	OnCommandSetAimLight();
	void	OnCommandAddAimLight();

	//-----------------------------------------------------
	// Saving Support

	//called to save out a specific light to the provided output stream
	void	SaveLight(std::ofstream& OutFile, HOBJECT hLight, uint32 nChangeMask);

	//called to save out all of the modified lights to the appropriate output file
	void	SaveModifiedLights();

	//-----------------------------------------------------
	// Edit Light List

	//given a light, this will determine if it is in the edit light list
	bool	IsEditLight(HOBJECT hLight);

	//called to add the specified light to the list of lights to be edited. This will fail
	//if the object is invalid or not a light
	bool	AddEditLight(HOBJECT hLight);

	//called to handle clearing out the current list of lights that are being edited
	void	ClearEditLights();

	//called to clear out any invalid lights from the edit list
	void	RemoveInvalidEditLights();

	//called to update the visualization for the edit lights (should be called when a property that
	//influences visualization changes)
	void	UpdateEditLightVisualizations();

	//-----------------------------------------------------
	// Modified Light List

	//given a light entry, this determines whether or not it can be modified
	bool	CanModifyLight(const SLightEntry& Entry, uint32 nMask) const;

	//called to clear out the list of modified lights
	void	ClearModifiedLights();

	//called to handle adding all of the lights on the edit list to the modified list
	void	AddEditLightToModified(HOBJECT hLight, uint32 nMask);

	//called to clear out any invalid lights from the modified list
	void	RemoveInvalidModifiedLights();

	//-----------------------------------------------------
	// Visualization of lights

	//called to update the visualization of all of the lights based upon the current light positions
	void	UpdateLightVisualizations();

	//called to create a base light visualization for the provided light, this will return
	//the debug line system for that light, or NULL if there was a failure
	DebugLineSystem*	CreateLightVisualization(HOBJECT hLight);

	//given a light object this will attempt to find the debug line system that provides the visualization
	//for that light, and return NULL if none can be found
	DebugLineSystem*	FindLightVisualization(HOBJECT hLight);

	//called to create all of the initial visualizations of lights
	void	CreateLightVisualizations();

	//called to destroy all currently created visualizations of lights
	void	DestroyAllLightVisualizations();

	//called to create a full light visualization for point, cube, and point fill
	void	CreateFullSphereLightVisualization(HOBJECT hLight, DebugLineSystem* pLineSystem, const DebugLine::Color& LightColor);

	//called to create a full light visualization for spot and black lights
	void	CreateFullSpotLightVisualization(HOBJECT hLight, DebugLineSystem* pLineSystem, const DebugLine::Color& LightColor);

	//called to create a full light visualization for directional lights
	void	CreateFullDirectionalLightVisualization(HOBJECT hLight, DebugLineSystem* pLineSystem, const DebugLine::Color& LightColor);

	//called to setup a full visualization (dimensions and all) of the specified light
	void	CreateFullLightVisualization(HOBJECT hLight);

	//called to restore the default light visualization for a light, removing any full light visualization
	void	RemoveFullLightVisualization(HOBJECT hLight);

	//-----------------------------------------------------
	// Aim light

	//called to update the light that is currently being aimed at
	void	UpdateAimLight();

	//-----------------------------------------------------
	// Data

    //the name of the currently active level
	std::string		m_sActiveLevel;

	//the light that we are currently aiming at
	LTObjRef		m_hAimLight;

	//have we modified any lights since the last save?
	bool			m_bModifiedSinceLastSave;

	//are we currently displaying the HUD?
	bool			m_bDisplayHud;

	//are we currently displaying the light debug informations?
	bool			m_bDisplayLightVisualization;

	//the movement scale that scales any movement values
	float			m_fMoveScale;

	typedef std::vector<SLightEntry*>	TLightEntryList;

	//the list of lights that we are currently modifying
	TLightEntryList	m_EditLights;

	//the list of lights that we have modified
	TLightEntryList	m_ModifiedLights;

	//the listing of the debug line systems we have created for the lights
	typedef std::vector<SLightVisualization*> TLightVisList;
	TLightVisList	m_LightVisualization;
};

#endif
