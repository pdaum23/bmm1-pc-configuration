#ifndef FCS16_H
#define FCS16_H

#include "htype.h"

#define FCS16_INIT    0xffff  /* Initial FCS value */
#define FCS16_GOOD    0xf0b8  /* Good final FCS value */


class C_FCS
{
public:

static HUINT16 Generate(HUINT16 fcs, HUINT8 *cp, HUINT32 len);
static void Add(HUINT8 *cp, HUINT32 *len);
static HBOOL Verify(HUINT8 *cp, HUINT32 len);
static HBOOL Try(HUINT8 *cp, HUINT32 len);


private:
};


#endif