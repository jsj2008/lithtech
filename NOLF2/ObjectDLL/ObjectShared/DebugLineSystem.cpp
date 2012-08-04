// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLineSystem.cpp
//
// PURPOSE :
//
// CREATED : 3/29/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"


#include "DebugLineSystem.h"
#include "SFXMsgIds.h"
#include "DebugLine.h"
#include "MsgIds.h"
#include "LTObjRef.h"
#if _MSC_VER >= 1300
#	include "ObjectTemplateMgr.h"
#endif // VC7

#pragma warning( disable : 4786 )
#include <hash_map>
#include <string>

LINKFROM_MODULE( DebugLineSystem );

#ifdef _DEBUG
//#define LINESYSTEM_DEBUG
#endif

BEGIN_CLASS(DebugLineSystem)
	ADD_LONGINTPROP_FLAG(MaxLines, 125, 0)
END_CLASS_DEFAULT_FLAGS(DebugLineSystem, BaseClass, NULL, NULL, CF_HIDDEN)



namespace /* unnamed */
{
	const float s_fUpdateDelay = 0.1f;

	// There are 12 bytes of overhead (see DebugLineSystem::Update),
	// plus 28 bytes per line.  The -100 is to allow some room just in case!
	const int   s_MaxLinesPerMessage = (MAX_PACKET_LEN - 12 - 100) / 28;

	void TrimToWorld( DebugLine * debug_line_ptr, const LTVector & vWorldMin, const LTVector & vWorldMax)
	{
		_ASSERT( vWorldMin.x <= vWorldMax.x );
		_ASSERT( vWorldMin.y <= vWorldMax.y );
		_ASSERT( vWorldMin.z <= vWorldMax.z );

		_ASSERT( debug_line_ptr );
		if( !debug_line_ptr ) return;

		if( debug_line_ptr->vSource.x < vWorldMin.x )
		{
			debug_line_ptr->vSource.x = vWorldMin.x;
		}
		if( debug_line_ptr->vSource.x > vWorldMax.x )
		{
			debug_line_ptr->vSource.x = vWorldMax.x;
		}

		if( debug_line_ptr->vSource.y < vWorldMin.y )
		{
			debug_line_ptr->vSource.y = vWorldMin.y;
		}
		if( debug_line_ptr->vSource.y > vWorldMax.y )
		{
			debug_line_ptr->vSource.y = vWorldMax.y;
		}

		if( debug_line_ptr->vSource.z < vWorldMin.z )
		{
			debug_line_ptr->vSource.z = vWorldMin.z;
		}
		if( debug_line_ptr->vSource.z > vWorldMax.z )
		{
			debug_line_ptr->vSource.z = vWorldMax.z;
		}

		if( debug_line_ptr->vDest.x < vWorldMin.x )
		{
			debug_line_ptr->vDest.x = vWorldMin.x;
		}
		if( debug_line_ptr->vDest.x > vWorldMax.x )
		{
			debug_line_ptr->vDest.x = vWorldMax.x;
		}

		if( debug_line_ptr->vDest.y < vWorldMin.y )
		{
			debug_line_ptr->vDest.y = vWorldMin.y;
		}
		if( debug_line_ptr->vDest.y > vWorldMax.y )
		{
			debug_line_ptr->vDest.y = vWorldMax.y;
		}

		if( debug_line_ptr->vDest.z < vWorldMin.z )
		{
			debug_line_ptr->vDest.z = vWorldMin.z;
		}
		if( debug_line_ptr->vDest.z > vWorldMax.z )
		{
			debug_line_ptr->vDest.z = vWorldMax.z;
		}
	}
};



namespace LineSystem
{
	struct SystemEntry
	{
		LTObjRef hObject;
		DebugLineSystem * pLineSystem;

		SystemEntry()
			: hObject(0),
			  pLineSystem(0) {}
	};

#if _MSC_VER >= 1300
	typedef std::hash_map< std::string, SystemEntry, ObjectTemplateMgrHashCompare > SystemMap;
#else
	typedef std::hash_map< std::string, SystemEntry > SystemMap;
#endif // VC7

