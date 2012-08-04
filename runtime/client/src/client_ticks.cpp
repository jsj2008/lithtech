
#include "bdefs.h"
#include "console.h"
#include "counter.h"

#include "client_ticks.h"

#include "clientmgr.h"

#include "render.h"
#include "debuggraphmgr.h"
#include "client_graphmgr.h"

static IClientDebugGraphMgr *g_iGraphMgr;
define_holder(IClientDebugGraphMgr, g_iGraphMgr);

// Tick status tracking class
struct STickStat
{
	STickStat() : m_nMaxTicks(0), m_nTotalTicks(0), m_nCurTicks(0) {};
	uint32 GetAveTicks(uint32 nUpdates) { return m_nTotalTicks / (nUpdates + 1); }
	void Clear() { m_nMaxTicks = 0; m_nTotalTicks = 0; m_nCurTicks = 0; }
	void Update(uint32 nClearIfZero, int nTicks) 
	{ 
		if (nClearIfZero == 0) 
			Clear(); 
		nTicks = LTMAX(nTicks, 0); 
		m_nCurTicks = (uint32)nTicks; 
		m_nTotalTicks += m_nCurTicks; 
		m_nMaxTicks = LTMAX(m_nMaxTicks, m_nCurTicks); 
	}
	uint32 m_nMaxTicks, m_nTotalTicks, m_nCurTicks;
};

extern int32 g_ShowTickCounts;

uint32 g_Ticks_Total;

// Rendering
uint32 g_Ticks_Render;
uint32 g_Ticks_Render_Objects;
uint32 g_Ticks_Render_Models;
uint32 g_Ticks_Render_WorldModels;
uint32 g_Ticks_Render_Sprites;
uint32 g_Ticks_Render_ParticleSystems;
uint32 g_Ticks_Render_Console;
uint32 g_Ticks_LastVis;

// Game
uint32 g_Ticks_ClientShell;

// Un-classified
uint32 g_Ticks_ServerUpdate;
extern uint32 g_Ticks_ClientVis;
extern uint32 g_Ticks_MoveObject;
extern uint32 g_nMoveObjectCalls;
extern uint32 g_Ticks_Intersect, g_nIntersectCalls;
extern uint32 g_Ticks_ClassUpdate;
uint32 g_Ticks_NetUpdate;
uint32 g_Ticks_Music;
uint32 g_Ticks_Sound;
uint32 g_Ticks_Input;
extern uint32 g_Ticks_UpdateObjects;

// Global time
CountPercent g_TotalGlobalTimeCounter;
unsigned long g_TotalGlobalTime = 0;

// SoundThread
uint32 g_Ticks_SoundThread;

// The client mgr (So we can get the framerate)
extern CClientMgr *g_pClientMgr;

void ctik_ResetCounters()
{
	// reset the client-side tick counts
	g_Ticks_Total = 0;
	g_Ticks_Render = 0;
	g_Ticks_NetUpdate = 0;
	g_Ticks_ClientShell = 0;
	g_Ticks_ServerUpdate = 0;
	g_Ticks_Render_Objects = 0;
	g_Ticks_Render_Models = 0;
	g_Ticks_Render_WorldModels = 0;
	g_Ticks_Render_Sprites = 0;
	g_Ticks_Render_ParticleSystems = 0;
	g_Ticks_Sound = 0;
	g_Ticks_SoundThread = 0;
	g_Ticks_Music = 0;
	g_Ticks_Render_Console = 0;
	g_Ticks_ClientVis = 0;
	g_Ticks_LastVis = g_Render.m_Time_Vis;
}


// Helper defines for the tick display function
#define TICK_PARAMS(stats, section) (((section) == 0) ? 0 : ((stats).m_nCurTicks * 100 / (section))), (stats).GetAveTicks(nUpdateCount), (stats).m_nMaxTicks, (stats).m_nCurTicks
//#define TICK_PARAMS(stats, section) (stats).m_nCurTicks * 100 / (section), (stats).GetAveTicks(nUpdateCount), (stats).m_nMaxTicks, (stats).m_nCurTicks

