// Copyright ©2008-2012 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __TOCHAR_H__
#define __TOCHAR_H__

#include <vcclr.h>

using namespace System;

/* This encapsulates converting and deleting a string so it may be done on a single line, with TOCHAR(string) */
typedef struct CharConvert
{
  inline explicit CharConvert(String^ str) { _str = (char*)System::Runtime::InteropServices::Marshal::StringToHGlobalUni(str).ToPointer(); }
  inline ~CharConvert() { System::Runtime::InteropServices::Marshal::FreeHGlobal((IntPtr)_str); }

  inline operator const char*() { return _str; }
protected:
  char* _str;
} TOCHAR;

#endif