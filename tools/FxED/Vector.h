//----------------------------------------------------------------------
//
//  MODULE   : VECTOR.H
//
//  PURPOSE  : Defines class CFXVector
//
//  CREATED  : 11/23/97 - 3:22:11 AM
//
//----------------------------------------------------------------------

#ifndef __VECTOR_H_
	#define __VECTOR_H_

	// Includes....

	#include "StandardDefs.h"
	#include "MemArray.h"
	#include "LinkList.h"

	// Forward definitions

	class CFXVector;

	// Type defines....

	typedef CMemArray<CFXVector>	CVecArray;
	typedef CLinkList<CFXVector>	CVecList;

	class CFXVector
	{
		public:

			// Constructor
								CFXVector() 
								{
									x = 0.0f;
									y = 0.0f;
									z = 0.0f;
								}

								CFXVector(float px, float py, float pz)
								{ 
									x = px;
									y = py;
									z = pz;
								}

			// Member Functions

			float				Dot(CFXVector v) { return (x * v.x) + (y * v.y) + (z * v.z); }
			float				Mag() { return (float)fabs(sqrt((x * x) + (y * y) + (z * z))); }
			float				MagSqr() { return (float)fabs((x * x) + (y * y) + (z * z)); }
			BOOL				Norm(float nVal = 1.0f);
			CFXVector				Cross(CFXVector v);
			void				Neg() { x = -x; y = -y; z = -z; }
			BOOL				IsNear(CFXVector v, float tolerance = 0.5f) { if ((fabs(x - v.x) < tolerance) && (fabs(y - v.y) < tolerance) && (fabs(z - v.z) < tolerance)) return TRUE; return FALSE; }
			
			// Operators

			CFXVector				operator + (CFXVector v) { return CFXVector(x + v.x, y + v.y, z + v.z); }
			CFXVector				operator - (CFXVector v) { return CFXVector(x - v.x, y - v.y, z - v.z); }
			CFXVector				operator * (CFXVector v) { return CFXVector(x * v.x, y * v.y, z * v.z); }
			CFXVector				operator / (CFXVector v) { return CFXVector(x / v.x, y / v.y, z / v.z); }
			void				operator += (CFXVector v) { x += v.x; y += v.y; z += v.z; }
			void				operator -= (CFXVector v) { x -= v.x; y -= v.y; z -= v.z; }
			void				operator *= (CFXVector v) { x *= v.x; y *= v.y; z *= v.z; }
			void				operator /= (CFXVector v) { x /= v.x; y /= v.y; z /= v.z; }

			CFXVector				operator + (float v) { return CFXVector(x + v, y + v, z + v); }
			CFXVector				operator - (float v) { return CFXVector(x - v, y - v, z - v); }
			CFXVector				operator * (float v) { return CFXVector(x * v, y * v, z * v); }
			CFXVector				operator / (float v) { return CFXVector(x / v, y / v, z / v); }
			void				operator += (float v) { x += v; y += v; z += v; }
			void				operator -= (float v) { x -= v; y -= v; z -= v; }
			void				operator *= (float v) { x *= v; y *= v; z *= v; }
			void				operator /= (float v) { x /= v; y /= v; z /= v; }	
			
			BOOL				operator == (CFXVector v) { return ((v.x == x) && (v.y == y) && (v.z == z)); }

		public:

			// Member Variables

			float				x;
			float				y;
			float				z;
		
	};

	//----------------------------------------------------------
	//
	// INLINE FUNCTIONS
	//
	//----------------------------------------------------------

	//----------------------------------------------------------
	//
	// FUNCTION : CFXVector::Norm()
	//
	// PURPOSE	: Normalises a vector
	//
	//----------------------------------------------------------

	inline BOOL CFXVector::Norm(float nVal)
	{
		float magnitude = Mag();

		if (magnitude > EPSILON)
		{
			nVal = nVal / magnitude;

			x *= nVal;
			y *= nVal;
			z *= nVal;

			// Success !!

			return TRUE;
		}

		// Failure !!
		
		return FALSE;
	}
	
	//----------------------------------------------------------
	//
	// FUNCTION : CFXVector::Cross()
	//
	// PURPOSE	: Calculates cross product
	//
	//----------------------------------------------------------

	inline CFXVector CFXVector::Cross(CFXVector v)
	{
		return CFXVector((y * v.z) - (z * v.y),
					   (z * v.x) - (x * v.z),
					   (x * v.y) - (y * v.x));
	}

#endif