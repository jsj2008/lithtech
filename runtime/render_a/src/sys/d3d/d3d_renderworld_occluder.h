//////////////////////////////////////////////////////////////////////////////
// Render world occluder/occludee classes

#ifndef __D3D_RENDERWORLD_OCCLUDER_H__
#define __D3D_RENDERWORLD_OCCLUDER_H__

#include "aabb.h"
#include <vector.h>

struct SRBGeometryPoly;
class ViewParams;

class COccludee
{
public:
	COccludee() 
	{
		m_aVisible.reserve(16);
		m_aOccluded.reserve(16);
	}

	// Effectively a static-length vector of vertices
	// Note that originally the array was changed to an array of floats to use quadword alignment
	// on the array elements.  It was much faster with the new code, but turned out to be unrelated
	// to the alignment of the array.
	class COutline
	{
	public:
		class iterator;
		class const_iterator;

		friend iterator;
		friend const_iterator;
	public:
		COutline() : m_pEndVert(m_aVerts) {}
		COutline(const COutline &cOther) : 
			m_pEndVert(iterator(m_aVerts) + cOther.size())
		{
			memcpy(m_aVerts, cOther.m_aVerts, sizeof(m_aVerts[0]) * k_nDataStride * size());
		}
		COutline &operator=(const COutline &cOther) 
		{ 
			uint32 nSize = cOther.size();
			m_pEndVert = iterator(m_aVerts) + nSize;
			memcpy(m_aVerts, cOther.m_aVerts, sizeof(m_aVerts[0]) * k_nDataStride * nSize);
			return *this; 
		}

		class const_iterator
		{
		public:
			const_iterator(const float *pPos) : m_pPos(pPos) {}
			const_iterator(const iterator &cOther) : m_pPos(cOther.m_pPos) {}
			const_iterator(const const_iterator &cOther) : m_pPos(cOther.m_pPos) {}
			const_iterator &operator=(const iterator &cOther) { m_pPos = cOther.m_pPos; return *this; }
			const_iterator &operator=(const const_iterator &cOther) { m_pPos = cOther.m_pPos; return *this; }
			bool operator==(const const_iterator &cOther) const { return m_pPos == cOther.m_pPos; }
			bool operator!=(const const_iterator &cOther) const { return m_pPos != cOther.m_pPos; }
			bool operator==(const iterator &cOther) const { return m_pPos == cOther.m_pPos; }
			bool operator!=(const iterator &cOther) const { return m_pPos != cOther.m_pPos; }
			const_iterator &operator++() { m_pPos += k_nDataStride; return *this; }
			const_iterator &operator--() { m_pPos -= k_nDataStride; return *this; }
			const_iterator operator+(size_t nOffset) const { return const_iterator(m_pPos + nOffset * k_nDataStride); }
			const_iterator operator-(size_t nOffset) const { return const_iterator(m_pPos - nOffset * k_nDataStride); }
			size_t operator-(const iterator &cOther) const { return (m_pPos - cOther.m_pPos) / k_nDataStride; }
			size_t operator-(const const_iterator &cOther) const { return (m_pPos - cOther.m_pPos) / k_nDataStride; }
			const LTVector &operator*() const { return *reinterpret_cast<const LTVector*>(m_pPos); }
			const LTVector &operator[](size_t nOffset) { return *reinterpret_cast<const LTVector*>(m_pPos + nOffset * k_nDataStride); }
		private:
			const float *m_pPos;
		};

		class iterator
		{
			friend const_iterator;
		public:
			iterator(float *pPos) : m_pPos(pPos) {}
			iterator(const iterator &cOther) : m_pPos(cOther.m_pPos) {}
			iterator &operator=(const iterator &cOther) { m_pPos = cOther.m_pPos; return *this; }
			bool operator==(const iterator &cOther) const { return m_pPos == cOther.m_pPos; }
			bool operator!=(const iterator &cOther) const { return m_pPos != cOther.m_pPos; }
			iterator &operator++() { m_pPos += k_nDataStride; return *this; }
			iterator &operator--() { m_pPos -= k_nDataStride; return *this; }
			iterator operator+(size_t nOffset) const { return iterator(m_pPos + nOffset * k_nDataStride); }
			iterator operator-(size_t nOffset) const { return iterator(m_pPos - nOffset * k_nDataStride); }
			size_t operator-(const iterator &cOther) const { return (m_pPos - cOther.m_pPos) / k_nDataStride; }
			LTVector &operator*() const { return *reinterpret_cast<LTVector*>(m_pPos); }
			LTVector &operator[](size_t nOffset) { return *reinterpret_cast<LTVector*>(m_pPos + nOffset * k_nDataStride); }
		private:
			float *m_pPos;
		};

