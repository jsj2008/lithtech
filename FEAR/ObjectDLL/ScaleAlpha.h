// ----------------------------------------------------------------------- //
//
// MODULE  :	ScaleAlpha.h
//
// PURPOSE :	Provides the definition for the ScaleAlpha object which handles
//				interpolating intensity values of which it will send off
//				to other objects
//
// CREATED :	9/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __SCALEALPHA_H__
#define __SCALEALPHA_H__

#ifndef __GAME_BASE_H__
#	include "GameBase.h"
#endif

LINKTO_MODULE( ScaleAlpha );

class ScaleAlpha :
	public GameBase
{
public:

	ScaleAlpha();
	virtual ~ScaleAlpha();

protected:

	enum EWaveTypes
	{
		eWave_Linear,
		eWave_Sine,
	};

	//called to determine the current intensity that should be used
	float	CalcCurrentIntensity() const;

	//called to handle updating the flicker object
	void	HandleUpdate();

	//called to handle loading in of the flicker properties
	void	ReadProperties(const GenericPropList *pProps);

	//called to send the specified intensity value to the associated object
	void	SendIntensityMessage(float fIntensity);

	//handles events sent from the engine. These are primarily messages
	//associated with saving and loading
	uint32 EngineMessageFn(uint32 messageID, void *pData, float fData);

	//handles saving and loading all data to a message stream
	void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
	void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

	// Message Handlers...
	DECLARE_MSG_HANDLER( ScaleAlpha, HandleSetAlphaMsg );

	//the wave form to use for interpolating
	EWaveTypes	m_eWave;

	//the previous flicker value (interpolating from)
	float		m_fPrevValue;
    
	//the next flicker value (interpolating to)
	float		m_fNextValue;

	//the duration of time from when we generated the last value until we generate another new one
	float		m_fElapsedTime;

	//the amount of time before we hit our target value
	float		m_fInterpolateTime;

	//the object that we are flickering
	LTObjRef	m_ScaleObject;

	//the name of the object that we are flickering
	std::string	m_sScaleObjectName;
};

#endif
