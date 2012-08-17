#ifndef __LTRECT_H__
#define __LTRECT_H__


//vector is used for the corners of the rectangle
#ifndef __LTVECTOR_H__
#	include "ltvector.h"
#endif


class LTRect {
public:

/*!
\param l x position of left side
\param t y position of top side
\param r x position of right side
\param b y position of bottom side

Used for: Misc.
*/
    LTRect(int l=0, int t=0, int r=0, int b=0) : left(l), top(t), right(r), bottom(b) {}

/*!
\param inLeft x position of left side
\param inTop y position of top side
\param inRight x position of right side
\param inBottom y position of bottom side

Initialize the values.

Used for:   2D Rendering.
*/
    void Init(int inLeft, int inTop, int inRight, int inBottom) {
        left = inLeft;
        top = inTop;
        right = inRight;
        bottom = inBottom;
    }

    int left, top, right, bottom;
};





#ifndef __LINUX
// Provides the definition for the utility class LTRect that represents a
// templatized 2d AABB. Each extent is stored in a 2d vector to allow easier
// operation on individual components. Most utility function assume that
// the components in the min vertex are less than that of the max vertex,
// which can be detected using IsSorted, and fixed with Sort. 



template <class T>
class TRect2 
{
public:

	//constructs a rect object without initializing it
	TRect2()
	{
		Init();
	}

	//constructs a rect object with a default set of values
    TRect2(T l, T t, T r, T b) : 
		m_vMin(l, t),
		m_vMax(r, b)
	{}

	//constructs a rect object from two corner points
	TRect2(const LTVector2<T>& vMin, const LTVector2<T>& vMax) :
		m_vMin(vMin),
		m_vMax(vMax)
	{}

	//initializes the rect with all zeros
	void Init()
	{
		m_vMin.Init();
		m_vMax.Init();
	}

	//initializes the rect with the specified values
    void Init(T l, T t, T r, T b)
	{
        m_vMin.Init(l, t);
		m_vMax.Init(r, b);
    }

	//initializes the rect from two corner points
	void Init(const LTVector2<T>& vMin, const LTVector2<T>& vMax)
	{
		m_vMin = vMin;
		m_vMax = vMax;
	}

	//access to each variable as a standard rectangle member
	T&		Left()			{ return m_vMin.x; }
	T&		Top()			{ return m_vMin.y; }
	T&		Right()			{ return m_vMax.x; }
	T&		Bottom()		{ return m_vMax.y; }

	const T&	Left() const	{ return m_vMin.x; }
	const T&	Top() const		{ return m_vMin.y; }
	const T&	Right() const	{ return m_vMax.x; }
	const T&	Bottom() const	{ return m_vMax.y; }

	//determines the width of the rectangle
	T GetWidth() const
	{
		return m_vMax.x - m_vMin.x;
	}

	//determines the height of the rectangle
	T GetHeight() const
	{
		return m_vMax.y - m_vMin.y;
	}

	//determines the length of the diagonal
	T GetDiagonal() const
	{
		return (T)(m_vMax - m_vMin).Mag();
	}

	//sorts the points so that max > min
	void Sort()
	{
		LTVector2<T> vOldMin = m_vMin;
		m_vMin.Min(m_vMax);
		m_vMax.Max(vOldMin);
	}

	//determines if this rectangle is properly sorted
	bool IsSorted() const
	{
		if((m_vMax.x < m_vMin.x) || (m_vMax.y < m_vMin.y))
			return false;
		return true;
	}

	//expands all edges out by the specified amount
	void Expand(T Amount)
	{
		m_vMin.x -= Amount;
		m_vMin.y -= Amount;
		m_vMax.x += Amount;
		m_vMax.y += Amount;
	}

	void Expand(T ExpandX, T ExpandY)
	{
		m_vMin.x -= ExpandX;
		m_vMin.y -= ExpandY;
		m_vMax.x += ExpandX;
		m_vMax.y += ExpandY;
	}

	void Expand(T ExpandLeft, T ExpandTop, T ExpandRight, T ExpandBottom)
	{
		m_vMin.x -= ExpandLeft;
		m_vMin.y -= ExpandTop;
		m_vMax.x += ExpandRight;
		m_vMax.y += ExpandBottom;
	}

	//offsets the entire rectangle by the specified amount
	void Offset(T XOffset, T YOffset)
	{
		m_vMin.x += XOffset;
		m_vMin.y += YOffset;
		m_vMax.x += XOffset;
		m_vMax.y += YOffset;
	}

