#ifndef SIJS_TABLE
#define SIJS_TABLE

typedef struct { unsigned short sjis, unicode; } sjis_transform;

extern sjis_transform sjis_transform_table[7037];

extern int sjis_charlen(unsigned short ch);
extern int sjis_decode_search(const void *_l, const void *_r);
extern unsigned short sjis_chardecode(unsigned short ch);

extern int sjis_strlen(unsigned char *i);

extern int sjis_decode(unsigned char *i, unsigned short *o);

#endif
