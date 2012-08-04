#include "bdefs.h"
#include "levelerrordb.h"
#include "detectors.h"

CLevelErrorDB::CLevelErrorDB()
{
	m_DetectorList.SetCacheSize(32);
	m_ErrorList.SetCacheSize(32);
}

CLevelErrorDB::~CLevelErrorDB()
{
	DeleteAndClearArray(m_DetectorList);
	RemoveAll();	
}


//utility macro
#define ADD_DETECTOR(cls)	{m_DetectorList.Append((CErrorDetector*)(new cls));}

//builds up the list of all the error detectors
bool CLevelErrorDB::BuildDetectorList()
{
	DeleteAndClearArray(m_DetectorList);

	//add all the classes
	ADD_DETECTOR(CConcavePolyDetector);
	ADD_DETECTOR(CNonplanarDetector);
	ADD_DETECTOR(CPolyEdgeLenDetector);
	ADD_DETECTOR(CPointsOffGridDetector);
	ADD_DETECTOR(CRedundantPointsDetector);
	ADD_DETECTOR(CSmallPolySizeDetector);
	ADD_DETECTOR(CInvalidPolyCountDetector);
	ADD_DETECTOR(CLargeLightmappedPoliesDetector);
	ADD_DETECTOR(CSmallLMGridSizeDetector);
	ADD_DETECTOR(CInvalidSunlightDetector);
	ADD_DETECTOR(CLightmappedSkyportalDetector);
	ADD_DETECTOR(CObjectsOutsideLevelDetector);
	ADD_DETECTOR(CMultipleSkyDimsDetector);
	ADD_DETECTOR(CDuplicateSkyIndexDetector);
	ADD_DETECTOR(CDuplicateObjectNameDetector);
	ADD_DETECTOR(CUntexturedPoliesDetector);
	ADD_DETECTOR(CMissingTexturesDetector);
	ADD_DETECTOR(CInvalidNonDetailBrushDetector);
	ADD_DETECTOR(CShadowMeshBrushDetector);

	return true;
}

#undef ADD_DETECTOR

void CLevelErrorDB::RemoveAll()
{
	DeleteAndClearArray(m_ErrorList);
}

//loads up the enabled status for each detector from the registry
bool CLevelErrorDB::LoadEnabledStatus(COptionsBase* pOptBase)
{
	//run through the list of the detectors, and find the approprate setting in
	//the registry
	for(uint32 nCurrDet = 0; nCurrDet < GetNumDetectors(); nCurrDet++)
	{
		CErrorDetector* pDet = GetDetector(nCurrDet);

		//try and load it in
		bool bEnable = pOptBase->GetBoolValue(pDet->GetName(), TRUE) ? true : false;

		pDet->SetEnabled(bEnable);
	}

	return true;
}

//saves the enabled status for each detector to the registry
bool CLevelErrorDB::SaveEnabledStatus(COptionsBase* pOptBase)
{
	//run through the list of the detectors, and set the approprate setting in
	//the registry
	for(uint32 nCurrDet = 0; nCurrDet < GetNumDetectors(); nCurrDet++)
	{
		CErrorDetector* pDet = GetDetector(nCurrDet);

		pOptBase->SetBoolValue(pDet->GetName(), pDet->IsEnabled() ? TRUE : FALSE);
	}

	return true;
}


//builds up the big list of errors
bool CLevelErrorDB::BuildErrorList(CRegionDoc* pDoc)
{
	DeleteAndClearArray(m_ErrorList);

	//now run through all active detectors and build the list of errors
	for(uint32 nCurrDet = 0; nCurrDet < GetNumDetectors(); nCurrDet++)
	{
		CErrorDetector* pDet = GetDetector(nCurrDet);

		//skip it if it is inactive
		if(pDet->IsEnabled() == false)
		{
			continue;
		}

		//let this detector find its share of errors
		pDet->BuildErrorList(pDoc, m_ErrorList);
	}

	return true;
}

//gets the number of detectors
uint32 CLevelErrorDB::GetNumDetectors() const
{
	return m_DetectorList.GetSize();
}

//gets the specified detector
CErrorDetector*	CLevelErrorDB::GetDetector(uint32 nIndex)
{
	return m_DetectorList[nIndex];
}

//gets the number of errors
uint32 CLevelErrorDB::GetNumErrors() const
{
	return m_ErrorList.GetSize();
}

//gets a specific error
CLevelError* CLevelErrorDB::GetError(uint32 nIndex)
{
	return m_ErrorList[nIndex];
}

