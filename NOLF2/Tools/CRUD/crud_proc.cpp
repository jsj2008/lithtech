// Main CRUD processing

#include <locale.h>
#include <vector.h>

#include <filectrl.hpp>

#include "crud_proc.h"
#include "crud_util.h"

#include "FMain.h"

using namespace std;

bool AddFileToList(TFileList &fileList, const char *pName)
{
	for (TFileList::iterator curFile = fileList.begin(); curFile != fileList.end(); ++curFile)
    {
    	if (stricmp(curFile->begin(), pName) == 0)
        	return false;
	}

	// Ok, add it to the end..
	fileList.push_back(pName);

	return true;
}

// Compare against a list of known extensions
int FoundExtension(char *cur)
{
	static char *extList[] = { "DTX", "WAV", "SPR", "LTB", "PCX", "SGT", "TXT", "PS", "LIP", "TFS", "TFG", NULL };

	for(int i = 0; extList[i]; i++) {
        if(toupper(cur[0]) == extList[i][0] && toupper(cur[1]) == extList[i][1] && toupper(cur[2]) == extList[i][2]) {
            return(1);
		}
    }
    return(0);
}

// Look in a file buffer for file names to add to a list
void SearchInFileBuffer(TFileList &fileList, vector<char> &fileBuf)
{
    char *cur = fileBuf.begin();
    char *start;

    while(cur < fileBuf.end()) {
        // Look for '.' and valid extensions
		if(*cur == '.' && FoundExtension(cur + 1)) {

            // Scan backward from '.' until space or unprintable char
            start = cur;
            while(isgraph(*start) && (*start != '"') && (*start != '\'')) {
                start--;
            }
			start++;
            // Terminate string after extension
            cur += 4;
            *cur = '\0';

			// Add string to list
			if (AddFileToList(fileList, CleanPath(start)))
			{
				string sResult("  Found file ");
				sResult += start;
				OutputResult(sResult.begin());
			}
		}
		cur++;
	}
}

// Look for new file names in the provided file, and add them to the end of the list.
bool SearchForNewFiles(TFileList &fileList, const char *pFileName)
{
	// Don't search in .wav's and .pcx's for other files
	AnsiString fileExt = ExtractFileExt(AnsiString(pFileName));
	if ((fileExt.AnsiCompareIC(".WAV") == 0) ||
		(fileExt.AnsiCompareIC(".PCX") == 0))
		return true;

	string sResult;

	sResult = "Searching in file ";
	sResult += pFileName;
	OutputResult(sResult.begin());

	FILE *fin = fopen(pFileName, "rb");
	if(!fin) {
        string sError = "** Unable to open file ";
        sError += pFileName;
		OutputResult(sError.begin(), true);
		return false;
	}

	// Get file size
	fseek(fin, 0, SEEK_END);
	int fileLen = ftell(fin);
	fseek(fin, 0, SEEK_SET);

	// Read in entire file
    vector<char> fileBuf(fileLen + 20);
    if(!fileBuf.capacity()) {
        fclose(fin);
        OutputResult("** Error allocating memory", true);
        return false;
    }
    fileBuf.resize(fileLen);
    fread(fileBuf.begin(), 1, fileLen, fin);
    fclose(fin);

    SetCurrentFile(pFileName);

    // Process it
    SearchInFileBuffer(fileList, fileBuf);

    return true;
}

bool ShouldCopyFile(const char *sourceFile, const char *destFile)
{
	// First, make sure both files exist..
	if (!FileExists(sourceFile))
    	return false;

	if (!FileExists(destFile))
    	return true;

	// Check the timestamps
	int inHandle = FileOpen(sourceFile, fmOpenRead);
	int outHandle = FileOpen(destFile, fmOpenRead);

	int inDate = FileGetDate(inHandle);
	int outDate = FileGetDate(outHandle);

	FileClose(inHandle);
	FileClose(outHandle);

	return inDate != outDate;
}

// Update the timestamp of destFile to match sourceFile
void UpdateTimestamp(const char *sourceFile, const char *destFile)
{
	int inHandle = FileOpen(sourceFile, fmOpenRead);
	int outHandle = FileOpen(destFile, fmOpenWrite);

	int inDate = FileGetDate(inHandle);
	FileSetDate(outHandle, inDate);

	FileClose(inHandle);
	FileClose(outHandle);
}

// Copy a file if it needs to be updated
bool MaybeCopyFile(const char *sourceFile, const char *destFile)
{
	string sMessage;
	if (!FileExists(sourceFile))
	{
		sMessage = "** File ";
		sMessage += sourceFile;
		sMessage += " not found";
		OutputResult(sMessage.begin(), true);
		return false;
	}

	if (!ShouldCopyFile(sourceFile, destFile))
	{
		sMessage = "Skipping file ";
		sMessage += sourceFile;
		OutputResult(sMessage.begin());
		return false;
	}

	if (!ForceDirectories(ExtractFilePath(AnsiString(destFile))))
	{
		sMessage = "** Error creating directory for ";
		sMessage += destFile;
		OutputResult(sMessage.begin(), true);
		return false;
	}

	sMessage = "Copying ";
	sMessage += sourceFile;
	sMessage += " to ";
	sMessage += destFile;
	OutputResult(sMessage.begin());

	if (CopyFile(sourceFile, destFile, FALSE) == 0)
	{
        string sError = "** Error copying ";
        sError += sourceFile;
        sError += " to ";
        sError += destFile;
		OutputResult(sError.begin());
		return false;
	}

	UpdateTimestamp(sourceFile, destFile);

	return true;
}

CCRUDProcessor::CCRUDProcessor()
{
	m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CCRUDProcessor::~CCRUDProcessor()
{
	CloseHandle(m_hStopEvent);
}

void CCRUDProcessor::ProcessFiles(TFileList &fileList, const char *pSourceDir, const char *pDestDir, bool bRecursive, CCRUDResults &results)
{
	OutputResult("Processing File List..");

	TFileList::iterator curFile = fileList.begin();

	TDateTime timeBegin;
	timeBegin = timeBegin.CurrentDateTime();

	results.m_iFilesFound = 0;
	results.m_iFilesCopied = 0;

    int nTopLevel = fileList.size();

	ResetEvent(m_hStopEvent);
	while ((WaitForSingleObject(m_hStopEvent, 0) == WAIT_TIMEOUT) &&
			(curFile != fileList.end()))
	{
		// Count this file
		++results.m_iFilesFound;

		// Get the source file name
		string curFileName(pSourceDir);
		curFileName += '\\';
		curFileName += *curFile;
		CleanPath(curFileName.begin());

		// Look for new files in this file
        bool bSearchInFile = bRecursive || nTopLevel;
		if (!bSearchInFile || SearchForNewFiles(fileList, curFileName.begin()))
		{
			// Get the destination file name
			string destFileName(pDestDir);
			destFileName += '\\';
			destFileName += *curFile;
			CleanPath(destFileName.begin());

			// Copy it if it needs to be updated
			if (MaybeCopyFile(curFileName.begin(), destFileName.begin()))
				++results.m_iFilesCopied;
		}

        if (nTopLevel)
            --nTopLevel;

		// Go to the next file in the list
		++curFile;

		// Breathe
		Application->ProcessMessages();
	}

	results.m_ProcessingTime = timeBegin.CurrentDateTime() - timeBegin;
}

void CCRUDProcessor::Stop()
{
	SetEvent(m_hStopEvent);
}
