#include "bdefs.h"
#include "lightattenuation.h"
#include "d_filemgr.h"
#include "editprojectmgr.h"
#include "edithelpers.h"
#include "ltamgr.h"
#include "stringdlg.h"
#include "regiondoc.h"
#include "propertyhelpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//---------------------------------
//--- CLightAttenuationFunction ---
//---------------------------------

// initialize the attenuation function
void CAttenuationFunction::Initialize( float lightRadius, const LTVector& coefs, const LTVector& exps )
{
	// a, b, and c must be [0,+inf)
	m_A = (coefs.x < 0.0f) ? 0.0f : coefs.x;
	m_B = (coefs.y < 0.0f) ? 0.0f : coefs.y;
	m_C = (coefs.z < 0.0f) ? 0.0f : coefs.z;

	// at least one of a, b, or c must be non-zero
	if( m_A + m_B + m_C == 0.0f )
		m_A = 1.0f;

	m_A *= pow( lightRadius, exps.x );
	m_B *= pow( lightRadius, exps.y );
	m_C *= pow( lightRadius, exps.z );
}

// function value at dist
float CAttenuationFunction::Evaluate( float dist )
{
	float inv = m_A + m_B*dist + m_C*dist*dist;
	if( inv < 0.00001f )
		inv = 0.00001f;
	return 1.0f / inv;
}



//----------------------------
//--- CLightAttenuationDlg ---
//----------------------------

BEGIN_MESSAGE_MAP( CLightAttenuationDlg, CDialog )
	ON_WM_VSCROLL()
	ON_WM_PAINT()
	ON_EN_CHANGE( IDC_A_EXP, OnExpAChange )
	ON_EN_CHANGE( IDC_B_EXP, OnExpBChange )
	ON_EN_CHANGE( IDC_C_EXP, OnExpCChange )
	ON_EN_CHANGE( IDC_A_COEF, OnCoefAChange )
	ON_EN_CHANGE( IDC_B_COEF, OnCoefBChange )
	ON_EN_CHANGE( IDC_C_COEF, OnCoefCChange )
	ON_CBN_SELCHANGE( IDC_PRESETS, OnPresetSelect )
	ON_BN_CLICKED( IDC_SHOW_COLORS, OnShowColors )
	ON_BN_CLICKED( IDC_SAVE_PRESET, OnSavePreset )
END_MESSAGE_MAP()

CLightAttenuationDlg::CLightAttenuationDlg( CPropList* m_pPropList, CWnd* pParent )
	: CDialog( IDD_LIGHTATTENUATIONEDIT, pParent ), m_pPropList(m_pPropList)
{
	m_ShowColors = false;

	// defaults
	m_LightRadius = 1.0f;
	m_InnerColor = LTVector( 1.0f, 1.0f, 1.0f );
	m_OuterColor = LTVector( 0.0f, 0.0f, 0.0f );
	m_Coefs = LTVector( 1.0f, 0.0f, 19.0f );
	m_Exps = LTVector( 0.0f, 0.0f, -2.0f );
	m_Type[0] = 0;

	CBaseProp* baseProp;
	ASSERT( m_pPropList );
	if( m_pPropList )
	{
		// load the radius of the light
		if( (baseProp = m_pPropList->GetProp( "LightRadius", true )) && baseProp->GetType() == LT_PT_REAL )
			m_LightRadius = ((CRealProp*)baseProp)->m_Value;

		// load the inner color of the light
		if( (baseProp = m_pPropList->GetProp( "LightColor", false )) && baseProp->GetType() == LT_PT_COLOR )
			m_InnerColor = ((CColorProp*)baseProp)->m_Vector;
		else if( (baseProp = m_pPropList->GetProp( "InnerColor", true )) && baseProp->GetType() == LT_PT_COLOR )
			m_InnerColor = ((CColorProp*)baseProp)->m_Vector;

		// load the attenuation coefficients
		if( (baseProp = m_pPropList->GetProp( "AttCoefs", true )) && baseProp->GetType() == LT_PT_VECTOR )
			m_Coefs = ((CVectorProp*)baseProp)->m_Vector;

		// load the attenuation exponents
		if( (baseProp = m_pPropList->GetProp( "AttExps", true )) && baseProp->GetType() == LT_PT_VECTOR )
			m_Exps = ((CVectorProp*)baseProp)->m_Vector;

		// load the attenuation type
		if( (baseProp = m_pPropList->GetProp( "AttType", true )) && baseProp->GetType() == LT_PT_STRING )
			strcpy( m_Type, ((CStringProp*)baseProp)->m_String );
	}
}

