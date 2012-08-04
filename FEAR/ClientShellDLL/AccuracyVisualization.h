// ----------------------------------------------------------------------- //
//
// MODULE  : AccuracyVisualization.h
//
// PURPOSE : declaration of class to visualize perturb
//
// CREATED : 04/11/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ACCURACYVISUALIZATION_H__
#define __ACCURACYVISUALIZATION_H__


// ****************************************************************************************** //

class AccuracyVisualization
{
public:

	// ------------------------------------------------------------------------------------------ //
	// Construction / destruction

	AccuracyVisualization();
	virtual ~AccuracyVisualization();

	bool			Initialize( HOBJECT hFollowObject );

	// ------------------------------------------------------------------------------------------ //
	// General updating

	void			Update( float fFrameTime );


	// ------------------------------------------------------------------------------------------ //
	// State control

	// Prevents/enables the AccuracyVisualization from being turned on...
	void			Enable( bool bEnable );

	// Turns on/off the AccuracyVisualization...
	bool			TurnOn();
	bool			TurnOff();


	// ------------------------------------------------------------------------------------------ //
	// Query interfaces

	bool			IsOn() const			{ return m_bOn; }
	bool			IsEnabled() const		{ return m_bEnabled; }

protected:

	// ------------------------------------------------------------------------------------------ //
	// Internal helper interfaces

	void			Helper_CreateLight();
	void			Helper_SetLightProperties();
	void			Helper_GetTransform( LTRigidTransform& iTrans );
	void			Helper_OffsetTranform( LTRigidTransform& iTrans, uint32 nLight );


protected:

	// ------------------------------------------------------------------------------------------ //
	// Internal data structures

	// Holds information about an individual light
	struct LightData
	{
		LightData() { Init(); }

		void Init()
		{
			m_hLight = NULL;
			m_bCreateLight = true;
			m_sLightTexture = NULL;
			m_sLightType = NULL;
			m_vLightRadiusRange.Init();
			m_nLightColor = 0xFFFFFFFF;
			m_vLightPositionOffset.Init();
			m_vLightRotationOffset.Init();
			m_vLightIntensity.Init();
			m_vLightFOV.Init();
		}

		HOBJECT					m_hLight;
		bool					m_bCreateLight;
		const char*				m_sLightTexture;
		const char*				m_sLightType;
		LTVector2				m_vLightRadiusRange;
		int32					m_nLightColor;
		LTVector				m_vLightPositionOffset;
		LTVector				m_vLightRotationOffset;
		LTVector2				m_vLightIntensity;
		LTVector2				m_vLightFOV;
	};


private:

	// Object that the light is associated with
	LTObjRef		m_hFollowObject;
	HRECORD			m_hRecord;

	// State variables
	bool			m_bOn;
	bool			m_bWasOn;
	bool			m_bEnabled;

	// Parameters gathered from the database
	LightData		m_sLight;


};

// ****************************************************************************************** //



#endif  // __ACCURACYVISUALIZATION_H__
