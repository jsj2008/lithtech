#if !defined(_BLOWFISH_H_)
#define _BLOWFISH_H_

#define UWORD_32bits  unsigned long
#define UWORD_16bits  unsigned short
#define UBYTE_08bits  unsigned char

short InitializeBlowfish(UBYTE_08bits key[], short keybytes);
void Blowfish_encipher(UWORD_32bits *xl, UWORD_32bits *xr);
void Blowfish_decipher(UWORD_32bits *xl, UWORD_32bits *xr);


#endif
