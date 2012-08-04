
// Defines the DebugGraph module.
// DebugGraphs can be used to display scrolling graphs of useful data.

#ifndef __DEBUGGRAPHMGR_H__
#define __DEBUGGRAPHMGR_H__


#ifndef __LTPVALUE_H__
#include "ltpvalue.h"
#endif

// Used to define color tables and sample values
//      Color tables are normalized values and associated colors, and the list is terminated
//      by a value of -1
class DGSample
{
public:
    DGSample(float fValue = 1.0f, uint32 Color = 0) : m_fValue(fValue), m_Color(Color) {};

    float m_fValue;
    uint32 m_Color;

    // Calculate an interpolated color based on m_fValue & the color table
    void CalcColor(DGSample *pColorTable);
    // Normalize the value to be between 0 & 1
    void NormalizeValue(float fMin, float fMax);
};

// Color tables generators (results are static and overwritten on each call!)
DGSample *GetDGColorTable(uint32 low, uint32 high);
DGSample *GetDGColorTable(uint32 low, uint32 middle, uint32 high);

// Convenience shortcuts to some standard-ish color tables
// Build a Red-Yellow-Green color table
inline DGSample *GetDGColorsRYG() { return GetDGColorTable(PValue_Set(0, 255,0,0), PValue_Set(0, 255,255,0), PValue_Set(0, 0,255,0)); }
// Build a Red-Green-Blue (spectrum) color table
inline DGSample *GetDGColorsRGB() { return GetDGColorTable(PValue_Set(0, 255,0,0), PValue_Set(0, 0,255,0), PValue_Set(0, 0,0,255)); }
// Build a Black-Red color table
inline DGSample *GetDGColorsKR() { return GetDGColorTable(PValue_Set(0, 1,0,0), PValue_Set(0, 255,0,0)); }
// Build a Black-Green color table
inline DGSample *GetDGColorsKG() { return GetDGColorTable(PValue_Set(0, 1,0,0), PValue_Set(0, 0,255,0)); }
// Build a Black-Blue color table
inline DGSample *GetDGColorsKB() { return GetDGColorTable(PValue_Set(0, 1,0,0), PValue_Set(0, 0,0,255)); }
// Build a White-Red color table
inline DGSample *GetDGColorsWR() { return GetDGColorTable(PValue_Set(0, 255,255,255), PValue_Set(0, 255,0,0)); }
// Build a White-Green color table
inline DGSample *GetDGColorsWG() { return GetDGColorTable(PValue_Set(0, 255,255,255), PValue_Set(0, 0,255,0)); }
// Build a White-Blue color table
inline DGSample *GetDGColorsWB() { return GetDGColorTable(PValue_Set(0, 255,255,255), PValue_Set(0, 0,0,255)); }

class DebugGraph
{
public:

                DebugGraph();
                ~DebugGraph();

    // Initialize the graph.  This allocates space for the samples and 
    // tells it where to draw on the screen.
    
    // If you set bAvoidReinitialize to TRUE and it's already initialize, it won't
    // do anything (so you can call this repeatedly and not worry about if it's 
    // initialized yet or not).

    // If you set pLabel, then it calls InitLabel automatically.
    LTRESULT        Init(
        LTRect *pRect,
        char *pLabel=LTNULL,
        LTBOOL bAvoidReinitialize=LTFALSE);

    // LT_OK if yes and LT_NOTINITIALIZED if not.
    LTRESULT        IsInitted();

    LTRESULT        Term();

    // Set the label text.
    LTRESULT        InitLabel(char *pLabel);
    LTRESULT        TermLabel();
    
    // Add a new sample.        
    LTRESULT        AddSample(DGSample &sample, LTBOOL bOverwrite = LTFALSE);

    // Render it.
    LTRESULT        Draw();

    // Move to a new position
    LTRESULT        MoveTo(LTRect *pRect);


private:

    inline int  GetWidth()  {return m_rRect.right - m_rRect.left;}
    inline int  GetHeight() {return m_rRect.bottom - m_rRect.top;}
    inline int  GetMaxLineHeight()  {return m_rRect.bottom - m_rRect.top - 2;}


private:

