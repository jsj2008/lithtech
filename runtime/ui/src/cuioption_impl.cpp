//-------------------------------------------------------------------
//
//   MODULE    : CUIOPTION_IMPL.CPP
//
//   PURPOSE   : Implements the CUIOption_Impl Utility Class
//
//   CREATED   : 4/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIOPTION_IMPL_H__
#include "cuioption_impl.h"
#endif

#ifndef __CUIPOLYTEX_H__
#include "cuipolytex.h"
#endif

#ifndef __CUIWIDGET_H__
#include "cuiwidget.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif


// interface database
static ILTDrawPrim *pDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);

static ILTTexInterface *pTexInterface;
define_holder(ILTTexInterface, pTexInterface);



//  ---------------------------------------------------------------------------
CUIOption_Impl::CUIOption_Impl(CUIBase* abstract, CUIGUID guid) :
	CUICheck_Impl(abstract, guid)
{

}


//  ---------------------------------------------------------------------------
CUIOption_Impl::~CUIOption_Impl()
{
	// delete resources
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIOption_Impl::SetState(uint32 flags)
{
	CUILinkList *pChildList;
	CUIListNode *pNode = NULL;
	CUIBase		*pWidget;

	// CUIOption is almost identical to CUICheck, except that only one CUIOption
	// in a group of siblings can have the CUIS_PRESSED state turned on.  So,
	// if the user asks us to turn on CUIS_PRESSED, we need to turn it off in
	// all our siblings.

	if (flags & CUIS_PRESSED) {
		
		pChildList = m_pParent->GetImpl()->GetChildList();
		
		if (pChildList)	pNode = pChildList->GetHead();

		while (pNode) {
			pWidget = (CUIWidget*) pNode->m_pData;

			if (pWidget) {
				// unset state if it's another option button
				if (pWidget->GetType() == CUIW_OPTION) {
					pWidget->UnsetState(CUIS_PRESSED);
				}
			}

			pNode = pNode->m_pNext;
		}
	}

	// ...and continue with the state-set
	return CUICheck_Impl::SetState(flags);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIOption_Impl::SetColors(CUI_ELEMENTTYPE elm, 
										 uint32 argb0,
										 uint32 argb1,
										 uint32 argb2,
										 uint32 argb3)
{	
	// CUIOption has the same elements as CUICheck, just named differently
	switch (elm) {
		case CUIE_OPTION_ON:
			elm = CUIE_CHECK_ON;
			break;

		case CUIE_OPTION_OFF:
			elm = CUIE_CHECK_OFF;
			break;

		case CUIE_OPTION_DISABLED:
			elm = CUIE_CHECK_DISABLED;
			break;
	}

	return CUICheck_Impl::SetColors(elm, argb0, argb1, argb2, argb3);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIOption_Impl::SetTexture(CUI_ELEMENTTYPE elm, 
										   HTEXTURE hTex, 
										   CUIRECT* pRect)
{
	// CUIOption has the same elements as CUICheck, just named differently
	switch (elm) {
		case CUIE_OPTION_ON:
			elm = CUIE_CHECK_ON;
			break;

		case CUIE_OPTION_OFF:
			elm = CUIE_CHECK_OFF;
			break;

		case CUIE_OPTION_DISABLED:
			elm = CUIE_CHECK_DISABLED;
			break;
	}

	return CUICheck_Impl::SetTexture(elm, hTex, pRect);
}