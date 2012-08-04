// PerfCounterManager.cpp
//	Quick note about the setup of this thing - The idea is that each section (renderer, AI, ...)
// can define it's own counter group ('REND', 'AI', etc.). In the console, you can add and remove
// counter groups from the display (e.g. PerfAddCounterGroup REND). 'ALL' is a special group,
// which (as you may have guessed) means all groups. PerfCounterDisp 1/0 to enable the display
// in the first place...

#include "bdefs.h"
#ifndef NO_PROFILE									// To Compile without any profiling code...
//#include "..\lith\dprintf.h"
#include "perfcountermanager.h"
#include "debuggeometry.h"

#ifndef DE_SERVER_COMPILE
#include "sysdrawprim.h"
#endif

#ifndef MAKEFOURCC_PC
#define MAKEFOURCC_PC(ch3, ch2, ch1, ch0)								\
		((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |			\
		((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))
#endif


extern char*  g_CV_EnablePerfCounterGroup;
extern char*  g_CV_DisablePerfCounterGroup;
extern int32  g_CV_PerfCounterDisplay;
extern int32  g_CV_PerfCounterColors;				// Dark/Light Colors...
extern int32  g_CV_PerfCounterPercent;				// Show Percent or actual time view...
extern int32  g_CV_PerfCounterFramesPerSample;		// How often you refresh your display (in frames)
extern int32  g_CV_PerfCounterShowValues;			// Print out the actual values?
extern int32  g_CV_PerfCounterFramesPerMaxReset;	// How often we reset our max values (for our counters)
extern int32  g_CV_PerfCounterMaxValues;			// Display Max Values
extern int32  g_CV_PerfCounterCounterSpacing;		// Vertical spacing between counters...

CDIPerfCounterMan::CDIPerfCounterMan() 
{
	m_FrameTimeCount.QuadPart	= 0;
	m_FrameTimeTmp.QuadPart		= 0;
	m_PerfCntFreq.QuadPart		= 0;
	m_DisplayCountUnder			= 5;
	m_DisplayCountOver			= 3;
//	m_ProfilerCounterID			= AddCounter('DBG',"Profiler");		// Add the Profiler Counter...
//	m_CounterGroupEnabledMap['DBG'] = true;		// Default enable the PROF counters...

	m_FramesTillNextSample		= g_CV_PerfCounterFramesPerSample;
	m_FramesTillNextMaxValClear	= g_CV_PerfCounterFramesPerMaxReset;
	m_MaxDisplayNum				= 1.0f;
}

CDIPerfCounterMan::~CDIPerfCounterMan()
{
}

// Add new counter (returns the ID, 0 is failure)...
int32 CDIPerfCounterMan::AddCounter(uint32 dwCounterGroup, const char* szCounterName)
{
	int32 uCounterID=GetCounterID(dwCounterGroup,szCounterName);	// Does it already exist?
	if (uCounterID >= 0) return uCounterID;

//	COUNTER_LIST::iterator it = m_Counters.begin(); uCounterID = 1;
//	while (it != m_Counters.end()) {				// Find the first unused ID...
//		if (uCounterID == it->iCounterID) { it = m_Counters.begin(); ++uCounterID; continue; }
//		++it; }

	SCOUNTER_INFO CounterInfo(dwCounterGroup,szCounterName);		// Add the new counter...
	CounterInfo.iCounterID = m_Counters.size(); 
	m_Counters.push_back(CounterInfo);

	return CounterInfo.iCounterID;
}

// Forget your counter's ID. Well, today is your lucky day (-1 is failure)...
int32 CDIPerfCounterMan::GetCounterID(uint32 dwCounterGroup, const char* szCounterName)
{
	COUNTER_LIST::iterator it = m_Counters.begin();
	while (it != m_Counters.end()) {				// Iterate through the counter map checking for the name...
		if (strcmp(it->szCounterName,szCounterName)==0 && it->dwCounterGroup == dwCounterGroup) return it->iCounterID;
		++it; } return -1;
}

// Delete the counter...
bool CDIPerfCounterMan::DeleteCounter(uint32 uCounterID)
{
	// Switched to using a vector instead of a map (for speed) so you can't delete the counters anymore...
//	COUNTER_LIST::iterator it = m_Counters.find(uCounterID);
//	if (it != m_Counters.end()) {					// Note: The reason I'm doing a find instead of just a reference is because map will add the element if it doesn't exist (and I don't want that)...
//		m_Counters.erase(it); return true; }
	return false;
}

// Start timing your code...
void CDIPerfCounterMan::StartCounter(uint32 uCounterID)
{
	if (!g_CV_PerfCounterDisplay) return;
	ASSERT(uCounterID < m_Counters.size());
	SCOUNTER_INFO* pCnt = &m_Counters[uCounterID];
	if (!m_CounterGroupEnabledMap[pCnt->dwCounterGroup]) return;	// Not enabled...don't worry about it...
	if (pCnt->TmpStartTime.QuadPart != 0) { return; }
	GetPerfCounter(pCnt->TmpStartTime);
}

// Stop timing your code...
void CDIPerfCounterMan::StopCounter(uint32 uCounterID)
{
	if (!g_CV_PerfCounterDisplay) return;
	ASSERT(uCounterID < m_Counters.size());
	SCOUNTER_INFO* pCnt = &m_Counters[uCounterID];
	if (!m_CounterGroupEnabledMap[pCnt->dwCounterGroup]) return;	// Not enabled...don't worry about it...
	if (!pCnt->TmpStartTime.QuadPart) { return; }

	LARGE_INTEGER uTmpEnd; GetPerfCounter(uTmpEnd); 
#ifdef __D3D
	pCnt->FrameCount.QuadPart += uTmpEnd.QuadPart - pCnt->TmpStartTime.QuadPart;
#endif
#ifdef __XBOX
	pCnt->FrameCount.QuadPart += uTmpEnd.QuadPart - pCnt->TmpStartTime.QuadPart;
#endif
	pCnt->TmpStartTime.QuadPart = 0;
}

// Start of a new frame ...
void CDIPerfCounterMan::SetupFrame()
{
	if (g_CV_EnablePerfCounterGroup && g_CV_EnablePerfCounterGroup[0]) {	// Check for changes in the console vars...
		uint32 dwKey = 0; int i = 0;
		while (g_CV_EnablePerfCounterGroup[i]) { dwKey = (dwKey<<8) + g_CV_EnablePerfCounterGroup[i]; ++i; }
        // Well, that's a special case, now isn't it...
		if (dwKey == MAKEFOURCC_PC(0,'A','L','L') || dwKey == MAKEFOURCC_PC(0,'a','l','l')) {
                for (map<uint32,bool>::iterator it = m_CounterGroupEnabledMap.begin(); it!=m_CounterGroupEnabledMap.end();++it) {
                    it->second = true; } 
                g_CV_EnablePerfCounterGroup[0] = 0; }
		else {
			m_CounterGroupEnabledMap[dwKey] = true; 
			g_CV_EnablePerfCounterGroup[0] = 0; } }
	if (g_CV_DisablePerfCounterGroup && g_CV_DisablePerfCounterGroup[0]) {
		uint32 dwKey = 0; int i = 0;
		while (g_CV_DisablePerfCounterGroup[i]) { dwKey = (dwKey<<8) + g_CV_DisablePerfCounterGroup[i]; ++i; }
		if (dwKey == MAKEFOURCC_PC(0,'A','L','L') || dwKey == MAKEFOURCC_PC(0,'a','l','l')) {		// Well, that's a special case, now isn't it...
			for (map<uint32,bool>::iterator it = m_CounterGroupEnabledMap.begin();it!=m_CounterGroupEnabledMap.end();++it) {
				it->second = false; } 
			g_CV_DisablePerfCounterGroup[0] =0; }
		else {
			m_CounterGroupEnabledMap[dwKey] = false; 
			g_CV_DisablePerfCounterGroup[0] = 0; } }

	if (!m_FramesTillNextSample) { m_FramesTillNextSample = g_CV_PerfCounterFramesPerSample; }
	else { --m_FramesTillNextSample; }				// We're only going to re-draw every X frames...

	if (!m_PerfCntFreq.QuadPart) { GetPerfCounterFrequency(m_PerfCntFreq); }
}

void CDIPerfCounterMan::FrameMarker()
{
	LARGE_INTEGER uTmpEnd; GetPerfCounter(uTmpEnd); 
#ifdef __D3D
	m_FrameTimeCount.QuadPart = uTmpEnd.QuadPart - m_FrameTimeTmp.QuadPart;
#endif
#ifdef __XBOX
	m_FrameTimeCount.QuadPart = uTmpEnd.QuadPart - m_FrameTimeTmp.QuadPart;
#endif
	for (COUNTER_LIST::iterator it = m_Counters.begin();it != m_Counters.end();++it) { // Iterate through the counter map...
		if (it->TmpStartTime.QuadPart) {			// It's currently running - effectively, we're stopping it, and then restart it (not using the functions here, because it's faster if we do some custom code)...
			it->FrameCount.QuadPart += uTmpEnd.QuadPart - it->TmpStartTime.QuadPart;
			it->TmpStartTime = uTmpEnd; } 
		if (it->MaxCount.QuadPart < it->FrameCount.QuadPart) it->MaxCount.QuadPart = it->FrameCount.QuadPart; }				// Update max values...
	GetPerfCounter(m_FrameTimeTmp);
}

void CDIPerfCounterMan::ClearCounters(bool bAllowMaxValueClear)
{
	bool bClearMaxValues = false;
	if (bAllowMaxValueClear) {
		if (!m_FramesTillNextMaxValClear) { 
			m_FramesTillNextMaxValClear = g_CV_PerfCounterFramesPerMaxReset; bClearMaxValues = true; }
		else { --m_FramesTillNextMaxValClear; } }	// We're only clear the max values every X frames...

	for (COUNTER_LIST::iterator it = m_Counters.begin();it != m_Counters.end();++it) { // Iterate through the counter map clearing the er out...
		it->FrameCount.QuadPart = 0; 
		if (bClearMaxValues) it->MaxCount.QuadPart = 0; }
}

// Go ahead and draw the graph of all counters...
#define PERFCOUNTERDISPLAY_BAROFFSET_X			100
#define PERFCOUNTERDISPLAY_TEXTOFFSET_X			20
#define PERFCOUNTERDISPLAY_OFFSET_Y				80
#define PERFCOUNTERDISPLAY_TEXTXOFFSET_Y		3

#define PERFCOUNTERDISPLAY_Z					SCREEN_NEAR_Z		

//#define PERFCOUNTERDISPLAY_OFFSET_DELTA_Y		25
#define PERFCOUNTERDISPLAY_BARHEIGHT			15
#define	PERFCOUNTERDISPLAY_BUCKETMAX_DIFF		0.10f
#define	PERFCOUNTERDISPLAY_BUCKETMAX_DIFF2		0.025f	// For time (ms) view...
extern int32 g_ScreenWidth, g_ScreenHeight;
void CDIPerfCounterMan::DrawCounters()
{		
    #if !defined(DE_SERVER_COMPILE) && !defined(DE_HEADLESS_CLIENT)
	SetupFrame();									// Setup and stuff...
	FrameMarker();									// Clear all the counts to zero...

	if (!g_CV_PerfCounterDisplay) { ClearCounters(); /*StopCounter(m_ProfilerCounterID);*/
		CDebugGeometry& dgPerf = getDebugGeometry(); dgPerf.clear(); return; } 
	if (m_FramesTillNextSample)   { ClearCounters(); /*StopCounter(m_ProfilerCounterID);*/ return; } // Skip out if it's not a redraw frame...

	CDebugGeometry& dg = getDebugGeometry();
	dg.clear();										// Clear out all the old debug geometry...

	float  fMaxBarWidth	= ((g_ScreenWidth - PERFCOUNTERDISPLAY_BAROFFSET_X) - 50);
	float  fBarSclWidth	= ((g_ScreenWidth - PERFCOUNTERDISPLAY_BAROFFSET_X) - 50) / m_MaxDisplayNum;
	uint32 CurYPos		= PERFCOUNTERDISPLAY_OFFSET_Y;
	
#ifdef __D3D
	uint32 lightColors[] = { 0x80F08080, 0x8080F080, 0x808080F0, 0x80F0F000, 0x8000F0F0, 0x80F000F0 }; 
	uint32 darkColors[]  = { 0x80804040, 0x80408040, 0x80404080, 0x80808000, 0x80008080, 0x80800080 }; 
	LTRGBColor LegendColor; LegendColor.dwordVal = 0xFF808080;
	LTRGBColor FPSColor; FPSColor.dwordVal = 0xFF8080FF;
	LTRGBColor PercentColor; PercentColor.dwordVal = 0xFF808080;
#endif	
#ifdef __XBOX
	uint32 lightColors[] = { 0x80F08080, 0x8080F080, 0x808080F0, 0x80F0F000, 0x8000F0F0, 0x80F000F0 }; 
	uint32 darkColors[]  = { 0x80804040, 0x80408040, 0x80404080, 0x80808000, 0x80008080, 0x80800080 }; 
	LTRGBColor LegendColor; LegendColor.dwordVal = 0xFF808080;
	LTRGBColor FPSColor; FPSColor.dwordVal = 0xFF8080FF;
	LTRGBColor PercentColor; PercentColor.dwordVal = 0xFF808080;
#endif	
	
	uint32* pMyColors = g_CV_PerfCounterColors ? darkColors : lightColors; uint32 iColorCount = 6;

	LTRGBColor CurColor; CurColor.dwordVal = pMyColors[0];

	// First check to see if any counters are enabled...
	COUNTER_LIST::iterator it = m_Counters.begin(); int iDispCounters = 0;
	while (it != m_Counters.end()) {
		if (!m_CounterGroupEnabledMap[it->dwCounterGroup]) { ++it; continue;	}	// Not enabled...don't worry about it...
		++iDispCounters; break; }
	
	// Draw the legend on top...
	char szTmp[16]; char szTmp2[8];
	if (iDispCounters) {
		if (g_CV_PerfCounterPercent) strcpy(szTmp2,"%d%%"); else strcpy(szTmp2,"%.1fms");
		dg.addLine(LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X,CurYPos,PERFCOUNTERDISPLAY_Z),LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth,CurYPos,PERFCOUNTERDISPLAY_Z),LegendColor,true);
		if (g_CV_PerfCounterPercent) {
			sprintf(szTmp,szTmp2,(int)(0.05f*m_MaxDisplayNum*100)); dg.addText(szTmp,LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.05f-4,CurYPos+PERFCOUNTERDISPLAY_TEXTXOFFSET_Y-13,0),LegendColor);
			sprintf(szTmp,szTmp2,(int)(0.10f*m_MaxDisplayNum*100)); dg.addText(szTmp,LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.10f-6,CurYPos+PERFCOUNTERDISPLAY_TEXTXOFFSET_Y-13,0),LegendColor);
			sprintf(szTmp,szTmp2,(int)(0.20f*m_MaxDisplayNum*100)); dg.addText(szTmp,LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.20f-6,CurYPos+PERFCOUNTERDISPLAY_TEXTXOFFSET_Y-13,0),LegendColor);
			sprintf(szTmp,szTmp2,(int)(0.40f*m_MaxDisplayNum*100)); dg.addText(szTmp,LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.40f-6,CurYPos+PERFCOUNTERDISPLAY_TEXTXOFFSET_Y-13,0),LegendColor);
			sprintf(szTmp,szTmp2,(int)(1.00f*m_MaxDisplayNum*100)); dg.addText(szTmp,LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*1.00f-6,CurYPos+PERFCOUNTERDISPLAY_TEXTXOFFSET_Y-15,0),LegendColor); }
		else {
			sprintf(szTmp,szTmp2,(0.05f*m_MaxDisplayNum*1000)); dg.addText(szTmp,LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.05f-12,CurYPos+PERFCOUNTERDISPLAY_TEXTXOFFSET_Y-13,0),LegendColor);
			sprintf(szTmp,szTmp2,(0.10f*m_MaxDisplayNum*1000)); dg.addText(szTmp,LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.10f-12,CurYPos+PERFCOUNTERDISPLAY_TEXTXOFFSET_Y-13,0),LegendColor);
			sprintf(szTmp,szTmp2,(0.20f*m_MaxDisplayNum*1000)); dg.addText(szTmp,LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.20f-12,CurYPos+PERFCOUNTERDISPLAY_TEXTXOFFSET_Y-13,0),LegendColor);
			sprintf(szTmp,szTmp2,(0.40f*m_MaxDisplayNum*1000)); dg.addText(szTmp,LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.40f-12,CurYPos+PERFCOUNTERDISPLAY_TEXTXOFFSET_Y-13,0),LegendColor);
			sprintf(szTmp,szTmp2,(1.00f*m_MaxDisplayNum*1000)); dg.addText(szTmp,LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*1.00f-12,CurYPos+PERFCOUNTERDISPLAY_TEXTXOFFSET_Y-15,0),LegendColor); } }
		
	sprintf(szTmp,"%d FPS",(int)((float)m_PerfCntFreq.QuadPart/(float)m_FrameTimeCount.QuadPart));
	
	dg.addText(szTmp,LTVector(PERFCOUNTERDISPLAY_TEXTOFFSET_X,CurYPos+PERFCOUNTERDISPLAY_TEXTXOFFSET_Y-13,PERFCOUNTERDISPLAY_Z),FPSColor);
	CurYPos += 5;

	// Go through the counters, if they're in an active group, show them...
	it = m_Counters.begin(); iDispCounters = 0; float fMaxDispNum = 0.0f; float fPercent;
	while (it != m_Counters.end()) {
		if (!m_CounterGroupEnabledMap[it->dwCounterGroup]) { ++it; continue;	}	// Not enabled...don't worry about it...
		
		LTRGBColor TextColor; TextColor.dwordVal = CurColor.dwordVal|0xFF000000;
		dg.addText(it->szCounterName,LTVector(PERFCOUNTERDISPLAY_TEXTOFFSET_X,CurYPos+PERFCOUNTERDISPLAY_TEXTXOFFSET_Y,PERFCOUNTERDISPLAY_Z),TextColor);

		// Draw the Bar...
		if (g_CV_PerfCounterPercent) fPercent = (((float)it->FrameCount.QuadPart)/(float)m_FrameTimeCount.QuadPart);
		else fPercent = (((float)it->FrameCount.QuadPart)/(float)m_PerfCntFreq.QuadPart);
		if (fPercent > fMaxDispNum) fMaxDispNum = fPercent;
		uint32 DaWidth  = (uint32)(fBarSclWidth * fPercent);
		CDIDebugPolygon dgPoly; dgPoly.SetColor(CurColor);	// Add the Bar...
        LTVector tmp; LTVector vFrom,vTo;
        tmp.Init(PERFCOUNTERDISPLAY_BAROFFSET_X+DaWidth, CurYPos,PERFCOUNTERDISPLAY_Z);								dgPoly.AddVertex(tmp); // UR
        tmp.Init(PERFCOUNTERDISPLAY_BAROFFSET_X+DaWidth,CurYPos+PERFCOUNTERDISPLAY_BARHEIGHT,PERFCOUNTERDISPLAY_Z);	dgPoly.AddVertex(tmp); // LR
        tmp.Init(PERFCOUNTERDISPLAY_BAROFFSET_X,CurYPos+PERFCOUNTERDISPLAY_BARHEIGHT,PERFCOUNTERDISPLAY_Z);			dgPoly.AddVertex(tmp); // LL
        tmp.Init(PERFCOUNTERDISPLAY_BAROFFSET_X,CurYPos,PERFCOUNTERDISPLAY_Z);										dgPoly.AddVertex(tmp); // LR
		dg.addPolygon(&dgPoly,true);

		if (g_CV_PerfCounterShowValues) {					// Print out the actual value, if requested...
			float fMult = 1000; if (g_CV_PerfCounterPercent) fMult = 100;
			char szTmp[32]; sprintf(szTmp,"%.1f",fPercent*fMult); 
			dg.addText(szTmp,LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+3,CurYPos+3,PERFCOUNTERDISPLAY_Z),PercentColor); }

		// Draw the Max value line...
		if (g_CV_PerfCounterMaxValues) {
			if (g_CV_PerfCounterPercent) fPercent = (((float)it->MaxCount.QuadPart)/(float)m_FrameTimeCount.QuadPart);
			else fPercent = (((float)it->MaxCount.QuadPart)/(float)m_PerfCntFreq.QuadPart);
			if (fPercent > fMaxDispNum) fMaxDispNum = fPercent;
			DaWidth  = (uint32)(fBarSclWidth * fPercent);
			vFrom.Init(PERFCOUNTERDISPLAY_BAROFFSET_X+DaWidth,CurYPos-1,PERFCOUNTERDISPLAY_Z);
			vTo.Init(PERFCOUNTERDISPLAY_BAROFFSET_X+DaWidth,CurYPos+PERFCOUNTERDISPLAY_BARHEIGHT+1,PERFCOUNTERDISPLAY_Z);
			dg.addLine(vFrom,vTo,CurColor,true); }
		
		++it; CurYPos += g_CV_PerfCounterCounterSpacing; CurColor.dwordVal = pMyColors[++iDispCounters % iColorCount]; }

	// Draw the Vert lines (for the legend)...
	dg.addLine(LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.00f,PERFCOUNTERDISPLAY_OFFSET_Y,PERFCOUNTERDISPLAY_Z),LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.00f,PERFCOUNTERDISPLAY_OFFSET_Y+iDispCounters*g_CV_PerfCounterCounterSpacing,PERFCOUNTERDISPLAY_Z),LegendColor,true);
	dg.addLine(LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.05f,PERFCOUNTERDISPLAY_OFFSET_Y,PERFCOUNTERDISPLAY_Z),LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.05f,PERFCOUNTERDISPLAY_OFFSET_Y+iDispCounters*g_CV_PerfCounterCounterSpacing,PERFCOUNTERDISPLAY_Z),LegendColor,true);
	dg.addLine(LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.10f,PERFCOUNTERDISPLAY_OFFSET_Y,PERFCOUNTERDISPLAY_Z),LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.10f,PERFCOUNTERDISPLAY_OFFSET_Y+iDispCounters*g_CV_PerfCounterCounterSpacing,PERFCOUNTERDISPLAY_Z),LegendColor,true);
	dg.addLine(LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.20f,PERFCOUNTERDISPLAY_OFFSET_Y,PERFCOUNTERDISPLAY_Z),LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.20f,PERFCOUNTERDISPLAY_OFFSET_Y+iDispCounters*g_CV_PerfCounterCounterSpacing,PERFCOUNTERDISPLAY_Z),LegendColor,true);
	dg.addLine(LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.40f,PERFCOUNTERDISPLAY_OFFSET_Y,PERFCOUNTERDISPLAY_Z),LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*0.40f,PERFCOUNTERDISPLAY_OFFSET_Y+iDispCounters*g_CV_PerfCounterCounterSpacing,PERFCOUNTERDISPLAY_Z),LegendColor,true);
	dg.addLine(LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*1.00f,PERFCOUNTERDISPLAY_OFFSET_Y,PERFCOUNTERDISPLAY_Z),LTVector(PERFCOUNTERDISPLAY_BAROFFSET_X+fMaxBarWidth*1.00f,PERFCOUNTERDISPLAY_OFFSET_Y+iDispCounters*g_CV_PerfCounterCounterSpacing,PERFCOUNTERDISPLAY_Z),LegendColor,true);

	// Want to change m_MaxDisplayNum? Do it here...
	float fDiffAmount = 0.2f; if (!g_CV_PerfCounterPercent) fDiffAmount = 0.03f;
	if (fMaxDispNum > m_MaxDisplayNum) {		// We're displaying bars that are off the scale, inc it up right away...
		--m_DisplayCountOver; 
		if (!m_DisplayCountOver) {
			m_DisplayCountOver  = 3;
			m_DisplayCountUnder = 5;
			if (g_CV_PerfCounterPercent) { 
				m_MaxDisplayNum	= max(min(0.9f,fMaxDispNum),0.1f); 
				m_MaxDisplayNum = ((int)(m_MaxDisplayNum/PERFCOUNTERDISPLAY_BUCKETMAX_DIFF)+1) * PERFCOUNTERDISPLAY_BUCKETMAX_DIFF; }
			else {
				m_MaxDisplayNum	= max(min(0.1f,fMaxDispNum),0.00f); 
				m_MaxDisplayNum = ((int)(m_MaxDisplayNum/PERFCOUNTERDISPLAY_BUCKETMAX_DIFF2)+1) * PERFCOUNTERDISPLAY_BUCKETMAX_DIFF2; } } }
	else {
		m_DisplayCountOver = 3;
		if (fMaxDispNum+fDiffAmount < m_MaxDisplayNum) { 
			--m_DisplayCountUnder; 
			if (!m_DisplayCountUnder) {
				m_DisplayCountOver  = 3;
				m_DisplayCountUnder = 5;
				if (g_CV_PerfCounterPercent) { 
					m_MaxDisplayNum	= max(min(0.9f,fMaxDispNum),0.1f); 
					m_MaxDisplayNum = ((int)(m_MaxDisplayNum/PERFCOUNTERDISPLAY_BUCKETMAX_DIFF)+1) * PERFCOUNTERDISPLAY_BUCKETMAX_DIFF; }
				else {
					m_MaxDisplayNum	= max(min(0.3f,fMaxDispNum),0.00f); 
					m_MaxDisplayNum = ((int)(m_MaxDisplayNum/PERFCOUNTERDISPLAY_BUCKETMAX_DIFF2)+1) * PERFCOUNTERDISPLAY_BUCKETMAX_DIFF2; } } } 
		else { m_DisplayCountUnder = 5; } }

	ClearCounters(true);
    #endif // !DE_SERVER_COMPILE && !DE_HEADLESS_CLIENT
}

#endif // NO_PROFILE
