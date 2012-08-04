#ifndef PIECELODGENDLG_H
#define PIECELODGENDLG_H

#pragma once


// dialog used to generate piece lods
class CPieceLODGenDlg : public CDialog
{
public:
	CPieceLODGenDlg( CWnd* parent = NULL );

	// template for this dialog
	enum { IDD = IDD_PIECE_LOD_GEN };

	// dialog data
	float m_Distance;			// distance at which this lod kicks in
	float m_Percent;			// percentage of tris this lod will ideally have
	float m_MaxEdgeLen;			// maximum length of an edge in the lod
	unsigned m_MinNumTris;		// minimum number of tris in the lod

protected:
	virtual void DoDataExchange( CDataExchange* pDX );
	DECLARE_MESSAGE_MAP()
};


#endif // PIECELODGENDLG_H