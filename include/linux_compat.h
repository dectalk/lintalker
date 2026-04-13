/*
 * linux_compat.h - Minimal Windows type stubs for 64-bit Linux port
 */
#ifndef __LINUX_COMPAT__
#define __LINUX_COMPAT__

#include <stdint.h>
#include <stddef.h>

/* Disable Mac/CodeWarrior alignment/import pragmas */
#define PRAGMA_ALIGN_SUPPORTED  0
#define PRAGMA_IMPORT_SUPPORTED 0

/* Basic Windows-style integer types (explicit widths) */
typedef uint32_t    DWORD;
typedef uint32_t    FOURCC;
typedef uint16_t    WORD;
typedef int32_t     LONG;
typedef uint8_t     BYTE;

/* Opaque handle types */
typedef void       *HANDLE;
typedef void       *HWND;
typedef void       *HINSTANCE;
typedef void       *HBRUSH;
typedef void       *HBITMAP;

/* Boolean */
typedef int BOOL;
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Simplified wave audio buffer header */
typedef struct tagWAVEHDR {
    char        *lpData;
    uint32_t     dwBufferLength;
    uint32_t     dwBytesRecorded;
    uint32_t     dwUser;
    uint32_t     dwFlags;
    uint32_t     dwLoops;
} WAVEHDR;
typedef WAVEHDR *LPWAVEHDR;

/* Wave format descriptor */
typedef struct tWAVEFORMATEX {
    uint16_t     wFormatTag;
    uint16_t     nChannels;
    uint32_t     nSamplesPerSec;
    uint32_t     nAvgBytesPerSec;
    uint16_t     nBlockAlign;
    uint16_t     wBitsPerSample;
    uint16_t     cbSize;
} WAVEFORMATEX;

#define WAVE_FORMAT_PCM 1

/* FOURCC helpers */
#define mmioFOURCC(a,b,c,d) \
    ((uint32_t)(uint8_t)(a)         | \
     ((uint32_t)(uint8_t)(b) <<  8) | \
     ((uint32_t)(uint8_t)(c) << 16) | \
     ((uint32_t)(uint8_t)(d) << 24))
#define FOURCC_RIFF mmioFOURCC('R','I','F','F')

/* DWORD bit-field extraction */
#define LOWORD(l)       ((uint16_t)((uint32_t)(l) & 0xFFFF))
#define HIWORD(l)       ((uint16_t)(((uint32_t)(l) >> 16) & 0xFFFF))
#define MAKELONG(lo,hi) ((uint32_t)(uint16_t)(lo) | ((uint32_t)(uint16_t)(hi) << 16))

#endif /* __LINUX_COMPAT__ */
