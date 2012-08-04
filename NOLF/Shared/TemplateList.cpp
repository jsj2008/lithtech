#include "StdAfx.h"
#include "TemplateList.h"

int s_cLTLinkBankRefCount = 0;
CBankedList<LTLink>* s_pLTLinkBank = LTNULL;
