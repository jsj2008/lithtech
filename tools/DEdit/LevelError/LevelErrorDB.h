#ifndef __LEVELERRORDB_H__
#define __LEVELERRORDB_H__

#ifndef __ERRORDETECTOR_H__
#	include "errordetector.h"
#endif

#include "optionsbase.h"

class CLevelErrorDB
{
public:

	CLevelErrorDB();
	~CLevelErrorDB();

	//builds up the list of all the error detectors
	bool			BuildDetectorList();

	//loads up the enabled status for each detector from the registry
	bool			LoadEnabledStatus(COptionsBase* pOptBase);

	//saves the enabled status for each detector to the registry
	bool			SaveEnabledStatus(COptionsBase* pOptBase);

	//builds up the big list of errors
	bool			BuildErrorList(CRegionDoc* pDoc);

	//gets the number of detectors
	uint32			GetNumDetectors() const;

	//gets the specified detector
	CErrorDetector*	GetDetector(uint32 nIndex);

	//gets the number of errors
	uint32			GetNumErrors() const;

	//gets a specific error
	CLevelError*	GetError(uint32 nIndex);

	//removes all errors from the list
	void			RemoveAll();

private:

	CMoArray<CErrorDetector*>	m_DetectorList;
	CMoArray<CLevelError*>		m_ErrorList;
	
};


#endif
