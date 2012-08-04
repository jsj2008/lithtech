#include "bdefs.h"

#include "debuggraphmgr.h"
#include "iltclient.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//the ILTClient game interface
#include "iltclient.h"
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);





// Constants
#define	DG_MINWIDTH		32
#define	DG_MINHEIGHT	16
#define DG_DEFAULTSPACING	2
#define DG_DEFAULTWIDTH		158
#define DG_DEFAULTHEIGHT	78

#define DG_BORDERWIDTH	1
#define DG_BORDERHEIGHT	1
#define DG_BORDERCOLOR	SETRGB(255,0,0)


// Color table builders

DGSample *GetDGColorTable(uint32 low, uint32 high)
{
	static DGSample resultTable[] = {
		DGSample(0.0f, PValue_Set(0, 1, 1, 1)),
		DGSample(1.0f, PValue_Set(0, 255, 255, 255)),
		DGSample(-1.0f, PValue_Set(0, 0, 0, 0))
	};

	resultTable[0].m_Color = low;
	resultTable[1].m_Color = high;

	return resultTable;
}

DGSample *GetDGColorTable(uint32 low, uint32 middle, uint32 high)
{
	static DGSample resultTable[] = {
		DGSample(0.0f, PValue_Set(0, 1, 1, 1)),
		DGSample(0.5f, PValue_Set(0, 127, 127, 127)),
		DGSample(1.0f, PValue_Set(0, 255, 255, 255)),
		DGSample(-1.0f, PValue_Set(0, 0, 0, 0))
	};

	resultTable[0].m_Color = low;
	resultTable[1].m_Color = middle;
	resultTable[2].m_Color = high;

	return resultTable;
}




DebugGraph::DebugGraph()
{
	m_hSurface = LTNULL;
	m_hLabel = LTNULL;
	m_rRect.left = m_rRect.top = m_rRect.right = m_rRect.bottom = 0;
	m_LabelWidth = m_LabelHeight = 0;
}


DebugGraph::~DebugGraph()
{
	Term();
}


LTRESULT DebugGraph::Init(
	LTRect *pRect,
	char *pLabel,
	LTBOOL bAvoidReinitialize)
{
	uint32 width, height;
	LTRect rRect;


	// Don't reinitialize if they don't want to.
	if(bAvoidReinitialize && IsInitted() == LT_OK)
		return LT_OK;

	// Make sure the parameters are ok.
	if(!pRect || pRect->left >= pRect->right || pRect->top >= pRect->bottom)
		return LT_ERROR;

	// Clear stuff.		
	Term();

	if(pLabel)
	{
		InitLabel(pLabel);
	}								 

	// Subtract some off the top to make room for the label.
	m_rRect = *pRect;	
	m_rRect.top += m_LabelHeight;
	if(m_rRect.top >= m_rRect.bottom)
	{
		Term();
		return LT_ERROR;
	}

	width = (uint32)(m_rRect.right - m_rRect.left);
	height = (uint32)(m_rRect.bottom - m_rRect.top);
	
	// Setup the surface.
	m_hSurface = ilt_client->CreateSurface(width, height);
	if(!m_hSurface)
	{
		Term();
		return LT_ERROR;
	}

	// Clear the surfaces.
	ilt_client->FillRect(m_hSurface, LTNULL, SETRGB(0,0,0));

	// Draw borders.
	rRect.left = 0;
	rRect.top = 0;
	rRect.right = GetWidth();
	rRect.bottom = DG_BORDERHEIGHT;
	ilt_client->FillRect(m_hSurface, &rRect, DG_BORDERCOLOR);

	rRect.left = 0;
	rRect.top = GetHeight() - DG_BORDERHEIGHT;
	rRect.right = GetWidth();
	rRect.bottom = GetHeight();
	ilt_client->FillRect(m_hSurface, &rRect, DG_BORDERCOLOR);

	rRect.left = 0;
	rRect.top = 0;
	rRect.right = DG_BORDERWIDTH;
	rRect.bottom = GetHeight();
	ilt_client->FillRect(m_hSurface, &rRect, DG_BORDERCOLOR);
	
	rRect.left = GetWidth() - DG_BORDERWIDTH;
	rRect.top = 0;
	rRect.right = GetWidth();
	rRect.bottom = GetHeight();
	ilt_client->FillRect(m_hSurface, &rRect, DG_BORDERCOLOR);

	return LT_OK;
}


