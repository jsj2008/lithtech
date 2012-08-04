#include "ltawriter.h"

CLTAWriter::CLTAWriter() :
	m_nNodeDepth(0),
	m_bPrintTab(false)
{
}

CLTAWriter::CLTAWriter(const char* pszFilename, bool bCompress, bool bAppend) :
	m_nNodeDepth(0)
{
	Open(pszFilename, bCompress, bAppend);
}


CLTAWriter::~CLTAWriter()
{
	Close();
}


//determines if this object is valid for writing
bool CLTAWriter::IsValid() const
{
	return m_File.IsValid();
}


//open up the specified file for writing
bool CLTAWriter::Open(const char* pszFilename, bool bCompress, bool bAppend)
{
	//close out the existing file
	Close();
	m_nNodeDepth = 0;

	//we don't need an extra tab to start out
	m_bPrintTab			= false;
	m_bPrintNewline		= true;

	//open up the new one
	return m_File.Open(pszFilename, false, bCompress, bAppend);
}


//close the currently open file
bool CLTAWriter::Close()
{
	m_nNodeDepth = 0;
	return m_File.Close();
}


//begin a new node
bool CLTAWriter::BeginNode()
{
	ASSERT(IsValid());

	//print out the new line if needed
	if(m_bPrintNewline)
	{
		m_File.WriteByte((uint8)'\n');

		//print out the tabs
		for(uint32 nCurrTab = 0; nCurrTab < m_nNodeDepth; nCurrTab++)
		{
			m_File.WriteByte((uint8)'\t');
		}
	}
	else
	{
		m_File.WriteByte('\t');
	}

	//update the node depth
	m_nNodeDepth++;

	//print out the opening
	m_File.WriteByte((uint8)'(');

	//clear the tab flag
	m_bPrintTab = false;

	return true;
}


//end a node
bool CLTAWriter::EndNode()
{
	ASSERT(IsValid());

	//close off the node
	m_File.WriteByte((uint8)')');

	//move on to the next line
	m_File.WriteByte((uint8)'\n');


	//update the node depth
	if(m_nNodeDepth > 0)
	{
		m_nNodeDepth--;
	}
	else
	{
		ASSERT(false);
		return false;
	}

	uint32 nNumTabs = (m_nNodeDepth == 0) ? 0 : m_nNodeDepth - 1;

	//print out the tabs
	for(uint32 nCurrTab = 0; nCurrTab < nNumTabs; nCurrTab++)
	{
		m_File.WriteByte((uint8)'\t');
	}

	//we just moved onto a new line, values will need a tab
	m_bPrintTab		= true;
	m_bPrintNewline	= false;

	return true;
}

//write out strings. The second with the passed in length
//is for optimizations so strlen need not be called if it
//can already be determined ahead of time. 
bool CLTAWriter::Write(const char* pszString, bool bAddQuotes)
{
	return Write(pszString, strlen(pszString), bAddQuotes);
}

bool CLTAWriter::Write(const char* pszString, uint32 nLength, bool bAddQuotes)
{
	ASSERT(IsValid());

	//see if we need to print a tab or not
	if(m_bPrintTab)
	{
		m_File.WriteByte('\t');
		m_bPrintTab = false;
	}

	m_bPrintNewline = true;

	//begin the quotes
	if(bAddQuotes)
	{
		if(m_File.WriteByte((uint8)'\"') == false)
		{
			return false;
		}
	}

	//send out the string
	if(m_File.Write((uint8*)pszString, nLength) == false)
	{
		return false;
	}

	//close off the quotes
	if(bAddQuotes)
	{
		if(m_File.WriteByte((uint8)'\"') == false)
		{
			return false;
		}
	}

	//write the last space
	if(m_File.WriteByte((uint8)' ') == false)
	{
		return false;
	}

	return true;
}

bool CLTAWriter::BreakLine()
{
	ASSERT(IsValid());

	//write the carriage return
	if(m_File.WriteByte((uint8)'\n') == false)
	{
		return false;
	}

	uint32 nNumTabs = (m_nNodeDepth == 0) ? 0 : m_nNodeDepth - 1;

	//print out the tabs
	for(uint32 nCurrTab = 0; nCurrTab < nNumTabs; nCurrTab++)
	{
		m_File.WriteByte((uint8)'\t');
	}

	return true;
}

//allows the for formatting of output to be written
bool CLTAWriter::WriteF(bool bAddQuotes, const char* pszString, ...)
{
	ASSERT(IsValid());

	//see if we need to print a tab or not
	if(m_bPrintTab)
	{
		m_File.WriteByte('\t');
		m_bPrintTab = false;
	}

	m_bPrintNewline = true;

	//begin the quotes
	if(bAddQuotes)
	{
		if(m_File.WriteByte((uint8)'\"') == false)
		{
			return false;
		}
	}

	va_list vargs;
	va_start(vargs, pszString);

	//send out the string
	if(m_File.WriteStrFA(pszString, vargs) == false)
	{
		return false;
	}

	//close off the quotes
	if(bAddQuotes)
	{
		if(m_File.WriteByte((uint8)'\"') == false)
		{
			return false;
		}
	}

	//write the last space
	if(m_File.WriteByte((uint8)' ') == false)
	{
		return false;
	}

	return true;
}