BOOL CLightAttenuationDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	// get the width of the graph windows
	CWnd* wnd = GetDlgItem( IDC_GRAPH );
	CRect rect;
	wnd->GetClientRect( &rect );
	m_GraphWidth = rect.Width() - 2;

	SetupControls();

	DrawAttenuation();

	return TRUE;
}

void CLightAttenuationDlg::OnOK( void )
{
	// setup undo
	CRegionDoc* doc = GetActiveRegionDoc();
	if( doc && m_pPropList )
	{
		doc->SetupUndoForSelections( FALSE );
	}

	CBaseProp* baseProp;
	ASSERT( m_pPropList );
	ASSERT( doc );
	if( m_pPropList )
	{
		// save the attenuation coefficients
		if( (baseProp = m_pPropList->GetProp( "AttCoefs", true )) && baseProp->GetType() == LT_PT_VECTOR )
		{
			((CVectorProp*)baseProp)->m_Vector = m_Coefs;
			ReadPropertyIntoSelections( doc, baseProp );
		}

		// save the attenuation exponents
		if( (baseProp = m_pPropList->GetProp( "AttExps", true )) && baseProp->GetType() == LT_PT_VECTOR )
		{
			((CVectorProp*)baseProp)->m_Vector = m_Exps;
			ReadPropertyIntoSelections( doc, baseProp );
		}

		// save the attenuation type
		if( (baseProp = m_pPropList->GetProp( "AttType", true )) && baseProp->GetType() == LT_PT_STRING )
		{
			strcpy( ((CStringProp*)baseProp)->m_String, m_Type );
			ReadPropertyIntoSelections( doc, baseProp );
		}

		doc->RedrawAllViews( );
	}

	CDialog::OnOK();
}

void CLightAttenuationDlg::OnPresetSelect( void )
{
	int item = m_pPresetCombo->GetCurSel();
	if( item >= 0 )
	{
		int preset = m_pPresetCombo->GetItemData( item );
		ASSERT( preset >= 0 && preset < m_Presets.GetSize() );
		m_Coefs = m_Presets[preset].m_Coefs;
		m_Exps = m_Presets[preset].m_Exps;
		strcpy( m_Type, m_Presets[preset].m_Name );

		UpdateFields( true );
		DrawAttenuation();
	}
}

void CLightAttenuationDlg::OnShowColors( void )
{
	m_ShowColors = ((CButton*)GetDlgItem( IDC_SHOW_COLORS ))->GetCheck();
	DrawAttenuation();
}

void CLightAttenuationDlg::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	if( (CWnd*)pScrollBar == m_pExpASpin )
		m_Exps.x = (float)((int)nPos - 10000) / 10.0f;
	else if( (CWnd*)pScrollBar == m_pExpBSpin )
		m_Exps.y = (float)((int)nPos - 10000) / 10.0f;
	else if( (CWnd*)pScrollBar == m_pExpCSpin )
		m_Exps.z = (float)((int)nPos - 10000) / 10.0f;
	else if( (CWnd*)pScrollBar == m_pCoefASpin )
		m_Coefs.x = (float)nPos / 10.0f;
	else if( (CWnd*)pScrollBar == m_pCoefBSpin )
		m_Coefs.y = (float)nPos / 10.0f;
	else if( (CWnd*)pScrollBar == m_pCoefCSpin )
		m_Coefs.z = (float)nPos / 10.0f;
	else
		ASSERT( 0 );

	SelectNone();
	UpdateFields( false );
	DrawAttenuation();
}

