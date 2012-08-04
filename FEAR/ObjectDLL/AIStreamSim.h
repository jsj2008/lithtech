// ----------------------------------------------------------------------- //
//
// MODULE  : AIStreamSim.h
//
// PURPOSE : Declares the AIStreamSim classes.
//
// CREATED : 12/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AISTREAMSIM_H__
#define __AISTREAMSIM_H__

class ILTOutStream;

class CGenLTOutStream : public ILTOutStream
{
public:
	virtual LTRESULT    WriteString(const char *pStr);
};

ILTOutStream*	streamsim_OutMemStream(std::vector<uint8>& OutVec);

#endif  // __AISTREAMSIM_H__