	//determines the intersection of two rectangles, and will return the intersection, or the
	//default empty rectangle if there is no overlap
	TRect2<T> GetIntersection(const TRect2<T>& rOther) const
	{
		TRect2<T> rv;
		rv.m_vMin = m_vMin.GetMax(rOther.m_vMin);
		rv.m_vMax = m_vMax.GetMin(rOther.m_vMax);

		//now if the rectangle isn't sorted, clear it (no intersection)
		if(!rv.IsSorted())
			rv.Init();

		return rv;
	}

	//determines a bounding box that encompasses both this rectangle and the specified
	//rectangle
	TRect2<T> GetUnion(const TRect2<T>& rOther) const
	{
		TRect2<T> rv(rOther);
		rv.m_vMin.Min(m_vMin);
		rv.m_vMax.Max(m_vMax);
		return rv;
	}

	//determines if these two rectangles overlap
	bool	Overlaps(const TRect2<T>& rOther) const
	{
		if(	(m_vMin.x >= rOther.m_vMax.x) ||
			(m_vMin.y >= rOther.m_vMax.y) ||
			(m_vMax.x <= rOther.m_vMin.x) ||
			(m_vMax.y <= rOther.m_vMin.y))
		{
			return false;
		}
		return true;
	}

	//determines if this rectangle contains the specified point
	bool	Contains(const LTVector2<T> pt) const
	{
		if( (pt.x >= m_vMin.x) && (pt.x <= m_vMax.x) &&
			(pt.y >= m_vMin.y) && (pt.y <= m_vMax.y))
			return true;
		return false;
	}

	//access to the different points of the rectangle
	const LTVector2<T>&	GetTopLeft() const
	{
		return m_vMin;
	}

	const LTVector2<T>&	GetBottomRight() const
	{
		return m_vMax;
	}

	const LTVector2<T>	GetTopRight() const
	{
		return LTVector2<T>(m_vMax.x, m_vMin.y);
	}

	const LTVector2<T>	GetBottomLeft() const
	{
		return LTVector2<T>(m_vMin.x, m_vMax.y);
	}

	//the actual extents of the rectangle stored as vectors
    LTVector2<T>	m_vMin;
	LTVector2<T> m_vMax;
};

template <class T>
class TRect3 
{
public:

	//constructs a rect object without initializing it
	TRect3()
	{
		Init();
	}

	//constructs a rect object with a default set of values
    TRect3(T l, T t, T n, T r, T b, T f) : 
		m_vMin(l, t, n),
		m_vMax(r, b, f)
	{}

	//constructs a rect object from two corner points
	TRect3(const TVector3<T>& vMin, const TVector3<T>& vMax) :
		m_vMin(vMin),
		m_vMax(vMax)
	{}

	//initializes the rect with all zeros
	void Init()
	{
		m_vMin.Init();
		m_vMax.Init();
	}

	//initializes the rect with the specified values
    void Init(T l, T t, T n, T r, T b, T f)
	{
        m_vMin.Init(l, t, n);
		m_vMax.Init(r, b, f);
    }

	//initializes the rect from two corner points
	void Init(const TVector3<T>& vMin, const TVector3<T>& vMax)
	{
		m_vMin = vMin;
		m_vMax = vMax;
	}

	//access to each variable as a standard rectangle member
	T&		Left()			{ return m_vMin.x; }
	T&		Top()			{ return m_vMin.y; }
	T&		Near()			{ return m_vMin.z; }
	T&		Right()			{ return m_vMax.x; }
	T&		Bottom()		{ return m_vMax.y; }
	T&		Far()			{ return m_vMax.z; }

	const T&	Left() const	{ return m_vMin.x; }
	const T&	Top() const		{ return m_vMin.y; }
	const T&	Near() const	{ return m_vMin.z; }
	const T&	Right() const	{ return m_vMax.x; }
	const T&	Bottom() const	{ return m_vMax.y; }
	const T&	Far() const		{ return m_vMax.z; }


	//determines the width of the rectangle
	T GetWidth() const
	{
		return m_vMax.x - m_vMin.x;
	}

	//determines the height of the rectangle
	T GetHeight() const
	{
		return m_vMax.y - m_vMin.y;
	}

	//determines the depth of the rectangle
	T GetDepth() const
	{
		return m_vMax.z - m_vMin.z;
	}

	//determines the length of the diagonal
	T GetDiagonal() const
	{
		return (T)(m_vMax - m_vMin).Mag();
	}

	//sorts the points so that max > min
	void Sort()
	{
		TVector3<T> vOldMin = m_vMin;
		m_vMin.Min(m_vMax);
		m_vMax.Max(vOldMin);
	}

	//determines if this rectangle is properly sorted
	bool IsSorted() const
	{
		if((m_vMax.x < m_vMin.x) || (m_vMax.y < m_vMin.y) || (m_vMax.z < m_vMin.z))
			return false;
		return true;
	}

