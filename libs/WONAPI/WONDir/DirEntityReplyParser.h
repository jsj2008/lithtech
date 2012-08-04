#ifndef __WON_DIRENTITYREPLYPARSER_H__
#define __WON_DIRENTITYREPLYPARSER_H__
#include "WONShared.h"

#include "DirEntity.h"
#include "WONStatus.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class DirEntityReplyParser
{
protected:
	static WONStatus ParseMultiEntityReply(const void *theMsg, unsigned long theMsgLen, DirEntityList &theList);
	static WONStatus ParseSingleEntityReply(const void *theMsg, unsigned long theMsgLen, DirEntityPtr &theEntity);

	static bool OnlyDirs(DWORD theFlags);
	static bool OnlyServices(DWORD theServices);

	friend class GetDirOp;
	friend class GetMultiDirOp;

	friend class GetEntityRequest;
	friend class GetServiceOp;
	friend class FindEntityOp;
};

} // namespace WONAPI

#endif
