////////////////////////////////////////////////////////////////////////////
//                            **** LZW-AB ****                            //
//               Adjusted Binary LZW Compressor/Decompressor              //
//                  Copyright (c) 2016-2020 David Bryant                  //
//                           All Rights Reserved                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

#ifndef LZW_H_
#define LZW_H_

void lzw_set_write_buffer(unsigned char* buffer, int size);
void lzw_set_read_buffer(unsigned char* buffer, int size);
int lzw_compress(int maxbits);
int lzw_decompress();

#endif /* LZWLIB_H_ */
