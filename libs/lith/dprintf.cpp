/***************************************************************************
  DPRINTF 1.10                                           written in C++

  A nice debugging library that lets you put lots of dprintf's in your code
  and control their output using the DPRINTF environment variable.

  If this module is compiled with NODPRINTF defined it will just create
  empty functions.

  Possible settings in the environment:
    SET DPRINTF=MONO       - will output directly to a monochrome monitor
    SET DPRINTF=FILE       - will output to the file DPRINTF.OUT
    SET DPRINTF=FILEAPPEND - will output to the file DPRINTF.OUT opening and
                             closing the file on each call to dprintf
    SET DPRINTF=STDOUT     - will output to STDOUT
    SET DPRINTF=COM1       - will output to COM1 @ 9600 baud N81
    SET DPRINTF=COM2       - will output to COM2 @ 9600 baud N81
	 SET DPRINTF=LPT1       - will output to dos device LPT1
	 SET DPRINTF=LPT2       - will output to dos device LPT2
	 SET DPRINTF=PRN        - will output to dos device PRN

  NOTE: If using the watcom compiler serial port options are not available.

  Level exclusions can be included to mask dprintf's that have certain
  level values.  If dprintf is used with no level parameter then it's level
  is zero.  Examples of setting the environment variables are as follows:
	 SET DPRINTF=MONO X5 X10-20
  This example would mask dprintf's with level 5 and levels 10 thru 20.
  A maximum of 16 X statements can exist.

  If the environment variable is not set to one of these settings then
  dprintf will do nothing.

  REVISION HISTORY:

    1.10 : Apr-20-94 by Bryan Bouwman (added watcom support include dos4gw).

    Started : May-30-93 by Bryan Bouwman (made from MONODOS.H)

  (MONODOS.H was originally written by Kevin Pinkerton and published in
	the Windows/DOS developer's journal Vol. 4, No. 6 )
***************************************************************************/

#include "dprintf.h"
#ifndef _LINUX
#include <dos.h>
#else
#include <ctype.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifdef _WINDOWS
#include "windows.h"
#endif

#define BUFSIZE 256     // buffer size
#define CPL 80          // Number of characters per line
#define LPP 25          // number of lines per page
#define ATTR 0x700      // the monochrome video attribute for white on black

#ifndef NODPRINTF

typedef unsigned char BYTE;
typedef unsigned char UINT8;
typedef unsigned short int UINT16;
typedef unsigned short int WORD;
typedef unsigned int UINT32;
typedef signed char INT8;
typedef signed short int INT16;
typedef int INT32;

#ifndef _WINDOWS
typedef unsigned char SHORTBOOLEAN;
#ifndef BOOLEAN
typedef int BOOLEAN;
#endif
#ifndef TRUE
# define TRUE    1
#endif
#ifndef FALSE
# define FALSE   0
#endif
#endif

enum dprintfOutputType {
  DPRINTF_UNKNOWN,
  DPRINTF_NOTHING,
  DPRINTF_MONOCHROME,
  DPRINTF_COM1,
  DPRINTF_COM2,
  DPRINTF_FILE,
  DPRINTF_FILEAPPEND,
  DPRINTF_STDOUT,
  DPRINTF_LPT1,
  DPRINTF_LPT2,
  DPRINTF_PRN
};

#ifdef _CONSOLE
unsigned short *dprintfmonoscreen;   // pointer to B000
#else
#ifdef _LINUX
unsigned short *dprintfmonoscreen;   // pointer to B000
#else
#ifndef __FLATMODEL__
unsigned short far *dprintfmonoscreen;   // pointer to B000
#else
unsigned short *dprintfmonoscreen;   // pointer to B000
#endif
#endif
#endif

unsigned int dbprintfcurrentLine = 0;  // current line
unsigned int dprintfcurrentChar = 0;   // current char
dprintfOutputType dprintfOutType = DPRINTF_UNKNOWN;
FILE* dprintffile = NULL;

class dprintfinittype {
public:
  dprintfinittype ();
  ~dprintfinittype ();
};

static dprintfinittype dprintfinit;

/***************************  LEVEL EXCLUDE ROUTINES ***************************/

