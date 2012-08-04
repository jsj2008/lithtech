#include "bdefs.h"

#include "columnctrl.h"	// defines column control class (derived from clistctrl)

// Default Constructor
CColumnCtrl::CColumnCtrl()
{
	::CListCtrl();

	nColumns=0;
	nRows=0;
}

BEGIN_MESSAGE_MAP(CColumnCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CColumnCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Create a column control (passing parent as a CWnd)
//
// rect         - Specifies the list control’s size and position.
// pParentWnd   - Pointer to the parent window
// hDefaultIcon - default icon to be displayed for each entry 
// csImage      - Size of the icons in the image list
//
// return: TRUE if successful
BOOL CColumnCtrl::Create( const RECT &rect, CWnd *pParentWnd, HICON hDefaultIcon, CSize csImage, HINSTANCE hAppInstance )
{	
	hInstance=hAppInstance;

	// TODO: Find out a good value for nID
	if ( CListCtrl::Create ( LVS_REPORT, rect, pParentWnd, 500 ) )
	{
		// Create the image list
		if ( ImageList.Create (csImage.cx, csImage.cy, TRUE, 0, 1 ) )
		{
			ImageList.Add(hDefaultIcon);
			SetImageList(&ImageList, LVSIL_SMALL);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

// Create a column control (passing parent as a HWND)
//
// rect         - Specifies the list control’s size and position.
// hParentWnd   - Handle for the parent window
// hDefaultIcon - default icon to be displayed for each entry 
// csImage      - Size of the icons in the image list
//
// return: TRUE if successful
BOOL CColumnCtrl::Create( const RECT &rect, HWND hParentWnd, HICON hDefaultIcon, CSize csImage, HINSTANCE hAppInstance )
{	
	// Call the Create(...) method that takes a CWnd
	return Create(rect, CWnd::FromHandle(hParentWnd), hDefaultIcon, csImage );
}

// Add a column to the control
//
// lpszColumnHeading - pointer to the column heading text
//
// return: TRUE if successful
BOOL CColumnCtrl::AddColumn ( LPCTSTR lpszColumnHeading, uint32 dwSortMethod, int nPercentWidth )
{
	// Number of columns must be positive
	ASSERT ( nColumns >= 0 );

	// Must be a know sort method
	if ( dwSortMethod <= SORT_MIN || dwSortMethod >= SORT_MAX )
	{
		ASSERT(FALSE);
		return FALSE;
	}	

	// Width of the column
	int nWidth;

	// If nPercentWidth < 0 then find a suitable width in pixels
	if ( nPercentWidth < 0 )
	{
		CDC dc;

		dc.CreateCompatibleDC(NULL);

		CSize csWidth=dc.GetTextExtent(lpszColumnHeading, strlen(lpszColumnHeading));
		nWidth=csWidth.cx + 15; // Adding some padding to both sides
	}
	else
	{
		CRect rClient;
		GetClientRect(rClient);

		nWidth=(nPercentWidth*rClient.Width())/100;	// The width is a percent of the total client area
	}

	if ( InsertColumn ( nColumns, lpszColumnHeading, LVCFMT_LEFT, nWidth ) == -1 )
	{
		// InsertColumn failed
		return FALSE;
	}

	dwSortArray.Add(dwSortMethod);
	nColumns++;
	return TRUE;
}

// Add a row to the column control
//
// sRowText - Specifies the column text separated by \t (ex: "John\t500 lives")
// hIcon	- Handle to an icon for the row (optional)
BOOL CColumnCtrl::AddRow ( CString sRowText, HICON hIcon )
{
	int nRowIndex=nRows;

	int nTextLength=sRowText.GetLength();
	if ( nTextLength <= 0 && nColumns == 0 ) return TRUE;

	ASSERT ( nRows >= 0 );

	// Add the row
	nRowIndex = InsertItem ( nRows, "" );
	if (  nRowIndex == -1 )
	{
		return FALSE;
	}
	else
	{
		nRows++;
	}

	// Add the text to the column(s)	
	int nLastIndex=0;
	int nCurrentColumn=0;

	int i;
	for ( i = 0; i < nTextLength; i++ )
	{
		if ( nCurrentColumn >= nColumns )
		{
			// More columns were specified than are in the control
			break;
		}

		if ( sRowText[i] == '\t' )		// Found a column break
		{
			if ( !SetItemText ( nRowIndex, nCurrentColumn, sRowText.Mid(nLastIndex, i-nLastIndex) ) )			
			{
				return FALSE;
			}
			nLastIndex=i+1;
			nCurrentColumn++;
		}		
	}

	// Add the last column
	if ( nCurrentColumn < nColumns )
	{
		if ( !SetItemText ( nRowIndex, nCurrentColumn, sRowText.Right(nTextLength-nLastIndex) ) )
		{
			return FALSE;
		}
	}

	// Add the icon (if any) to the image list
	if (ImageList.m_hImageList != NULL )
	{
		if ( hIcon != NULL )
		{
			int nImageIndex=ImageList.Add(hIcon);
			if ( nImageIndex != -1 )
			{				
				SetItem ( nRowIndex, 0, LVIF_IMAGE, NULL, nImageIndex, NULL, NULL, NULL );				
				dwImageArray.Add(nImageIndex);
			}
		}
		else
		{
			SetItem ( nRowIndex, 0, LVIF_IMAGE, NULL, 0 /* Default Icon */, NULL, NULL, NULL );
			dwImageArray.Add(0);
		}
	}

	return TRUE;
}

// Deletes a row from the column
//
// nIndex - zero based index of the row to delete
BOOL CColumnCtrl::DeleteRow ( int nIndex )
{
	if (DeleteItem(nIndex))
	{
		nRows--;
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

BOOL CColumnCtrl::DeleteAllRows()
{
	nRows = 0;
	dwImageArray.RemoveAll();
	return(DeleteAllItems());
}

// Sort the control based on a column and method. Bubble sort method is used
//
// nColumnIndex - Column to sort off of
// dwSortMethod - Use either SORT_ALPHA or SORT_NUMERIC
void CColumnCtrl::Sort ( int nColumnIndex, DWORD dwSortMethod )
{
	if ( nColumnIndex >= nColumns || nRows <= 1 || dwSortMethod == SORT_NONE )
	{
		return;
	}

	ASSERT ( nColumnIndex >= 0 );

	// Must be a know sort method
	if ( dwSortMethod <= SORT_MIN || dwSortMethod >= SORT_MAX )
	{
		ASSERT(FALSE);
		return;
	}	


	CString sItemText1;	// Used to compare two items
	CString sItemText2;

	int nItemNumber1;	// Used to compare two numbers
	int nItemNumber2;

	BOOL bSorted=FALSE;	// The list is not sorted

	int i;
	while ( !bSorted )
	{
		bSorted=TRUE;
		for ( i = 0; i < nRows-1; i++ )
		{
			sItemText1 = GetItemText(i, nColumnIndex);
			sItemText2 = GetItemText(i+1, nColumnIndex);

			switch ( dwSortMethod )
			{	
			case SORT_ALPHA_ASCENDING:
				{
					if ( _stricmp ( sItemText1, sItemText2 ) > 0 )
					{
						SwapRows ( i, i+1 );
						bSorted=FALSE;
					}
					break;
				}
			case SORT_ALPHA_DESCENDING:
			{
				if ( _stricmp ( sItemText1, sItemText2 ) < 0 )
				{
					SwapRows ( i, i+1 );
					bSorted=FALSE;
				}
				break;
			}
			case SORT_NUMERIC_ASCENDING:
				{
					nItemNumber1=atoi(sItemText1);
					nItemNumber2=atoi(sItemText2);

					if ( nItemNumber1 > nItemNumber2 )
					{
						SwapRows ( i, i+1 );
						bSorted=FALSE;
					}
					break;
				}
			case SORT_NUMERIC_DESCENDING:
				{
					nItemNumber1=atoi(sItemText1);
					nItemNumber2=atoi(sItemText2);

					if ( nItemNumber1 < nItemNumber2 )
					{
						SwapRows ( i, i+1 );
						bSorted=FALSE;
					}
					break;
				}
			default:
				{
					// The switch statement should have been handled above
					ASSERT(FALSE);
					break;
				}
			}			
		}
	}
}

// Swap the items of two rows
//
// nRow1 and nRow2 are the indexes of the two rows to swap
void CColumnCtrl::SwapRows ( int nRow1, int nRow2 )
{
	// Check the ranges of the rows
	ASSERT ( nRow1 >= 0 && nRow1 < nRows && nRow2 >= 0 && nRow2 < nRows );

	// Do nothing if they are the same row
	if ( nRow1 == nRow2 )
	{
		return;
	}

	// Swap the strings
	CString sSwapString;
	int i;
	for ( i = 0; i < nColumns; i++ )
	{		
		sSwapString=GetItemText(nRow1, i);
		SetItemText(nRow1, i, GetItemText(nRow2, i));
		SetItemText(nRow2, i, sSwapString);
	}

	// Swap the icons
	if (ImageList.m_hImageList != NULL )
	{
		int nTempIndex=dwImageArray[nRow1];
		dwImageArray[nRow1]=dwImageArray[nRow2];
		dwImageArray[nRow2]=nTempIndex;

		SetItem ( nRow1, 0, LVIF_IMAGE, NULL, dwImageArray[nRow1], NULL, NULL, NULL );
		SetItem ( nRow2, 0, LVIF_IMAGE, NULL, dwImageArray[nRow2], NULL, NULL, NULL );
	}
}

// The user clicked on a column heading
void CColumnCtrl::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	int nIndex=pNMListView->iSubItem;
	Sort ( nIndex, dwSortArray.GetAt(nIndex) /* type of sort */ );
	*pResult = 0;
}
