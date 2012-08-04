//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : CCS.h
//
//	PURPOSE	  : Defines the CCS (Coordinate System) class
//
//	CREATED	  : 1st May 1996
//
//
//------------------------------------------------------------------

#ifndef __CCS_H__
	#define __CCS_H__
	

	// Includes....
	#include "orientation.h"



	// Defines....
	#define CCS		_CCS<CReal>
	#define CCSD	_CCS<CDReal>



	template<class T>
	class _CCS : public _COrientation<T>
	{
		public:

			// Constructor
						_CCS()	{}

			_CCS<T>&	operator=( _CCS<float> &ccs )
			{
				m_Position = ccs.m_Position;
				m_Up = ccs.m_Up;
				m_Forward = ccs.m_Forward;
				m_Right = ccs.m_Right;
				return *this;
			}
/*
			_CCS<T>&	operator=( _CCS<double> &ccs )
			{
				m_Position = ccs.m_Position;
				m_Up = ccs.m_Up;
				m_Forward = ccs.m_Forward;
				m_Right = ccs.m_Right;
				return *this;
			}
*/
			// Member functions
			BOOL		Init()	
			{ 
				m_Position.x = m_Position.y = m_Position.z = 0;
				m_Up.Init( 0, 1, 0 );
				m_Forward.Init( 0, 0, 1 );
				m_Right.Init( 1, 0, 0 );
				return TRUE;
			}
			
			// These move it along its axes.
			void		MoveX( T amount )	{ m_Position += Right() * amount; }
			void		MoveY( T amount )	{ m_Position += m_Up * amount; }
			void		MoveZ( T amount )	{ m_Position += m_Forward * amount; }

			// These rotate it around its axes.
			void		RotX( T amount )
			{
				_CMatrix<T>	mat;

				mat.SetupRot( Right(), amount );
				mat.Apply( m_Position );
			}

			void		RotY( T amount )
			{
				_CMatrix<T>	mat;

				mat.SetupRot( m_Up, amount );
				mat.Apply( m_Position );
			}

			void		RotZ( T amount )
			{
				_CMatrix<T>	mat;

				mat.SetupRot( m_Forward, amount );
				mat.Apply( m_Position );
			}

			
			// Convenience function .. just sets up the matrix to
			// transform into the CCS's coordinate system.
			
			// If you want it to translate, then rotate, set bRotateFirst to FALSE.			
			void		PutInMatrix( DMatrix &mat, BOOL bRotateFirst )
			{
				Normalize();

				if( bRotateFirst )
				{
					mat.m[0][0] = m_Right.x;	mat.m[0][1] = m_Right.y;	mat.m[0][2] = m_Right.z;	mat.m[0][3] = m_Position.x;
					mat.m[1][0] = m_Up.x;		mat.m[1][1] = m_Up.y;		mat.m[1][2] = m_Up.z;		mat.m[1][3] = m_Position.y;
					mat.m[2][0] = m_Forward.x;	mat.m[2][1] = m_Forward.y;	mat.m[2][2] = m_Forward.z;	mat.m[2][3] = m_Position.z;
					mat.m[3][0] = 0;			mat.m[3][1] = 0;			mat.m[3][2] = 0;			mat.m[3][3] = 1;
				}
				else
				{
					LTMatrix rot;

					mat.Identity();
					mat.m[0][3] = m_Position.x;  mat.m[1][3] = m_Position.y;  mat.m[2][3] = m_Position.z;
					
					rot.Identity();
					rot.m[0][0] = m_Right.x;	rot.m[0][1] = m_Right.y;	rot.m[0][2] = m_Right.z;
					rot.m[1][0] = m_Up.x;		rot.m[1][1] = m_Up.y;		rot.m[1][2] = m_Up.z;
					rot.m[2][0] = m_Forward.x;  rot.m[2][1] = m_Forward.y;  rot.m[2][2] = m_Forward.z;

					rot.Apply( mat );
				}
			}


			// This actually transforms the vector like a matrix would.
			void		Apply( TVector3<T> &s, TVector3<T> &d )
			{
				TVector3<T>		temp;


				d = s + m_Position;
				
				temp.x = d.Dot(m_Right);
				temp.y = d.Dot(m_Up);
				temp.z = d.Dot(m_Forward);

				d = temp;
			}
			


			// Accessors
			TVector3<T>	&Pos()	{ return m_Position; }


		public:

			// Member variables

			TVector3<T>	m_Position;

	};


#endif

