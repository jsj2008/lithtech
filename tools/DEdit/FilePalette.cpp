
// FilePalette.cpp - Implementation for the CFilePalette class

#include "bdefs.h"
#include "filepalette.h"

void CFilePalette::ctor()
{
	m_csName = "";
	m_csHeader = "DEdit Palette File";
}

BOOL CFilePalette::AddEntry(LPCTSTR pFileName)
{
	// Jump out if the entry is already in the list
	for (DWORD uLoop = 0; uLoop < GetSize(); uLoop++)
		if (GetAt(uLoop).CompareNoCase(pFileName) == 0)
			return FALSE;

	// Add the texture to the list
	Add(pFileName);

	return TRUE;
}

int CFilePalette::RemoveEntry(LPCTSTR pFileName)
{
	int iResult = 0;

	// Find all instances of the entry
	for (DWORD uLoop = 0; uLoop < GetSize(); uLoop++)
		if (GetAt(uLoop).CompareNoCase(pFileName) == 0)
		{
			Remove(uLoop);
			iResult++;
		}
		
	return iResult;
}

BOOL CFilePalette::Load(LPCTSTR pFileName, BOOL bQuiet, BOOL bStrictType)
{
	CString csTemp;

	// Load a new palette
	CStdioFile cInFile;
	if (!cInFile.Open(pFileName, CFile::modeRead | CFile::typeText))
	{
		if (!bQuiet)
		{
			csTemp.Format("Error opening palette file %s.", pFileName);
			MessageBox(0, csTemp, "Error", MB_ICONERROR | MB_OK);
		}
		SetLastError(ERROR_OPEN);
		return FALSE;
	}

	// Check for the correct header
	if (!cInFile.ReadString(csTemp))
		csTemp = "";
	if (csTemp.Compare(GetHeader()) && bStrictType)
	{
		if (!bQuiet)
		{
			csTemp.Format("Invalid palette file %s: Invalid file header.", pFileName);
			MessageBox(0, csTemp, "Error", MB_ICONERROR | MB_OK);
		}
		SetLastError(ERROR_BADHEADER);
		return FALSE;
	}
	else if (!bStrictType)
		SetHeader(csTemp);

	// Read the palette name
	cInFile.ReadString(csTemp);
	SetName(csTemp);
	// Read the palette list
	Term();
	while (cInFile.ReadString(csTemp))
		Append(csTemp);

	SetLastError(ERROR_OK);
	return TRUE;
}

BOOL CFilePalette::Save(LPCTSTR pFileName, BOOL bQuiet)
{
	CString csTemp;

	// Save the palette
	CStdioFile cOutFile;
	if (!cOutFile.Open(pFileName, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
	{
		if (!bQuiet)
		{
			csTemp.Format("Error opening palette file %s.", pFileName);
			MessageBox(0, csTemp, "Error", MB_ICONERROR | MB_OK);
		}
		SetLastError(ERROR_OPEN);
		return FALSE;
	}

	// Write the header
	cOutFile.WriteString(GetHeader());
	cOutFile.WriteString("\n");
	// Write the palette name
	cOutFile.WriteString(GetName());
	cOutFile.WriteString("\n");
	// Write the palette entries
	for (DWORD uLoop = 0; uLoop < GetSize(); uLoop++)
	{
		cOutFile.WriteString(GetAt(uLoop));
		cOutFile.WriteString("\n");
	}

	SetLastError(ERROR_OK);

	return TRUE;
}

