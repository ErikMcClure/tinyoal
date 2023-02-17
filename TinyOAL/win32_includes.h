// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __WIN32_INCLUDES_H__
#define __WIN32_INCLUDES_H__

#pragma pack(push)
#pragma pack(8)
#define WINVER        0x0601 //_WIN32_WINNT_WIN7
#define _WIN32_WINNT  0x0601
#define NTDDI_VERSION 0x06010000 // NTDDI_WIN7
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX // Some compilers enable this by default
  #define NOMINMAX
#endif
#define NODRAWTEXT
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#include <windows.h>
#pragma pack(pop)

#endif