// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLineSystem.cpp
//
// PURPOSE : 
//
// CREATED : 3/29/00
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"


#include "DebugLineSystem.h"
#include "SFXMsgIds.h"
#include "DebugLine.h"
#include "MsgIDs.h"
#include "ltobjref.h"
#include <string>
#include <map>

LINKFROM_MODULE( DebugLineSystem );

BEGIN_CLASS(DebugLineSystem)
END_CLASS_FLAGS(DebugLineSystem, BaseClass, CF_HIDDEN, "This object provides the API for drawing debugging lines at runtime.")

CMDMGR_BEGIN_REGISTER_CLASS( DebugLineSystem )
CMDMGR_END_REGISTER_CLASS( DebugLineSystem, BaseClass )

namespace /* unnamed */
{
	const float s_fUpdateDelay = 0.1f;

	// There are 12 bytes of overhead (see DebugLineSystem::Update), 
	// plus 28 bytes per line.  The -100 is to allow some room just in case!
	const int   s_MaxLinesPerMessage = (MAX_PACKET_LEN - 12 - 100) / 28;
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

	typedef std::map<std::string, SystemEntry> SystemMap;
	SystemMap g_systems;

	DebugLineSystem null_line_system;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DebugLineSystem::~DebugLineSystem
//
//	PURPOSE:	Handles destroying the debug line system.  This includes 
//				removing it from the global line system list to insure no
//				dangling reference remain.
//
// ----------------------------------------------------------------------- //
DebugLineSystem::~DebugLineSystem()
{
	// If we are being destroyed, make sure we are removed from g_systems.  
	// If we aren't, it will end up with a dangling reference to us.

	// Piotr - 12/01/04
	// g_pLTServer doesn't exist in the tools and this causes a crash on exit
	if( g_pLTServer )
	{
		char szObjectName[256];
		g_pLTServer->GetObjectName(m_hObject, szObjectName, LTARRAYSIZE(szObjectName));
		LineSystem::SystemMap::iterator iter = LineSystem::g_systems.find(szObjectName);
		if( iter != LineSystem::g_systems.end() )
		{
			LineSystem::g_systems.erase(iter);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Spawn
//
//	PURPOSE:	Creates a new debug line system.
//
// ----------------------------------------------------------------------- //
DebugLineSystem * DebugLineSystem::Spawn(const char * name, bool bRelative)
{
	const HCLASS hClass = g_pLTServer->GetClass("DebugLineSystem");

	ObjectCreateStruct theStruct;

	theStruct.SetName(name);
	theStruct.m_Pos = LTVector(0.0f,0.0f,0.0f);

	theStruct.m_Flags = FLAG_FORCECLIENTUPDATE | FLAG_GOTHRUWORLD | FLAG_NOTINWORLDTREE;
	theStruct.m_ObjectType = OT_NORMAL;

	DebugLineSystem* pRV = (DebugLineSystem*)g_pLTServer->CreateObjectProps(hClass, &theStruct, "");
	if(!pRV)
		return NULL;

	pRV->m_bRelative = bRelative;
	return pRV;
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

	m_lstDebugLines.push_back( new_line );
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
					ReadProp(&pStruct->m_cProperties);
				}
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if( !m_hObject ) return 0;

			uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
			InitialUpdate();
			SetNextUpdate(m_hObject, s_fUpdateDelay);
			return dwRet;
		}

		case MID_UPDATE:
		{
			if( !m_hObject ) return 0;

			Update();
			SetNextUpdate(m_hObject, s_fUpdateDelay);
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
void DebugLineSystem::ReadProp(const GenericPropList *pProps)
{
	if( !pProps )
		return;
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
	m_lstDebugLines.clear();

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

	while( ( !m_lstDebugLines.empty() ) || m_bClearOldLines )
	{
		// Set up the message.
		CAutoMessage cMsg;

		cMsg.Writeuint8( MID_SFX_MESSAGE );

		// Record the ID and server object, used to route the message.
		cMsg.Writeuint8( SFX_DEBUGLINE_ID );
		cMsg.WriteObject( m_hObject );

		// Record the number of entries.
		int cLines = m_lstDebugLines.size();
		if( cLines < s_MaxLinesPerMessage )
		{
			cMsg.Writeuint16( cLines );
		}
		else
		{
			cMsg.Writeuint16( s_MaxLinesPerMessage );
		}

		// Tell whether we want to clear old lines or not, 
		cMsg.Writebool( m_bClearOldLines );
		cMsg.Writebool( m_bRelative );

		// Record each entry.

		int iLine=0;
		DEBUG_LINE_LIST::iterator itLine;
		for( itLine = m_lstDebugLines.begin(); itLine != m_lstDebugLines.end(); ++itLine )
		{
			cMsg.WriteType( *itLine );
			++iLine;
			if( iLine >= s_MaxLinesPerMessage )
			{
				break;
			}
		}
		m_lstDebugLines.erase( m_lstDebugLines.begin(), itLine );

		cMsg.WriteString( m_DebugString.c_str() );
		cMsg.WriteLTVector( m_vDebugStringPos );

		// Send the message!
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
		
		// If we have cleared out our lines and have no more to send,
		// why should we exist?
		if( m_bClearOldLines && ( cLines == 0 ) )
		{
			char szObjectName[256];
			g_pLTServer->GetObjectName(m_hObject, szObjectName, LTARRAYSIZE(szObjectName));
			LineSystem::SystemMap::iterator iter = LineSystem::g_systems.find( szObjectName );
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

void DebugLineSystem::AddArrow( const LTVector & vStart, const LTVector & vEnd,
								const DebugLine::Color & color /* = Color::White */,
								uint8 nAlpha /* = 255 */ )
{
	const float fHeadSize = 4.0f;
	LTVector vStartToEnd = vEnd - vStart;
	float fLen = vStartToEnd.Mag();
	if( vStartToEnd != LTVector::GetIdentity() )
	{
		vStartToEnd.Normalize();
	}

	AddLine( vStart, vEnd, color, nAlpha);

	LTVector vArrow = vStart + ( ( fLen * 0.9f ) * vStartToEnd );

	LTVector vUp( 0.f, 1.f, 0.f );
	LTVector vNorm;
	if( vStartToEnd != vUp )
	{
		vNorm = vStartToEnd.Cross( vUp );
	}
	else {
		vNorm = LTVector( 1.f, 0.f, 0.f );
	}

	vNorm *= ( fHeadSize/2.0f );
	AddLine( vArrow - vNorm, vArrow + vNorm, color, nAlpha);

	AddLine( vArrow + vNorm, vEnd, color, nAlpha);
	AddLine( vArrow - vNorm, vEnd, color, nAlpha);
}

void DebugLineSystem::AddOrientation( const LTVector& vCenter, const LTRotation& rRot, float fLength, uint8 nAlpha)
{
	AddArrow(vCenter, vCenter + rRot.Right() * fLength, Color::Red, nAlpha);
	AddArrow(vCenter, vCenter + rRot.Up() * fLength, Color::Green, nAlpha);
	AddArrow(vCenter, vCenter + rRot.Forward() * fLength, Color::Blue, nAlpha);
}

void DebugLineSystem::AddOBB(const LTOBB& rOBB,
							 const DebugLine::Color & color /* = Color::White */,
							 uint8 nAlpha /* = 255 */)
{
	LTVector vTopFrontRight = rOBB.Center() + rOBB.Right()*rOBB.HalfDims().x + rOBB.Up()*rOBB.HalfDims().y + rOBB.Forward()*rOBB.HalfDims().z;
	LTVector vTopFrontLeft = rOBB.Center() - rOBB.Right()*rOBB.HalfDims().x + rOBB.Up()*rOBB.HalfDims().y + rOBB.Forward()*rOBB.HalfDims().z;
	LTVector vTopBackLeft = rOBB.Center() - rOBB.Right()*rOBB.HalfDims().x + rOBB.Up()*rOBB.HalfDims().y - rOBB.Forward()*rOBB.HalfDims().z;
	LTVector vTopBackRight = rOBB.Center() + rOBB.Right()*rOBB.HalfDims().x + rOBB.Up()*rOBB.HalfDims().y - rOBB.Forward()*rOBB.HalfDims().z;

	LTVector vBottomFrontRight = rOBB.Center() + rOBB.Right()*rOBB.HalfDims().x - rOBB.Up()*rOBB.HalfDims().y + rOBB.Forward()*rOBB.HalfDims().z;
	LTVector vBottomFrontLeft = rOBB.Center() - rOBB.Right()*rOBB.HalfDims().x - rOBB.Up()*rOBB.HalfDims().y + rOBB.Forward()*rOBB.HalfDims().z;
	LTVector vBottomBackLeft = rOBB.Center() - rOBB.Right()*rOBB.HalfDims().x - rOBB.Up()*rOBB.HalfDims().y - rOBB.Forward()*rOBB.HalfDims().z;
	LTVector vBottomBackRight = rOBB.Center() + rOBB.Right()*rOBB.HalfDims().x - rOBB.Up()*rOBB.HalfDims().y - rOBB.Forward()*rOBB.HalfDims().z;

	AddLine(vTopFrontLeft, vTopFrontRight, color, nAlpha);
	AddLine(vTopFrontLeft, vTopBackLeft, color, nAlpha);
	AddLine(vTopFrontRight, vTopBackRight, color, nAlpha);
	AddLine(vTopBackRight, vTopBackLeft, color, nAlpha);

	AddLine(vTopFrontLeft, vBottomFrontLeft, color, nAlpha);
	AddLine(vTopFrontRight, vBottomFrontRight, color, nAlpha);
	AddLine(vTopBackRight, vBottomBackRight, color, nAlpha);
	AddLine(vTopBackLeft, vBottomBackLeft, color, nAlpha);

	AddLine(vBottomFrontLeft, vBottomFrontRight, color, nAlpha);
	AddLine(vBottomFrontLeft, vBottomBackLeft, color, nAlpha);
	AddLine(vBottomFrontRight, vBottomBackRight, color, nAlpha);
	AddLine(vBottomBackRight, vBottomBackLeft, color, nAlpha);
}

void DebugLineSystem::AddSphere(const LTVector& vCenter, float fRadius, uint32 nHorzSubdiv, uint32 nVertSubdiv, 
								const DebugLine::Color & color, uint8 nAlpha)
{
	//parameter check
	if((fRadius < 0.1f) || (nHorzSubdiv < 3) || (nVertSubdiv < 3))
		return;

	//the basis vectors, useful for equations
	const LTVector vRight	= LTVector(1.0f, 0.0f, 0.0f);
	const LTVector vUp		= LTVector(0.0f, 1.0f, 0.0f);
	const LTVector vForward = LTVector(0.0f, 0.0f, 1.0f);

	//now we need to draw the top to bottom segments
	for(uint32 nHorz = 0; nHorz < nHorzSubdiv; nHorz++)
	{
		//determine at what angle this is
		float fHorzAngle = MATH_CIRCLE * nHorz / nHorzSubdiv;

		//determine the axis in the XZ-plane, since Y will be straight up for all of these
		LTVector vAxis = vRight * LTCos(fHorzAngle) + vForward * LTSin(fHorzAngle);;

		//alright, and now draw our segment, but we already know the starting point so determine that
		LTVector vPrevPos = vCenter + vUp * fRadius;

		for(uint32 nVert = 1; nVert < nVertSubdiv; nVert++)
		{
			//determine our angle
			float fVertAngle = MATH_PI * nVert / (nVertSubdiv - 1);

			//determine our current position
			LTVector vCurrPos = vCenter + vAxis * LTSin(fVertAngle) * fRadius + vUp * LTCos(fVertAngle) * fRadius;

			//now draw a line between the positions
			AddLine(vPrevPos, vCurrPos, color, nAlpha);

			//store our position
			vPrevPos = vCurrPos;
		}
	}

	//and now do the horizontal stripes
	for(uint32 nVert = 1; nVert < nVertSubdiv - 1; nVert++)
	{
		//determine the Y value of this stripe
		float fVertAngle = MATH_PI * nVert / (nVertSubdiv - 1);

		//find the center of the sub circle
		float fY = LTCos(fVertAngle) * fRadius;
		LTVector vSubCenter = vCenter + vUp * fY;

		//and this sub cirlce radius
		float fSubRadius = LTSqrt(fRadius * fRadius - fY * fY);

		//and now make this full circle
		LTVector vPrevPos = vSubCenter + vRight * fSubRadius;

		//go up to the last item since we have to close this loop
		for(uint32 nHorz = 1; nHorz <= nHorzSubdiv; nHorz++)
		{
			//determine the angle on the circle
			float fHorzAngle = MATH_CIRCLE * nHorz / nHorzSubdiv;

			//and figure our our final point
			LTVector vCurrPos = vSubCenter + vRight * LTCos(fHorzAngle) * fSubRadius + vForward * LTSin(fHorzAngle) * fSubRadius;

			//draw the line
			AddLine(vPrevPos, vCurrPos, color, nAlpha);

			//store our position
			vPrevPos = vCurrPos;
		}
	}
}

void DebugLineSystem::AddSkeleton(HOBJECT hObject, const DebugLine::Color & color, uint8 nAlpha )
{
	HMODELNODE hNode;
	if ( LT_OK == g_pModelLT->GetRootNode( hObject, hNode ) ) 
	{
		while ( LT_OK == g_pModelLT->GetNextNode( hObject, hNode, hNode ) )
		{
			HMODELNODE hParentNode;
			if ( LT_OK != g_pModelLT->GetParent( hObject, hNode, hParentNode ) )
			{
				continue;
			}

			LTTransform tCurTransform;
			LTTransform tParentTransform;

			if ( LT_OK != g_pModelLT->GetNodeTransform( hObject, hNode, tCurTransform, true ) )
			{
				continue;
			}

			if ( LT_OK != g_pModelLT->GetNodeTransform( hObject, hParentNode, tParentTransform, true ) )
			{
				continue;
			}

			AddLine( tCurTransform.m_vPos, tParentTransform.m_vPos, color, nAlpha );
		}
	}
}

void DebugLineSystem::AddRigidBodyVelocities( HOBJECT hObject, float flMagnitudeScalar, const DebugLine::Color & color, uint8 nAlpha )
{
	//get our physics simulation interface
	ILTPhysicsSim* pPhysicsSim = g_pLTBase->PhysicsSim();

	uint32 nNumBodies = 0;
	if (pPhysicsSim->GetNumModelRigidBodies(hObject, nNumBodies) != LT_OK)
		return;

	// Draw the velocity of each of the rigid bodies
	HPHYSICSRIGIDBODY hBestRigidBody = INVALID_PHYSICS_RIGID_BODY;
	for (uint32 nCurrentBody = 0; nCurrentBody < nNumBodies; nCurrentBody++)
	{
		HPHYSICSRIGIDBODY hRigidBody;
		if (pPhysicsSim->GetModelRigidBody(hObject, nCurrentBody, hRigidBody) != LT_OK)
			continue;

		LTRigidTransform tRigidBody;
		if (pPhysicsSim->GetRigidBodyTransform(hRigidBody, tRigidBody) != LT_OK)
			continue;

		LTVector vVelocity;
		if (pPhysicsSim->GetRigidBodyVelocity(hRigidBody, vVelocity) != LT_OK)
			continue;

		AddArrow(tRigidBody.m_vPos, tRigidBody.m_vPos + (vVelocity * flMagnitudeScalar) );

		pPhysicsSim->ReleaseRigidBody(hRigidBody);
	}
}


namespace LineSystem
{

	DebugLineSystem * GetSystemPtr( SystemMap & system_map, const char* name)
	{
		SystemEntry & entry = system_map[name];

		if( !entry.hObject )
		{

#ifndef _FINAL
			entry.pLineSystem = DebugLineSystem::Spawn(name);
#endif // _FINAL
			if( entry.pLineSystem )
			{
				entry.hObject = entry.pLineSystem->m_hObject;
			}
			else
			{
				entry.pLineSystem = &null_line_system;
				entry.hObject = NULL;
			}
		}

		LTASSERT( entry.pLineSystem, "TODO: Add description here");

		return entry.pLineSystem;
	}

	DebugLineSystem & GetSystem(const char* name)
	{
		return *GetSystemPtr(g_systems,name);
	}


	DebugLineSystem & GetSystem(const void * pOwner, const char* name)
	{
		char buffer[256];
		LTSNPrintF(buffer, LTARRAYSIZE(buffer), "%d%s", reinterpret_cast<long>(pOwner), name);
		return GetSystem(buffer);
	}

	void RemoveSystem( const char* name )
	{
		SystemMap::iterator it = g_systems.find( name );
		if( it != g_systems.end( ))
		{
			DebugLineSystem* pLineSystem = it->second.pLineSystem;
			if( pLineSystem && it->second.hObject )
			{
				pLineSystem->Clear( );
			}
		}
	}

	void RemoveSystem( const void * pOwner, const char* name )
	{
		char buffer[256];
		LTSNPrintF(buffer, LTARRAYSIZE(buffer), "%d%s", reinterpret_cast<long>(pOwner), name);
		RemoveSystem(buffer);
	}

	void RemoveAll()
	{
		DebugLineSystem* pLineSystem;
		SystemMap::iterator it;
		for( it = g_systems.begin(); it != g_systems.end(); ++it )
		{
			pLineSystem = it->second.pLineSystem;
			if( pLineSystem && it->second.hObject )
			{
				pLineSystem->Clear( );
			}
		}
	}

}

