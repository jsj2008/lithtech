//----------------------------------------------------------------------------
//              
//	MODULE:		AIMovementModifier.h
//              
//	PURPOSE:	CAIMovementModifier declaration
//              
//	CREATED:	26.03.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AIMOVEMENTMODIFIER_H__
#define __AIMOVEMENTMODIFIER_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class ILTMessage_Read;
class ILTMessage_Write;

// Includes

// Forward declarations
class AIVolume;

// Globals

// Statics

//----------------------------------------------------------------------------
//              
//	CLASS:		IMovementModifier
//              
//	PURPOSE:	Pure virtual interface class for movement modifiers.
//              
//----------------------------------------------------------------------------
class IMovementModifier
{
public:
	virtual int Save(ILTMessage_Write* pMsg) = 0;
	virtual int Load(ILTMessage_Read* pMsg) = 0;

	virtual void Init() = 0;
	virtual LTVector Update( HOBJECT hObject, const LTVector& vDims, const LTVector& vPos, const LTVector& vNewPostion, AIVolume* pLastVolume ) = 0;
};

//----------------------------------------------------------------------------
//              
//	CLASS:		CHoverMovementModifier 
//              
//	PURPOSE:	Handles the vertical movement of swimming AIs.
//              
//----------------------------------------------------------------------------
class CSwimmingMovementModifier : public IMovementModifier
{	
	public:
		// Public members

		// Ctors/Dtors/etc
		CSwimmingMovementModifier();
		virtual ~CSwimmingMovementModifier();

		// Interface Implementation functions:

		virtual int Save(ILTMessage_Write* pMsg);
		virtual int Load(ILTMessage_Read* pMsg);
		virtual void Init();
		virtual LTVector Update( HOBJECT hObject, const LTVector& vDims, const LTVector& vOldPos, const LTVector& vNewPos, AIVolume* pLastVolume );

	protected:
		// Protected members

	private:
		// Private members

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		CSwimmingMovementModifier(const CSwimmingMovementModifier& rhs) {}
		CSwimmingMovementModifier& operator=(const CSwimmingMovementModifier& rhs ) {}
};

//----------------------------------------------------------------------------
//              
//	CLASS:		CHoverMovementModifier 
//              
//	PURPOSE:	Handles the vertical movement of hovering AIs.
//              
//----------------------------------------------------------------------------
class CHoverMovementModifier : public IMovementModifier
{
	public:
		// Public members

		// Ctors/Dtors/etc
		CHoverMovementModifier();
		virtual ~CHoverMovementModifier();

		// Interface Implementation functions:

		virtual int Save(ILTMessage_Write* pMsg);
		virtual int Load(ILTMessage_Read* pMsg);
		virtual void Init();
		virtual LTVector Update( HOBJECT hObject, const LTVector& vDims, const LTVector& vOldPos, const LTVector& vNewPos, AIVolume* pLastVolume );

	protected:
		// Protected members

	private:
		// Private members

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		CHoverMovementModifier(const CHoverMovementModifier& rhs) {}
		CHoverMovementModifier& operator=(const CHoverMovementModifier& rhs ) {}

		float	GetLowerBound( HOBJECT hObject, float flCheckDist, const LTVector& vDims, const LTVector& vPos );
		float	GetUpperBound( HOBJECT hObject, float flCheckDist, const LTVector& vDims, const LTVector& vPos );

		float	Interpolate( float, float, float);
		float	Snap(float, float, float);

		static const float m_cMaxVerticalDifference;
		static const float m_cCheckDist;
		static const float m_cMaxRateSpeedScalar;
		static const float m_cStoppedMovementRate;
};


#endif // __AIMOVEMENTMODIFIER_H__

