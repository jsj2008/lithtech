

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Wed Mar 08 17:24:11 2006
 */
/* Compiler settings for .\ThumbDtxShellEx.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __ThumbDtxShellEx_i_h__
#define __ThumbDtxShellEx_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IDtxShellExExtractor_FWD_DEFINED__
#define __IDtxShellExExtractor_FWD_DEFINED__
typedef interface IDtxShellExExtractor IDtxShellExExtractor;
#endif 	/* __IDtxShellExExtractor_FWD_DEFINED__ */


#ifndef __DtxShellExExtractor_FWD_DEFINED__
#define __DtxShellExExtractor_FWD_DEFINED__

#ifdef __cplusplus
typedef class DtxShellExExtractor DtxShellExExtractor;
#else
typedef struct DtxShellExExtractor DtxShellExExtractor;
#endif /* __cplusplus */

#endif 	/* __DtxShellExExtractor_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IDtxShellExExtractor_INTERFACE_DEFINED__
#define __IDtxShellExExtractor_INTERFACE_DEFINED__

/* interface IDtxShellExExtractor */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDtxShellExExtractor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A7460053-5421-49C5-9183-7F4AEE7C0F2F")
    IDtxShellExExtractor : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IDtxShellExExtractorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtxShellExExtractor * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtxShellExExtractor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtxShellExExtractor * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDtxShellExExtractor * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDtxShellExExtractor * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDtxShellExExtractor * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDtxShellExExtractor * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IDtxShellExExtractorVtbl;

    interface IDtxShellExExtractor
    {
        CONST_VTBL struct IDtxShellExExtractorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtxShellExExtractor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtxShellExExtractor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtxShellExExtractor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtxShellExExtractor_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDtxShellExExtractor_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDtxShellExExtractor_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDtxShellExExtractor_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDtxShellExExtractor_INTERFACE_DEFINED__ */



#ifndef __THUMBDTXSHELLEXLib_LIBRARY_DEFINED__
#define __THUMBDTXSHELLEXLib_LIBRARY_DEFINED__

/* library THUMBDTXSHELLEXLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_THUMBDTXSHELLEXLib;

EXTERN_C const CLSID CLSID_DtxShellExExtractor;

#ifdef __cplusplus

class DECLSPEC_UUID("6F5AAFDC-CB3D-456C-B863-3FF1201987D5")
DtxShellExExtractor;
#endif
#endif /* __THUMBDTXSHELLEXLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


