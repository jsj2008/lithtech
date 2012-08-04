//////////////////////////////////////////////////////////////////////////////
// PC-Specific rendering triangle implementation

#include "bdefs.h"

#include "pcrendertri.h"
#include "pcfileio.h"

/********************************************************/
/* AABB-triangle overlap test code                      */
/* by Tomas Möller                                      */
/* History:                                             */
/*   2001-03-05: released the code in its first version */
/*   2001-04-26: Cleaned up for LT use by Kevin Francis */
/*                                                      */
/* Acknowledgement: Many thanks to Pierre Terdiman for  */
/* suggestions and discussions on how to optimize code. */
/********************************************************/

static inline void FindMinMax(float x0, float x1, float x2, float &min, float &max)
{
	min = max = x0;   
	if (x1 < min)
		min = x1;
	if (x1 > max)
		max = x1;
	if (x2 < min)
		min = x2;
	if (x2 > max)
		max = x2;
}

static bool planeBoxOverlap(const LTVector &vNormal, float d, const LTVector &vHalfDims)
{
	LTVector vMin, vMax;
	for (int q = 0; q <= 2; ++q)
	{
		if (vNormal[q] > 0.0f)
		{
			vMin[q] = -vHalfDims[q];
			vMax[q] = vHalfDims[q];
		}
		else
		{
			vMin[q] = vHalfDims[q];
			vMax[q] = -vHalfDims[q];
		}
	}
	if (vNormal.Dot(vMin) + d > 0.0f)
		return false;
	if (vNormal.Dot(vMax) + d > 0.0f)
		return true;
	
	return false;
}

/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)			   \
	p0 = a*v0.y - b*v0.z;			       	   \
	p2 = a*v2.y - b*v2.z;			       	   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * vHalfDims.y + fb * vHalfDims.z;   \
	if(min>rad || max<-rad) return false;

#define AXISTEST_X2(a, b, fa, fb)			   \
	p0 = a*v0.y - b*v0.z;			           \
	p1 = a*v1.y - b*v1.z;			       	   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * vHalfDims.y + fb * vHalfDims.z;   \
	if(min>rad || max<-rad) return false;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)			   \
	p0 = -a*v0.x + b*v0.z;		      	   \
	p2 = -a*v2.x + b*v2.z;	       	       	   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * vHalfDims.x + fb * vHalfDims.z;   \
	if(min>rad || max<-rad) return false;

#define AXISTEST_Y1(a, b, fa, fb)			   \
	p0 = -a*v0.x + b*v0.z;		      	   \
	p1 = -a*v1.x + b*v1.z;	     	       	   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * vHalfDims.x + fb * vHalfDims.z;   \
	if(min>rad || max<-rad) return false;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)			   \
	p1 = a*v1.x - b*v1.y;			           \
	p2 = a*v2.x - b*v2.y;			       	   \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
	rad = fa * vHalfDims.x + fb * vHalfDims.y;   \
	if(min>rad || max<-rad) return false;

#define AXISTEST_Z0(a, b, fa, fb)			   \
	p0 = a*v0.x - b*v0.y;				   \
	p1 = a*v1.x - b*v1.y;			           \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * vHalfDims.x + fb * vHalfDims.y;   \
	if(min>rad || max<-rad) return false;

bool CPCRenderTri::IntersectAABB(const LTVector &vCenter, const LTVector &vHalfDims)
{
	
	/*    use separating axis theorem to test overlap between triangle and box */
	/*    need to test for overlap in these directions: */
	/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
	/*       we do not even need to test these) */
	/*    2) normal of the triangle */
	/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
	/*       this gives 3x3=9 more tests */
	LTVector v0,v1,v2;
	LTVector axis;
	float min,max,d,p0,p1,p2,rad;
	
	/* 1) first test overlap in the {x,y,z}-directions */
	/*    find min, max of the triangle each direction, and test for overlap in */
	/*    that direction -- this is equivalent to testing a minimal AABB around */
	/*    the triangle against the AABB */
	
	/*    test in X */
	v0.x = m_Vert0.m_vPos.x - vCenter.x;
	v1.x = m_Vert1.m_vPos.x - vCenter.x;
	v2.x = m_Vert2.m_vPos.x - vCenter.x;
	FindMinMax(v0.x,v1.x,v2.x,min,max);
	if ((min > vHalfDims.x) || (max < -vHalfDims.x))
		return false;
	
	/*    test in Y */
	v0.y = m_Vert0.m_vPos.y - vCenter.y;
	v1.y = m_Vert1.m_vPos.y - vCenter.y;
	v2.y = m_Vert2.m_vPos.y - vCenter.y;
	FindMinMax(v0.y,v1.y,v2.y,min,max);
	if ((min > vHalfDims.y) || (max < -vHalfDims.y))
		return false;
	
	/*    test in Z */
	v0.z = m_Vert0.m_vPos.z - vCenter.z;
	v1.z = m_Vert1.m_vPos.z - vCenter.z;
	v2.z = m_Vert2.m_vPos.z - vCenter.z;
	FindMinMax(v0.z,v1.z,v2.z,min,max);
	if ((min > vHalfDims.z) || (max < -vHalfDims.z))
		return false;
	
	
	/*    2) */
	/*    test if the box intersects the plane of the triangle */
	/*    compute plane equation of triangle: normal*x+d=0 */
	LTVector e0,e1,e2;
	e0 = v1 - v0;	/* tri edge 0 */
	e1 = v2 - v1;	/* tri edge 1 */
	d = -m_vNormal.Dot(v0);  /* plane eq: normal.x+d=0 */
	
	if (!planeBoxOverlap(m_vNormal, d, vHalfDims)) 
		return 0;
	
	/*    compute the last triangle edge */
	e2 = v0 - v2;
	
	/*    3) */
	LTVector vAbsEdge;
	vAbsEdge.Init((float)fabs(e0.x), (float)fabs(e0.y), (float)fabs(e0.z));
	AXISTEST_X01(e0.z, e0.y, vAbsEdge.z, vAbsEdge.y);
	AXISTEST_Y02(e0.z, e0.x, vAbsEdge.z, vAbsEdge.x);
	AXISTEST_Z12(e0.y, e0.x, vAbsEdge.y, vAbsEdge.x);
	
	vAbsEdge.Init((float)fabs(e1.x), (float)fabs(e1.y), (float)fabs(e1.z));
	AXISTEST_X01(e1.z, e1.y, vAbsEdge.z, vAbsEdge.y);
	AXISTEST_Y02(e1.z, e1.x, vAbsEdge.z, vAbsEdge.x);
	AXISTEST_Z0(e1.y, e1.x, vAbsEdge.y, vAbsEdge.x);
	
	
	vAbsEdge.Init((float)fabs(e2.x), (float)fabs(e2.y), (float)fabs(e2.z));
	AXISTEST_X2(e2.z, e2.y, vAbsEdge.z, vAbsEdge.y);
	AXISTEST_Y1(e2.z, e2.x, vAbsEdge.z, vAbsEdge.x);
	AXISTEST_Z12(e2.y, e2.x, vAbsEdge.y, vAbsEdge.x);
	
	return true;
}

