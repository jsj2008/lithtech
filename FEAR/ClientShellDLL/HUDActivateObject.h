// ----------------------------------------------------------------------- //
//
// MODULE  : HUDActivateObject.h
//
// PURPOSE : HUDItem to display the current object that can be activated.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDACTIVATEOBJECT_H__
#define __HUDACTIVATEOBJECT_H__

#include "HUDItem.h"
#include "ActivateDB.h"

// Forwards...
class CActivateObjectHandler;

//******************************************************************************************

class CHUDActivateObject : public CHUDItem, public ILTObjRefReceiver
{
	public:

		enum Type
		{
			AOT_INVALID			= -1,
			AOT_GENERAL			= 0,
			AOT_PICKUP,

			AOT_COUNT,
			AOT_FORCE32BIT		= 0x7FFFFFFF,
		};


	public:

		CHUDActivateObject();

		virtual bool	Init();
		virtual void	Term();

		virtual void	Render();
		virtual void	Update();
		virtual void	UpdateLayout();
		virtual void	ScaleChanged();

		void			SetObject( LTObjRef iObjRef, uint32 nNewType );
		HOBJECT			GetObject();
		int				GetType();

		const CActivateObjectHandler* GetActivateObjectHandler( ) const { return m_pActivateObject;	}
		
	private:

		uint32			GetObjectType( HOBJECT hObject );
		virtual void	OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

	private:

		// Allocated texture handles
		TextureReference			m_aButton;
		TextureReference			m_aObjects[ AOT_COUNT ];
		TextureReference			m_aFallbacks[ AOT_COUNT ];

		// Relative texture positions
		LTVector2n		m_vBGBasePos;
		LTVector2n		m_vObjectBasePos;
		LTVector2n		m_vButtonBasePos;

		// Texture sizes
		LTVector2n		m_vBGSize;
		LTVector2n		m_vObjectSize;
		LTVector2n		m_vButtonSize;

		// Locations to render
		LTPoly_GT4			m_ObjectRect;
		LTPoly_GT4			m_ButtonRect;

		// Effect trackers
		LTObjRefNotifier	m_iObjRef;
		uint32				m_nUserFlags;
		int					m_nType;
		float				m_fEffectTime;

		// Filename of the texture to display, this is used to determine if the texture
		// reference needs to be recreated.
		std::string			m_sObjectsIcon;

		// Final alpha value when fading...
		uint32				m_dwEffectAlpha;

		// Cache the activate object handler of the target object if it has one...
		const CActivateObjectHandler	*m_pActivateObject;

		// Texture load latches
		bool			m_bObjectTextureValid;
};

#endif//__HUDACTIVATEOBJECT_H__

