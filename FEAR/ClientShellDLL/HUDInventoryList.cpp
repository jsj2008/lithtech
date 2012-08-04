// ----------------------------------------------------------------------- //
//
// MODULE  : HUDInventoryList.cpp
//
// PURPOSE : Implementation of a generic inventory list HUD element
//
// CREATED : 12/16/03
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "HUDInventoryList.h"

// ----------------------------------------------------------------------- //
// Contructor
// ----------------------------------------------------------------------- //
CHUDInventoryList::CHUDInventoryList()
{
	m_vItemOffset.Init();
}


// ----------------------------------------------------------------------- //
// Clean up before removal - delete allocated objects
// ----------------------------------------------------------------------- //
void CHUDInventoryList::Term()
{
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		debug_delete( (*iter) );
		++iter;
	}
	m_Items.clear();
}

// ----------------------------------------------------------------------- //
// Render our items to the screen
// ----------------------------------------------------------------------- //
void CHUDInventoryList::Render()
{
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		(*iter)->Render();
		++iter;
	}
}

// ----------------------------------------------------------------------- //
// Update our items
// ----------------------------------------------------------------------- //
void CHUDInventoryList::Update()
{
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		(*iter)->Update();
		++iter;
	}
}

// ----------------------------------------------------------------------- //
// Update after screen resolution change
// ----------------------------------------------------------------------- //
void CHUDInventoryList::ScaleChanged()
{
	UpdatePos();
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		(*iter)->ScaleChanged();
		++iter;
	}

}

// ----------------------------------------------------------------------- //
// update each items fade state
// ----------------------------------------------------------------------- //
void CHUDInventoryList::UpdateFade()
{
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		(*iter)->UpdateFade();
		++iter;
	}
}


// ----------------------------------------------------------------------- //
// update each items flicker state
// ----------------------------------------------------------------------- //
void CHUDInventoryList::StartFlicker()
{
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		(*iter)->StartFlicker();
		++iter;
	}
}
void CHUDInventoryList::UpdateFlicker()
{
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		(*iter)->UpdateFlicker();
		++iter;
	}
}
void CHUDInventoryList::EndFlicker()
{
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		(*iter)->EndFlicker();
		++iter;
	}
}

void CHUDInventoryList::UpdateFlash()
{
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		(*iter)->UpdateFlash();
		++iter;
	}
}

// ----------------------------------------------------------------------- //
// reset each items fade
// ----------------------------------------------------------------------- //
void CHUDInventoryList::ResetFade()
{
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		(*iter)->ResetFade();
		++iter;
	}
}

// ----------------------------------------------------------------------- //
// position each item on screen
// ----------------------------------------------------------------------- //
void CHUDInventoryList::UpdatePos()
{
	LTVector2n vCurPos = m_vBasePos;
	g_pInterfaceResMgr->ScaleScreenPos(vCurPos);
	
	// the position of our first item is the same as our base position
	// each subsequent item is placed relative to the last using m_vItemOffset
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		(*iter)->SetBasePos(vCurPos);
		vCurPos += m_vItemOffset;
		++iter;
	}
}