	//expands all edges out by the specified amount
	void Expand(T Amount)
	{
		m_vMin.x -= Amount;
		m_vMin.y -= Amount;
		m_vMin.z -= Amount;
		m_vMax.x += Amount;
		m_vMax.y += Amount;
		m_vMax.z += Amount;
	}

	void Expand(T ExpandX, T ExpandY, T ExpandZ)
	{
		m_vMin.x -= ExpandX;
		m_vMin.y -= ExpandY;
		m_vMin.z -= ExpandZ;
		m_vMax.x += ExpandX;
		m_vMax.y += ExpandY;
		m_vMax.z += ExpandZ;
	}

	void Expand(const TVector3<T>& Expand)
	{
		m_vMin -= Expand;
		m_vMax += Expand;
	}

	void Expand(T ExpandLeft, T ExpandTop, T ExpandNear, T ExpandRight, T ExpandBottom, T ExpandFar)
	{
		m_vMin.x -= ExpandLeft;
		m_vMin.y -= ExpandTop;
		m_vMin.z -= ExpandNear;
		m_vMax.x += ExpandRight;
		m_vMax.y += ExpandBottom;
		m_vMax.z += ExpandFar;
	}

	void Expand(const TVector3<T>& Min, const TVector3<T>& Max)
	{
		m_vMin -= Min;
		m_vMax += Max;
	}

	//merges a point or rect into the rectangle, by expanding rectangle to encompass the point(s).
	void Merge(const TVector3<T>& Pt)
	{
		// Expand the rect to fit the Pt.
		m_vMin.Min(Pt);
		m_vMax.Max(Pt);
	}

	void Merge(const TRect3<T>& rOther)
	{
		// Expand the rect to fit the rOther.
		m_vMin.Min(rOther.m_vMin);
		m_vMax.Max(rOther.m_vMax);
	}

	//offsets the entire rectangle by the specified amount
	void Offset(T XOffset, T YOffset, T ZOffset)
	{
		m_vMin.x += XOffset;
		m_vMin.y += YOffset;
		m_vMin.z += ZOffset;
		m_vMax.x += XOffset;
		m_vMax.y += YOffset;
		m_vMax.z += ZOffset;
	}

	void Offset(const TVector3<T>& Offset)
	{
		m_vMin += Offset;
		m_vMax += Offset;
	}

	//determines the intersection of two rectangles, and will return the intersection, or the
	//default empty rectangle if there is no overlap
	TRect3<T> GetIntersection(const TRect3<T>& rOther) const
	{
		TRect3<T> rv;
		rv.m_vMin = m_vMin.GetMax(rOther.m_vMin);
		rv.m_vMax = m_vMax.GetMin(rOther.m_vMax);

		//now if the rectangle isn't sorted, clear it (no intersection)
		if(!rv.IsSorted())
			rv.Init();

		return rv;
	}

	//determines a bounding box that encompasses both this rectangle and the specified
	//rectangle
	TRect3<T> GetUnion(const TRect3<T>& rOther) const
	{
		TRect3<T> rv(rOther);
		rv.m_vMin.Min(m_vMin);
		rv.m_vMax.Max(m_vMax);
	}

	//determines if these two rectangles overlap
	bool	Overlaps(const TRect3<T>& rOther) const
	{
		if(	(m_vMin.x >= rOther.m_vMax.x) ||
			(m_vMin.y >= rOther.m_vMax.y) ||
			(m_vMin.z >= rOther.m_vMax.z) ||
			(m_vMax.x <= rOther.m_vMin.x) ||
			(m_vMax.y <= rOther.m_vMin.y) ||
			(m_vMax.z <= rOther.m_vMin.z)) 
		{
			return false;
		}
		return true;
	}

	//determines if this rectangle contains the specified point
	bool	Contains(const TVector3<T> pt) const
	{
		if( (pt.x >= m_vMin.x) && (pt.x <= m_vMax.x) &&
			(pt.y >= m_vMin.y) && (pt.y <= m_vMax.y) &&
			(pt.z >= m_vMin.z) && (pt.z <= m_vMax.z))
			return true;
		return false;
	}

	//the actual extents of the rectangle stored as vectors
    TVector3<T>	m_vMin;
	TVector3<T> m_vMax;
};

//common rectangle types
typedef TRect2<int32>	LTRect2n;
typedef TRect2<float>	LTRect2f;
typedef TRect3<int32>	LTRect3n;
typedef TRect3<float>	LTRect3f;

#endif //!LINUX

#endif  //! __LTRECT_H__