		// Functions to make it look somewhat like an STL container
		void push_back(const LTVector &vVec)
		{
			ASSERT(size() < k_MaxVerts);
			*m_pEndVert = vVec;
			++m_pEndVert;
		}
		void clear() { m_pEndVert = iterator(m_aVerts); }
		const_iterator begin() const { return const_iterator(m_aVerts); }
		const_iterator end() const { return m_pEndVert; }
		iterator begin() { return iterator(m_aVerts); }
		iterator end() { return m_pEndVert; }
		uint32 size() const { return const_iterator(m_pEndVert) - const_iterator(m_aVerts); }
		void resize(uint32 nSize) { ASSERT(nSize <= k_MaxVerts); m_pEndVert = iterator(m_aVerts) + nSize; }
		bool empty() const { return const_iterator(m_pEndVert) == const_iterator(m_aVerts); }
		LTVector &front() { return *iterator(m_aVerts); }
		LTVector &back() { return *(m_pEndVert - 1); }
		const LTVector &front() const { return *const_iterator(m_aVerts); }
		const LTVector &back() const { return *(m_pEndVert - 1); }
		LTVector &operator[](uint32 nIndex) { return iterator(m_aVerts)[nIndex]; }
		const LTVector &operator[](uint32 nIndex) const { return const_iterator(m_aVerts)[nIndex]; }

		// Geometric functions
		float CalcArea() const;
		float CalcArea2D() const;
	private:
		enum { k_MaxVerts = 32 };
		enum { k_nDataStride = 3 };
		float m_aVerts[k_MaxVerts * k_nDataStride];
		iterator m_pEndVert;
	};

	void Init() { m_aVisible.clear(); m_aOccluded.clear(); }
	void InitAABB(const ViewParams& pParams, const LTVector &vMin, const LTVector &vMax, bool bVisible);
	void InitPolyWorld(const SRBGeometryPoly &sPoly, bool bVisible);
	// bSplitOccluded dictates whether or not occluded polys should also be split
	PolySide SplitPlane(const LTPlane &cPlane, bool bSplitOccluded);
	PolySide SplitPlane2D(const LTPlane &cPlane, bool bSplitOccluded);
	// Get the occludee's classification
	PolySide ClassifyPlane(const LTPlane &cPlane, bool bVisible) const;

	// Calculate the remaining area of the occludee
	float CalcArea(bool bVisible) const;
	float CalcArea2D(bool bVisible) const;

	typedef std::vector<COutline> TOutlineList;
	TOutlineList m_aVisible;
	TOutlineList m_aOccluded;

	LTVector m_vMin, m_vMax;

private:
	static PolySide SplitOutline(const LTPlane &cPlane, const COutline &sInput, COutline &sVisible, COutline &sOccluded);
	static PolySide SplitOutline2D(const LTPlane &cPlane, const COutline &sInput, PolySide nCoplanarResult, COutline &sVisible, COutline &sOccluded);
	static PolySide ClassifyOutline(const LTPlane &cPlane, const COutline &sInput);
	// Given a set of points transformed into screen space, this function creates an outline describing the convex hull
	// Note : This modifies the order of aPts for reasons of efficiency
	// Also note : This is a class masquerading as a function to get around a VC bug with explicitly templatized functions
	template <int NUM_PTS>
	struct CreatePtsOutline
	{
		CreatePtsOutline(LTVector aPts[NUM_PTS], COutline *pResult);
	};
};

