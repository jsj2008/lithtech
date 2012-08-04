///////////////////////////////////////////////////////////////////
// PolygridBuffer.h
//
// Template class used for holding an array of the specified size
// for polygrid operations
//
#ifndef __POLYGRIDBUFFER_H__
#define __POLYGRIDBUFFER_H__

template <class T>
class CPolyGridBuffer
{
public:
	
	//creates a new buffer of the specified dimensions
	CPolyGridBuffer() :
		m_pData(NULL), m_nWidth(0), m_nHeight(0), m_bOwnBuffer(false)
	{
	}

	//uses a preconstructed buffer of the specified dimensions. Note that
	//it will not delete this buffer
	CPolyGridBuffer(uint32 nWidth, uint32 nHeight, T* pData) :
		m_pData(pData), m_nWidth(nWidth), m_nHeight(nHeight), m_bOwnBuffer(false)
	{
	}

	//copy constructor
	CPolyGridBuffer(const CPolyGridBuffer<typename T>& rhs) :
		m_pData(NULL), m_nWidth(0), m_nHeight(0), m_bOwnBuffer(false)
	{
		*this = rhs;
	}

	//destructor
	~CPolyGridBuffer()			{ FreeBuffer();	}

	uint32 GetWidth() const		{ return m_nWidth; }
	uint32 GetHeight() const	{ return m_nHeight; }

	//gets a specified value
	T&			Get(uint32 nX, uint32 nY)		{ assert(nX < GetWidth()); assert(nY < GetHeight()); return m_pData[nY * GetWidth() + nX]; }
	const T&	Get(uint32 nX, uint32 nY) const	{ assert(nX < GetWidth()); assert(nY < GetHeight()); return m_pData[nY * GetWidth() + nX]; }

	//index operators
	T&			operator[](uint32 nVal)			{ assert(nVal < GetWidth() * GetHeight()); return m_pData[nVal]; }
	const T&	operator[](uint32 nVal) const	{ assert(nVal < GetWidth() * GetHeight()); return m_pData[nVal]; }

	//gets a specified row
	T*			GetRow(uint32 nY)				{ assert(nY < GetHeight()); return &m_pData[nY * GetWidth()]; }		

	//gets a pointer to the first element of the buffer
	T*			GetBuffer()						{ return m_pData; }
	const T*	GetBuffer() const				{ return m_pData; }

	//resizes the buffer, note that values will be lost
	bool Resize(uint32 nWidth, uint32 nHeight)
	{
		//do nothing if the size matches
		if((nWidth == GetWidth()) && (nHeight == GetHeight()))
			return true;

		//out with the old
		FreeBuffer();

		if((nWidth == 0) || (nHeight == 0))
			return true;

		//we now own this buffer
		m_bOwnBuffer = true;

		//in with the new
		m_pData = new T [nWidth * nHeight];

		//check the allocation
		if(m_pData)
		{
			m_nWidth = nWidth;
			m_nHeight = nHeight;
			return true;
		}

		//allocation failed
		return false;
	}

	//assignement operator
	CPolyGridBuffer<typename T>& operator=(const CPolyGridBuffer<typename T>& rhs)
	{
		if(Resize(rhs.GetWidth(), rhs.GetHeight()))
		{
			memcpy(m_pData, rhs.m_pData, sizeof(T) * GetWidth() * GetHeight());
		}
		return *this;
	}
 
private:

	//frees the buffer
	void FreeBuffer()
	{
		if(m_bOwnBuffer)
		{
			delete [] m_pData;
		}
		m_pData = NULL;
		m_nWidth = 0;
		m_nHeight = 0;
	}

	//the actual buffer
	T*			m_pData;

	//the buffer dimensions
	uint32		m_nWidth;
	uint32		m_nHeight;

	//flag indicating if we own the buffer or not
	bool		m_bOwnBuffer;
};


#endif

