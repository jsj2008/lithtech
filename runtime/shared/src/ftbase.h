
// Defines all the base file transfer stuff like packet IDs.

#ifndef __FTBASE_H__
#define __FTBASE_H__


// How many file transfer blocks get sent before we wait for an ack packet?
#define NUM_UNVERIFIED_BLOCKS   6


#define PACKETID_FTBASE     50


// Server telling client about a file.
//     WORD: file ID
//     DWORD: file size
//     string: filename
#define STC_FILEDESC            (PACKETID_FTBASE+0)

// Start a file transferring.
//     WORD: file ID
#define STC_STARTTRANSFER       (PACKETID_FTBASE+1)

// Cancel the current file transfer.
#define STC_CANCELFILETRANSFER  (PACKETID_FTBASE+2)

// File data block.
#define STC_FILEBLOCK           (PACKETID_FTBASE+3)
                            

// Telling if we need a file.
//     WORD: file ID, high bit says if we have it or not.
#define CTS_FILESTATUS          (PACKETID_FTBASE+4)

// Client acknowledging data blocks.
#define CTS_DATARECEIVED        (PACKETID_FTBASE+6)



#endif  // __FTBASE_H__




