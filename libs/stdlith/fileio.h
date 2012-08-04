//------------------------------------------------------------------
//
//  FILE      : FileIO.h
//
//  PURPOSE   : Defines the CMoFileIO class.
//
//  CREATED   : 1st May 1996
//
//  COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __FILEIO_H__
#define __FILEIO_H__


// Includes....
#ifndef __ABSTRACTIO_H__
#include "abstractio.h"
#endif


class CMoFileIO : public CAbstractIO
{
    public:

        // Constructor
                            CMoFileIO();
                            ~CMoFileIO();

        
        // Member functions

        LTBOOL              Open(const char *pFilename, const char *pAccess);
        void                Close();

        LTBOOL              IsOpen();
        
        void                SetBoundaries(uint32 min, uint32 max);

        LTBOOL              Write(const void *pBlock, uint32 blockSize);
        LTBOOL              Read(void *pBlock, uint32 blockSize);

        uint32              GetCurPos();
        uint32              GetLen();

        LTBOOL              SeekTo(uint32 pos);

    
    public:

        // Private member functions


    public:
        
        // Private member variables
        FILE                *m_pFile;
        
        // File boundaries...
        uint32              m_FileMin, m_FileMax;
        
        uint32              m_FileLen;

};


#endif