LTRESULT DebugGraph::IsInitted()
{
	return (ilt_client != NULL) ? LT_OK : LT_NOTINITIALIZED;
}


LTRESULT DebugGraph::Term()
{
	if(ilt_client)
	{
		if(m_hSurface)
		{
			ilt_client->DeleteSurface(m_hSurface);
		}
	}

	m_hSurface = LTNULL;

	return LT_OK;
}


LTRESULT DebugGraph::InitLabel(char *pLabel)
{
	HLTFONT hFont;
	HSTRING hString;
	uint32 dwLabelWidth, dwLabelHeight;


	if(!ilt_client)
		return LT_NOTINITIALIZED;

	TermLabel();

	hFont = ilt_client->CreateFont("MS Sans Serif", 6, 6, LTFALSE, LTFALSE, LTFALSE);
	if(!hFont)
		return LT_ERROR;

	hString = ilt_client->CreateString(pLabel);
	if(!hString)
	{
		ilt_client->DeleteFont(hFont);
		return LT_ERROR;
	}

	m_hLabel = ilt_client->CreateSurfaceFromString(
		hFont,
		hString,
		SETRGB(255,255,255),
		SETRGB(0,0,0),
		0,
		0);
	
	ilt_client->FreeString(hString);
	ilt_client->DeleteFont(hFont);

	if(m_hLabel)
	{
		ilt_client->GetSurfaceDims(m_hLabel, &dwLabelWidth, &dwLabelHeight);
		m_LabelWidth = (int)dwLabelWidth;
		m_LabelHeight = (int)dwLabelHeight;
		return LT_OK;
	}
	else
	{
		return LT_ERROR;
	}
}


LTRESULT DebugGraph::TermLabel()
{
	if(ilt_client && m_hLabel)
	{
		ilt_client->DeleteSurface(m_hLabel);
	}

	m_hLabel = LTNULL;
	m_LabelWidth = m_LabelHeight = 0;
	return LT_OK;
}


LTRESULT DebugGraph::AddSample(DGSample &sample, LTBOOL bOverwrite)
{
	LTRESULT dResult;
	LTRect rShift, rFill;
	float fVal;

	if(IsInitted() != LT_OK)
		return LT_NOTINITIALIZED;

	// Initialize the width of the rectangle
	rFill.left = GetWidth() - (DG_BORDERWIDTH + 1);
	rFill.right = rFill.left + 1;

	if (!bOverwrite)
	{
		// Shift the previous contents over.
		rShift.left = DG_BORDERWIDTH + 1;
		rShift.top = DG_BORDERHEIGHT;
		rShift.right = GetWidth();
		rShift.bottom = GetHeight() - DG_BORDERHEIGHT;

		dResult = ilt_client->DrawSurfaceToSurface(
			m_hSurface,
			m_hSurface,
			&rShift,
			DG_BORDERWIDTH,
			DG_BORDERHEIGHT);
		if(dResult != LT_OK)
			return dResult;

		// First clear out the area.
		rFill.top = DG_BORDERHEIGHT;
		rFill.bottom = GetHeight() - DG_BORDERHEIGHT;

		dResult = ilt_client->FillRect(m_hSurface, &rFill, SETRGB(0,0,0));
		if(dResult != LT_OK)
			return dResult;
	}

	// Now draw the slice.
	fVal = sample.m_fValue * (float)GetHeight();
	rFill.top = GetMaxLineHeight() - (int)fVal;
	rFill.bottom = GetHeight() - DG_BORDERHEIGHT;

	dResult = ilt_client->FillRect(m_hSurface, &rFill, sample.m_Color);
	if(dResult != LT_OK)
		return dResult;

	return LT_OK;
}


LTRESULT DebugGraph::Draw()
{
	LTRESULT dResult;
	uint32 width, height;


	if(IsInitted() != LT_OK)
		return LT_NOTINITIALIZED;

	dResult = ilt_client->DrawSurfaceToSurfaceTransparent(
		ilt_client->GetScreenSurface(),
		m_hSurface,
		LTNULL,
		m_rRect.left,
		m_rRect.top,
		SETRGB_T(0,0,0));
	if(dResult != LT_OK)
		return dResult;

	// Draw the label above us.
	if(m_hLabel)
	{
		ilt_client->GetSurfaceDims(m_hLabel, &width, &height);

		dResult = ilt_client->DrawSurfaceToSurfaceTransparent(
			ilt_client->GetScreenSurface(),
			m_hLabel,
			LTNULL,
			m_rRect.left, 
			m_rRect.top - (int)height,
			SETRGB_T(0,0,0));

		if(dResult != LT_OK)
			return dResult;
	}

	return LT_OK;
}