	SystemMap g_systems;

	DebugLineSystem null_line_system;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Spawn
//
//	PURPOSE:	Creates a new debug line system.
//
// ----------------------------------------------------------------------- //
DebugLineSystem * DebugLineSystem::Spawn(const char * name, int max_lines /* = 300 */)
{
	const HCLASS hClass = g_pLTServer->GetClass("DebugLineSystem");

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	SAFE_STRCPY( theStruct.m_Name, name);
	theStruct.m_Pos = LTVector(0.0f,0.0f,0.0f);

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE | FLAG_NOLIGHT | FLAG_GOTHRUWORLD;
	theStruct.m_ObjectType = OT_NORMAL;

	char szProp[20];
	sprintf(szProp,"MaxLines %d", max_lines);

	return (DebugLineSystem*)g_pLTServer->CreateObjectProps(hClass, &theStruct, szProp);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::ResetNextLineToSend
//
//	PURPOSE:	Reset the nextline to send so that all lines are sent again.
//
// ----------------------------------------------------------------------- //
void DebugLineSystem::ResetNextLineToSend( )
{
	nextLineToSend = lines.begin();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::AddLine
//
//	PURPOSE:	Adds a new line to be drawn.  This should be the only
//				way a line is added to the system.
//
// ----------------------------------------------------------------------- //
void DebugLineSystem::AddLine(const DebugLine & new_line)
{
	if( !m_hObject ) return;

	// Record the offset of the sending iterator
	int send_offset = nextLineToSend - lines.begin();

	_ASSERT( (uint32)send_offset <= lines.size() );
	_ASSERT( send_offset >= 0 );

	// Trim the system if the number of lines is more than
	// the max_size.
	if( !lines.empty() && lines.size() >= m_nMaxLines )
	{
		// Adjust the centering.
		m_vVertexSum -= lines.front().vSource;
//		m_vVertexSum -= lines.front().vDest;
		m_fNumSummedVertices -= 1.0f;

		// Remove the front (oldest) element.
		lines.pop_front();

		// Reduce the send offset.
		if( send_offset > 0 ) --send_offset;
	}

	// Add the new line to the system.
	lines.push_back(new_line);

	// Trim the line to the world coordinates.
	LTVector vWorldMax, vWorldMin;
	g_pLTServer->GetWorldBox(vWorldMin,vWorldMax);
	TrimToWorld( &lines.back(), vWorldMin, vWorldMax );

	// Reset the system center.

	m_vVertexSum += lines.back().vSource;
//	m_vVertexSum += lines.back().vDest;
	m_fNumSummedVertices += 1.0f;

	LTVector engine_should_use_const_ref = (m_vVertexSum/m_fNumSummedVertices);
	g_pLTServer->SetObjectPos(m_hObject, &engine_should_use_const_ref );


#ifdef LINESYSTEM_DEBUG
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject,&vPos);

	g_pLTServer->CPrint("%f Linesystem %s located at (%.2f,%.2f,%.2f).",
		g_pLTServer->GetTime(),
		g_pLTServer->GetObjectName(m_hObject),
		vPos.x, vPos.y, vPos.z );
#endif

	// Reset nextLineToSend.  This must be done even if the iterator
	// will point to the same spot.  An iterator into a deque or vector may
	// become invalid after an insert.
	nextLineToSend = lines.begin() + send_offset;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::EngineMessageFn
//
//	PURPOSE:	The engine stuff.
//
// ----------------------------------------------------------------------- //
LTRESULT DebugLineSystem::EngineMessageFn(LTRESULT messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE :
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				{
					ReadProp(pStruct);
				}
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if( !m_hObject ) return 0;

			uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
			InitialUpdate();
			if (g_pLTServer) SetNextUpdate(m_hObject, s_fUpdateDelay);
			return dwRet;
		}

		case MID_UPDATE:
		{
			if( !m_hObject ) return 0;

			Update();
			if (g_pLTServer) SetNextUpdate(m_hObject, s_fUpdateDelay);
			break;
		}

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::ReadProp
//
//	PURPOSE:	Reads creation parameters.
//
// ----------------------------------------------------------------------- //
void DebugLineSystem::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;
	if (!pStruct) return;

	if ( g_pLTServer->GetPropGeneric( "MaxLines", &genProp ) == LT_OK )
	{
		m_nMaxLines = genProp.m_Long;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::InitialUpdate
//
//	PURPOSE:	Sets up the object's SFX message.
//
// ----------------------------------------------------------------------- //
void DebugLineSystem::InitialUpdate()
{
	//
	// Create the special fx.
	//

	// Create the message.
	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_DEBUGLINE_ID );

	// Send it.
	g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Clear
//
//	PURPOSE:	Removes all lines from the system.
//
// ----------------------------------------------------------------------- //
void DebugLineSystem::Clear()
{
	lines.clear();
	nextLineToSend = lines.end();
	m_vVertexSum = LTVector(0,0,0);
	m_fNumSummedVertices = 0.0f;
	m_bClearOldLines = true;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Update
//
//	PURPOSE:	Checks for new lines, clients, or a clear line and sends
//				the data down to any clients.
//
// ----------------------------------------------------------------------- //
void DebugLineSystem::Update()
{
	if( !m_hObject ) return;

	if( nextLineToSend != lines.end() || m_bClearOldLines )
	{
		// Set up the message.
		CAutoMessage cMsg;

		cMsg.Writeuint8( MID_SFX_MESSAGE );

		// Record the ID and server object, used to route the message.
		cMsg.Writeuint8( SFX_DEBUGLINE_ID );
		cMsg.WriteObject( m_hObject );

		// Record the number of entries.
		const int num_lines_left = (lines.end() - nextLineToSend);
		if( num_lines_left < s_MaxLinesPerMessage )
		{
			cMsg.Writeuint16( num_lines_left );
		}
		else
		{
			cMsg.Writeuint16( s_MaxLinesPerMessage );
		}

		// Record the maximum number of lines.
		cMsg.Writeuint32( m_nMaxLines );

		// Tell whether we want to clear old lines or not,
		cMsg.Writeuint8( m_bClearOldLines );

		// Record each entry.

		int num_lines_sent = 0;
		LTVector system_center(0,0,0);
		LTFLOAT  system_center_count = 0;
		while( nextLineToSend != lines.end() && num_lines_sent < s_MaxLinesPerMessage)
		{
			cMsg.WriteType( *nextLineToSend );

			++nextLineToSend;
			++num_lines_sent;
		}

#ifdef LINESYSTEM_DEBUG
		g_pLTServer->CPrint("Sent %d lines. %d lines left to send.",
			num_lines_sent, lines.end() - nextLineToSend );
#endif

		cMsg.WriteString( m_DebugString.c_str() );

		// Send the message!
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);


		// If we have cleared out our lines and have no more to send,
		// why should we exist?
		if( m_bClearOldLines && lines.empty() )
		{

			char szObjectName[256];
			g_pLTServer->GetObjectName(m_hObject, szObjectName, 256);
			LineSystem::SystemMap::iterator iter = LineSystem::g_systems.find( std::string(szObjectName) );
			if( iter != LineSystem::g_systems.end() )
			{
				LineSystem::g_systems.erase(iter);
			}

			g_pLTServer->RemoveObject(m_hObject);
		}

		// Reset m_bClearOldLines so that we don't re-enter this block.
		m_bClearOldLines = false;

	}
}


void DebugLineSystem::AddBox( const LTVector & vBoxPos, const LTVector & vBoxDims,
							   const DebugLine::Color & vColor /* = Color::White */,
							   uint8 nAlpha /* = 255 */ )
{
	// Front face.
	AddLine(vBoxPos + LTVector(-vBoxDims.x,vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(vBoxDims.x,vBoxDims.y,vBoxDims.z), vColor, nAlpha );
	AddLine(vBoxPos + LTVector(vBoxDims.x,vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(vBoxDims.x,-vBoxDims.y,vBoxDims.z), vColor, nAlpha);
	AddLine(vBoxPos + LTVector(vBoxDims.x,-vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(-vBoxDims.x,-vBoxDims.y,vBoxDims.z), vColor, nAlpha);
	AddLine(vBoxPos + LTVector(-vBoxDims.x,-vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(-vBoxDims.x,vBoxDims.y,vBoxDims.z), vColor, nAlpha);

	// Sides.
	AddLine(vBoxPos + LTVector(-vBoxDims.x,vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(-vBoxDims.x,vBoxDims.y,-vBoxDims.z), vColor, nAlpha);
	AddLine(vBoxPos + LTVector(vBoxDims.x,vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(vBoxDims.x,vBoxDims.y,-vBoxDims.z), vColor, nAlpha);
	AddLine(vBoxPos + LTVector(-vBoxDims.x,-vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(-vBoxDims.x,-vBoxDims.y,-vBoxDims.z), vColor, nAlpha);
	AddLine(vBoxPos + LTVector(vBoxDims.x,-vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(vBoxDims.x,-vBoxDims.y,-vBoxDims.z), vColor, nAlpha);

	// Back face.
	AddLine(vBoxPos + LTVector(-vBoxDims.x,vBoxDims.y,-vBoxDims.z),vBoxPos + LTVector(vBoxDims.x,vBoxDims.y,-vBoxDims.z), vColor, nAlpha);
	AddLine(vBoxPos + LTVector(vBoxDims.x,vBoxDims.y,-vBoxDims.z),vBoxPos + LTVector(vBoxDims.x,-vBoxDims.y,-vBoxDims.z), vColor, nAlpha);
	AddLine(vBoxPos + LTVector(vBoxDims.x,-vBoxDims.y,-vBoxDims.z),vBoxPos + LTVector(-vBoxDims.x,-vBoxDims.y,-vBoxDims.z), vColor, nAlpha);
	AddLine(vBoxPos + LTVector(-vBoxDims.x,-vBoxDims.y,-vBoxDims.z),vBoxPos + LTVector(-vBoxDims.x,vBoxDims.y,-vBoxDims.z), vColor, nAlpha);
}




namespace LineSystem
{

	DebugLineSystem * GetSystemPtr( SystemMap & system_map, const std::string & name)
	{
		SystemEntry & entry = system_map[name];

		if( !entry.hObject )
		{

#ifndef _FINAL
			entry.pLineSystem = DebugLineSystem::Spawn(name.c_str());
#endif // _FINAL
			if( entry.pLineSystem )
			{
				entry.hObject = entry.pLineSystem->m_hObject;
			}
			else
			{
				entry.pLineSystem = &null_line_system;
				entry.hObject = LTNULL;
			}
		}

		_ASSERT( entry.pLineSystem );

		return entry.pLineSystem;
	}

	DebugLineSystem & GetSystem(const std::string & name)
	{
		return *GetSystemPtr(g_systems,name);
	}


	DebugLineSystem & GetSystem(const void * pOwner, const std::string & name)
	{
		char buffer[20];

		return GetSystem( std::string(_ltoa(reinterpret_cast<long>(pOwner),buffer,10)) + name);
	}

	void RemoveSystem( const std::string& name )
	{
		SystemMap::iterator it = g_systems.find( name );
		if( it != g_systems.end( ))
		{
			DebugLineSystem* pLineSystem = it->second.pLineSystem;
			if( pLineSystem )
			{
				pLineSystem->Clear( );
			}
		}
	}

	void RemoveSystem( const void * pOwner, const std::string & name )
	{
		char buffer[20];

		RemoveSystem( std::string(_ltoa(reinterpret_cast<long>(pOwner),buffer,10)) + name);
	}

	void RemoveAll()
	{
		g_systems.clear( );
	}

	// Tells all systems to resend their lines.
	void ResendAll()
	{
		for( SystemMap::iterator iter = g_systems.begin();
		     iter != g_systems.end(); ++iter )
		{
			SystemEntry & entry = iter->second;

			if( entry.hObject )
			{
				entry.pLineSystem->ResetNextLineToSend( );
			}
		}
	}


}

