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

#pragma warning (disable : 4786)
#include <deque>
#include <map>
#include <string>

LINKTO_MODULE( DebugLineSystem );

class DebugLineSystem : public BaseClass
{
public:

	typedef std::deque<DebugLine> LineList;

	static DebugLineSystem * Spawn(const char * name, int max_lines = 300);

public:

	DebugLineSystem() 
		: BaseClass(OT_NORMAL),
		  nextLineToSend(lines.begin()),
		  m_nMaxLines(0),
		  m_bClearOldLines(false),
		  m_vVertexSum(0.0f,0.0f,0.0f),
		  m_fNumSummedVertices(0.0f) {}

	// Called when a new client enters.
	void ResetNextLineToSend( );

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

	void Clear();


	void SetMaxLines(uint32 max_lines) { m_nMaxLines = max_lines; }

	void SetDebugString(const char *pStr) 
	{
		if (pStr) 
			m_DebugString = pStr;
		else
			m_DebugString = "";
	}

protected:

	LTRESULT	EngineMessageFn (LTRESULT messageID, void *pData, float lData);

private:
	// These are called by EngineMessageFn.
	void ReadProp(ObjectCreateStruct *pStruct);
	void InitialUpdate(); 
	void	Update();  

	// Member Variables.
	LineList lines;
	LineList::iterator nextLineToSend;

	uint32	 m_nMaxLines;
	bool	 m_bClearOldLines;

	LTVector m_vVertexSum;
	float    m_fNumSummedVertices;

	std::string m_DebugString;
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

	DebugLineSystem & GetSystem(const std::string & name);
	DebugLineSystem & GetSystem(const void * pOwner, const std::string & name);
	
	void RemoveSystem( const std::string& name );
	void RemoveSystem( const void * pOwner, const std::string & name );
	void RemoveAll();

	// Tells all systems to resend their lines.
	void ResendAll();

	class Clear {};

	class Line
	{
	public :

		const LTVector start;
		const LTVector end;
		const DebugLine::Color color;
		const uint8    alpha;


		Line( const LTVector & new_start, 
			  const LTVector & new_end,
			  const DebugLine::Color & new_color = Color::White,
			  uint8 new_alpha = 255 )
			: start(new_start),
			  end(new_end),
			  color(new_color),
			  alpha(new_alpha) {}
	};

	inline DebugLineSystem & operator<<( DebugLineSystem & out, const LineSystem::Line & line)
	{
		out.AddLine(line.start,line.end,line.color,line.alpha);
		return out;
	}



	class Box
	{
	public :

		const LTVector position;
		const LTVector dimensions;
		const DebugLine::Color color;
		const uint8    alpha;


		Box(const LTVector & new_position, 
			const LTVector & new_dimensions,
			const DebugLine::Color & new_color = Color::White,
			uint8 new_alpha = 255 )
			: position(new_position),
			  dimensions(new_dimensions),
			  color(new_color),
			  alpha(new_alpha) {}
	};

	inline DebugLineSystem & operator<<( DebugLineSystem & out, const LineSystem::Box & box)
	{
		out.AddBox( box.position, box.dimensions, box.color, box.alpha );
		return out;
	}

	class Arrow
	{
	public :

		const LTVector start;
		const LTVector end;
		const DebugLine::Color color;
		const uint8    alpha;


		Arrow( const LTVector & new_start, 
			   const LTVector & new_end,
			   const DebugLine::Color & new_color = Color::White,
			   uint8 new_alpha = 255 )
			 : start(new_start),
			   end(new_end),
			   color(new_color),
			   alpha(new_alpha) {}
	};

	inline DebugLineSystem & operator<<( DebugLineSystem & out, const LineSystem::Arrow & arrow)
	{
		const LTFLOAT head_size = 4.0f;
		LTVector vStartToEnd = arrow.end - arrow.start;
		vStartToEnd.Norm(head_size);

		out.AddLine(arrow.start,arrow.end,arrow.color,arrow.alpha);

		out.AddLine(arrow.end,arrow.end - vStartToEnd + LTVector(head_size/2.0f,0.0f,0.0f), arrow.color,arrow.alpha);
		out.AddLine(arrow.end,arrow.end - vStartToEnd + LTVector(-head_size/2.0f,0.0f,0.0f), arrow.color,arrow.alpha);

		out.AddLine(arrow.end,arrow.end - vStartToEnd + LTVector(0.0f,head_size/2.0f,0.0f), arrow.color,arrow.alpha);
		out.AddLine(arrow.end,arrow.end - vStartToEnd + LTVector(0.0f,-head_size/2.0f,0.0f), arrow.color,arrow.alpha);

		out.AddLine(arrow.end,arrow.end - vStartToEnd + LTVector(0.0f,0.0f,head_size/2.0f), arrow.color,arrow.alpha);
		out.AddLine(arrow.end,arrow.end - vStartToEnd + LTVector(0.0f,0.0f,-head_size/2.0f), arrow.color,arrow.alpha);

		return out;
	}

} //namespace LineSystem


#endif //__DEBUG_LINE_SYSTEM_H__