void CLightAttenuationDlg::CoefChanged( CEdit* edit, float* val, CSpinButtonCtrl* spinner )
{
	CString str;
	edit->GetWindowText( str );
	float tmp = (float)atof(str);
	if( *val == tmp )
		return;
	*val = tmp;

	if( *val < 0.0f )
		*val = 0.0f;

	spinner->SetPos((int)( *val * 10.0f ));
	SelectNone();
	DrawAttenuation();
}

void CLightAttenuationDlg::ExpChanged( CEdit* edit, float* val, CSpinButtonCtrl* spinner )
{
	CString str;
	edit->GetWindowText( str );
	float tmp = (float)atof( str );
	if( *val == tmp )
		return;
	*val = tmp;

	spinner->SetPos((int)( *val * 10.0f) + 10000 );
	SelectNone();
	DrawAttenuation();
}

void CLightAttenuationDlg::SelectNone( void )
{
	m_pPresetCombo->SetCurSel( -1 );
	m_Type[0] = '\0';
}

void CLightAttenuationDlg::DrawAttenuation( void )
{
	ASSERT( m_GraphWidth > 0 );

	m_Att.Initialize( m_LightRadius, m_Coefs, m_Exps );

	// initialize the list of values to be passed into the draw functions
	float* values = new float[m_GraphWidth];

	// calculate the value at each distance on the graph
	for( int i = 0; i < m_GraphWidth; i++ )
	{
		float dist = ((float)i / (float)(m_GraphWidth - 1)) * m_LightRadius;
		values[i] = m_Att.Evaluate( dist );
	}

	DrawGraph( values );
	DrawGradient( values );

	delete [] values;
}

void CLightAttenuationDlg::DrawGraph( float* values )
{
	CWnd* wnd = GetDlgItem( IDC_GRAPH );

	if( wnd )
	{
		CRect rect;
		wnd->GetClientRect( &rect );
		ASSERT( (rect.Width() - 2) == m_GraphWidth );

		CDC* dc = wnd->GetDC();
		if( dc )
		{
			// pens that we'll be using to draw the graph
			CPen* oldPen;
			CPen whitePen( PS_SOLID, 0, RGB(255,255,255) );
			CPen blackPen( PS_SOLID, 0, RGB(64,64,64) );

			// display the maximum value for this graph (which is strictly decreasing)
			float maxVal = values[0];
			CString maxValString;
			maxValString.Format( "%.3f", maxVal );
			CWnd* maxValWnd = GetDlgItem( IDC_INTENSITY_MAX );
			if( maxValWnd )
				maxValWnd->SetWindowText( maxValString );

			// draw the bottom part of the graph
			CPoint top( rect.left + 1, rect.top + 1 );
			CPoint bottom( rect.left + 1, rect.bottom - 1 );
			oldPen = dc->SelectObject( &whitePen );
			for( int i = 0; i < m_GraphWidth; i++ )
			{
				float curVal = 1.0f - values[i] / maxVal;
				if( curVal > 1.0f ) curVal = 1.0f;
				else if( curVal < 0.0f ) curVal = 0.0f;

				top.y = (int)((rect.bottom - 2) * curVal) + 1;
				dc->MoveTo( top );
				dc->LineTo( bottom );

				top.x++;
				bottom.x++;
			}

			// draw the top part of the graph
			top = CPoint( rect.left + 1, rect.top + 1 );
			bottom = CPoint( rect.left + 1, rect.bottom - 1 );
			dc->SelectObject( &blackPen );
			for( i = 0; i < m_GraphWidth; i++ )
			{
				float curVal = 1.0f - values[i] / maxVal;
				if( curVal > 1.0f ) curVal = 1.0f;
				else if( curVal < 0.0f ) curVal = 0.0f;

				bottom.y = (int)((rect.bottom - 2) * curVal) + 1;
				dc->MoveTo( top );
				dc->LineTo( bottom );

				top.x++;
				bottom.x++;
			}

			// return things to how they were
			dc->SelectObject( oldPen );
			wnd->ReleaseDC( dc );
		}
	}
}

