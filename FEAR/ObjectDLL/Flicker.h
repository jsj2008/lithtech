// ----------------------------------------------------------------------- //
//
// MODULE  :	Flicker.h
//
// PURPOSE :	Provides the definition for the flicker object which handles
//				generating random intensity values of which it will send off
//				to other objects
//
// CREATED :	9/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __FLICKER_H__
#define __FLICKER_H__

#ifndef __GAME_BASE_H__
#	include "GameBase.h"
#endif

LINKTO_MODULE( Flicker );

class Flicker :
	public GameBase
{
public:

	Flicker();
	virtual ~Flicker();

protected:

	enum EWaveTypes
	{
		eWave_None,
		eWave_Sine,
		eWave_Cosine,
		eWave_Square,
	};

	//given a string, this will convert it into a wave type
	EWaveTypes StringToWaveType(const char* pszStr) const;

	//called to handle updating the flicker object
	void	HandleFlickerUpdate();

	//called to handle loading in of the flicker properties
	void	ReadFlickerProperties(const GenericPropList *pProps);

	//called to send the specified intensity value to the associated object
	void	SendIntensityMessage(float fIntensity);

	//called to send an intensity message based upon the current intensity values
	void	SendCurrentIntensity();

	//called to generate a random value given the provided min/max and also the wave to use
	float	GenerateRandom(float fMin, float fMax, EWaveTypes eWave, float fWavePeriod);

	//handles events sent from the engine. These are primarily messages
	//associated with saving and loading
	uint32 EngineMessageFn(uint32 messageID, void *pData, float fData);

	//handles saving and loading all data to a message stream
	void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
	void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

	// Message Handlers...
	DECLARE_MSG_HANDLER( Flicker, HandleOnMsg );
	DECLARE_MSG_HANDLER( Flicker, HandleOffMsg );
	DECLARE_MSG_HANDLER( Flicker, HandleLockMsg );
	DECLARE_MSG_HANDLER( Flicker, HandleUnlockMsg );

	//the previous flicker value (interpolating from)
	float		m_fPrevFlicker;

	//the next flicker value (interpolating to)
	float		m_fNextFlicker;

	//the total time that has elapsed
	float		m_fTotalElapsed;

	//the duration of time from when we generated the last value until we generate another new one
	float		m_fTimeDelta;

	//the amount of time left before we switch flicker values
	float		m_fNextValueTimer;

	//the range of the flickering
	float		m_fFlickerMin;
	float		m_fFlickerMax;
	EWaveTypes	m_eFlickerWave;
	float		m_fFlickerWavePeriod;
	
	//the interval for how often flicker values should be generated
	float		m_fFrequencyMin;
	float		m_fFrequencyMax;
	EWaveTypes	m_eFrequencyWave;
	float		m_fFrequencyWavePeriod;
	
	//should we interpolate
	bool		m_bInterpolate;

	//are we currently enabled
	bool		m_bEnabled;

	//are we currently locked?
	bool		m_bLocked;

	//the object that we are flickering
	LTObjRef	m_FlickerObject;

	//the name of the object that we are flickering
	std::string	m_sFlickerObjectName;
};

#endif
