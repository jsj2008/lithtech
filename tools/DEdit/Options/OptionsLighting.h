//--------------------------------------------------------------
//OptionsLighting.h
//
// Contains the definition for COptionsLighting which holds the
// user options for displaying lighting within DEdit
//
// Author: John O'Rorke
// Created: 3/20/01
// Modification History:
//
//---------------------------------------------------------------
#ifndef __OPTIONSLIGHTING_H__
#define __OPTIONSLIGHTING_H__

#include "optionsbase.h"

class COptionsLighting : public COptionsBase  
{
public:

	COptionsLighting();
	virtual ~COptionsLighting();

	// Load/Save
	BOOL	Load();
	BOOL	Save();

	// Access to the options

	bool		IsLambertian() const				{return m_bLambertian;}
	void		SetLambertian(bool bVal)			{m_bLambertian = bVal;}

	bool		IsShadows() const					{return m_bShadows;}
	void		SetShadows(bool bVal)				{m_bShadows = bVal;}

	uint32		GetTimeSlice() const				{return m_nTimeSlice;}
	void		SetTimeSlice(uint32 nVal)			{m_nTimeSlice = nVal;}

	uint32		GetMaxLMSize() const				{return m_nMaxLMSize;}
	void		SetMaxLMSize(uint32 nVal)			{m_nMaxLMSize = nVal;}

	uint32		GetMinLMSize() const				{return m_nMinLMSize;}
	void		SetMinLMSize(uint32 nVal)			{m_nMinLMSize = nVal;}

	uint32		GetLMTexelSize() const				{return m_nLMTexelSize;}
	void		SetLMTexelSize(uint32 nVal)			{m_nLMTexelSize = nVal;}

	bool		IsLightMap() const					{return m_bLightMap;}
	void		SetLightMap(bool bVal)				{m_bLightMap = bVal;}
	
	bool		IsVertex() const					{return m_bVertex;}
	void		SetVertex(bool bVal)				{m_bVertex = bVal;}

	uint32		GetLightLeakDist() const			{return m_nLightLeakDist;}
	void		SetLightLeakDist(uint32 nVal)		{m_nLightLeakDist = nVal;}

protected:

	//use lambertian lighting
	bool		m_bLambertian;

	//cast shadows
	bool		m_bShadows;

	//the time slice length (ms)
	uint32		m_nTimeSlice;

	//the minimum lightmap size
	uint32		m_nMinLMSize;

	//the maximum lightmap size
	uint32		m_nMaxLMSize;

	//the minimum size of a lightmap texel
	uint32		m_nLMTexelSize;

	//whether or not to lightmap
	bool		m_bLightMap;

	//whether or not to vertex light
	bool		m_bVertex;

	//the distance that light can leak beneath a surface to prevent darkness from leaking
	uint32		m_nLightLeakDist;
};

#endif 
