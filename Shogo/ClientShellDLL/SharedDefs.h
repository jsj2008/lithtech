#ifndef __SHAREDEFS_H__
#define __SHAREDEFS_H__

// Various useful defines


#define PI				(LTFLOAT)3.14159
#define PIx2			(LTFLOAT)PI*2
#define DEG2RAD(x)		((x*PI)/180)
#define RAD2DEG(x)		((x*180)/PI)

#define VEC_EQU(v1, v2) (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z)
#define ROTN_EQU(r1, r2) (VEC_EQU(r1.m_Vec, r2.m_Vec) && r1.m_Quat[3] == r2.m_Quat[3])
#define ROTN_SUB(d, r1, r2) VEC_SUB(d.m_Vec, r1.m_Vec, r2.m_Vec); d.m_Quat[3] = r1.m_Quat[3] - r2.m_Quat[3];
#define ROTN_ADD(d, r1, r2) VEC_ADD(d.m_Vec, r1.m_Vec, r2.m_Vec); d.m_Quat[3] = r1.m_Quat[3] + r2.m_Quat[3];


#define CLIPLOWHIGH(x,l,h)  if (x < l) x = l; if (x > h) x = h;


#endif  // __SHAREDEFS_H__