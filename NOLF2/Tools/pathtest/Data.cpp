#include "stdafx.h"
#include "Data.h"
#include "World.h"

//-----------------------------------------------------------------------

void CData1::Init(CWorld* pWorld)
{
	pWorld->AddVolume( 30, 50, 100, 120 );
	pWorld->AddVolume( 100, 60, 190, 120 );
	pWorld->AddVolume( 150, 20, 410, 60 );
	pWorld->AddVolume( 320, 60, 430, 130 );
	pWorld->AddVolume( 290, 130, 400, 210 );
	pWorld->AddVolume( 290, 210, 370, 370 );
	pWorld->AddVolume( 200, 300, 290, 380 );
	pWorld->AddVolume( 130, 260, 200, 350 );
	pWorld->AddVolume( 70, 240, 130, 310 );

	pWorld->SetStart( 60, 60 );
	pWorld->SetEnd( 110, 250 );
}

//-----------------------------------------------------------------------

void CData2::Init(CWorld* pWorld)
{
	pWorld->AddVolume( 30, 50, 100, 120 );
	pWorld->AddVolume( 100, 60, 190, 120 );
	pWorld->AddVolume( 150, 20, 410, 60 );
	pWorld->AddVolume( 320, 60, 430, 130 );
	pWorld->AddVolume( 290, 130, 400, 210 );
	pWorld->AddVolume( 290, 210, 370, 370 );

	pWorld->AddVolume( 270, 280, 290, 300 );

	pWorld->AddVolume( 200, 300, 290, 380 );
	pWorld->AddVolume( 130, 260, 200, 350 );
	pWorld->AddVolume( 70, 240, 130, 310 );

	pWorld->SetStart( 60, 60 );
	pWorld->SetEnd( 110, 250 );
}

//-----------------------------------------------------------------------

void CData3::Init(CWorld* pWorld)
{
	pWorld->AddVolume( 10, 130, 190, 150 );
	pWorld->AddVolume( 20, 110, 50, 130 );
	pWorld->AddVolume( 30, 90, 60, 110 );
	pWorld->AddVolume( 40, 70, 70, 90 );
	pWorld->AddVolume( 50, 50, 80, 70 );
	pWorld->AddVolume( 60, 30, 90, 50 );
	pWorld->AddVolume( 70, 10, 130, 30 );
	pWorld->AddVolume( 110, 30, 140, 50 );
	pWorld->AddVolume( 120, 50, 150, 70 );
	pWorld->AddVolume( 130, 70, 160, 90 );
	pWorld->AddVolume( 140, 90, 170, 110 );
	pWorld->AddVolume( 150, 110, 180, 130 );

	pWorld->SetStart( 170, 140 );
	pWorld->SetEnd( 170, 120 );



/*	
	pWorld->AddVolume( 10, 10, 40, 30 );
	pWorld->AddVolume( 20, 30, 50, 50 );
	pWorld->AddVolume( 30, 50, 60, 70 );
	pWorld->AddVolume( 40, 70, 70, 90 );
	pWorld->AddVolume( 50, 90, 80, 110 );

	pWorld->SetStart( 15, 25 );
	pWorld->SetEnd( 75, 95 );
*/
}

//-----------------------------------------------------------------------
