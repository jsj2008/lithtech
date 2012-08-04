// ----------------------------------------------------------------------- //
//
// MODULE  : Controller.h
//
// PURPOSE : Controller - Definition
//
// CREATED : 4/17/99
//
// PURPOSE :
//
// The controller object is general-purpose object that can be set into one of several states
// by sending it a message.  The messages it accepts are (parenthesis means the parameter is
// necessary, brackets means the parameter is optional).
//
// FADE <parameter type> <destination value> <duration> [Wave type]
// - Fades to the specified destination value over time.
//
// FLICKER <interval min> <interval max> <message to send> [count, default -1 which means forever]
// - Sends a trigger message to the objects in a random time between the specified interval.
//
// OFF
// - Stops whatever it's doing.
//
// Supported parameter types:
// Alpha - value 0-1
// Color - values 0-255, must be specified in quotes like "1 2 3"
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__


#include "GameBaseLite.h"
#include "commonutilities.h"
#include "WaveFn.h"

LINKTO_MODULE( Controller );


	#define MAX_CONTROLLER_TARGETS	8
	#define MAX_FLICKERMSG_LEN		64
	#define FLICKER_FOREVER			0xFFFFFFFF


	typedef enum
	{
		CState_Off=0,
		CState_Fade,
		CState_Flicker
	} CState;


	typedef enum
	{
		Param_Alpha=0,	// Alpha 0 - 1
		Param_Color		// Color 0 - 255
	} ParamType;


	// Generic parameter value.
	class ParamValue
	{
	public:
		float	GetAlpha()			{return m_Color.x;}
		void	SetAlpha(float x)	{m_Color.x = x;}

        LTVector GetColor()          {return m_Color;}
        void    SetColor(LTVector x) {m_Color = x;}

		void	Load(ILTMessage_Read *pMsg);
		void	Save(ILTMessage_Write *pMsg);

        LTVector m_Color;
	};


	class FadeState
	{
	public:
					FadeState()
					{
                        m_hTarget = LTNULL;
						m_ObjectName[0] = '\0'; 
					}

		ParamValue	m_StartVal;
		char		m_ObjectName[64];	// Needs to be large to handle Prefabs
		LTObjRef	m_hTarget;			// The object being controlled.
	};


	class CommandMode
	{
	public:

	};


	class Controller : public GameBaseLite
	{
	public:

		Controller();

	protected:

    	bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	public:

		bool		ReadProp(ObjectCreateStruct *pStruct);
		void		FirstUpdate();
		void		Update();

        void        Load(ILTMessage_Read *pMsg);
        void        Save(ILTMessage_Write *pMsg);

		void		HandleFadeCommand(const CParsedMsg &cMsg);
		void		HandleFlickerCommand(const CParsedMsg &cMsg);
		void		HandleOffCommand(const CParsedMsg &cMsg);

		void		ShowTriggerError(const CParsedMsg &cMsg);

		ParamValue	ParseValue(ParamType paramType, const char *pValue);
		void		SetupCurrentValue(FadeState *pState);
		void		InterpolateValue(FadeState *pState, float t);


	public:

		CState		m_State;			// What state are we in?
        LTBOOL       m_bFirstUpdate;

		// Vars for FLICKER state.
		float		m_fNextFlickerTime;	// Next time to flicker.
		float		m_fIntervalMin;		// Flicker interval..
		float		m_fIntervalMax;
		DWORD		m_FlickerCounter;	// Decremented each time it flickers.
		char		m_FlickerMsg[MAX_FLICKERMSG_LEN];

		// Vars for FADE state.
		float		m_fStartTime;		// When we started fading.
		float		m_fDuration;		// How long the fade takes place over.
		WaveType	m_WaveType;			// What kind of wave we're using.
		ParamType	m_ParamType;		// What parameter we're controlling.
		ParamValue	m_DestValue;		// What value we're fading to.

		// One for each object.
		FadeState	m_Fades[MAX_CONTROLLER_TARGETS];
	};


#endif
