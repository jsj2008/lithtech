// ----------------------------------------------------------------------- //
//
// MODULE  : OrbitalScreenshotMgr.h
//
// PURPOSE : Provides the definition of a manager that hooks into the console
//			 system and upon activation takes a series of screenshots on an orbit
//			 which can then be pieced together into movies.
//
// CREATED : 10/28/04
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#ifndef __ORBITALSCREENSHOTMGR_H__
#define __ORBITALSCREENSHOTMGR_H__

class COrbitalScreenshotMgr
{
public:

	~COrbitalScreenshotMgr();

	//singleton access
	static COrbitalScreenshotMgr&	Singleton();

	//this must be called on this manager at some point in order to register the
	//console variables and programs with the engine so that this manager can be
	//used. This assumes that the global client interface is valid when called.
	void	InitConsolePrograms();

	//called to set the marker to the provided position for capture. This will override any
	//previously placed markers. Orbital screenshots are taken around a marker (unless none
	//is provided in which case it goes around the camera)
	void	SetMarker(const LTVector& vMarker)		{ m_bMarkerSet = true; m_vMarker = vMarker; }

	//called to clear any currently set marker
	void	ClearMarker()							{ m_bMarkerSet = false; }

	//called to determine if a marker is currently set
	bool	IsMarkerSet() const						{ return m_bMarkerSet; }

	//called to get the marker. This is undefined if the marker is not set.
	const LTVector&	GetMarker() const				{ return m_vMarker; }

	//called to capture a screenshot using the current marker, provided radius, number
	//of images, and vertical offset
	bool	Capture(float fOrbitalRadius, uint32 nNumImages, float fVerticalOffset);

private:

	//only allow for singleton construction
	COrbitalScreenshotMgr();
	PREVENT_OBJECT_COPYING(COrbitalScreenshotMgr);

	//the current marker
	LTVector	m_vMarker;

	//flag indicating whether or not a marker is set
	bool		m_bMarkerSet;
};

#endif