void CLightAttenuationDlg::DrawGradient( float* values )
{
	CWnd* wnd = GetDlgItem( IDC_GRADIENT );

	if( wnd )
	{
		CRect rect;
		wnd->GetClientRect( &rect );
		ASSERT( (rect.Width() - 2) == m_GraphWidth );

		CDC* dc = wnd->GetDC();
		if( dc )
		{
			CPen* oldPen;
			CPen pen( PS_SOLID, 0, RGB(255,255,255) );
			oldPen = dc->SelectObject( &pen );

			CPoint top( rect.left + 1, rect.top + 1 );
			CPoint bottom( rect.left + 1, rect.bottom - 1 );

			for( int i = 0; i < m_GraphWidth; i++ )
			{
				float curVal = values[i];
				if( curVal > 1.0f ) curVal = 1.0f;
				else if( curVal < 0.0f ) curVal = 0.0f;

				// choose the color for this line
				COLORREF curColor;
				if( m_ShowColors )
				{
					LTVector col = (curVal * m_InnerColor) + ((1.0f - curVal) * m_OuterColor);
					curColor = RGB(col.x,col.y,col.z);
				}
				else
				{
					curColor = RGB(curVal*255,curVal*255,curVal*255);
				}

				CPen pen( PS_SOLID, 0, curColor );
				dc->SelectObject( &pen );

				dc->MoveTo( top );
				dc->LineTo( bottom );

				dc->SelectObject( oldPen );

				top.x++;
				bottom.x++;
			}

			wnd->ReleaseDC( dc );
		}
	}
}

void CLightAttenuationDlg::OnPaint( void )
{
	CPaintDC dc( this );

	DrawAttenuation();
}

void CLightAttenuationDlg::SetupControls( void )
{
	m_pPresetCombo = (CComboBox*)GetDlgItem( IDC_PRESETS );
	m_pCoefA = (CEdit*)GetDlgItem( IDC_A_COEF );
	m_pCoefB = (CEdit*)GetDlgItem( IDC_B_COEF );
	m_pCoefC = (CEdit*)GetDlgItem( IDC_C_COEF );
	m_pExpA = (CEdit*)GetDlgItem( IDC_A_EXP );
	m_pExpB = (CEdit*)GetDlgItem( IDC_B_EXP );
	m_pExpC = (CEdit*)GetDlgItem( IDC_C_EXP );
	m_pCoefASpin = (CSpinButtonCtrl*)GetDlgItem( IDC_A_COEF_SPIN );
	m_pCoefBSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_B_COEF_SPIN );
	m_pCoefCSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_C_COEF_SPIN );
	m_pExpASpin = (CSpinButtonCtrl*)GetDlgItem( IDC_A_EXP_SPIN );
	m_pExpBSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_B_EXP_SPIN );
	m_pExpCSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_C_EXP_SPIN );

	m_pCoefASpin->SetRange32( 0, 10000 );
	m_pCoefBSpin->SetRange32( 0, 10000 );
	m_pCoefCSpin->SetRange32( 0, 10000 );
	m_pExpASpin->SetRange32( 0, 20000 );
	m_pExpBSpin->SetRange32( 0, 20000 );
	m_pExpCSpin->SetRange32( 0, 20000 );

	UpdatePresets();
	UpdateFields( true );

	// select the preset, if it matches
	m_pPresetCombo->SelectString( -1, m_Type );
	int curSel = m_pPresetCombo->GetCurSel();
	if( curSel >= 0 )
	{
		int preset = m_pPresetCombo->GetItemData( curSel );
		ASSERT( preset >= 0 && preset < m_Presets.GetSize() );
		LTVector coefs = m_Presets[preset].m_Coefs;
		LTVector exps = m_Presets[preset].m_Exps;
		if( !((fabs( coefs.x - m_Coefs.x ) < 0.0001) &&
			 (fabs( coefs.y - m_Coefs.y ) < 0.0001) &&
			 (fabs( coefs.z - m_Coefs.z ) < 0.0001) &&
			 (fabs( exps.x - m_Exps.x ) < 0.0001) &&
			 (fabs( exps.y - m_Exps.y ) < 0.0001) &&
			 (fabs( exps.z - m_Exps.z ) < 0.0001)) )
		{
			SelectNone();
		}
	}
	else
		SelectNone();

	// set the focus to the preset combo box
	m_pPresetCombo->SetFocus();
}

