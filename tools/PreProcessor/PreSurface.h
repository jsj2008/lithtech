//------------------------------------------------------------------
//
//	FILE	  : PreSurface.h
//
//	PURPOSE	  : Defines the CPreSurface class, which is a surface description
//              (basically a polygon without the vertices) for world polies.
//
//	CREATED	  : February 12 1997
//
//	COPYRIGHT : Microsoft 1997 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __PRESURFACE_H__
	#define __PRESURFACE_H__

	#include "PreTexture.h"

	class CPrePlane;
	class CPrePoly;
	class CEditPoly;

	class CPreSurface : public CGLLNode
	{
		public:

			enum	{ NUM_TEXTURES = 2 };

							CPreSurface();

			bool			ShouldLightmap() const;
			void			SetupLMVectors();

			inline uint32	GetLMPlaneIndex() const		{return m_Flags >> 29;}

			//gets the LM grid size from this surface
			//if it is near 0, the default grid size is returned if a world
			//is specified
			inline uint8	GetLMGridSize() const		{ return m_LMGridSize; }

		public:

			//the textures for this surface
			CPreTexture		m_Texture[NUM_TEXTURES];

			// The poly this surface came from.. only valid while the CEditRegion is around.
			CEditPoly		*m_pOriginalPoly;

			// List of polies on this surface.
			CMultiLinkList<CPrePoly*> m_PolyList;

			// Vectors defining LIGHTMAP texture space on this surface.
			// These are the INVERSE texture vectors (just like in the de_world structures).
			// ie: these are what are used to calculate texture coordinates.
			PVector			P, Q;

			// These are the texture vectors with inverse magnitude.
			// These are used when finding out how many pixels span a polygon.
			// They are initialized in CPreWorld::SetupSurfaceTextureVectors().
            // These vectors have inverse length of P and Q.  When a lightmap is 
            // "stretched" larger, these Inverse variables will get larger, and
            // the P and Q vars will get smaller.
			PVector			InverseP, InverseQ;

			// Plane for this surface.
			CPrePlane		*m_pPlane;

			// Surface flags (flags defined in surface.h).
			uint32			m_Flags;

			// Index of the surface into the used surface list.. only used during saving.
			uint32			m_UseIndex;

			//the lightmap grid size, which is the ratio of texture pixels to 
			//lightmap pixels for this surface
			uint8			m_LMGridSize;

			//the ambient light of the surface (RGB)
			uint8			m_Ambient[3];

			// Used when saving to get rid of unused surfaces.
			uint8			m_bUsed;

			//this value scales the amount that light is able to pass through another
			//surface to hit a lighting sample. This is used for controlling light
			//leaking vs. shadow leaking
			PReal			m_fLightPenScale;

			//the name of the texture effect that is being applied to this surface
			char			*m_pTextureEffect;

	};


#endif  // __PRESURFACE_H__

