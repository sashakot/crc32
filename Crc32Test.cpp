// //////////////////////////////////////////////////////////
// Crc32Test.cpp
// Copyright (c) 2016 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

#include "Crc32.h"
#include <cstdlib>
#include <cstdio>

#ifdef USE_ISA_LIB
#include <isa-l/crc.h>
#endif

// //////////////////////////////////////////////////////////
// test code

/// one gigabyte
const size_t NumBytes = 1024*1024*1024;
/// 4k chunks during last test
const size_t DefaultChunkSize = 4*1024;


#include <cstdio>
//#if defined(_MSC_VER) || defined(__CYGWIN__)
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <ctime>
#endif

// timing
static double seconds()
{
//#if defined(_MSC_VER) || defined(__CYGWIN__)
#if defined(_WIN32) || defined(_WIN64)
  LARGE_INTEGER frequency, now;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter  (&now);
  return now.QuadPart / double(frequency.QuadPart);
#else
  timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return now.tv_sec + now.tv_nsec / 1000000000.0;
#endif
}


int main(int, char**)
{
  printf("Please wait ...\n");

  uint32_t randomNumber = 0x27121978;
  // initialize
  char* data = new char[NumBytes];
  for (size_t i = 0; i < NumBytes; i++)
  {
    data[i] = char(randomNumber & 0xFF);
    // simple LCG, see http://en.wikipedia.org/wiki/Linear_congruential_generator
    randomNumber = 1664525 * randomNumber + 1013904223;
  }

  // re-use variables
  double startTime, duration;
  uint32_t crc;

  // bitwise
  startTime = seconds();
  crc = crc32_bitwise(data, NumBytes);
  duration  = seconds() - startTime;
  printf("bitwise          : CRC=%08X, %.3fs, %.3f MB/s\n",
         crc, duration, (NumBytes / (1024*1024)) / duration);

  // half-byte
  startTime = seconds();
  crc = crc32_halfbyte(data, NumBytes);
  duration  = seconds() - startTime;
  printf("half-byte        : CRC=%08X, %.3fs, %.3f MB/s\n",
         crc, duration, (NumBytes / (1024*1024)) / duration);

  // one byte at once (without lookup tables)
  startTime = seconds();
  crc = crc32_1byte_tableless(data, NumBytes);
  duration  = seconds() - startTime;
  printf("tableless (byte) : CRC=%08X, %.3fs, %.3f MB/s\n",
         crc, duration, (NumBytes / (1024*1024)) / duration);

  // one byte at once (without lookup tables)
  startTime = seconds();
  crc = crc32_1byte_tableless2(data, NumBytes);
  duration  = seconds() - startTime;
  printf("tableless (byte2): CRC=%08X, %.3fs, %.3f MB/s\n",
         crc, duration, (NumBytes / (1024*1024)) / duration);

#ifdef CRC32_USE_LOOKUP_TABLE_BYTE
  // one byte at once
  startTime = seconds();
  crc = crc32_1byte(data, NumBytes);
  duration  = seconds() - startTime;
  printf("  1 byte  at once: CRC=%08X, %.3fs, %.3f MB/s\n",
         crc, duration, (NumBytes / (1024*1024)) / duration);
#endif

#ifdef CRC32_USE_LOOKUP_TABLE_SLICING_BY_4
  // four bytes at once
  startTime = seconds();
  crc = crc32_4bytes(data, NumBytes);
  duration  = seconds() - startTime;
  printf("  4 bytes at once: CRC=%08X, %.3fs, %.3f MB/s\n",
         crc, duration, (NumBytes / (1024*1024)) / duration);
#endif // CRC32_USE_LOOKUP_TABLE_SLICING_BY_4

#ifdef CRC32_USE_LOOKUP_TABLE_SLICING_BY_8
  // eight bytes at once
  startTime = seconds();
  crc = crc32_8bytes(data, NumBytes);
  duration  = seconds() - startTime;
  printf("  8 bytes at once: CRC=%08X, %.3fs, %.3f MB/s\n",
         crc, duration, (NumBytes / (1024*1024)) / duration);

  // eight bytes at once, unrolled 4 times (=> 32 bytes per loop)
  startTime = seconds();
  crc = crc32_4x8bytes(data, NumBytes);
  duration  = seconds() - startTime;
  printf("4x8 bytes at once: CRC=%08X, %.3fs, %.3f MB/s\n",
         crc, duration, (NumBytes / (1024*1024)) / duration);
#endif // CRC32_USE_LOOKUP_TABLE_SLICING_BY_8

#ifdef CRC32_USE_LOOKUP_TABLE_SLICING_BY_16
  // sixteen bytes at once
  startTime = seconds();
  crc = crc32_16bytes(data, NumBytes);
  duration  = seconds() - startTime;
  printf(" 16 bytes at once: CRC=%08X, %.3fs, %.3f MB/s\n",
         crc, duration, (NumBytes / (1024*1024)) / duration);

  // sixteen bytes at once
  startTime = seconds();
  crc = crc32_16bytes_prefetch(data, NumBytes, 0, 256);
  duration  = seconds() - startTime;
  printf(" 16 bytes at once: CRC=%08X, %.3fs, %.3f MB/s (including prefetching)\n",
         crc, duration, (NumBytes / (1024*1024)) / duration);
#endif // CRC32_USE_LOOKUP_TABLE_SLICING_BY_16

#ifdef USE_ISA_LIB
  // ISA-L ieee
  startTime = seconds();
  crc = crc32_ieee(0, (uint8_t*)data, NumBytes);
  duration  = seconds() - startTime;
  printf(" ISA-L ieee: CRC=%08X, %.3fs, %.3f MB/s\n",
		  crc, duration, (NumBytes / (1024*1024)) / duration);

  // ISA-L iscsi
  startTime = seconds();
  crc = crc32_iscsi((uint8_t*)data, NumBytes, 0);
  duration  = seconds() - startTime;
  printf(" ISA-L iscsi: CRC=%08X, %.3fs, %.3f MB/s\n",
		  crc, duration, (NumBytes / (1024*1024)) / duration);

#else
#error "No ISA-L"
#endif

  // process in 4k chunks
  startTime = seconds();
  crc = 0; // also default parameter of crc32_xx functions
  size_t bytesProcessed = 0;
  while (bytesProcessed < NumBytes)
  {
    size_t bytesLeft = NumBytes - bytesProcessed;
    size_t chunkSize = (DefaultChunkSize < bytesLeft) ? DefaultChunkSize : bytesLeft;

    crc = crc32_fast(data + bytesProcessed, chunkSize, crc);

    bytesProcessed += chunkSize;
  }
  duration  = seconds() - startTime;
  printf("    chunked      : CRC=%08X, %.3fs, %.3f MB/s\n",
         crc, duration, (NumBytes / (1024*1024)) / duration);

  delete[] data;
  return 0;
}