inline void PrettyPrintFloat( CString& str, float f )
{
	str.Format( "%f", f );
	str.TrimRight( '0' );
	if( str[str.GetLength() - 1] == '.' || str[str.GetLength() - 1] == ',' )
		str += '0';
}

void CLightAttenuationDlg::UpdateFields( bool updateSpinners )
{
	CString edit;

	PrettyPrintFloat( edit, m_Coefs.x );
	m_pCoefA->SetWindowText( edit );

	PrettyPrintFloat( edit, m_Coefs.y );
	m_pCoefB->SetWindowText( edit );

	PrettyPrintFloat( edit, m_Coefs.z );
	m_pCoefC->SetWindowText( edit );

	PrettyPrintFloat( edit, m_Exps.x );
	m_pExpA->SetWindowText( edit );

	PrettyPrintFloat( edit, m_Exps.y );
	m_pExpB->SetWindowText( edit );

	PrettyPrintFloat( edit, m_Exps.z );
	m_pExpC->SetWindowText( edit );

	if( updateSpinners )
	{
		m_pCoefASpin->SetPos( (int)(m_Coefs.x * 10.0f) );
		m_pCoefBSpin->SetPos( (int)(m_Coefs.y * 10.0f) );
		m_pCoefCSpin->SetPos( (int)(m_Coefs.z * 10.0f) );
		m_pExpASpin->SetPos( (int)(m_Exps.x * 10.0f) + 10000 );
		m_pExpBSpin->SetPos( (int)(m_Exps.y * 10.0f) + 10000 );
		m_pExpCSpin->SetPos( (int)(m_Exps.z * 10.0f) + 10000 );
	}
}

void CLightAttenuationDlg::UpdatePresets( void )
{
	// load the presets
	LoadPresets();

	// clear the preset list
	m_pPresetCombo->ResetContent();

	// load the preset list
	for( uint32 i = 0; i < m_Presets; i++ )
	{
		int item = m_pPresetCombo->AddString( m_Presets[i].m_Name );
		ASSERT( item >= 0 );
		m_pPresetCombo->SetItemData( item, i );
	}
}

void CLightAttenuationDlg::OnSavePreset( void )
{
	CStringDlg dlg;
	dlg.m_bAllowFile = TRUE;
	dlg.m_MaxStringLen = 70;
	dlg.m_bBeeping = TRUE;

	// get the name of the preset to be created
	if( dlg.DoModal( IDS_NEWLIGHTPRESET, IDS_ENTERLIGHTPRESETNAME ) == IDOK )
	{
		bool allow = true;

		// check to see if this preset already exists
		for( uint32 i = 0; i < m_Presets; i++ )
		{
			if( !(m_Presets[i].m_Name.CompareNoCase( dlg.m_EnteredText )) )
			{
				// found a match, warn the user
				CString str;
				str.FormatMessage( IDS_OVERWRITE_LIGHT_PRESET, dlg.m_EnteredText );
				if( IDCANCEL == MessageBox( str, AfxGetAppName(), MB_OKCANCEL ) )
					allow = false;
				break;
			}
		}

		if( allow )
		{
			LightAttenuationPreset preset;
			preset.m_Coefs = m_Coefs;
			preset.m_Exps = m_Exps;
			preset.m_Name = dlg.m_EnteredText;

			if( SavePreset( preset ) )
			{
				m_Presets.Append( preset );
				UpdatePresets();
				m_pPresetCombo->SelectString( -1, preset.m_Name );
				strcpy( m_Type, preset.m_Name );
			}
		}
	}
}

