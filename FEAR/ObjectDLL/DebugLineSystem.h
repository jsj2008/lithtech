// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLineSystem.h
//
// PURPOSE :
//
// CREATED : 3/29/2000
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DEBUG_LINE_SYSTEM_H__
#define __DEBUG_LINE_SYSTEM_H__

#include "DebugLine.h"

#include <string>

LINKTO_MODULE( DebugLineSystem );

class DebugLineSystem : public BaseClass
{
public:

	typedef std::vector<DebugLine, LTAllocator<DebugLine, LT_MEM_TYPE_OBJECTSHELL> > DEBUG_LINE_LIST;

	static DebugLineSystem * Spawn(const char * name, bool bRelative = false);

public:

	DebugLineSystem() : 
		BaseClass(OT_NORMAL),
		m_bClearOldLines(false),
		m_bRelative(false)
		{}

	~DebugLineSystem();

	void AddLine(const DebugLine & new_line);

	void AddLine(const LTVector & vSource, const LTVector & vDest, 
			     const DebugLine::Color & color = DebugLine::Color(255,255,255),
				 uint8 nAlpha = 255 )
	{
		AddLine(DebugLine(vSource,vDest,color,nAlpha));
	}

	void AddBox( const LTVector & vBoxPos, const LTVector & vBoxDims, 
			     const DebugLine::Color & color = DebugLine::Color(255,255,255),
				 uint8 nAlpha = 255 );

	void AddArrow( const LTVector & vStart, const LTVector & vEnd,
				   const DebugLine::Color & color = DebugLine::Color(255,255,255),
			       uint8 nAlpha = 255 );

	void AddOrientation( const LTVector& vCenter, const LTRotation& rRot, float fLength, uint8 nAlpha = 255);

	void AddOBB(const LTOBB& rOBB,
			     const DebugLine::Color & color = DebugLine::Color(255,255,255),
				 uint8 nAlpha = 255 );

	void AddSphere(	const LTVector& vCenter, float fRadius, uint32 nHorzSubdiv, uint32 nVertSubdiv, 
					const DebugLine::Color & color = DebugLine::Color(255,255,255),
					uint8 nAlpha = 255);

	void AddSkeleton(HOBJECT hObject, 
		const DebugLine::Color & color = DebugLine::Color(255,255,255),
		uint8 nAlpha = 255 );

	void AddRigidBodyVelocities( HOBJECT hObject,
		float flMagnitudeScalar,
		const DebugLine::Color & color = DebugLine::Color(255,255,255),
		uint8 nAlpha = 255 );

	void Clear();


	void SetDebugString(const char *pStr) 
	{
		if (pStr) 
			m_DebugString = pStr;
		else
			m_DebugString = "";
	}

	void SetDebugStringPos(const LTVector& vPos) 
	{
		m_vDebugStringPos = vPos;
	}

protected:

	LTRESULT	EngineMessageFn (LTRESULT messageID, void *pData, float lData);

private:
	// These are called by EngineMessageFn.
	void	ReadProp(const GenericPropList *pProps);
	void	InitialUpdate(); 
	void	Update();  

	// Member Variables.

	DEBUG_LINE_LIST m_lstDebugLines;

	bool		m_bClearOldLines;
	bool		m_bRelative;

	std::string m_DebugString;
	LTVector	m_vDebugStringPos;
};


namespace Color
{
	const DebugLine::Color White(255,255,255);

	const DebugLine::Color Red(255,0,0);
	const DebugLine::Color Green(0,255,0);
	const DebugLine::Color Blue(0,0,255);

	const DebugLine::Color Yellow(255,255,0);
	const DebugLine::Color Purple(255,0,255);
	const DebugLine::Color Cyan(0,255,255);

	const DebugLine::Color DkRed(128,32,32);
	const DebugLine::Color DkGreen(32,128,32);
	const DebugLine::Color DkBlue(32,32,128);

};


namespace LineSystem
{

	DebugLineSystem & GetSystem(const char* name);
	DebugLineSystem & GetSystem(const void * pOwner, const char* name);
	
	void RemoveSystem( const char* name );
	void RemoveSystem( const void * pOwner, const char* name );
	void RemoveAll();
	
} //namespace LineSystem


#endif //__DEBUG_LINE_SYSTEM_H__
