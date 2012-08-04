//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// OptionsPrimitives.h: interface for the COptionsPrimitives class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONSPRIMITIVES_H__15797194_F420_11D2_BE16_0060971BDC6D__INCLUDED_)
#define AFX_OPTIONSPRIMITIVES_H__15797194_F420_11D2_BE16_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "optionsbase.h"

class COptionsPrimitives : public COptionsBase  
{
public:
	COptionsPrimitives();
	virtual ~COptionsPrimitives();

	// Saves/Loads all of the options classes
	BOOL		Load();
	BOOL		Save();

	// Sphere primitive
	int			SphereGetNumSides()								{ return m_sphereOptions.m_nSides; }
	int			SphereGetVerticalSubdivisions()					{ return m_sphereOptions.m_nVerticalSubdivisions; }
	float		SphereGetRadius()								{ return m_sphereOptions.m_fRadius; }

	void		SphereSetNumSides(int nSides)					{ m_sphereOptions.m_nSides=nSides; }
	void		SphereSetVerticalSubdivisions(int nDivisions)	{ m_sphereOptions.m_nVerticalSubdivisions=nDivisions; }
	void		SphereSetRadius(float fRadius)					{ m_sphereOptions.m_fRadius=fRadius; }

	// Dome primitive
	int			DomeGetNumSides()								{ return m_domeOptions.m_nSides; }
	int			DomeGetVerticalSubdivisions()					{ return m_domeOptions.m_nVerticalSubdivisions; }
	float		DomeGetRadius()									{ return m_domeOptions.m_fRadius; }

	void		DomeSetNumSides(int nSides)						{ m_domeOptions.m_nSides=nSides; }
	void		DomeSetVerticalSubdivisions(int nDivisions)		{ m_domeOptions.m_nVerticalSubdivisions=nDivisions; }
	void		DomeSetRadius(float fRadius)					{ m_domeOptions.m_fRadius=fRadius; }

protected:
	// Sphere primitive structure
	typedef struct sphereOptionsStruct_t
	{
		int					m_nSides;
		int					m_nVerticalSubdivisions;
		float				m_fRadius;
	} sphereOptionsStruct;

protected:
	// Saves/Loads a sphere options struct with a prefix (either sphere or dome)
	void		SaveSphereOptions(CString sKeyPrefix, sphereOptionsStruct &source);
	void		LoadSphereOptions(CString sKeyPrefix, sphereOptionsStruct &dest);

protected:
	sphereOptionsStruct		m_sphereOptions;	// The sphere options
	sphereOptionsStruct		m_domeOptions;		// The dome options
};

#endif // !defined(AFX_OPTIONSPRIMITIVES_H__15797194_F420_11D2_BE16_0060971BDC6D__INCLUDED_)
