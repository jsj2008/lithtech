//------------------------------------------------------------------
//
//   MODULE  : SOUNDFX.H
//
//   PURPOSE : Defines class CSoundFX
//
//   CREATED : On 12/15/98 At 5:06:02 PM
//
//------------------------------------------------------------------

#ifndef __SOUNDFX__H__
#define __SOUNDFX__H__

// Includes....

#include "basefx.h"

class CSoundProps : public CBaseFXProps
{
  public:

	CSoundProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(	ILTInStream* pStream, const char* pszName, 
								const char* pszStringTable, const uint8* pCurveData);

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	enum constants { kMaxNumSounds=10 };

	const char*	m_pszSoundNames[kMaxNumSounds];	
	const char*	m_pszAltSoundNames[kMaxNumSounds];	
	float		m_fOuterRadius;						
	float		m_fInnerRadius;	
	float		m_fAltSoundRadius;
	float		m_fPitch;	
	uint8		m_nNumSoundNames;
	uint8		m_nNumAltSoundNames;
	uint8		m_nPriority;						
	uint8		m_nVolume;						
	uint8		m_nMixChannel;
	bool		m_bLoop;						
	bool		m_bPlayLocal;						
};


class CSoundFX : public CBaseFX
{
  public :

	CSoundFX();
	~CSoundFX();

	// Member Functions

	bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps);
	void	Term();
	bool	Update(float tmCur);

	// Accessors

	protected :

		const CSoundProps*	GetProps()	{ return (const CSoundProps*)m_pProps; }

		// Member Variables

		HLTSOUND	m_hSound;

		void		PlaySound();
		void		StopSound();
};

#endif  // __SOUNDFX__H__
