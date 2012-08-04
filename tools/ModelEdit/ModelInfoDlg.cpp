// ModelInfoDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "modelinfodlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModelInfoDlg dialog


CModelInfoDlg::CModelInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModelInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModelInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CModelInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModelInfoDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModelInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CModelInfoDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModelInfoDlg message handlers


void CModelInfoDlg::DrawToTextThing(char *pStr, ...)
{
	CWnd *pWnd = GetDlgItem(IDC_MODELINFOTEXT);
	va_list marker;
	char msg[256];
	int nLen;

	if(pWnd)
	{
		va_start(marker, pStr);
		vsprintf(msg, pStr, marker);
		va_end(marker);

		strcat(msg, "\r\n");
		nLen = pWnd->SendMessage(EM_GETLIMITTEXT, 0, 0);
		pWnd->SendMessage(EM_SETSEL, nLen, nLen);
		pWnd->SendMessage(EM_REPLACESEL, FALSE, (LPARAM)msg);
	}
}


DWORD CModelInfoDlg::ShowPieceMem(DWORD iLOD)
{
	uint32 i, memPieces, memVerts, memTris;
	PieceLOD *pLOD;


	memPieces = m_pModel->NumPieces() * sizeof(ModelPiece);
	
	memTris = 0;
	memVerts = 0;
	for(i=0; i < m_pModel->NumPieces(); i++)
	{
		pLOD = m_pModel->GetPiece(i)->GetLOD((uint32)iLOD);
		
		memTris += pLOD->m_Tris.GetSize() * sizeof(ModelTri);
		memVerts += pLOD->m_Verts.GetSize() * sizeof(ModelVert);
	}
	
	DrawToTextThing("Pieces: %d", memPieces);
	DrawToTextThing("Verts: %d", memVerts);
	DrawToTextThing("Tris: %d", memTris);
	
	return memPieces+memVerts+memTris;
}

void DrawToTextThingTabs( CModelInfoDlg *dlg, int depth )
{
	static char space_buffer[60];
	CWnd *pWnd = dlg->GetDlgItem(IDC_MODELINFOTEXT);
	if(!pWnd) return ;
	for( int i = 0 , cnt=0; i < depth ; i++,cnt+=2 ){
		space_buffer[cnt] = ' ';
		space_buffer[cnt+1] = ' ';
	}
	space_buffer[cnt] = '\0';
	int nLen = pWnd->SendMessage(EM_GETLIMITTEXT, 0, 0);
	pWnd->SendMessage(EM_SETSEL, nLen, nLen);
	pWnd->SendMessage(EM_REPLACESEL, FALSE, (LPARAM)space_buffer);
}

void recur_print_nodes( CModelInfoDlg *dlg, ModelNode *root, int depth )
{

	DrawToTextThingTabs(dlg,depth);
	dlg->DrawToTextThing("%s", root->GetName());
	for( uint32 i = 0 ;i < root->NumChildren() ; i++ )
	{
		recur_print_nodes(dlg,root->GetChild(i),depth+1);
	}
}

BOOL CModelInfoDlg::OnInitDialog() 
{
	DWORD i, total;
	ModelPiece *pPiece;
	DWORD totalMem, memWeights;


	CDialog::OnInitDialog();

	DrawToTextThing("File Version : %d" , m_pModel->GetFileVersion() );
	DrawToTextThing(" ");
	DrawToTextThing("Number of animations: %d", m_pModel->m_Anims.GetSize());
	DrawToTextThing("------------------------------");
	total = 0;
	for(i=0; i < m_pModel->m_Anims; i++)
	{
		DrawToTextThing("   %s", m_pModel->GetAnim(i)->GetName());
		total += m_pModel->GetAnim(i)->m_KeyFrames.GetSize();
	}
	DrawToTextThing("------------------------------");
	DrawToTextThing("Average keyframes per animation: %d", total / m_pModel->m_Anims.GetSize());

	DrawToTextThing("Number of triangles: %d", m_pModel->CalcNumTris());
	DrawToTextThing("Number of vertices: %d", m_pModel->CalcNumVerts());
	DrawToTextThing("Number of nodes: %d", m_pModel->m_Transforms.GetSize());
	DrawToTextThing("Number of vertex weights: %d", m_pModel->CalcNumVertexWeights());
	DrawToTextThing("Average weights per vertex: %.3f", (float)m_pModel->CalcNumVertexWeights() / m_pModel->CalcNumVerts());

	DrawToTextThing("");
	DrawToTextThing("Nodes ----------------");
	
	DrawToTextThing("Number of Nodes : %d ", m_pModel->m_FlatNodeList.GetSize());
	DrawToTextThing(" ");
	if( m_pModel->m_FlatNodeList )
	{
		recur_print_nodes(this,m_pModel->m_FlatNodeList[0],0);
	}

	
	DrawToTextThing("");
	DrawToTextThing("Pieces ----------------");
	for(i=0; i < m_pModel->NumPieces(); i++)
	{
		pPiece = m_pModel->GetPiece(i);

		// T.F ModelPiece ;
		for( uint32 per_lod = 0 ; per_lod < pPiece->NumLODs() ; per_lod++ )
		{
			DrawToTextThing("%s, %d textures, texture[0] %d", pPiece->GetName(), 
					pPiece->GetLOD(per_lod)->m_nNumTextures, pPiece->GetLOD(per_lod)->m_iTextures[0]);
		}
	}


	DrawToTextThing("");
	DrawToTextThing("Geometry Memory ----------------");

	totalMem = 0 ;
	for( i = 0 ; i < m_pModel->NumPieces() ; i++ )
	{
		uint32 memPiece, memVerts, memTris;
		memPiece= memVerts= memTris=0;

		DrawToTextThing("piece %s", m_pModel->GetPiece(i)->GetName() );
		ModelPiece *pPiece = m_pModel->GetPiece(i);
		memPiece += sizeof(ModelPiece);
		for( uint32 j = 0 ; j < pPiece->NumLODs() ; j++ )
		{
			
			DrawToTextThing("-------- LOD %d dist :%f -------",j, pPiece->m_LODDists[j]);		
			DrawToTextThing(" n verts : %d  n tris %d ", pPiece->GetLOD(j)->m_Verts.GetSize(),pPiece->GetLOD(j)->m_Tris.GetSize());
			memPiece 	+= sizeof(PieceLOD);
			memVerts += pPiece->GetLOD(j)->m_Verts;
			memTris  += pPiece->GetLOD(j)->m_Tris;
		}
		DrawToTextThing("Pieces: %d", memPiece);
		DrawToTextThing("Verts: %d", memVerts);
		DrawToTextThing("Tris: %d", memTris);
		totalMem += memPiece + memVerts + memTris ;
	}

	DrawToTextThing("");
	memWeights = m_pModel->m_VertexWeights.GetSize() * sizeof(NewVertexWeight);
	DrawToTextThing("Weights: %d", memWeights);
	DrawToTextThing("TOTAL: %d", totalMem);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


