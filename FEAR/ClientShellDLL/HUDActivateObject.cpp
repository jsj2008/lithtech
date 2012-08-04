// ----------------------------------------------------------------------- //
//
// MODULE  : HUDActivateObject.cpp
//
// PURPOSE : HUDItem to display the current object that can be activated.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDActivateObject.h"
#include "HUDMgr.h"
#include "TargetMgr.h"
#include "PickupItemFX.h"
#include "ClientWeaponMgr.h"
#include "ActivateObjectFX.h"
#include "sys/win/mpstrconv.h"

extern VarTrack			g_vtXUIUseOldHUD;

//******************************************************************************************

CHUDActivateObject::CHUDActivateObject()
{
	m_bObjectTextureValid = false;

	m_UpdateFlags = kHUDFrame;

	m_aButton = NULL;
	for( uint32 i = 0; i < AOT_COUNT; ++i )
	{
		m_aObjects[ i ] = NULL;
	}
	
	m_iObjRef.SetReceiver( *this );

	m_nUserFlags = 0;
	m_nType = AOT_INVALID;
	m_fEffectTime = 0.0f;
	m_pActivateObject = NULL;
	m_dwEffectAlpha = 255;
}

//******************************************************************************************

bool CHUDActivateObject::Init()
{
	UpdateLayout();
	ScaleChanged();
	return true;
}

//******************************************************************************************

void CHUDActivateObject::Term()
{
}

//******************************************************************************************

void CHUDActivateObject::Render()
{

	if( m_nType != AOT_INVALID )
	{
		if (g_vtXUIUseOldHUD.GetFloat())
		{			
			SetRenderState();

			if( m_hIconTexture )
			{
				g_pDrawPrim->SetTexture( m_hIconTexture );
				g_pDrawPrim->DrawPrim( &m_IconPoly );
			}

			if( m_aObjects[ m_nType ] )
			{
				g_pDrawPrim->SetTexture( m_aObjects[ m_nType ] );
				g_pDrawPrim->DrawPrim( &m_ObjectRect );
			}

			if( m_aButton )
			{
				g_pDrawPrim->SetTexture( m_aButton );
				g_pDrawPrim->DrawPrim( &m_ButtonRect );
			}

			if (g_pGameClientShell->GetDifficulty() == GD_EASY && m_nType == AOT_GENERAL)
			{
				// Only render if we have an activate object and it is not disabled.
				if( m_pActivateObject && !m_pActivateObject->m_bDisabled )
					m_Text.Render();
			}

		}
	}
	else // we're not looking at an item pickup
	{
		m_bObjectTextureValid = false;
	}
}

//******************************************************************************************

void CHUDActivateObject::Update()
{
	uint32 nNewType = AOT_INVALID;

	// Check Target
	HOBJECT hTarget = NULL;
	
	// Get type.
	// check for forensic objects
	/*	if( !hTarget )
	{
	hTarget = g_pPlayerMgr->GetForensicObjectDetector().GetObject();
	nNewType = AOT_SCAN;
	}
	*/

	// check for pickups
	if( !hTarget )
	{
		// don't show the pickup icon if we're holding a tool (since we can't swap tools).
		CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
		bool bHoldingTool = (pWeapon && IS_ACTIVATE_FORENSIC(pWeapon->GetActivationType()));
		if (!bHoldingTool)
		{
			HOBJECT hPickupObject = g_pPlayerMgr->GetPickupObjectDetector().GetObject();
			if (hPickupObject)
			{
				// make sure we can actually get the pickup
				CPickupItemFX* pPickupItemFX = static_cast< CPickupItemFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_PICKUPITEM_ID, hPickupObject ));
				if (pPickupItemFX && (pPickupItemFX->IsMustSwap() || pPickupItemFX->CanPickup()))
				{
					hTarget = hPickupObject;
					nNewType = AOT_PICKUP;
				}
			}
		}
	}

	// check for targetable objects
	if( !hTarget )
	{
		if( g_pPlayerMgr->GetTargetMgr()->GetTargetObject() && g_pPlayerMgr->GetTargetMgr()->IsTargetInRange() )      
		{
			CActivationData data = g_pPlayerMgr->GetTargetMgr( )->GetActivationData( );
			if( data.m_nType == MID_ACTIVATE_SURFACESND )
			{
				hTarget = data.m_hTarget;
				nNewType = AOT_INVALID;
			}
			else
			{
				HOBJECT hTest = data.m_hTarget;

				// See whether or not it's disabled
				const CActivateObjectHandler *pActivateObj = CActivateObjectHandler::FindActivateObject( hTest );
				if( pActivateObj )
				{
					hTarget = hTest;
					nNewType = AOT_GENERAL;
				}
				else if( hTest )
				{
					hTarget = hTest;
					nNewType = AOT_GENERAL;
				}
			}
		}
	}

	SetObject(hTarget, nNewType);

	// Update Effect
	if( m_fEffectTime > 0.0f )
	{
		float fScale = 1.0f - ( m_fEffectTime / m_fFadeTime );
		float fFrameTime = g_pLTClient->GetFrameTime();

		// Update the current effect time
		m_fEffectTime -= fFrameTime;

		if( m_fEffectTime <= 0.0f )
		{
			if( !m_iObjRef )
			{
				m_nType = AOT_INVALID;
				return;
			}
		}

		// Set the alpha fade for each graphic
		uint8 nAlpha;

		if( m_iObjRef )
			nAlpha = ( uint8 )( m_dwEffectAlpha * fScale );
		else
			nAlpha = ( uint8 )( m_dwEffectAlpha * ( 1.0 - fScale ) );

		DrawPrimSetAlpha( m_IconPoly,	nAlpha ); 
		DrawPrimSetAlpha( m_ObjectRect, nAlpha );
		DrawPrimSetAlpha( m_ButtonRect, nAlpha );
	}
	else
	{
		if( !m_iObjRef )
		{
			m_nType = AOT_INVALID;
			return;
		}
	}
}

