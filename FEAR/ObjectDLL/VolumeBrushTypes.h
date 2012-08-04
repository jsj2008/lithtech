// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrushTypes.h
//
// PURPOSE : VolumeBrushTypes definition
//
// CREATED : 2/16/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VOLUME_BRUSH_TYPES_H__
#define __VOLUME_BRUSH_TYPES_H__

#include "VolumeBrush.h"

LINKTO_MODULE( VolumeBrushTypes );

class Water : public VolumeBrush
{
	public:

		Water() : VolumeBrush()
		{
			m_eContainerCode = CC_WATER;
			m_eSurfaceOverrideType = ST_LIQUID;
			m_dwFlags |= FLAG_RAYHIT;
		}
		
	
		DECLARE_MSG_HANDLER( Water, HandleAllowSwimmingMsg );
};

class CorrosiveFluid : public VolumeBrush
{
	public:

		CorrosiveFluid() : VolumeBrush()
		{
			m_eContainerCode = CC_CORROSIVE_FLUID;
			m_eSurfaceOverrideType = ST_LIQUID;
			m_dwFlags |= FLAG_RAYHIT;
		}

	protected:

		virtual void ReadProp( const GenericPropList *pProps );
};


class EndlessFall : public VolumeBrush
{
	public:

		EndlessFall() : VolumeBrush()
		{
			m_eContainerCode = CC_ENDLESS_FALL;
		}

	protected:

		virtual void ReadProp( const GenericPropList *pProps );
};

class Burn : public VolumeBrush
{
	public:

		Burn() : VolumeBrush()
		{
			m_eContainerCode = CC_BURN;
		}

	protected:

		virtual void ReadProp( const GenericPropList *pProps );
};

class Electricity : public VolumeBrush
{
public:

	Electricity() : VolumeBrush()
	{
		m_eContainerCode = CC_ELECTRICITY;
	}

protected:

	virtual void ReadProp( const GenericPropList *pProps );
};


class Filter : public VolumeBrush
{
	public:

		Filter() : VolumeBrush()
		{
			m_eContainerCode = CC_FILTER;
		}

protected :

	virtual void ReadProp(const GenericPropList* pProps);
};

class SafteyNet : public VolumeBrush
{
	public:

		SafteyNet() : VolumeBrush()
		{
			m_eContainerCode = CC_SAFTEY_NET;
		}

protected :

	virtual void ReadProp(const GenericPropList* pProps);
};


class Gravity : public VolumeBrush
{
	public:

		Gravity() : VolumeBrush()
		{
			m_eContainerCode = CC_GRAVITY;
			m_fGravity = DEFAULT_WORLD_GRAVITY;
		}


	protected:

		virtual void ReadProp( const GenericPropList *pProps );
};

class Wind : public VolumeBrush
{
	public:

		Wind( ) : VolumeBrush( )
		{
			m_eContainerCode = CC_WIND;
		}
};

#endif // __VOLUME_BRUSH_TYPES_H__
