//////////////////////////////////////////////////////////////////////////////
// Yet another axis aligned bounding box implementation.  This one's used by lightmapping

#ifndef __LMAABB_H__
#define __LMAABB_H__

struct CAABB
{
	CAABB() {};
	CAABB(const LTVector &vMin, const LTVector &vMax) : m_vMin(vMin), m_vMax(vMax) {}
	CAABB(const CAABB &cOther) : m_vMin(cOther.m_vMin), m_vMax(cOther.m_vMax) {}
	CAABB &operator=(const CAABB &cOther) {
		m_vMin = cOther.m_vMin;
		m_vMax = cOther.m_vMax;
		return *this;
	}
	CAABB &operator=(const LTVector &vPt) {
		m_vMin = m_vMax = vPt;
		return *this;
	}

	bool operator==(const CAABB &cOther) const {
		return (m_vMin == cOther.m_vMin) && (m_vMax == cOther.m_vMax);
	}
	bool operator!=(const CAABB &cOther) const {
		return (m_vMin != cOther.m_vMin) || (m_vMax != cOther.m_vMax);
	}

	void Union(const CAABB &cOther) {
		VEC_MIN(m_vMin, m_vMin, cOther.m_vMin);
		VEC_MAX(m_vMax, m_vMax, cOther.m_vMax);
	}
	void Union(const LTVector &vPt) {
		VEC_MIN(m_vMin, m_vMin, vPt);
		VEC_MAX(m_vMax, m_vMax, vPt);
	}
	void Intersection(const CAABB &cOther) {
		VEC_MAX(m_vMin, m_vMin, cOther.m_vMin);
		VEC_MIN(m_vMax, m_vMax, cOther.m_vMax);
	}
	bool Intersects(const CAABB &cOther) const {
		return (m_vMin.x <= cOther.m_vMax.x) && (m_vMin.y <= cOther.m_vMax.y) && (m_vMin.z <= cOther.m_vMax.z) &&
			(cOther.m_vMin.x <= m_vMax.x) && (cOther.m_vMin.y <= m_vMax.y) && (cOther.m_vMin.z <= m_vMax.z);
	}
	bool Intersects(const LTVector &vPt) const {
		return (m_vMin.x <= vPt.x) && (m_vMax.x >= vPt.x) && 
			(m_vMin.y <= vPt.y) && (m_vMax.y >= vPt.y) &&
			(m_vMin.z <= vPt.z) && (m_vMax.z >= vPt.z);
	}
	bool Empty() const {
		return (m_vMin.x > m_vMax.x) || (m_vMin.y > m_vMax.y) || (m_vMin.z > m_vMax.z);
	}

	LTVector Size() const { return m_vMax - m_vMin; }
	LTVector Center() const { return (m_vMax + m_vMin) * 0.5f; }

	CAABB operator+(const CAABB &cOther) const {
		CAABB cTemp(*this);
		cTemp.Union(cOther);
		return cTemp;
	}
	CAABB operator+(const LTVector &vPt) const {
		CAABB cTemp(*this);
		cTemp.Union(vPt);
		return cTemp;
	}
	CAABB operator-(const CAABB &cOther) const {
		CAABB cTemp(*this);
		cTemp.Intersection(cOther);
		return cTemp;
	}
	CAABB &operator+=(const CAABB &cOther) {
		Union(cOther);
		return *this;
	}
	CAABB &operator+=(const LTVector &vPt) {
		Union(vPt);
		return *this;
	}

	LTVector m_vMin, m_vMax;
};


#endif //__LMAABB_H__