//******************************************************************************************

void CHUDActivateObject::SetObject( LTObjRef iObjRef, uint32 nNewType )
{
	// Check to see if this is a valid activate object...
	if( nNewType == AOT_INVALID )
	{
		if( m_iObjRef.GetData() )
		{
			m_iObjRef = NULL;
			m_nUserFlags = 0;
			m_pActivateObject = NULL;

			// Reset the fade time...
			if( m_fEffectTime <= 0 )
				m_fEffectTime = m_fFadeTime;
			else
				m_fEffectTime = m_fFadeTime - m_fEffectTime;
		}

		return;
	}


	m_Text.SetText(CreateHelpString("Training_Action"));


	if( m_iObjRef != iObjRef )
	{
		// A new object is being targeted...
		m_iObjRef = iObjRef;
		m_fEffectTime = m_fFadeTime;

		// Cache the activate object, if it has one...
		m_pActivateObject = CActivateObjectHandler::FindActivateObject( iObjRef );
	}


	// Assign the new activate object...
	m_iObjRef = iObjRef;
	g_pLTClient->Common()->GetObjectFlags( m_iObjRef.GetData(), OFT_User, m_nUserFlags );

	m_nType = nNewType;
	
	// fetch any specific textures
	switch( nNewType )
	{
		case AOT_GENERAL:
		{
			bool bUseFallbackIcon = true;
			if( m_pActivateObject )
			{
				HRECORD hRecord;
				HATTRIBUTE hStates;
				const char* pszHUDTexture = NULL;
				uint32 dwColor = m_cIconColor;

				// Fetch the proper string from the database depending on the state...
				hRecord = DATABASE_CATEGORY( Activate ).GetRecordByIndex( m_pActivateObject->m_nId );
				if( hRecord )
				{
					hStates = DATABASE_CATEGORY( Activate ).GETRECORDSTRUCT( hRecord, States );

					if( hStates )
					{
						pszHUDTexture = DATABASE_CATEGORY( Activate ).GETSTRUCTATTRIB( States, hStates, m_pActivateObject->m_eState, HudIcon );
					}

					// If a separate disabled icon is listed use that instead of the state icon...
					if( m_pActivateObject->m_bDisabled )
					{
						const char *pszDisabledIcon = DATABASE_CATEGORY( Activate ).GETRECORDATTRIB( hRecord, DisabledIcon );
						if( !LTStrEmpty( pszDisabledIcon ))
							pszHUDTexture = pszDisabledIcon;

						// Use the disabled color of the icon...
						dwColor = DATABASE_CATEGORY( Activate ).GETRECORDATTRIB( hRecord, DisabledColor );	
					}
				}

				// Set the color, alpha will be changed by the fade effect...
				DrawPrimSetRGBA( m_ObjectRect, dwColor );

				// Set the alpha value to fade into...
				m_dwEffectAlpha = GETA(dwColor);

				if( !LTStrEmpty( pszHUDTexture ) )
				{
					bUseFallbackIcon = false;

					// Don't change to the same icon...
					if( !LTStrIEquals( m_sObjectsIcon.c_str( ), pszHUDTexture ))
					{
						// Reset the effect time and load the new icon...
						m_sObjectsIcon = pszHUDTexture;
						m_aObjects[ AOT_GENERAL ].Load( pszHUDTexture );
					}
				}

			}
			else
			{
				// Use the default icon color...
				DrawPrimSetRGBA( m_ObjectRect, m_cIconColor );
				m_dwEffectAlpha = GETA(m_cIconColor);
			}

			if( bUseFallbackIcon )
			{
				// If no specific activate type was specified, use the fallback...
				m_aObjects[AOT_GENERAL] = m_aFallbacks[AOT_GENERAL];
				m_sObjectsIcon.clear( );
			}
		}
		break;

		case AOT_PICKUP:
		{
			// generic fallback texture
			m_aObjects[AOT_PICKUP] = m_aFallbacks[AOT_PICKUP];

			// get the client side pickup item
			CPickupItemFX* pPickupItemFX = static_cast< CPickupItemFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_PICKUPITEM_ID, iObjRef ));
			if (pPickupItemFX && pPickupItemFX->GetPickupItemType( ) == kPickupItemType_Weapon )
			{
				// get pickup object record
				HRECORD hPickupType = pPickupItemFX->GetTypeRecord();
				if( hPickupType )
				{
					// get weapon data of pickup object
					HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( hPickupType, !USE_AI_DATA );
					if( hWpnData )
					{
						// get hud texture from weapon data
						const char* szTexture = g_pWeaponDB->GetString(hWpnData,WDB_WEAPON_sHUDTexture);
						if( !LTStrEmpty( szTexture ))
						{
							m_aObjects[ AOT_PICKUP ].Load( szTexture );
							m_bObjectTextureValid = false;
						}
					}
				}
			}
		}
		break;

		default:
		break;
	}
}