#define NUM_UPDATES_BEFORE_CLEAR 500

void ctik_ShowTickStatus()
{
	// Keep track of the global time
	g_TotalGlobalTime = g_TotalGlobalTimeCounter.In();
	g_TotalGlobalTimeCounter.Out();

	// Jump out if they don't want anything...
	if (g_ShowTickCounts == 0)
		return;

	// We're going to need graphmgr....
	if (!g_iGraphMgr)
	{
		dsi_ConsolePrint("!! ShowTicks Error : No GraphMgr available");
		g_ShowTickCounts = 0;
		return;
	}

	// Update the update count
	static uint nUpdateCount = 0;
	nUpdateCount = (nUpdateCount + 1) % NUM_UPDATES_BEFORE_CLEAR;

	// Calculate the total times
	int iTotalRender = (int)g_Ticks_Render + (int)g_Ticks_Render_Console;
	static STickStat sTicksRender;
	sTicksRender.Update(nUpdateCount, iTotalRender);
	int iTotalGame = ((int)g_Ticks_ClientShell + (int)g_Ticks_ClassUpdate) - ((int)sTicksRender.m_nCurTicks + (int)g_Ticks_MoveObject + (int)g_Ticks_Intersect);
	static STickStat sTicksGame;
	sTicksGame.Update(nUpdateCount, iTotalGame);
	int iTotalEngine = (int)g_Ticks_Total - ((int)sTicksRender.m_nCurTicks + (int)sTicksGame.m_nCurTicks);
	static STickStat sTicksEngine;
	sTicksEngine.Update(nUpdateCount, iTotalEngine);
	int iTotalSystem = (int)g_TotalGlobalTime - ((int)sTicksRender.m_nCurTicks + (int)sTicksGame.m_nCurTicks + (int)sTicksEngine.m_nCurTicks);
	static STickStat sTicksSystem;
	sTicksSystem.Update(nUpdateCount, iTotalSystem);

	// Sound thread counted outside other counters.  Since it's in another thread, it's
	// time is spent spread out amoung all the other counters.
	int iTotalSoundThread = (int)g_Ticks_SoundThread;
	static STickStat sTicksSoundThread;
	sTicksSoundThread.Update(nUpdateCount, iTotalSoundThread);

	// Show the graph if they want..
	if(g_iGraphMgr->Mgr()->CheckGraph(&g_ShowTickCounts, (g_ShowTickCounts & CLIENT_TICKS_GRAPH) != 0))
	{
		float fValue, fTotal;
		DGParams tickParams("ShowTicks", 0.0f, 100.0f);

		fTotal = (float)sTicksRender.m_nCurTicks + (float)sTicksGame.m_nCurTicks + (float)sTicksEngine.m_nCurTicks + (float)sTicksSystem.m_nCurTicks;

		// Renderer.
		tickParams.m_pColorTable = GetDGColorTable(PValue_Set(0, 255, 0, 0), PValue_Set(0, 255, 0, 0));
		g_iGraphMgr->Mgr()->UpdateGraph(&g_ShowTickCounts, 100.0f, tickParams);

		// Game.
		tickParams.m_pColorTable = GetDGColorTable(PValue_Set(0, 0, 255, 0), PValue_Set(0, 0, 255, 0));
		fValue = ((fTotal - (float)sTicksRender.m_nCurTicks) / fTotal) * 100.0f;
		g_iGraphMgr->Mgr()->UpdateGraph(&g_ShowTickCounts, fValue, tickParams);

		// Engine.
		tickParams.m_pColorTable = GetDGColorTable(PValue_Set(0, 0, 0, 255), PValue_Set(0, 0, 0, 255));
		fValue = ((fTotal - (float)(sTicksRender.m_nCurTicks + sTicksGame.m_nCurTicks)) / fTotal) * 100.0f;
		g_iGraphMgr->Mgr()->UpdateGraph(&g_ShowTickCounts, fValue, tickParams);
		
		// System.
		tickParams.m_pColorTable = GetDGColorTable(PValue_Set(0, 255, 255, 255), PValue_Set(0, 255, 255, 255));
		fValue = ((fTotal - (float)(sTicksRender.m_nCurTicks + sTicksGame.m_nCurTicks + sTicksEngine.m_nCurTicks)) / fTotal) * 100.0f;
		g_iGraphMgr->Mgr()->UpdateGraph(&g_ShowTickCounts, fValue, tickParams);

		// Jump out if that's all they want
		if (g_ShowTickCounts == CLIENT_TICKS_GRAPH)
			return;
	}

	// Title..
	con_WhitePrintf("--------- Tick Counts (percent / ticks) (%lu ticks per second) ----------", cnt_NumTicksPerSecond());

	// Summary info
	if ((g_ShowTickCounts & CLIENT_TICKS_SUMMARY) != 0)
	{
		static STickStat sTicksGlobal;
		sTicksGlobal.Update(nUpdateCount, g_TotalGlobalTime);
		con_Printf(CONRGB(192,192,255), 0, "---- Summary (%6.2ffps /%7u /%7u /%7u) ----", g_pClientMgr->m_FramerateTracker.GetRate(), sTicksGlobal.GetAveTicks(nUpdateCount), sTicksGlobal.m_nMaxTicks, sTicksGlobal.m_nCurTicks);
		con_WhitePrintf("     Render: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksRender, g_TotalGlobalTime));
		con_WhitePrintf("       Game: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksGame, g_TotalGlobalTime));
		con_WhitePrintf("     Engine: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksEngine, g_TotalGlobalTime));
		con_WhitePrintf("     System: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksSystem, g_TotalGlobalTime));
		con_WhitePrintf("     -------");
		con_WhitePrintf("SoundThread: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksSoundThread, g_TotalGlobalTime));
	}		

	// Rendering-specific
	if ((g_ShowTickCounts & CLIENT_TICKS_RENDER) != 0)
	{
		con_Printf(CONRGB(192,192,255), 0, "---- Rendering (%3d%% /%6u /%6u /%6u) ----", TICK_PARAMS(sTicksRender, g_TotalGlobalTime));
		static STickStat sTicksConsole;
		sTicksConsole.Update(nUpdateCount, g_Ticks_Render_Console);
		con_WhitePrintf("  Console: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksConsole, iTotalRender));
		static STickStat sTicksVis;
		sTicksVis.Update(nUpdateCount, g_Render.m_Time_Vis - g_Ticks_LastVis);
		con_WhitePrintf("      Vis: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksVis, iTotalRender));
		static STickStat sTicksMisc;
		sTicksMisc.Update(nUpdateCount, sTicksRender.m_nCurTicks - (g_Ticks_Render_Objects + g_Ticks_Render_Console + sTicksVis.m_nCurTicks));
		con_WhitePrintf("     Misc: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksMisc, iTotalRender));
		static STickStat sTicksObjects;
		sTicksObjects.Update(nUpdateCount, g_Ticks_Render_Objects);
		con_WhitePrintf("Objects  : (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksObjects, iTotalRender));
		static STickStat sTicksModels;
		sTicksModels.Update(nUpdateCount, g_Ticks_Render_Models);
		con_WhitePrintf("-      Models: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksModels, g_Ticks_Render_Objects));
		static STickStat sTicksSprites;
		sTicksSprites.Update(nUpdateCount, g_Ticks_Render_Sprites);
		con_WhitePrintf("-     Sprites: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksSprites, g_Ticks_Render_Objects));
		static STickStat sTicksParticles;
		sTicksParticles.Update(nUpdateCount, g_Ticks_Render_ParticleSystems);
		con_WhitePrintf("-   Particles: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksParticles, g_Ticks_Render_Objects));
		static STickStat sTicksWorldModels;
		sTicksWorldModels.Update(nUpdateCount, g_Ticks_Render_WorldModels);
		con_WhitePrintf("- WorldModels: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksWorldModels, g_Ticks_Render_Objects));
	}

	// Game
	if ((g_ShowTickCounts & CLIENT_TICKS_GAME) != 0)
	{
		// Keep the class percentage from going over 100, but still show the full tick count
		con_Printf(CONRGB(192,192,255), 0, "---- Game (%3d%% /%6u /%6u /%6u) ----", TICK_PARAMS(sTicksGame, g_TotalGlobalTime));
		static STickStat sTicksClass;
		sTicksClass.Update(nUpdateCount, g_Ticks_ClassUpdate);
		con_WhitePrintf(" Class Update: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksClass, sTicksGame.m_nCurTicks));
		static STickStat sTicksMisc;
		sTicksMisc.Update(nUpdateCount, sTicksGame.m_nCurTicks - g_Ticks_ClassUpdate);
		con_WhitePrintf("Client Update: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksMisc, sTicksGame.m_nCurTicks));
	}

	// Engine
	if ((g_ShowTickCounts & CLIENT_TICKS_ENGINE) != 0)
	{
		con_Printf(CONRGB(192,192,255), 0, "---- Engine (%3d%% /%6u /%6u /%6u) ----", TICK_PARAMS(sTicksEngine, g_TotalGlobalTime));
		static STickStat sTicksVis;
		sTicksVis.Update(nUpdateCount, g_Ticks_ClientVis);
		con_WhitePrintf("Client Vis: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksVis, sTicksEngine.m_nCurTicks));
		static STickStat sTicksMove;
		sTicksMove.Update(nUpdateCount, g_Ticks_MoveObject);
		con_WhitePrintf("MoveObject: (%3d%% /%6u /%6u /%6u) calls =%3d", TICK_PARAMS(sTicksMove, sTicksEngine.m_nCurTicks),
			g_nMoveObjectCalls);
		static STickStat sTicksIntersect;
		sTicksIntersect.Update(nUpdateCount, g_Ticks_Intersect);
		con_WhitePrintf(" Intersect: (%3d%% /%6u /%6u /%6u) calls =%3d", TICK_PARAMS(sTicksIntersect, sTicksEngine.m_nCurTicks), g_nIntersectCalls);
		static STickStat sTicksNet;
		sTicksNet.Update(nUpdateCount, g_Ticks_NetUpdate);
		con_WhitePrintf("Net Update: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksNet, sTicksEngine.m_nCurTicks));
		static STickStat sTicksSound;
		sTicksSound.Update(nUpdateCount, g_Ticks_Sound);
		con_WhitePrintf("     Sound: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksSound, sTicksEngine.m_nCurTicks));
		static STickStat sTicksMusic;
		sTicksMusic.Update(nUpdateCount, g_Ticks_Music);
		con_WhitePrintf("     Music: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksMusic, sTicksEngine.m_nCurTicks));
		static uint32 nLastInput = 0;
		static STickStat sTicksInput;
		sTicksInput.Update(nUpdateCount, (nLastInput) ? (g_Ticks_Input - nLastInput) : 0);
		nLastInput = g_Ticks_Input;
		con_WhitePrintf("     Input: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksInput, sTicksEngine.m_nCurTicks));
		static STickStat sTicksObjects;
		sTicksObjects.Update(nUpdateCount, g_Ticks_UpdateObjects);
		con_WhitePrintf("  CObj Upd: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksObjects, sTicksEngine.m_nCurTicks));
		static STickStat sTicksMisc;
		sTicksMisc.Update(nUpdateCount, sTicksEngine.m_nCurTicks - (g_Ticks_ClientVis + g_Ticks_MoveObject + g_Ticks_Intersect + g_Ticks_NetUpdate + g_Ticks_Sound + g_Ticks_Music + sTicksInput.m_nCurTicks + g_Ticks_UpdateObjects));
		con_WhitePrintf("      Misc: (%3d%% /%6u /%6u /%6u)", TICK_PARAMS(sTicksMisc, sTicksEngine.m_nCurTicks));
	}
			
	con_WhitePrintf("");
}

