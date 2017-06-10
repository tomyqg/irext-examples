/**************************************************
 *
 * Code that performs initialization of global data
 * for the "lz77" compression algorithm.
 * New style init table.
 *
 * Copyright 2011 IAR Systems. All rights reserved.
 *
 * $Revision: 37432 $
 *
 **************************************************/

#include "data_init.h"
#include "init_streams.h"

_DLIB_ELF_INIT_MODULE_ATTRIBUTES

#pragma optimize=no_unroll
_DLIB_ELF_INIT_FUNCTION_ATTRIBUTES
table_ptr_t
IAR_LZ77_INIT(table_ptr_t p)
{
  InStream  in;
  OutStream out;

  InStream_init(&in, p);
  OutStream_init(&out, p);

  while (!OutStream_AtEnd(&out))
  {
    uint8_t x = InStream_Read(&in);
    int d = x & 3;
    if (d == 0)
      d = InStream_Read(&in) + 3;
    int l0 = x >> 4;
    if (l0 == 15)
      l0 += InStream_Read(&in);
    while (--d != 0)
    {
      OutStream_Write(&out, InStream_Read(&in));
    }
    if (l0 != 0)
    {
      uint8_t ol = InStream_Read(&in);
      uint8_t oh = (x & 0xc) >> 2;
      if (oh == 0x3)
        oh = InStream_Read(&in);
      OInStream qin = OutStream_GetOInStream(&out, ol + (oh << 8));
      for (int i = 0; i != l0 + 2; ++i)
      {
        OutStream_Write(&out, OInStream_Read(&qin));
      }
    }
  }

  return OutStream_End(&out);
}