LTRESULT DebugGraph::MoveTo(LTRect *pRect)
{
	if (!pRect)
		return LT_ERROR;

	if ((pRect->top + m_LabelHeight >= pRect->bottom) || (pRect->left >= pRect->right))
		return LT_ERROR;

	m_rRect = *pRect;
	m_rRect.top += m_LabelHeight;

	return LT_OK;
}




void DGSample::CalcColor(DGSample *pColorTable)
{
	// Default to white if no color table is available
	if (!pColorTable || (pColorTable[0].m_fValue < 0.0f))
	{
		m_Color = SETRGB(0xFF, 0xFF, 0xFF);
		return;
	}

	// Use the first color if that's the only one available
	if (pColorTable[1].m_fValue < 0.0f)
	{
		m_Color = pColorTable[0].m_Color;
		return;
	}

	uint32 i, iColor1, iColor2;
	float fPercent;
	LTBOOL bFound;
	LTVector vPrev, vNext, vColor;

	iColor1 = iColor2 = 0;
	fPercent = 0.0f;

	// See what colors we're in between.
	bFound = LTFALSE;
	for(i=0; pColorTable[i+1].m_fValue >= 0.0f; i++)
	{
		if(m_fValue >= pColorTable[i].m_fValue && m_fValue <= pColorTable[i+1].m_fValue)
		{
			iColor1 = i;
			iColor2 = i+1;
			fPercent = (m_fValue - pColorTable[i].m_fValue) / (pColorTable[i+1].m_fValue - pColorTable[i].m_fValue);

			bFound = LTTRUE;
			break;
		}
	}

	// Didn't find one so the value is over the min or max.
	if(!bFound)
	{
		if(m_fValue > pColorTable[0].m_fValue)
			m_Color = pColorTable[i].m_Color;
		else
			m_Color = pColorTable[0].m_Color;
		return;
	}

	// Set the final color.
	vPrev.Init(
		(float)PValue_GetR(pColorTable[iColor1].m_Color),
		(float)PValue_GetG(pColorTable[iColor1].m_Color),
		(float)PValue_GetB(pColorTable[iColor1].m_Color));

	vNext.Init(
		(float)PValue_GetR(pColorTable[iColor2].m_Color),
		(float)PValue_GetG(pColorTable[iColor2].m_Color),
		(float)PValue_GetB(pColorTable[iColor2].m_Color));

	vColor = vPrev + (vNext - vPrev) * fPercent;
	m_Color = SETRGB((uint32)vColor.x, (uint32)vColor.y, (uint32)vColor.z);
}

void DGSample::NormalizeValue(float fMin, float fMax)
{
	// Keep the value in the range
	m_fValue = LTCLAMP(m_fValue, fMin, fMax);
	// Scale the value to be between 0 and 1 (inclusive)
	m_fValue = (m_fValue - fMin) / (fMax - fMin);
}

CDebugGraphMgr::CDebugGraphMgr()
{
	m_nGraphWidth = DG_DEFAULTWIDTH;
	m_nGraphHeight = DG_DEFAULTHEIGHT;
	m_nGraphSpacing = DG_DEFAULTSPACING;
	m_nGraphXCount = 0;
	m_nGraphYCount = 0;
	m_nLeftBorder = 0;
	m_nBottomBorder = 0;
}

CDebugGraphMgr::~CDebugGraphMgr()
{
	Term();
}

LTRESULT CDebugGraphMgr::Init(LTRect *pRect)
{
	if (!ilt_client)
		return LT_ERROR;

	LTRESULT result = MoveTo(pRect);
	
	if (result != LT_OK)
		return result;

	if (IsInitted())
		Term();

	m_ActiveIDs.Init();
	m_ActiveGraphs.Init();

	return LT_OK;
}

LTRESULT CDebugGraphMgr::IsInitted()
{
	return (ilt_client != NULL) ? LT_OK : LT_NOTINITIALIZED;
}


LTRESULT CDebugGraphMgr::Term()
{
	if (!IsInitted())
		return LT_OK;

	// Clear out the graph list
	FlushGraphs();

	m_ActiveIDs.Term();
	m_ActiveGraphs.Term();

	return LT_OK;
}

