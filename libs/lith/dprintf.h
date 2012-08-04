/***************************************************************************
  DPRINTF 1.10                                           written in C++
***************************************************************************/

#ifndef __DPRINTF_H__
#define __DPRINTF_H__


void dprintf(char*, ...);
void dprintf(unsigned int Level, char*, ...);
void dprintf(int X, int Y, char*, ...);
void dprintf(unsigned int Level, int X, int Y, char*, ...);
void dgotoxy(int X, int Y);
void dgotoxy(unsigned int Level, int X, int Y);
void dclrscr(void);
void dclrscr(unsigned int Level);

#endif
