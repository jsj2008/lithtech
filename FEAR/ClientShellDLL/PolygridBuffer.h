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
		m_pData(NULL), m_nWidth(0), m_nHeight(0)
	{
	}

	//uses a preconstructed buffer of the specified dimensions. Note that
	//it will not delete this buffer
	CPolyGridBuffer(uint32 nWidth, uint32 nHeight, T* pData) :
		m_pData(pData), m_nWidth(nWidth), m_nHeight(nHeight)
	{
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

	//gets the size of this buffer
	uint32		GetBufferSize() const			{ return sizeof(T) * m_nWidth * m_nHeight; }

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

		//in with the new
		m_pData = debug_newa(T, nWidth * nHeight);

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

private:

	PREVENT_OBJECT_COPYING(CPolyGridBuffer);

	//frees the buffer
	void FreeBuffer()
	{
		delete [] m_pData;
		m_pData = NULL;
		m_nWidth = 0;
		m_nHeight = 0;
	}

	//the actual buffer
	T*			m_pData;

	//the buffer dimensions
	uint32		m_nWidth;
	uint32		m_nHeight;
};


#endif