class COccluder
{
public:
	COccluder() { m_ePolyPlaneCorner = eAABB_None; }
	COccluder(const COccluder &cOther) :
		m_aEdgePlanes(cOther.m_aEdgePlanes),
		m_cPolyPlane(cOther.m_cPolyPlane),
		m_ePolyPlaneCorner(cOther.m_ePolyPlaneCorner)
	{}
	~COccluder() {}
	COccluder &operator=(const COccluder &cOther) {
		m_aEdgePlanes = cOther.m_aEdgePlanes;
		m_cPolyPlane = cOther.m_cPolyPlane;
		m_ePolyPlaneCorner = cOther.m_ePolyPlaneCorner;
		return *this;
	}

	void Swap(COccluder &cOther) {
		m_aEdgePlanes.swap(cOther.m_aEdgePlanes);
		std::swap(m_cPolyPlane, cOther.m_cPolyPlane);
		std::swap(m_ePolyPlaneCorner, cOther.m_ePolyPlaneCorner);
	}

	void Init() { m_aEdgePlanes.clear(); }

	// Add an occlusion plane
	void AddPlane(const LTPlane &cScreenPlane);

	PolySide Occlude(COccludee &cOccludee) const;
	PolySide Occlude2D(COccludee &cOccludee) const;

	typedef std::vector<LTPlane> TPlaneList;
	TPlaneList m_aEdgePlanes;
	LTPlane m_cPolyPlane;
	EAABBCorner m_ePolyPlaneCorner;
};

class COccluder_Frustum : public COccluder
{
public:
	COccluder_Frustum() : COccluder() {}
	COccluder_Frustum(const COccluder_Frustum &cOther) :
		COccluder(cOther),
		m_aWorldEdgePlanes(cOther.m_aWorldEdgePlanes),
		m_aEdgeCorners(cOther.m_aEdgeCorners)
	{}
	~COccluder_Frustum() {}
	COccluder_Frustum &operator=(const COccluder_Frustum &cOther) {
		static_cast<COccluder&>(*this) = cOther;
		m_aWorldEdgePlanes = cOther.m_aWorldEdgePlanes;
		m_aEdgeCorners = cOther.m_aEdgeCorners;
		return *this;
	}

	void Swap(COccluder_Frustum &cOther) {
		COccluder::Swap(cOther);
		m_aWorldEdgePlanes.swap(cOther.m_aWorldEdgePlanes);
		m_aEdgeCorners.swap(cOther.m_aEdgeCorners);
	}

	void Init() { COccluder::Init(); m_aWorldEdgePlanes.clear(); m_aEdgeCorners.clear(); }

	void InitFrustum(const ViewParams& pParams);

	// Add an occlusion plane
	void AddPlane(const LTPlane &cScreenPlane, const LTPlane &cWorldPlane);

	PolySide Occlude(COccludee &cOccludee) const;
	PolySide Occlude2D(COccludee &cOccludee) const;
	PolySide OccludeWorld(COccludee &cOccludee) const;

	// Some rather more specific routines for handling the algo properly
	PolySide ClipNear(COccludee &cOccludee) const;
	PolySide Classify(COccludee &cOccludee) const;
	PolySide ClassifyAABB(const LTVector &vMin, const LTVector &vMax, float fFarZ) const;

	TPlaneList m_aWorldEdgePlanes;
	typedef std::vector<EAABBCorner> TCornerList;
	TCornerList m_aEdgeCorners; 
};

class COccluder_2D : public COccluder
{
public:
	COccluder_2D() : COccluder(), m_fScreenArea(0.0f) {}
	COccluder_2D(const COccluder_2D &cOther) :
		COccluder(cOther),
		m_fScreenArea(cOther.m_fScreenArea)
	{}
	~COccluder_2D() {}
	COccluder_2D &operator=(const COccluder_2D &cOther) {
		static_cast<COccluder&>(*this) = cOther;
		m_fScreenArea = cOther.m_fScreenArea;
		return *this;
	}

	void Swap(COccluder_2D &cOther) {
		COccluder::Swap(cOther);
		std::swap(m_fScreenArea, cOther.m_fScreenArea);
	}

	void Init() { COccluder::Init(); m_fScreenArea = 0.0f; }

	void InitOutline(const ViewParams& pParams, const COccludee::COutline &cOutline, const LTPlane &cPlane);

	float m_fScreenArea;
};

// Useful container typedefs
typedef std::vector<COccluder_2D> TOccluderList;


#endif //__D3D_RENDERWORLD_OCCLUDER_H__