//******************************************************************************************

HOBJECT CHUDActivateObject::GetObject()
{
	return m_iObjRef;
}

//******************************************************************************************

int CHUDActivateObject::GetType()
{
	return m_nType;
}

//******************************************************************************************

void CHUDActivateObject::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	m_iObjRef = NULL;
	m_nUserFlags = 0;
	m_fEffectTime = m_fFadeTime;
	m_nType = AOT_INVALID;
}

// ****************************************************************************************** //

void CHUDActivateObject::ScaleChanged()
{
	CHUDItem::ScaleChanged();

	// background image size used for alignment
	LTVector2n vNewPos = AlignBasePosition( m_vIconSize );

	DrawPrimSetXYWH( m_IconPoly,   float(vNewPos.x + m_vIconOffset.x),    float(vNewPos.y + m_vIconOffset.y),    float(m_vIconSize.x),   float(m_vIconSize.y) );
	DrawPrimSetXYWH( m_ObjectRect, float(vNewPos.x + m_vObjectBasePos.x), float(vNewPos.y + m_vObjectBasePos.y), float(m_vObjectSize.x), float(m_vObjectSize.y) );
	DrawPrimSetXYWH( m_ButtonRect, float(vNewPos.x + m_vButtonBasePos.x), float(vNewPos.y + m_vButtonBasePos.y), float(m_vButtonSize.x), float(m_vButtonSize.y) );
}


// ****************************************************************************************** //

void CHUDActivateObject::UpdateLayout()
{
	if( !m_hLayout )
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDActivateObject");
	}

	CHUDItem::UpdateLayout();

	// Set the alpha value to fade into...
	m_dwEffectAlpha = GETA(m_cIconColor);

	// Initially set the button and object positions to the base pos...
	m_vObjectBasePos = m_vButtonBasePos = m_vIconOffset;
	m_vObjectSize = m_vButtonSize = m_vIconSize;

	InitAdditionalTextureData( m_hLayout, 0, m_hIconTexture, m_vIconOffset, m_vIconSize, m_IconPoly );
	InitAdditionalTextureData( m_hLayout, 1, m_aButton, m_vButtonBasePos, m_vButtonSize, m_ButtonRect );
	InitAdditionalTextureData( m_hLayout, 2, m_aFallbacks[ AOT_GENERAL ], m_vObjectBasePos, m_vObjectSize, m_ObjectRect );
	InitAdditionalTextureData( m_hLayout, 3, m_aFallbacks[ AOT_PICKUP  ], m_vObjectBasePos, m_vObjectSize, m_ObjectRect );
}
