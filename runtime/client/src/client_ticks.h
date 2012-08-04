
// This module defines all the client tick counters and has functions to
// output their status to the console.

#ifndef __CLIENT_TICKS_H__
#define __CLIENT_TICKS_H__

	#include "counter.h"

	// Count for the whole update loop.
	extern uint32 g_Ticks_Total;

	// Rendering
	extern uint32 g_Ticks_Render;
	extern uint32 g_Ticks_Render_Objects;
	extern uint32 g_Ticks_Render_Models;
	extern uint32 g_Ticks_Render_WorldModels;
	extern uint32 g_Ticks_Render_Sprites;
	extern uint32 g_Ticks_Render_ParticleSystems;
	extern uint32 g_Ticks_Render_Console;

	// Game
	extern uint32 g_Ticks_ClientShell;

	// Engine
	extern uint32 g_Ticks_ServerUpdate;
	extern uint32 g_Ticks_ClassUpdate;
	extern uint32 g_Ticks_NetUpdate;
	extern uint32 g_Ticks_Sound;

	// SoundThread
	extern uint32 g_Ticks_SoundThread;

	// Tick display mask
	const uint32 CLIENT_TICKS_SUMMARY = 1;
	const uint32 CLIENT_TICKS_RENDER = 2;
	const uint32 CLIENT_TICKS_GAME = 4;
	const uint32 CLIENT_TICKS_ENGINE = 8;
	const uint32 CLIENT_TICKS_GRAPH = 0x10;
	const uint32 CLIENT_TICKS_ALL = 0xFFFFFFFF;

	void ctik_ResetCounters();
	void ctik_ShowTickStatus();

#endif  // __CLIENT_TICKS_H__



