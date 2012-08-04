#ifndef __LTAICON_H__
#define __LTAICON_H__

#include "stdafx.h"

#ifndef __LTWINTREEICON_H__
#	include "ltwintreeicon.h"
#endif

class CLTAIcon
{
public:
	//the name of the keyword that this icon is associted with
	CString			m_sKeyword;

	//the icon associate with the item
	CLTWinTreeIcon	m_Icon;
	
	//the default icon for each of its children unless specifed
	CLTWinTreeIcon	m_ChildDefIcon;
};
	

#endif

