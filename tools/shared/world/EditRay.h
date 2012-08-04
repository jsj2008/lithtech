//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditRay.h
//
//	PURPOSE	  : Defines the CEditRay class.
//
//	CREATED	  : October 5 1996
//
//
//------------------------------------------------------------------

#ifndef __EDITRAY_H__
	#define __EDITRAY_H__


	// Includes....


	class CEditRay
	{
		public:
			
						CEditRay()								{}
						CEditRay( CVector &pos, CVector &dir )	{ m_Pos=pos; m_Dir=dir; }
			
			// Get the distance from the ray to a point
			float		DistTo(const CVector& vPoint) 
			{
				//find the vector to it
				CVector vDiff = vPoint - m_Pos;

				//find the length along the ray that the point lies
				float fT = vDiff.Dot(m_Dir);

				if (fT < 0.0f)
				{
					//it lies behind, so just use the absolute distance
					return vDiff.Mag();
				}

				//it lies in front, so find out the closest distance to the ray
				CVector vOnRay = m_Pos + m_Dir * fT;
				return (vPoint - vOnRay).Mag();
			}



			CVector		m_Pos, m_Dir;

	};


#endif  // __EDITRAY_H__


