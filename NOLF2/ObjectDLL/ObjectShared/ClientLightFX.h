// ----------------------------------------------------------------------- //
//
// MODULE  : LightFX.h
//
// PURPOSE : LightFX Inventory Item
//
// CREATED : 02/03/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHT_FX_H__
#define __LIGHT_FX_H__

#include "GameBase.h"
#include "Destructible.h"
#include "iobjectplugin.h"


LINKTO_MODULE( ClientLightFX );

class ClientLightFX : public GameBase
{
	public :

 		ClientLightFX();
		~ClientLightFX();

        LTBOOL Init();

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);
        LTBOOL  ReadProp(ObjectCreateStruct *pData);
        void    PostPropRead(ObjectCreateStruct *pStruct);
        LTBOOL  InitialUpdate(LTVector *pMovement);
        LTBOOL  Update();

		void	SendEffectMessage();

	public :

		// Member Variables

        LTBOOL		m_bStartOn;                 // Should we start on?

        LTVector	m_vColor;                   // First color to use

        LTFLOAT		m_fIntensityMin;            // How Dark light gets
        LTFLOAT		m_fIntensityMax;            // How Bright light gets
        LTFLOAT		m_fIntensityPhase;
        LTFLOAT		m_fIntensityFreq;
        uint8		m_nIntensityWaveform;

        LTFLOAT		m_fRadiusMin;               // How small light gets
        LTFLOAT		m_fRadiusMax;               // How large light gets
        LTFLOAT		m_fRadiusPhase;
        LTFLOAT		m_fRadiusFreq;
        uint8		m_nRadiusWaveform;

        LTFLOAT		m_fLifeTime;                // How long should this light stay around

        HSTRING		m_hstrRampUpSound;           // Sounds for RampUp and RampDown
        HSTRING		m_hstrRampDownSound;

        LTBOOL		m_bDynamic;                 // Was this object dynamically create

	private :

        LTFLOAT		m_fStartTime;               // When did this light get created

        uint32		m_dwLightFlags;

        LTFLOAT		m_fHitPts;
		CDestructible m_damage;

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
};


// Additional light wave classes
class SquareWaveLightFX : public ClientLightFX
{
};


class SawWaveLightFX : public ClientLightFX
{
};


class RampUpWaveLightFX : public ClientLightFX
{
};


class RampDownWaveLightFX : public ClientLightFX
{
};


class SineWaveLightFX : public ClientLightFX
{
};


class Flicker1WaveLightFX : public ClientLightFX
{
};


class Flicker2WaveLightFX : public ClientLightFX
{
};


class Flicker3WaveLightFX : public ClientLightFX
{
};


class Flicker4WaveLightFX : public ClientLightFX
{
};


class StrobeWaveLightFX : public ClientLightFX
{
};


class SearchWaveLightFX : public ClientLightFX
{
};


class FlickerLight : public ClientLightFX
{
};


class GlowingLight : public ClientLightFX
{
};


// The LightFX plugin class to handle notifications in DEdit
class CLightFXPlugin : public IObjectPlugin
{
public:
	virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

protected:

	CDestructiblePlugin m_DestructiblePlugin;

};


#endif // __LIGHT_FX_H__