void CDebugGraphMgr::FlushCache()
{ 
	m_FindCacheID = LTNULL; 
	m_FindCacheGraph = LTNULL; 
	m_FindCacheIndex = 0;
}

void CDebugGraphMgr::CacheGraph(uint32 index)
{ 
	m_FindCacheIndex = index;
	m_FindCacheID = m_ActiveGraphs[index].m_ID; 
	m_FindCacheGraph = m_ActiveGraphs[index].m_pGraph; 
}

DebugGraph *CDebugGraphMgr::FindGraph(DGuid id)
{
	// Try the empty shortcut
	if (!m_ActiveIDs.GetSize())
		return LTNULL;

	// Try the cache shortcut
	if (id == m_FindCacheID)
		return m_FindCacheGraph;

	// Go through the list the hard way...
	DebugGraph *pResult = LTNULL;
	// Start at the last cached index, since finds are probably going to be linear through the list
	//	Note : Starting in the middle will probably only be of any real benefit if the entire graph
	//		grid is full, and even then it's minimal, but I couldn't stop thinking about doing this..
	uint32 i = m_FindCacheIndex;
	do
	{
		i++;
		if (i >= m_ActiveIDs.GetSize())
			i = 0;

		if (m_ActiveIDs[i] == id)
		{
			pResult = m_ActiveGraphs[i].m_pGraph;
			break;
		}
	} while (i != m_FindCacheIndex);

	// Cache the result
	if (pResult)
		CacheGraph(i);

	return pResult;
}

void CDebugGraphMgr::AddGraph(DGuid id, DebugGraph *pGraph)
{
	m_ActiveIDs.Add(id); 
	m_ActiveGraphs.Add(DGTracker(id, pGraph));
}

void CDebugGraphMgr::RemoveGraph(uint32 index)
{
	// Delete the graph from the list
	m_ActiveIDs.Remove(index);
	delete m_ActiveGraphs[index].m_pGraph;
	m_ActiveGraphs.Remove(index);

	// Flush the cache if necessary
	if (index <= m_FindCacheIndex)
		FlushCache();

	// Update the graph positions
	LTRect rect;
	while (index < m_ActiveGraphs.GetSize())
	{
		// Get the new rectangle
		if (!CalcGraphRect(rect, index))
			break;
		// Move the graph
		if (!m_ActiveGraphs[index].m_pGraph->MoveTo(&rect))
		{
			// If it couldn't be moved, remove it too and jump out
			RemoveGraph(index);
			return;
		}
		index++;
	}
}

void CDebugGraphMgr::FlushGraphs()
{
	while (m_ActiveGraphs.GetSize())
		RemoveGraph(m_ActiveGraphs.GetSize() - 1);
	m_ActiveIDs.RemoveAll();
}

LTRESULT CDebugGraphMgr::Draw()
{
	// Shortcut out if nothing's in the list
	if (!m_ActiveGraphs.GetSize())
		return LT_OK;

	LTRESULT dResult = LT_OK;

	// Draw all of the graphs
	uint32 i,j;

	for (i = 0; (i < m_ActiveGraphs.GetSize()) && (dResult == LT_OK); i++)
	{
		DGTracker *pActiveGraph = &m_ActiveGraphs[i];
		DebugGraph *pGraph = pActiveGraph->m_pGraph;
		// Draw each of the samples
		if (pActiveGraph->m_Samples.GetSize())
		{
			for (j = 0; j < pActiveGraph->m_Samples.GetSize(); j++)
				pGraph->AddSample(pActiveGraph->m_Samples[j], j != 0);
			// Clear out the array without killing the cache
			pActiveGraph->m_Samples.NiceSetSize(0);
		}
		dResult = pGraph->Draw();
	}

	return dResult;
}

LTBOOL CDebugGraphMgr::CheckGraph(DGuid id, LTBOOL bActivate)
{
	if ((!FindGraph(id)) && (!bActivate))
		return LTFALSE;

	if (!bActivate)
		RemoveGraph(m_FindCacheIndex);


	return bActivate;
}