#define MaxExcludeRegions 16

class dprintfFromTo {
public:
  unsigned int From;
  unsigned int To;
};

class dprintfExcludeRegions {
public:
  unsigned int NumRegions;
  dprintfFromTo Ary[MaxExcludeRegions];
  BOOLEAN In(unsigned int Num);
  void Add(unsigned int From,unsigned int To);
  void Scan(char *Str); // scans a string and adds excludes for all x=from-to or x=num
};

BOOLEAN dprintfExcludeRegions::In(unsigned int Num) {
  unsigned int Loop;
  for (Loop=0;Loop<NumRegions;Loop++) {
	 if ((Num >= Ary[Loop].From) && (Num <= Ary[Loop].To)) return TRUE;
  }
  return FALSE;
}

void dprintfExcludeRegions::Add(unsigned int From,unsigned int To) {
  if ((NumRegions+1) < MaxExcludeRegions) {
	 Ary[NumRegions].From = From;
	 Ary[NumRegions].To = To;
	 NumRegions++;
  }
}

void dprintfExcludeRegions::Scan(char *Str) {
  char TmpStr[BUFSIZE];
  char *P;
  int From;
  int To;
  while (*Str != '\0') {
	 Str = strstr(Str,"X");
	 if (Str == NULL) return;
	 Str = strpbrk(Str,"0123456789");
	 if (Str == NULL) return;
	 strcpy(TmpStr,Str);
	 P = TmpStr;
	 while (*P != '\0') {
		if ((*P >= '0') && (*P <= '9')) {
		  P++;
		  Str++;
		}
		else *P = '\0';
	 }
	 From = atol(TmpStr);
	 if (*Str == '-') {
		Str = strpbrk(Str,"0123456789");
	   if (Str == NULL) return;
	   strcpy (TmpStr,Str);
		P = TmpStr;
	   while (*P != '\0') {
  		  if ((*P >= '0') && (*P <= '9')) {
			 P++;
		  	 Str++;
		  }
		  else *P = '\0';
	   }
	   To = atol(TmpStr);
	 }
	 else To = From;
	 Add (From,To);
  }
}

dprintfExcludeRegions dprintfExReg;

#ifndef __WATCOM__
#ifndef _WINDOWS
#ifndef _CONSOLE
#ifndef _LINUX

/********************************  SERIAL ROUTINES *****************************/

#ifdef __TURBOC__
typedef void _far __interrupt _dpSerComIntFuncType(...);
#else
typedef void _far __interrupt _dpSerComIntFuncType();
#endif

// The possible uart types
#define SerComUARTType8250 0
#define SerComUARTType16550 1

#define SerComCompTypeXT 0
#define SerComCompTypeAT 1

typedef struct dpPortHandleType {
  BYTE *RecvBuf;
  unsigned int RecvBufSize;
  unsigned int RecvBufStart,RecvBufEnd;
  unsigned int NumCharsInRecvBuf;
  BYTE *SendBuf;
  unsigned int SendBufSize;
  unsigned int SendBufStart,SendBufEnd;
  unsigned int NumCharsInSendBuf;
  UINT16 PortAddr;
  BYTE Interrupt;
  BYTE DosInt;
  int UARTType;
  int CompType;
  unsigned long int BaudRate;
  _dpSerComIntFuncType *OldInterruptVector;
  BOOLEAN Err;
  BOOLEAN LastSendIntrEmpty; // if true we need to start sending again to get send interrupts going
} dpPortHandleType;

dpPortHandleType *dprintfPortHandle = NULL;

// Registers in the 8250 UART
#define Data 0
#define LowBaud 0
#define IntEnable 1
#define HighBaud 1
#define IntID 2
#define LineControl 3
#define ModemControl 4
#define LineStatus 5
#define ModemStatus 6

// Additional registers in the 16550 buffered UART
#define FIFOControl 2
#define ScratchReg 7

// IO Address for the PIC (Programmable Interrupt Controller)
#define PIC1Reset 0x20
#define PIC1Mask 0x21
#define PIC2Reset 0xA0
#define PIC2Mask 0xA1

#ifndef __TURBOC__
#define outp _outp
#define inp _inp
#endif

