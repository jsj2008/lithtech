// ----------------------------------------------------------------------- //
//
// MODULE  : HitBox.h
//
// PURPOSE : Client side reresentation of the CCharacterHitBox object
//
// CREATED : 8/26/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HITBOX_H__
#define __HITBOX_H__

//
// Includes...
//


class CHitBox
{
	public: // Methods...

		CHitBox();
		~CHitBox();

		virtual	bool Init( HOBJECT hModel, const LTVector &vDims, const LTVector &vOffset );
		virtual bool Update();

		virtual void SetDims( const LTVector &vDims );
		virtual void SetOffset( const LTVector &vOffset );
				
		const LTVector& GetDims() const { return m_vDims; }
		const LTVector& GetOffset() const { return m_vOffset; }
		
		HOBJECT	GetObject() const { return m_hObject; }
		HOBJECT GetModel( ) const { return m_hModel; }

	protected:	// Members...

		LTObjRef		m_hObject;		

		LTObjRef		m_hModel;		// Object we are associated with

		LTVector	m_vDims;		// HitBox dimensions
		LTVector	m_vOffset;		// HitBox offset relative to the position and rotation of our model object
};

#endif // __HITBOX_H__
