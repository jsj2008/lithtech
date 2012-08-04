
#ifndef __LIGHT_TABLE_H__
#define __LIGHT_TABLE_H__

#ifndef __SYSDDSTRUCTS_H__
#include "sysddstructs.h"
#endif

#include <list>

// Used to determine how to shade things throughout the level.
class CLightTable 
{
public :
    CLightTable();
    ~CLightTable();

    void		Reset();
    void		FreeAll();
	uint32		GetMemAllocSize() const { return (m_DataDims.x * m_DataDims.y * m_DataDims.z * 3); }

	// Load up the light grid...
    bool		Load(ILTStream* pStream);		
	// Load the light grid data associated with a lightgroup
	bool		LoadLightGroup(ILTStream *pStream, uint32 nID, const LTVector &vColor);
	// Clear the lightgroup list
	void		ClearLightGroups() { m_aLightGroups.clear(); }

	void		GetLightVal(const LTVector& vWorldPos,bool bFilter,LTRGB* pRGB) const;

	void		SetLightGroupColor(uint32 nID, const LTVector &vColor);

private :
	// Stream in compressed data...
	bool		Load_RLE_DeCompress(ILTStream* pStream, uint8* pOutData, uint32 iUncompSize);

	/* Add lightgroup data to the provided samples
		The samples are stored in the following order:
		0 = (x,y+1,z)
		1 = (x+1,y+1,z)
		2 = (x,y,z)
		3 = (x+1,y,z)
		4 = (x,y+1,z+1)
		5 = (x+1,y+1,z+1)
		6 = (x,y,z+1)
		7 = (x+1,y,z+1)
	*/
	void		AddLightGroupSamples(LTVector aSamples[], const TVector3<int32> &vGridCoords) const;

	// UnCompressed data...
    uint8*		m_pLightData;
	// Dimensions of the light grid...
	TVector3<int32> m_DataDims;

	// Base position (in world co-ords of the table)...
	LTVector	m_vWorldBasePos;
	// Use to convert from world co-ords to light data position...
	LTVector	m_vWorldToLightDataScale;

	struct SLightGroup
	{
		SLightGroup() : m_pSamples(NULL) {}
		~SLightGroup() { delete[] m_pSamples; }
		SLightGroup(const SLightGroup &cOther) { InternalCopy(cOther); }
		SLightGroup &operator=(const SLightGroup &cOther) 
		{
			if (this == &cOther)
				return *this;
			delete[] m_pSamples;
			InternalCopy(cOther);
			return *this;
		}

		// Sets up the extents of the lightgrid given the minimum point and also the extents that
		// it covers in each direction
		void  SetupExtents(const TVector3<int32>& vMin, const TVector3<int32>& vExtents)
		{
			//setup the bounding box
			m_vMin = vMin;
			m_vMax = vMin + vExtents;

			//now determine the pitches and element counts
			m_nNumElements = vExtents.x * vExtents.y * vExtents.z;

			m_nZPitch = vExtents.x * vExtents.y;
			m_nYPitch = vExtents.x;
		}

		//access to the bounding box of this grid
		const TVector3<int32>&	GetMin() const		{ return m_vMin; }
		const TVector3<int32>&	GetMax() const		{ return m_vMax; }

		// How many samples are there, total?  (i.e. how big is m_pSamples?)
		uint32 GetTotalSampleCount() const { return m_nNumElements; }

		// Get a sample at the given position. Note that this assumes that the values passed in
		//are within range
		LTVector GetSample(int32 nX, int32 nY, int32 nZ) const
		{
			return m_vColor * (float)m_pSamples[nZ * m_nZPitch + nY * m_nYPitch + nX];
		}
		// Is this lightgroup "black"?
		bool IsBlack() const { return (m_vColor.x < (1.0f / 255.0f)) || (m_vColor.y < (1.0f / 255.0f)) || (m_vColor.z < (1.0f / 255.0f)); }

		// Sample data
		uint8 *m_pSamples;

		// ID
		uint32 m_nID;
		// Color
		LTVector m_vColor;
		
	private:

		void InternalCopy(const SLightGroup &cOther)
		{
			m_vMin = cOther.m_vMin;
			m_vMax = cOther.m_vMax;
			m_nNumElements = cOther.m_nNumElements;
			m_nZPitch = cOther.m_nZPitch;
			m_nYPitch = cOther.m_nYPitch;

			if (cOther.m_pSamples)
			{
				LT_MEM_TRACK_ALLOC(m_pSamples = new uint8[GetTotalSampleCount()],LT_MEM_TYPE_WORLD);
				memcpy(m_pSamples, cOther.m_pSamples, GetTotalSampleCount());
			}
			else
				m_pSamples = NULL;
		}

		// Offset inside of the main world's lighting grid
		TVector3<int32> m_vMin;
		
		//the extents of the light table in the main world's lighting grid
		TVector3<int32> m_vMax;

		//the total number of elements
		uint32			m_nNumElements;

		//the pitch for Z
		uint32			m_nZPitch;

		//the pitch for Y
		uint32			m_nYPitch;
	};

	// The list of lightgroups
	// Note : The list is maintained with black (off) lightgroups at the end
	// of the list.  (That's why it's a list instead of a vector.)
	typedef std::list<SLightGroup> TLightGroupList;
	TLightGroupList m_aLightGroups;
};

#endif