static int dbMostInRecvBuf;
static int dbMostInSendBuf = 0;

#define PHnd _dpSerComPortHandle // we want the name _dpSerComPortHandle in the library, but lets just use PHnd in the source
static dpPortHandleType *PHnd;

void _dpSerComTransmittChar() {
  if (PHnd->NumCharsInSendBuf > 0) {
    PHnd->LastSendIntrEmpty = FALSE;
    outp (PHnd->PortAddr+Data,PHnd->SendBuf[PHnd->SendBufStart]);
    PHnd->NumCharsInSendBuf--;
    PHnd->SendBufStart++;
    if (PHnd->SendBufStart >= PHnd->SendBufSize) PHnd->SendBufStart = 0;
  }
  else PHnd->LastSendIntrEmpty = TRUE;
}


void _dpSerComRecvChar() {
  if (PHnd->NumCharsInRecvBuf < PHnd->RecvBufSize) {
    PHnd->RecvBuf[PHnd->RecvBufEnd] = inp(PHnd->PortAddr+Data);
    PHnd->RecvBufEnd++;
    if (PHnd->RecvBufEnd >= PHnd->RecvBufSize) PHnd->RecvBufEnd = 0;
    PHnd->NumCharsInRecvBuf++;
    if (dbMostInRecvBuf < PHnd->NumCharsInRecvBuf) dbMostInRecvBuf = PHnd->NumCharsInRecvBuf;
  }
  else PHnd->Err = TRUE; // Error, receive buffer is full!
}


