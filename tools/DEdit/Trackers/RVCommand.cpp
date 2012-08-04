//////////////////////////////////////////////////////////////////////
// RVTracker.cpp - Implementation of the RegionView tracker class

#include "bdefs.h"
#include "rvcommand.h"

CRegionViewCommand::CRegionViewCommand(LPCTSTR pName, CRegionView *pView, BOOL bOnStart) : 
	CRegionViewTracker(pName, pView),
	m_bFireOnStart(bOnStart),
	m_dwData(0)
{
	m_bAutoCenter = FALSE;
	m_bAutoHide = FALSE;
}

CRegionViewCommand::~CRegionViewCommand() 
{
	for (DWORD iLoop = 0; iLoop < m_cToggleList.GetSize(); iLoop++)
		delete m_cToggleList[iLoop];
}

void CRegionViewCommand::SetToggle(DWORD dwIndex, const CUIEvent &cEvent)
{
	if (GetToggleCount() <= dwIndex)
		SetToggleCount(dwIndex + 1);
	delete m_cToggleList[dwIndex];
	m_cToggleList[dwIndex] = cEvent.Clone();
}

const CUIEvent &CRegionViewCommand::GetToggle(DWORD dwIndex) const
{
	static CUIEvent cBadResult(UIEVENT_NONE);
	if (GetToggleCount() <= dwIndex)
		return cBadResult;
	else
		return *(m_cToggleList[dwIndex]);
}

void CRegionViewCommand::SetToggleCount(DWORD dwSize)
{
	while (m_cToggleList.GetSize() < dwSize)
		m_cToggleList.Add(new CUIEvent(UIEVENT_NONE));

	while (m_cToggleList.GetSize() > dwSize)
	{
		delete m_cToggleList[m_cToggleList.GetSize() - 1];
		m_cToggleList.Remove(m_cToggleList.GetSize() - 1);
	}

	while (m_cToggleStateList.GetSize() < dwSize)
		m_cToggleStateList.Add(FALSE);
	while (m_cToggleStateList.GetSize() > dwSize)
		m_cToggleStateList.Remove(m_cToggleStateList.GetSize() - 1);
}

void CRegionViewCommand::SetToggleState(DWORD dwIndex, BOOL bValue)
{
	if (GetToggleCount() <= dwIndex)
		SetToggleCount(dwIndex + 1);
	m_cToggleStateList[dwIndex] = bValue;
}

BOOL CRegionViewCommand::GetToggleState(DWORD dwIndex) const
{
	if (GetToggleCount() <= dwIndex)
		return FALSE;
	else
		return m_cToggleStateList[dwIndex];
}

void CRegionViewCommand::WatchEvent(const CUIEvent &cEvent)
{
	for (DWORD dwLoop = 0; dwLoop < m_cToggleList.GetSize(); dwLoop++)
	{
		if (cEvent == *m_cToggleList[dwLoop])
			m_cToggleStateList[dwLoop] = TRUE;
		else if (cEvent.IsInverse(*m_cToggleList[dwLoop]))
			m_cToggleStateList[dwLoop] = FALSE;
	}

	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRegionViewCommand::OnStart()
{
	BOOL bResult = TRUE;
	if (GetFireOnStart()) 
		bResult = OnCommand(); 
	
	return bResult && (GetEndEventList().GetSize() > 0); 
}
	
BOOL CRegionViewCommand::OnEnd() 
{ 
	BOOL bResult = TRUE;
	if (!GetFireOnStart()) 
		bResult = OnCommand(); 
	
	return bResult; 
};

