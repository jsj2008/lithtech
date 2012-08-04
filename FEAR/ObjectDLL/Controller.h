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
// OFF
// - Stops whatever it's doing.
//
// Supported parameter types:
// Alpha - value 0-1
// Color - values 0-255, must be specified in quotes like "1 2 3"
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__


#include "GameBase.h"
#include "CommonUtilities.h"
#include "WaveFn.h"

LINKTO_MODULE( Controller );


	#define MAX_CONTROLLER_TARGETS	8
	

	typedef enum
	{
		CState_Off=0,
		CState_Fade,
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
		float		GetAlpha()				{return m_Color.x;}
		void		SetAlpha(float x)		{m_Color.x = x;}

		LTVector	GetColor()				{return m_Color;}
		void		SetColor(LTVector x)	{m_Color = x;}

		void		Load(ILTMessage_Read *pMsg);
		void		Save(ILTMessage_Write *pMsg);

		LTVector	m_Color;
	};


	class FadeState
	{
	public:
					FadeState()
					{
						m_hTarget = NULL;
						m_ObjectName[0] = '\0'; 
						m_bControllAttachments = true;
					}

		ParamValue	m_StartVal;
		char		m_ObjectName[64];	// Needs to be large to handle Prefabs
		LTObjRef	m_hTarget;			// The object being controlled.
		bool		m_bControllAttachments;
	};


	class CommandMode
	{
	public:

	};


	class Controller : public GameBase
	{
	public:

		Controller();


	public:

		uint32		EngineMessageFn( uint32 messageID, void *pvData, float fData );

		bool		ReadProp(ObjectCreateStruct *pStruct);
		void		SetupTargetObjects();
		void		Update();

		void		Load(ILTMessage_Read *pMsg);
		void		Save(ILTMessage_Write *pMsg);

		void		ShowTriggerError(const CParsedMsg &cMsg);

		ParamValue	ParseValue(ParamType paramType, const char *pValue);
		void		SetupCurrentValue(FadeState *pState);
		void		InterpolateValue(FadeState *pState, float t);

		void		TurnOff();

	public:

		CState		m_State;			// What state are we in?
		bool		m_bTargetsLinked;

		// Vars for FADE state.
		double		m_fStartTime;		// When we started fading.
		float		m_fDuration;		// How long the fade takes place over.
		WaveType	m_WaveType;			// What kind of wave we're using.
		ParamType	m_ParamType;		// What parameter we're controlling.
		ParamValue	m_DestValue;		// What value we're fading to.

		// One for each object.
		FadeState	m_Fades[MAX_CONTROLLER_TARGETS];

		
		// Message Handlers...

		DECLARE_MSG_HANDLER( Controller, HandleFadeMsg );
		DECLARE_MSG_HANDLER( Controller, HandleOffMsg );
	};


#endif
