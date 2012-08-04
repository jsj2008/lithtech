//------------------------------------------------------------------
//
//	FILE	  : PreLightMap.h
//
//	PURPOSE	  : Defines the CPreLightMap class, which represents
//              a single light map for a polygon.
//
//	CREATED	  : September 30 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __PRELIGHTMAP_H__
	#define __PRELIGHTMAP_H__


	// Includes....
	#include "bdefs.h"
	#include "preprocessorbase.h"
	

	// Defines....
	#define CPreLightMapArray		CMoArray<CPreLightMap*>

	typedef uint8					LM_DATA;



	class CPreLightMap
	{
	public:

						CPreLightMap();

		void			Term();

		// Returns TRUE if all of the lightmap's data is black.
		bool			AllBlack();

		// Initialize our data with the specified buffer.
		bool			Compress(uint32 width, uint32 height, LM_DATA *pData);
		
		// The destination buffer MUST be at least LIGHTMAP_MAX_TOTAL_PIXELS in size.
		bool			Decompress(LM_DATA *pData) const;

	public:

		// Width and height.  These should ALWAYS equal it's poly's m_LMWidth and m_LMHeight.
		uint32			m_Width, m_Height;

		// Run-length encoded data.
		CMoArray<uint8>	m_Data;
	};	



	// Used temporarily for each polygon, then put into a packed CPreLightMap.
	class CFullLightMap
	{

		public:
			
								CFullLightMap()
								{
									m_Width = m_Height = 0;
								}

			void				Term()		{ m_Map.Term(); }

			void				AllocateAt(uint32 width, uint32 height)
			{
				m_Map.SetSize(width*height);
			}			

			void				SetSize( uint32 width, uint32 height )
			{
				if((width*height) > m_Map.GetSize())
				{
					m_Map.SetSize(width*height);
				}

				m_Width		= width;
				m_Height	= height;
			}
			
			void				ColorFill( CVector color );
			CVector&			Map( uint32 x, uint32 y )		{ return m_Map[y*m_Width+x]; }

		public:

			uint32				m_Width, m_Height;

			// The size of this is m_Width*m_Height.
			CMoArray<PVector>	m_Map;

			// Helper.. boundaries of the poly.
			PVector				m_Min, m_Max;

	};



#endif  // __PRELIGHTMAP_H__
