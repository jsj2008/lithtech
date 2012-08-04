//----------------------------------------------------------------------
//
//  MODULE   : MATRIX.H
//
//  PURPOSE  : Defines class CFXMatrix
//
//  CREATED  : 11/23/97 - 3:23:46 AM
//
//----------------------------------------------------------------------

#ifndef __MATRIX_H_
	#define __MATRIX_H_

	// Includes....

	#include "StandardDefs.h"
	#include "Vector.h"	

	class CFXMatrix
	{
		public:

			// Constructor

								CFXMatrix() {}

			// Member Functions

			void				Identity();
			void				Transpose3x3();
			void				Inverse();
			void				MakeRotation(CFXVector v, float theta);
			void				MakeRotation(float xRot, float yRot, float zRot);
			void				SetVectors(CFXVector r, CFXVector u, CFXVector f);
			void				GetVectors(CFXVector *pR, CFXVector *pU, CFXVector *pF);
			void				Row(int r, float v1, float v2, float v3, float v4) 
								{ 
									float *pElem = m_Elem + (r << 2); 
									*pElem ++ = v1;
									*pElem ++ = v2;
									*pElem ++ = v3;
									*pElem ++ = v4;
								}
						
			void				Apply(CFXVector *pIn, CFXVector *pOut = NULL, DWORD nVerts = 1);
			void				Apply3x3(CFXVector *pIn, CFXVector *pOut = NULL, DWORD nVerts = 1);


			// Operators

			CFXMatrix				operator * (CFXMatrix &s);
			float&				operator [] (DWORD i) { return m_Elem[i]; }

			// Accessors

			CFXMatrix				GetRotation();
			CFXMatrix				GetTranslation();

		public:

			// Member Variables

			float				m_Elem[16];
	};

	//----------------------------------------------------------------------
	//
	// INLINE FUNCTIONS
	//
	//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	//
	// FUNCTION : CFXMatrix::Transpose3x3()
	//
	// PURPOSE	: Transposes the 3 x 3 rotation values
	//
	//----------------------------------------------------------------------

	inline void CFXMatrix::Transpose3x3()
	{
		float tmp;

		tmp		  = m_Elem[1];
		m_Elem[1] = m_Elem[4];
		m_Elem[4] = tmp;

		tmp		  = m_Elem[2];
		m_Elem[2] = m_Elem[8];
		m_Elem[8] = tmp;

		tmp		  = m_Elem[6];
		m_Elem[6] = m_Elem[9];
		m_Elem[9] = tmp;
	}

	//----------------------------------------------------------------------
	//
	// FUNCTION : CFXMatrix::Inverse()
	//
	// PURPOSE	: Creates an inverse transform matrix
	//
	//----------------------------------------------------------------------

	inline void CFXMatrix::Inverse()
	{
		CFXMatrix t1, t2, t3;
		
		memcpy(t1.m_Elem, m_Elem, sizeof(float) * 16);
		t1[3]  = 0;
		t1[7]  = 0;
		t1[11] = 0;
		t1.Transpose3x3();

		t2.Identity();
		t2[3]  = -m_Elem[3];
		t2[7]  = -m_Elem[7];
		t2[11] = -m_Elem[11];

		t3 = t1 * t2;

		memcpy(m_Elem, t3.m_Elem, sizeof(float) * 16);
	}

	//----------------------------------------------------------------------
	//
	// FUNCTION : CFXMatrix::operator *
	//
	// PURPOSE	: Multiplies two matrices together
	//
	//----------------------------------------------------------------------

	inline CFXMatrix CFXMatrix::operator * (CFXMatrix &s)
	{
		CFXMatrix	d;
		
		d.m_Elem[0]  = (m_Elem[0] * s.m_Elem[0]) + (m_Elem[1] * s.m_Elem[4]) + (m_Elem[2] * s.m_Elem[8]) + (m_Elem[3] * s.m_Elem[12]);
		d.m_Elem[1]  = (m_Elem[0] * s.m_Elem[1]) + (m_Elem[1] * s.m_Elem[5]) + (m_Elem[2] * s.m_Elem[9]) + (m_Elem[3] * s.m_Elem[13]);
		d.m_Elem[2]  = (m_Elem[0] * s.m_Elem[2]) + (m_Elem[1] * s.m_Elem[6]) + (m_Elem[2] * s.m_Elem[10]) + (m_Elem[3] * s.m_Elem[14]);
		d.m_Elem[3]  = (m_Elem[0] * s.m_Elem[3]) + (m_Elem[1] * s.m_Elem[7]) + (m_Elem[2] * s.m_Elem[11]) + (m_Elem[3] * s.m_Elem[15]);

		d.m_Elem[4]  = (m_Elem[4] * s.m_Elem[0]) + (m_Elem[5] * s.m_Elem[4]) + (m_Elem[6] * s.m_Elem[8]) + (m_Elem[7] * s.m_Elem[12]);
		d.m_Elem[5]  = (m_Elem[4] * s.m_Elem[1]) + (m_Elem[5] * s.m_Elem[5]) + (m_Elem[6] * s.m_Elem[9]) + (m_Elem[7] * s.m_Elem[13]);
		d.m_Elem[6]  = (m_Elem[4] * s.m_Elem[2]) + (m_Elem[5] * s.m_Elem[6]) + (m_Elem[6] * s.m_Elem[10]) + (m_Elem[7] * s.m_Elem[14]);
		d.m_Elem[7]  = (m_Elem[4] * s.m_Elem[3]) + (m_Elem[5] * s.m_Elem[7]) + (m_Elem[6] * s.m_Elem[11]) + (m_Elem[7] * s.m_Elem[15]);

		d.m_Elem[8]  = (m_Elem[8] * s.m_Elem[0]) + (m_Elem[9] * s.m_Elem[4]) + (m_Elem[10] * s.m_Elem[8]) + (m_Elem[11] * s.m_Elem[12]);
		d.m_Elem[9]  = (m_Elem[8] * s.m_Elem[1]) + (m_Elem[9] * s.m_Elem[5]) + (m_Elem[10] * s.m_Elem[9]) + (m_Elem[11] * s.m_Elem[13]);
		d.m_Elem[10] = (m_Elem[8] * s.m_Elem[2]) + (m_Elem[9] * s.m_Elem[6]) + (m_Elem[10] * s.m_Elem[10]) + (m_Elem[11] * s.m_Elem[14]);
		d.m_Elem[11] = (m_Elem[8] * s.m_Elem[3]) + (m_Elem[9] * s.m_Elem[7]) + (m_Elem[10] * s.m_Elem[11]) + (m_Elem[11] * s.m_Elem[15]);

		d.m_Elem[12] = (m_Elem[12] * s.m_Elem[0]) + (m_Elem[13] * s.m_Elem[4]) + (m_Elem[14] * s.m_Elem[8]) + (m_Elem[15] * s.m_Elem[12]);
		d.m_Elem[13] = (m_Elem[12] * s.m_Elem[1]) + (m_Elem[13] * s.m_Elem[5]) + (m_Elem[14] * s.m_Elem[9]) + (m_Elem[15] * s.m_Elem[13]);
		d.m_Elem[14] = (m_Elem[12] * s.m_Elem[2]) + (m_Elem[13] * s.m_Elem[6]) + (m_Elem[14] * s.m_Elem[10]) + (m_Elem[15] * s.m_Elem[14]);
		d.m_Elem[15] = (m_Elem[12] * s.m_Elem[3]) + (m_Elem[13] * s.m_Elem[7]) + (m_Elem[14] * s.m_Elem[11]) + (m_Elem[15] * s.m_Elem[15]);

		return d;
	}

	//----------------------------------------------------------------------
	//
	// FUNCTION : CFXMatrix::Apply
	//
	// PURPOSE	: Applies matrix to an array of vertices
	//
	//----------------------------------------------------------------------

	inline void CFXMatrix::Apply(CFXVector *pIn, CFXVector *pOut, DWORD nVerts)
	{
		float w;

		if ((pIn == pOut) || (!pOut))
		{
			CFXVector tmp = *pIn;
			pOut = pIn;

			w = (tmp.x * m_Elem[12]) + (tmp.y * m_Elem[13]) + (tmp.z * m_Elem[14]) + m_Elem[15];
			pOut->x = (tmp.x * m_Elem[0] + tmp.y * m_Elem[1] + tmp.z * m_Elem[2] + m_Elem[3]) / w;
			pOut->y = (tmp.x * m_Elem[4] + tmp.y * m_Elem[5] + tmp.z * m_Elem[6] + m_Elem[7]) / w;
			pOut->z = (tmp.x * m_Elem[8] + tmp.y * m_Elem[9] + tmp.z * m_Elem[10] + m_Elem[11]) / w;
		}
		else
		{
			while (nVerts --)
			{			
				w = (pIn->x * m_Elem[12]) + (pIn->y * m_Elem[13]) + (pIn->z * m_Elem[14]) + m_Elem[15];
				pOut->x = (pIn->x * m_Elem[0] + pIn->y * m_Elem[1] + pIn->z * m_Elem[2] + m_Elem[3]) / w;
				pOut->y = (pIn->x * m_Elem[4] + pIn->y * m_Elem[5] + pIn->z * m_Elem[6] + m_Elem[7]) / w;
				pOut->z = (pIn->x * m_Elem[8] + pIn->y * m_Elem[9] + pIn->z * m_Elem[10] + m_Elem[11]) / w;

				pIn ++;
				pOut ++;
			}
		}
	}

	//----------------------------------------------------------------------
	//
	// FUNCTION : CFXMatrix::Apply3x3
	//
	// PURPOSE	: Applies matrix to an array of vertices
	//
	//----------------------------------------------------------------------

	inline void CFXMatrix::Apply3x3(CFXVector *pIn, CFXVector *pOut, DWORD nVerts)
	{
		if ((pIn == pOut) || (!pOut))
		{
			CFXVector tmp = *pIn;
			pOut = pIn;
			
			pOut->x = tmp.x * m_Elem[0] + tmp.y * m_Elem[1] + tmp.z * m_Elem[2] + m_Elem[3];
			pOut->y = tmp.x * m_Elem[4] + tmp.y * m_Elem[5] + tmp.z * m_Elem[6] + m_Elem[7];
			pOut->z = tmp.x * m_Elem[8] + tmp.y * m_Elem[9] + tmp.z * m_Elem[10] + m_Elem[11];
		}
		else
		{
			while (nVerts --)
			{							
				pOut->x = pIn->x * m_Elem[0] + pIn->y * m_Elem[1] + pIn->z * m_Elem[2] + m_Elem[3];
				pOut->y = pIn->x * m_Elem[4] + pIn->y * m_Elem[5] + pIn->z * m_Elem[6] + m_Elem[7];
				pOut->z = pIn->x * m_Elem[8] + pIn->y * m_Elem[9] + pIn->z * m_Elem[10] + m_Elem[11];

				pIn ++;
				pOut ++;
			}
		}
	}

	//----------------------------------------------------------------------
	//
	// FUNCTION : CFXMatrix::MakeRotation()
	//
	// PURPOSE	: Creates a matrix to rotate around an axis
	//
	//----------------------------------------------------------------------

	inline void CFXMatrix::MakeRotation(CFXVector v, float angle)
	{
		float s, c, t;
		float tx, ty, tz;
		float sx, sy, sz;
		
		s = (float)Sine(angle);
		c = (float)Cosine(angle);
		t = 1 - c;

		tx = t * v.x;	ty = t * v.y;	tz = t * v.z;
		sx = s * v.x;	sy = s * v.y;	sz = s * v.z;

		Identity();

		m_Elem[0]  = tx * v.x + c;
		m_Elem[4]  = tx * v.y + sz;
		m_Elem[8]  = tx * v.z - sy;

		m_Elem[1]  = tx * v.y - sz;
		m_Elem[5]  = ty * v.y + c;
		m_Elem[9]  = ty * v.z + sx;

		m_Elem[2]  = tx * v.z + sy;
		m_Elem[6]  = ty * v.z - sx;
		m_Elem[10] = tz * v.z + c;
	}

	//----------------------------------------------------------------------
	//
	// FUNCTION : CFXMatrix::MakeRotation()
	//
	// PURPOSE	: Creates a matrix to rotate around an axis
	//
	//----------------------------------------------------------------------

	inline void CFXMatrix::MakeRotation(float xRot, float yRot, float zRot)
	{
		float cp = Cosine(xRot);
		float sp = Sine(xRot);
		float cy = Cosine(yRot);
		float sy = Sine(yRot);
		float cr = Cosine(zRot);
		float sr = Sine(zRot);

		Identity();

		m_Elem[0] = cy * cr;
		m_Elem[1] = -cy * sr;
		m_Elem[2] = sy;
		
		m_Elem[4] = (sp * sy * cr) + (cp * sr); 
		m_Elem[5] = (-sp * sy * sr) + (cp * cr);
		m_Elem[6] = -sp * cy;
		
		m_Elem[8] = (-cp * sy * cr) + (sp * sr);
		m_Elem[9] = (cp * sy * sr) + (sp * cr);
		m_Elem[10] = cp * cy;
	}


	//----------------------------------------------------------------------
	//
	// FUNCTION : CFXMatrix::MakeRotation()
	//
	// PURPOSE	: Creates a matrix to rotate around an axis
	//
	//----------------------------------------------------------------------

	inline void CFXMatrix::SetVectors(CFXVector r, CFXVector u, CFXVector f)
	{
		m_Elem[0]  = r.x;
		m_Elem[1]  = r.y;
		m_Elem[2]  = r.z;

		m_Elem[4]  = u.x;
		m_Elem[5]  = u.y;
		m_Elem[6]  = u.z;

		m_Elem[8]  = f.x;
		m_Elem[9]  = f.y;
		m_Elem[10] = f.z;
	}

	//----------------------------------------------------------------------
	//
	//  FUNCTION : GetVectors()
	//
	//  PURPOSE  : Retrieves right, up and forward vectors
	//
	//----------------------------------------------------------------------

	inline void CFXMatrix::GetVectors(CFXVector *pR, CFXVector *pU, CFXVector *pF)
	{
		pR->x = m_Elem[0];
		pR->y = m_Elem[1];
		pR->z = m_Elem[2];

		pU->x = m_Elem[4];
		pU->y = m_Elem[5];
		pU->z = m_Elem[6];

		pF->x = m_Elem[8];
		pF->y = m_Elem[9];
		pF->z = m_Elem[10];
	}

#endif