#ifdef __TURBOC__
void __interrupt __far _dpSerComInterrupt (...) {
#else
void __interrupt __far _dpSerComInterrupt () {
#endif
  _disable();
  BYTE IntType;
  IntType = (inp (PHnd->PortAddr+IntID) & 0x07);
  switch (IntType) {
    case 0: // modem status
      break;
    case 2: // transmitter buffer empty
      _dpSerComTransmittChar();
      break;
    case 4: // received data
      _dpSerComRecvChar();
      break;
    case 6: // line status has changed
      break;
    default:
      break;
  }
  outp (PIC1Reset,0x20);  // Reset the PIC
  if (PHnd->Interrupt > 8) outp (PIC2Reset,0x20); // Reset 2nd PIC if necessary
  _enable();
}


dpPortHandleType *dpSerComOpen(int NewUARTType, int NewCompType, UINT16 PortAddr,
                           BYTE PortIRQ, unsigned long int BaudRate,
                           unsigned int DataBits, unsigned int StopBits, unsigned int Parity,
                           unsigned int SendBufSize, unsigned long int RecvBufSize)
{
  dbMostInRecvBuf = 0;
  dbMostInSendBuf = 0;

  // create the PortHandle
  dpPortHandleType *PortHandle;
  PortHandle = (dpPortHandleType*)malloc(sizeof(dpPortHandleType));
  PHnd = PortHandle;
  PortHandle->RecvBuf = (BYTE*)malloc(RecvBufSize);
  PortHandle->RecvBufSize = RecvBufSize;
  PortHandle->RecvBufStart = 0;
  PortHandle->RecvBufEnd = 0;
  PortHandle->NumCharsInRecvBuf = 0;
  PortHandle->SendBuf = (BYTE*)malloc(SendBufSize);
  PortHandle->SendBufSize = SendBufSize;
  PortHandle->SendBufStart = 0;
  PortHandle->SendBufEnd = 0;
  PortHandle->NumCharsInSendBuf = 0;
  PortHandle->PortAddr = PortAddr;
  PortHandle->CompType = NewCompType;
  PortHandle->UARTType = NewUARTType;
  if ((PortHandle->CompType == SerComCompTypeAT) && (PortIRQ == 2)) PortIRQ = 9; // IRQ 2 is really IRQ 9 on the AT
  if (PortIRQ > 8) PortHandle->DosInt = 0x70-8+PortIRQ;
  else PortHandle->DosInt = PortIRQ+8;
  PortHandle->Interrupt = PortIRQ;
  PortHandle->BaudRate = BaudRate;
  PortHandle->Err = FALSE;
  PortHandle->LastSendIntrEmpty = TRUE;

  _disable();

  // set the interrupt vector to point to our handeling routine
  PortHandle->OldInterruptVector =  _dos_getvect (PortHandle->DosInt);
  _dos_setvect (PortHandle->DosInt,_dpSerComInterrupt);

  // Set the baud rate
  BaudRate = 115200/BaudRate;
  outp (PortAddr+LineControl,0x83);
  outp (PortAddr+LowBaud,BaudRate & 0x00ff);
  outp (PortAddr+HighBaud,BaudRate >> 8);
  outp (PortAddr+LineControl,0x03);

  // Set to 1 stop no parity 8 bits
  if (Parity == 4) Parity = 7;
  DataBits -= 5;
  StopBits -= 1;
  outp (PortAddr+LineControl,((3<<Parity)|(2<StopBits)|(DataBits)));

  // reset the registers in the UART
  outp (PortAddr+LineStatus,0x00);

  // if we have a buffered UART then enable the FIFO's (RCVR Trigger is 14 bytes)
  if (PortHandle->UARTType == SerComUARTType16550) outp (PortAddr+FIFOControl,0xC7);

  // enable the interrupts
  outp (PortAddr+IntEnable,0x03);
  outp (PortAddr+ModemControl,0x0b);
  _enable();
  // Enable the IRQ mask in the PIC for the XT (or low AT interrupts except for 2)
  if ((PortHandle->CompType == SerComCompTypeXT) || ((PortHandle->Interrupt < 8) && (PortHandle->Interrupt > 2))) {
    outp (PIC1Mask,inp(PIC1Mask)&~(1<<PortIRQ));
  }
  // Enable the IRQ mask in the PIC for the AT
  else {
    outp (PIC1Mask,inp(PIC1Mask)&~(1<<2)); // make sure second PIC is enabeled
    outp (PIC2Mask,inp(PIC2Mask)&~(1<<(PortIRQ-8)));
  }

  return PortHandle;
}

void dpSerComClose (dpPortHandleType *PortHandle) {
  _disable();

  // disable the interrupts
  outp (PortHandle->PortAddr+IntEnable,0x00);
  outp (PortHandle->PortAddr+ModemControl,0x00);
  if ((PortHandle->CompType == SerComCompTypeXT) || ((PortHandle->Interrupt < 8) && (PortHandle->Interrupt > 2)))
    outp (PIC1Mask,inp(PIC1Mask)|(1<<PortHandle->Interrupt));
  else
    outp (PIC1Mask,inp(PIC2Mask)|(1<<(PortHandle->Interrupt-8)));

  // put the interrupt vector back the way it was
  _dos_setvect (PortHandle->DosInt,PortHandle->OldInterruptVector);

  _enable();

  free (PortHandle->RecvBuf);
  free (PortHandle->SendBuf);
  free (PortHandle);

}


void dpSerComPut (dpPortHandleType *PH, char Ch) {
  while (PH->SendBufSize <= PH->NumCharsInSendBuf) ; // wait until send buffer has room
  PH->NumCharsInSendBuf++;
  if (dbMostInSendBuf < PH->NumCharsInSendBuf) dbMostInSendBuf = PH->NumCharsInSendBuf;
  PH->SendBuf[PH->SendBufEnd] = Ch;
  PH->SendBufEnd++;
  if (PH->SendBufEnd >= PH->SendBufSize) PH->SendBufEnd = 0;
  if (PH->LastSendIntrEmpty == TRUE) _dpSerComTransmittChar();
}

void dpSerComPutStr (char *Str) {
  while (*Str != '\0') {
    dpSerComPut (dprintfPortHandle,*Str);
    if (*Str == 10) dpSerComPut (dprintfPortHandle,13); // output CR with line feed
    Str++;
  }
}
#endif
#endif
#endif
#endif

/********************************  MONOCHROME ROUTINES *****************************/

void dprintfmonoincline( void )
{
//  This is a local routine.  It increments the
//  current line pointer and performs the scroll
//  function if required.

    int i;
    dprintfcurrentChar = 0;
    dbprintfcurrentLine++;
    /* scroll the screen if required */
    if ( dbprintfcurrentLine == LPP ) {
        /* move lines 2 thru 25 up one line */
        for ( i = CPL; i < (CPL*LPP); i++)
            dprintfmonoscreen[i-CPL] = dprintfmonoscreen[i];
        /* clear last line */
        for ( i = CPL*(LPP-1); i < (CPL*LPP);i++ )
            dprintfmonoscreen[i] = ATTR + ' ';
        /* decrement current line pointer */
        dbprintfcurrentLine -= 1;
    }
}

void dprintfmonoclrscr( void )
// This is a local routine used to clear the
// screen and reposition the pointers
{
    int i;
    for ( i = 0; i < (CPL*LPP); i++ ) {
         dprintfmonoscreen[i] = ATTR + ' ';
    }
    dbprintfcurrentLine = 0;
    dprintfcurrentChar = 0;
}

#ifdef _CONSOLE
void dprintfmonoprint(char *message )
#else
#ifdef _LINUX
void dprintfmonoprint(char *message )
#else
#ifndef __FLATMODEL__
void dprintfmonoprint(char far *message )
#else
void dprintfmonoprint(char *message )
#endif
#endif
#endif

{
//  This function will display a string on the
//  secondary monitor and generate a CR/LF if
//  requested. Wrapping is supported and any
//  string that extends past column 79 is wrapped
//  around.  Scrolling is also supported once line
//  25 is written to.

#ifndef _CONSOLE
#ifndef _WINDOWS
#ifndef _LINUX
    int i;
    static firstTimeThru = 1;   /* used to clear the screen
                                            first time in */
    if ( firstTimeThru ) {
        firstTimeThru = 0;
#ifdef DOS386
        dprintfmonoscreen = (unsigned short far*)_x386_mk_protected_ptr(0x000B0000);
#else
#ifndef __FLATMODEL__
        dprintfmonoscreen = (unsigned short far*)
            MK_FP( 0xB000, 0 );
#else
        dprintfmonoscreen = (unsigned short *)0xB0000;
#endif
#endif
        dprintfmonoclrscr();
    }
    /* send out the message  */
    i = 0;
    while ( message[i] != 0 ) {
        /* check for word or character wrap */
        if ( dprintfcurrentChar == CPL ) {
            dprintfmonoincline();
        }
		  if (message[i] == 0x0a) dprintfmonoincline(); // increment line if line feed
		  if (message[i] <= 0x1f) { i++; continue; } // skip control characters
        dprintfmonoscreen[(CPL*dbprintfcurrentLine)+dprintfcurrentChar++] =
            ATTR + message[i++];
    }
#else
  printf(message);
#endif
#else
  OutputDebugString(message);
#endif
#else
  printf(message);
#endif
}


/********************************  GENERAL ROUTINES *****************************/

void dprintfdoprint (char *Str) {

#ifdef _CONSOLE
#ifdef _DEBUG
	printf(Str);
	return;
#endif
#else
#ifdef _WINDOWS
#ifdef _DEBUG
	OutputDebugString(Str);
	return;
#endif
#else
#ifdef _LINUX
#ifdef _DEBUG
	printf(Str);
	return;
#endif
#else

  switch (dprintfOutType) {
    case DPRINTF_MONOCHROME :
      dprintfmonoprint (Str);
      break;
    case DPRINTF_STDOUT :
      printf ("%s",Str);
      break;
	 case DPRINTF_LPT1 :
    case DPRINTF_LPT2 :
    case DPRINTF_PRN :
    case DPRINTF_FILE :
      if (dprintffile != NULL) fprintf (dprintffile,"%s",Str);
      break;
    case DPRINTF_FILEAPPEND :
      dprintffile = fopen ("DPRINTF.OUT","a");
      if (dprintffile != NULL) fprintf (dprintffile,"%s",Str);
      fclose (dprintffile);
      break;
#ifndef __WATCOM__
#ifndef _WINDOWS
    case DPRINTF_COM1 :
    case DPRINTF_COM2 :
      dpSerComPutStr (Str);
      break;
#endif
#endif
  }
#endif
#endif
#endif
}
#endif

void dprintf(char *format, ...)
{
#ifndef NODPRINTF
	 if ((dprintfOutType == DPRINTF_NOTHING) || (dprintfOutType == DPRINTF_UNKNOWN)) return;
	 if (dprintfExReg.In(0)) return;

    va_list argptr;
    char buf[BUFSIZE];

    va_start( argptr, format );
    vsprintf( buf, format, argptr );
    va_end( argptr );

    dprintfdoprint( buf );
#endif
}

void dprintf(int X, int Y, char *format, ...)
{
#ifndef NODPRINTF
	 if ((dprintfOutType == DPRINTF_NOTHING) || (dprintfOutType == DPRINTF_UNKNOWN)) return;
	 if (dprintfExReg.In(0)) return;

    dgotoxy (X,Y);

    va_list argptr;
    char buf[BUFSIZE];

    va_start( argptr, format );
    vsprintf( buf, format, argptr );
    va_end( argptr );

    dprintfdoprint( buf );
#endif
}

void dprintf(unsigned int Level, char *format, ...)
{
#ifndef NODPRINTF
	 if ((dprintfOutType == DPRINTF_NOTHING) || (dprintfOutType == DPRINTF_UNKNOWN)) return;
	 if (dprintfExReg.In(Level)) return;

    va_list argptr;
    char buf[BUFSIZE];

    va_start( argptr, format );
    vsprintf( buf, format, argptr );
    va_end( argptr );

    dprintfdoprint( buf );
#endif
}

void dprintf(unsigned int Level, int X, int Y, char* format, ...)
{
#ifndef NODPRINTF
	 if ((dprintfOutType == DPRINTF_NOTHING) || (dprintfOutType == DPRINTF_UNKNOWN)) return;
	 if (dprintfExReg.In(Level)) return;

    dgotoxy (X,Y);

    va_list argptr;
    char buf[BUFSIZE];

    va_start( argptr, format );
    vsprintf( buf, format, argptr );
    va_end( argptr );

    dprintfdoprint( buf );
#endif
}

void dgotoxy (int X, int Y) {
#ifndef NODPRINTF
  dgotoxy (0,X,Y);
#endif
}

void dgotoxy (unsigned int Level, int X, int Y) {

#ifdef _CONSOLE
	return;
#endif

#ifdef _WINDOWS
	return;
#endif

#ifdef _LINUX
	return; 
#endif

#ifndef NODPRINTF
  if (dprintfExReg.In(Level)) return;
  switch (dprintfOutType) {
    case DPRINTF_MONOCHROME :
      dbprintfcurrentLine = Y;
      dprintfcurrentChar = X;
      break;
    default :
      dprintfdoprint ("\n");
      break;
  }
#endif
}

void dclrscr () {
#ifndef NODPRINTF
  dclrscr (0);
#endif
}

void dclrscr (unsigned int Level) {

#ifdef _CONSOLE
	return;
#endif

#ifdef _WINDOWS
	return;
#endif

#ifdef _LINUX
	return;
#endif

#ifndef NODPRINTF
  if (dprintfExReg.In(Level)) return;
  switch (dprintfOutType) {
    case DPRINTF_MONOCHROME :
      dbprintfcurrentLine = 0;
      dprintfcurrentChar = 0;
      dprintfmonoclrscr();
      break;
    default :
      dprintfdoprint ("\n");
      break;
  }
#endif
}

#ifndef NODPRINTF
dprintfinittype::dprintfinittype () {
  // set up initial variables
  dprintfExReg.NumRegions = 0;

  dprintfOutType = DPRINTF_NOTHING;

#ifdef _CONSOLE
#ifdef _DEBUG
  dprintfOutType = DPRINTF_MONOCHROME;
#else
  dprintfOutType = DPRINTF_NOTHING;
#endif
#endif

#ifdef _WINDOWS
#ifdef _DEBUG
  dprintfOutType = DPRINTF_MONOCHROME;
#else
  dprintfOutType = DPRINTF_NOTHING;
#endif
#endif

#ifdef _LINUX
#ifdef _DEBUG
  dprintfOutType = DPRINTF_MONOCHROME;
#else
  dprintfOutType = DPRINTF_NOTHING;
#endif
#endif

  // search environment for flags
  char *Str = getenv("DPRINTF");
  if (Str != NULL) {
    char Buf[BUFSIZE];
    strcpy (Buf,Str); // this could be a problem if environment string is greater than BUFSIZE
    
    #ifdef _LINUX
    	char * p = Buf;
	
	while ( *p )
	{
		*p = toupper( *p );
		p++;
	}

    #else
    strupr (Buf); // convert buf to upper case
    #endif
    if (strstr(Buf,"MONO") != NULL) dprintfOutType = DPRINTF_MONOCHROME;
    if (strstr(Buf,"FILE") != NULL) dprintfOutType = DPRINTF_FILE;
    if (strstr(Buf,"FILEAPPEND") != NULL) dprintfOutType = DPRINTF_FILEAPPEND;
    if (strstr(Buf,"COM1") != NULL) dprintfOutType = DPRINTF_COM1;
    if (strstr(Buf,"COM2") != NULL) dprintfOutType = DPRINTF_COM2;
    if (strstr(Buf,"STDOUT") != NULL) dprintfOutType = DPRINTF_STDOUT;
    if (strstr(Buf,"LPT1") != NULL) dprintfOutType = DPRINTF_LPT1;
    if (strstr(Buf,"LPT2") != NULL) dprintfOutType = DPRINTF_LPT1;
    if (strstr(Buf,"PRN") != NULL) dprintfOutType = DPRINTF_PRN;

    dprintfExReg.Scan (Buf);
  }

#ifdef _WINDOWS
    dprintfOutType = DPRINTF_MONOCHROME;
#endif

#ifdef _LINUX
    dprintfOutType = DPRINTF_MONOCHROME;
#endif

#ifdef _CONSOLE
    dprintfOutType = DPRINTF_MONOCHROME;
#endif

  // open file if necessary
  switch (dprintfOutType) {
    case DPRINTF_FILE :
      dprintffile = fopen ("DPRINTF.OUT","w");
      if (dprintffile == NULL) dprintfOutType = DPRINTF_NOTHING;
      break;
    case DPRINTF_FILEAPPEND :
      dprintffile = fopen ("DPRINTF.OUT","w");
      fclose (dprintffile);
      break;
#ifndef __WATCOM__
#ifndef _WINDOWS
#ifndef _CONSOLE
#ifndef _LINUX    
    case DPRINTF_COM1 :
      dprintfPortHandle = dpSerComOpen (SerComUARTType8250,SerComCompTypeAT,0x3f8,4,9600,8,0,1,128,128);
      break;
    case DPRINTF_COM2 :
      dprintfPortHandle = dpSerComOpen (SerComUARTType8250,SerComCompTypeAT,0x2f8,3,9600,8,0,1,128,128);
      break;
#endif
#endif
#endif
#endif

    case DPRINTF_LPT1 :
      dprintffile = fopen ("LPT1","w");
      if (dprintffile == NULL) dprintfOutType = DPRINTF_NOTHING;
      break;
    case DPRINTF_LPT2 :
      dprintffile = fopen ("LPT2","w");
      if (dprintffile == NULL) dprintfOutType = DPRINTF_NOTHING;
      break;
    case DPRINTF_PRN :
      dprintffile = fopen ("PRN","w");
      if (dprintffile == NULL) dprintfOutType = DPRINTF_NOTHING;
      break;
  }
}

dprintfinittype::~dprintfinittype () {
  switch (dprintfOutType) {
    case DPRINTF_FILE :
    case DPRINTF_LPT1 :
    case DPRINTF_LPT2 :
    case DPRINTF_PRN :
      fclose (dprintffile);
      break;
#ifndef __WATCOM__
#ifndef _WINDOWS
#ifndef _CONSOLE
#ifndef _LINUX
    case DPRINTF_COM1 :
    case DPRINTF_COM2 :
      if (dprintfPortHandle != NULL) {
        unsigned long int Loop;
        for (Loop = 0;Loop < 1000000;Loop++) { // wait for characters to get finished outputting
          if (dprintfPortHandle->NumCharsInSendBuf <= 0) break;
        }
      }
      dpSerComClose (dprintfPortHandle);
      break;
#endif
#endif
#endif
#endif

  }
}
#endif

