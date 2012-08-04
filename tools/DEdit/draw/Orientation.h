//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : Orientation.h
//
//	PURPOSE	  : Defines the COrientation.h
//
//	CREATED	  : 1st May 1996
//
//
//------------------------------------------------------------------

#ifndef __ORIENTATION_H__
	#define __ORIENTATION_H__
	


	// Defines....
	#define COrientation	_COrientation<CReal>
	#define CDOrientation	_COrientation<CDReal>



	template<class T>
	class _COrientation
	{
		public:

			// Constructor		
								_COrientation() {}


			// Member functions
			
			BOOL				Init()
			{
				m_Up.Init( 0.0f, 1.0f, 0.0f );
				m_Forward.Init( 0.0f, 0.0f, 1.0f );
				m_Right.Init( 1.0f, 0.0f, 0.0f );
				
				return TRUE;
			}
			
			
			// These rotate its orientation about its own axes.
			void				ORotX( T angle )
			{
				LTMatrix mat;

				mat.SetupRot( Right(), angle );
				
				mat.Apply( m_Up );
				mat.Apply( m_Forward );

				m_Up.Norm();
				m_Forward.Norm();
			}

			void				ORotY( T angle )
			{
				LTMatrix mat;

				mat.SetupRot( Up(), angle );
				mat.Apply( m_Forward );
				mat.Apply( m_Right );
			}

			void				ORotZ( T angle )
			{
				LTMatrix	mat;

				mat.SetupRot( Forward(), angle );
				mat.Apply( m_Up );
				mat.Apply( m_Right );
			}


			// Normalize it .. should do this every once in a while to keep things accurate.
			void				Normalize()	{ m_Up.Norm(); m_Forward.Norm(); m_Right.Norm(); }


			// Stick it into a matrix for transformation.
			void				PutInMatrix( DMatrix &mat )
			{
				mat[0] = m_Right.x;		mat[1] = m_Right.y;		mat[2] = m_Right.z;		mat[3] = 0.0f;
				mat[4] = m_Up.x;		mat[5] = m_Up.y;		mat[6] = m_Up.z;		mat[7] = 0.0f;
				mat[8] = m_Forward.x;   mat[9] = m_Forward.y;   mat[10] = m_Forward.z;	mat[11] = 0.0f;
				mat[12] = 0.0f;			mat[13] = 0.0f;			mat[14] = 0.0f;			mat[15] = 1.0f;
			}

			
			// If you've messed with the vectors at all, you can have it generate the 
			// specified one automatically from the other 2 (cross-product...)
			void				MakeUp()		{ m_Up = m_Right.Cross(m_Forward); }
			void				MakeForward()	{ m_Forward = m_Up.Cross(m_Right); }
			void				MakeRight()		{ m_Right = m_Forward.Cross(m_Up); }
			
			// Accessors

			TVector3<T>			&Up()		{ return m_Up; }
			TVector3<T>			&Forward()	{ return m_Forward; }
			TVector3<T>			&Right()	{ return m_Right; }

		public:

			// Member variables
			TVector3<T>			m_Up, m_Forward, m_Right;

	};


#endif


																		   