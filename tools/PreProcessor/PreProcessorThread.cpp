// PreProcessorThread.cpp : implementation file
//

#include "bdefs.h"
#include <mmsystem.h>

#include "preprocessorthread.h"
#include "processing.h"

uint32 AskQuestion(const char *pQuestion, uint32 type)
{
	uint32		mbType = MB_YESNO;
	int32		ret;

	if((type & QUES_YES) && (type & QUES_NO) && (type & QUES_CANCEL))
		mbType = MB_YESNOCANCEL;
	else if((type & QUES_YES) && (type & QUES_NO))
		mbType = MB_YESNO;


	ret = ::MessageBox(NULL, pQuestion, "WorldPacker", mbType);
	if(ret == IDYES)
	{
		ret = QUES_YES;
	}
	else if(ret == IDNO)
	{
		ret = QUES_NO;
	}
	else if(ret == IDCANCEL)
	{
		ret = QUES_CANCEL;
	}

	return (uint32)ret;
}



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