    // The surface we blit to the screen.
    HSURFACE            m_hSurface;

    // Our text label.
    HSURFACE            m_hLabel;
    int                 m_LabelWidth;
    int                 m_LabelHeight;

    // Where we draw on the screen.
    LTRect              m_rRect;
};

// Parameter wrapper for initializing a graph
struct DGParams
{
    // Default parameter constructor
    DGParams(char *pLabel = LTNULL, float fMin = 0.0f, float fMax = 1.0f,   DGSample *pColorTable = LTNULL) : 
        m_pLabel(pLabel), m_fMin(fMin), m_fMax(fMax), m_pColorTable(pColorTable) {};
    // The text label for the graph
    char *m_pLabel;
    // The extents of the values
    float m_fMin, m_fMax;
    // The color table interpolated between for colors.
    DGSample *m_pColorTable;
};

// The graph identifier type
typedef void *DGuid;

const uint MAX_GRAPH_SIZE = 1024;

// Internal class for tracking graphs
class DGTracker
{
public:
    DGTracker(DGuid ID = 0, DebugGraph *pGraph = LTNULL) : m_ID(ID), m_pGraph(pGraph) { m_Samples.Init(0, MAX_GRAPH_SIZE); };
    DGuid m_ID;
    DebugGraph *m_pGraph;
    CMoArray<DGSample> m_Samples;
};

class CDebugGraphMgr
{
// Constructors, initializers, etc.
public:
                CDebugGraphMgr();
                ~CDebugGraphMgr();

    // Initialize the graph manager
    LTRESULT        Init(LTRect *pRect);

    // LT_OK if yes and LT_NOTINITIALIZED if not.
    LTRESULT        IsInitted();

    LTRESULT        Term();

// Graph list access
public:

    // Finds a graph object in the list
    DebugGraph*     FindGraph(DGuid id);

    // Called to keep the graph's active state correct
    LTBOOL          CheckGraph(DGuid id, LTBOOL bActivate);

    // Called to actually update the graph.  (Should be inside of an if (CheckGraph(...)) block)
    LTRESULT            UpdateGraph(DGuid id, float fValue, const DGParams &Params);

    // Change the size of the graphs.  (Note : This will flush the active graphs)
    LTRESULT            SetGraphSize(uint32 nWidth, uint32 nHeight);
    void            GetGraphSize(uint32 &nWidth, uint32 &nHeight) const { nWidth = m_nGraphWidth; nHeight = m_nGraphHeight; };

    // Controls the spacing between graphs (Note : This calls SetGraphSize)
    LTRESULT            SetGraphSpacing(uint32 nSpacing);
    uint32          GetGraphSpacing() const { return m_nGraphSpacing; };

// Rendering
public:
    LTRESULT        Draw();

    LTRESULT        MoveTo(LTRect *pRect);

private:

    // The list of currently active graphs
    CMoArray<DGuid>         m_ActiveIDs;
    CMoArray<DGTracker>     m_ActiveGraphs;
    // Add a graph to the list
    void    AddGraph(DGuid id, DebugGraph *pGraph);
    // Remove a graph from the list
    void    RemoveGraph(uint32 index);
    // Remove all graphs from the list
    void    FlushGraphs();

    // Graph searching cache to avoid linear searches when possible
    DGuid       m_FindCacheID;
    DebugGraph  *m_FindCacheGraph;
    uint32      m_FindCacheIndex;
    void    FlushCache();
    void    CacheGraph(uint32 index);

    // Where we draw on the screen.
    LTRect  m_rRect;

    // The current graph layout information
    uint32  m_nGraphWidth, m_nGraphHeight;
    uint32  m_nGraphSpacing;
    uint32  m_nGraphXCount, m_nGraphYCount;
    uint32  m_nLeftBorder, m_nBottomBorder;

    // Calculate the next screen rectangle for a new graph
    LTBOOL  CalcGraphRect(LTRect &rRect, uint32 nIndex);
};

#endif