void CLightAttenuationDlg::LoadPresets( void )
{
	CFileIterator iterator;
	char fileName[MAX_PATH];
	char ext[MAX_PATH];

	m_Presets.RemoveAll();

	CString presetDir = GetProject()->m_BaseProjectDir + "\\LightAttenuationPresets\\";
	CString searchSpec = "LightAttenuationPresets\\*.lta";

	while( iterator.Next( searchSpec, TRUE ) )
	{
		if( !(iterator.m_Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			CHelpers::ExtractFileNameAndExtension( iterator.GetFilename(), fileName, ext );
			CString fullPath = presetDir + iterator.GetFilename();

			LightAttenuationPreset preset;
			CLTAReader reader;
			CLTANode* curNode;
			bool success = true;

			if( reader.Open( fullPath, CLTAUtil::IsFileCompressed( fullPath ) ) )
			{
				CLTADefaultAlloc Allocator;

				CLTANode* root = CLTANodeReader::LoadNode( &reader, "lightattenuationpreset", &Allocator );

				if( root )
				{
					if( curNode = CLTAUtil::ShallowFindList( root, "coefs" ) )
					{
						preset.m_Coefs.x = GetFloat( curNode->GetElement( 1 ) );
						preset.m_Coefs.y = GetFloat( curNode->GetElement( 2 ) );
						preset.m_Coefs.z = GetFloat( curNode->GetElement( 3 ) );
					}
					else
						success = false;

					if( curNode = CLTAUtil::ShallowFindList( root, "exps" ) )
					{
						preset.m_Exps.x = GetFloat( curNode->GetElement( 1 ) );
						preset.m_Exps.y = GetFloat( curNode->GetElement( 2 ) );
						preset.m_Exps.z = GetFloat( curNode->GetElement( 3 ) );
					}
					else
						success = false;

					Allocator.FreeNode(root);
				}

				reader.Close();
			}
			else
				success = false;

			// loaded the preset .lta ok, so add the preset
			if( success && strlen( fileName ) )
			{
				preset.m_Name = fileName;
				m_Presets.Append( preset );
			}
		}
	}

	// couldn't find any presets, create the defaults
	if( !m_Presets.GetSize() )
	{
		LightAttenuationPreset preset;

		preset.m_Name = "Default";
		preset.m_Coefs = LTVector( 1.0f, 0.0f, 19.0f );
		preset.m_Exps = LTVector( 0.0f, 0.0f, -2.0f );

		m_Presets.Append( preset );

		SavePreset( preset, false );
	}
}

bool CLightAttenuationDlg::SavePreset( const LightAttenuationPreset& preset, bool warn )
{
	CString presetDir = GetProject()->m_BaseProjectDir + "\\LightAttenuationPresets";
	CString fullPath = presetDir + "\\" + preset.m_Name + ".lta";

	// attempt to create the directory
	if( !CreateDirectory( presetDir, NULL ) && warn )
	{
		if( GetLastError() != ERROR_ALREADY_EXISTS )
		{
			CString str;
			str.FormatMessage( IDS_UNABLETOCREATEDIR, fullPath );
			MessageBox( str, AfxGetAppName(), MB_OK );
			return false;
		}
	}

	// create the file
	CLTAFile file( fullPath, false, CLTAUtil::IsFileCompressed( fullPath ) );

	// save the preset
	if( file.IsValid() )
	{
		file.WriteStr( "( lightattenuationpreset" );
		file.WriteStrF( "\n\t( coefs %f %f %f )", preset.m_Coefs.x, preset.m_Coefs.y, preset.m_Coefs.z );
		file.WriteStrF( "\n\t( exps %f %f %f )", preset.m_Exps.x, preset.m_Exps.y, preset.m_Exps.z );
		file.WriteStr( "\n)" );
		file.Close();
	}
	else if( warn )
	{
		CString str;
		CString fileName = preset.m_Name + ".lta";
		str.FormatMessage( IDS_ERROR_SAVING_FILE, fileName );
		MessageBox( str, AfxGetAppName(), MB_OK );
	}

	return true;
}
