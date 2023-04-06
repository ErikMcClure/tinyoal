// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef TOAL__DLLDEF_H__
#define TOAL__DLLDEF_H__

#define TINYOAL_VERSION_MAJOR    1
#define TINYOAL_VERSION_MINOR    1
#define TINYOAL_VERSION_REVISION 1

#ifndef _WINRESRC_

  #include "bss-util/compiler.h"

  #ifdef TINYOAL_EXPORTS
    #define TINYOAL_DLLEXPORT BSS_COMPILER_DLLEXPORT
  #else
    #ifndef TINYOAL_STATICLIB
      #define TINYOAL_DLLEXPORT BSS_COMPILER_DLLIMPORT
    #else
      #define TINYOAL_DLLEXPORT
    #endif
  #endif

#endif

#endif
