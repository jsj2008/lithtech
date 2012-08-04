#ifndef __LTBBOX_H__
#define __LTBBOX_H__

#ifndef __LTVECTOR_H__
#include "ltvector.h"
#endif


struct LTBBox {
    LTVector3f  m_vMin;//!the box's lower bound
    LTVector3f  m_vMax;//!the box's upper bound
    LTBOOL PtInsideBox(LTVector3f p);
    LTBOOL PtTouchingBox(LTVector3f p);
    LTBOOL IntersectsBox(LTBBox B);
    LTBOOL TouchesBox(LTBBox B);
    LTVector3f GetCenter();
    LTVector3f  GetDims();
};

inline LTBOOL LTBBox::PtInsideBox(LTVector3f p) {
    return p.x > m_vMin.x && 
        p.y > m_vMin.y &&
        p.z > m_vMin.z &&
        p.x < m_vMax.x &&
        p.y < m_vMax.y &&
        p.z < m_vMin.z;
}

inline LTBOOL LTBBox::PtTouchingBox(LTVector3f p) {
    return p.x >= m_vMin.x && 
        p.y >= m_vMin.y &&
        p.z >= m_vMin.z &&
        p.x <= m_vMax.x &&
        p.y <= m_vMax.y &&
        p.z <= m_vMin.z;
}

inline LTBOOL LTBBox::IntersectsBox(LTBBox theBox) {
    return !(
        theBox.m_vMin.x >= m_vMax.x || 
        theBox.m_vMin.y >= m_vMax.y || 
        theBox.m_vMin.z >= m_vMax.z || 
        theBox.m_vMax.x <= m_vMin.z || 
        theBox.m_vMax.y <= m_vMin.y || 
        theBox.m_vMax.z <= m_vMin.z
   );
}

inline LTBOOL LTBBox::TouchesBox(LTBBox theBox) {
    return !(
        theBox.m_vMin.x > m_vMax.x || 
        theBox.m_vMin.y > m_vMax.y || 
        theBox.m_vMin.z > m_vMax.z || 
        theBox.m_vMax.x < m_vMin.z || 
        theBox.m_vMax.y < m_vMin.y || 
        theBox.m_vMax.z < m_vMin.z
   );
}

inline LTVector3f LTBBox::GetCenter() {
    return (m_vMin + m_vMax) * 0.5f;
}

inline LTVector3f LTBBox::GetDims() {
    return (m_vMax - m_vMin) * 0.5f;
}

#endif