LTRESULT	CDebugGraphMgr::UpdateGraph(DGuid id, float fValue, const DGParams &Params)
{
	if (!FindGraph(id))
	{
		LTRect rect;
		
		// Calculate the rectangle for the graph
		if (!CalcGraphRect(rect, m_ActiveGraphs.GetSize()))
			return LT_ERROR;

		// Add the graph to the list if it's not being tracked yet
		m_ActiveIDs.Add(id);
		LT_MEM_TRACK_ALLOC(m_ActiveGraphs.Add(DGTracker(id, new DebugGraph)),LT_MEM_TYPE_MISC);
		CacheGraph(m_ActiveGraphs.GetSize() - 1);

		m_FindCacheGraph->Init(&rect, Params.m_pLabel);
	}

	DGTracker *pTracker = &m_ActiveGraphs[m_FindCacheIndex];

	// Set up the sample value
	DGSample sample(fValue);
	sample.NormalizeValue(Params.m_fMin, Params.m_fMax);
	sample.CalcColor(Params.m_pColorTable);

	// Sort the value into the sample list
	uint32 i;
	for (i = 0; i < pTracker->m_Samples.GetSize(); i++)
	{
		if (pTracker->m_Samples[i].m_fValue < sample.m_fValue)
		{
			pTracker->m_Samples.Insert(i, sample);
			break;
		}
	}
	// Add it to the end if it's past the maximum
	if (i >= pTracker->m_Samples.GetSize())
		pTracker->m_Samples.Add(sample);

	return LT_OK;
}

LTBOOL CDebugGraphMgr::CalcGraphRect(LTRect &rRect, uint32 nIndex)
{
	// Make sure we've got space left
	if (nIndex >= (m_nGraphXCount * m_nGraphYCount))
		return LTFALSE;

	uint32 nGraphX = m_nGraphXCount - ((nIndex / m_nGraphXCount) + 1);
	uint32 nGraphY = nIndex % m_nGraphYCount;

	// Get the rectangle based on the grid coordinate, left border, and grid size and spacing
	rRect.left = nGraphX * (m_nGraphWidth + m_nGraphSpacing) + m_nLeftBorder + m_nGraphSpacing;
	rRect.top = nGraphY * (m_nGraphHeight + m_nGraphSpacing);
	rRect.right = rRect.left + m_nGraphWidth;
	rRect.bottom = rRect.top + m_nGraphHeight;

	return LTTRUE;
}

LTRESULT CDebugGraphMgr::SetGraphSize(uint32 nWidth, uint32 nHeight)
{
	// Verify the new parameters
	if ((nWidth < DG_MINWIDTH) || (nHeight < DG_MINHEIGHT) || 
		((nWidth + m_nGraphSpacing) > (uint32)(m_rRect.right - m_rRect.left)) ||
		((nHeight + m_nGraphSpacing) > (uint32)(m_rRect.bottom - m_rRect.top)))
		return LT_ERROR;

	// Remove the current graphs
	FlushGraphs();

	// Save the new sizes
	m_nGraphWidth = nWidth;
	m_nGraphHeight = nHeight;

	// Update the grid size
	m_nGraphXCount = (m_rRect.right - m_rRect.left) / (m_nGraphWidth + m_nGraphSpacing);
	m_nGraphYCount = (m_rRect.bottom - m_rRect.top) / (m_nGraphHeight + m_nGraphSpacing);

	// Update the borders
	m_nLeftBorder = (m_rRect.right - m_rRect.left) - (m_nGraphXCount * m_nGraphWidth);
	m_nBottomBorder = (m_rRect.bottom - m_rRect.top) - (m_nGraphYCount * m_nGraphHeight);

	return LT_OK;
}

LTRESULT CDebugGraphMgr::SetGraphSpacing(uint32 nSpacing)
{
	LTRESULT result;
	uint32 oldSpacing = m_nGraphSpacing;

	// Change the spacing
	m_nGraphSpacing = nSpacing;
	// Resize the graphs
	result = SetGraphSize(m_nGraphWidth, m_nGraphHeight);
	// If it didn't work, go back to the old spacing
	if (result != LT_OK)
		m_nGraphSpacing = oldSpacing;

	return result;
}

LTRESULT CDebugGraphMgr::MoveTo(LTRect *pRect)
{
	if (!pRect)
		return LT_ERROR;

	LTRESULT result;

	// Change the rectangle
	m_rRect = *pRect;
	// Resize the graphs
	result = SetGraphSize(m_nGraphWidth, m_nGraphHeight);
	// If it didn't work, set the graph counts to 0 to avoid drawing anything
	if (result != LT_OK)
	{
		m_nGraphXCount = 0;
		m_nGraphYCount = 0;
	}

	return result;
}