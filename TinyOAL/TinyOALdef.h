// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __TINYOALDEF_H__TOAL__
#define __TINYOALDEF_H__TOAL__

#define SAFE_DELETE(p) if(p) { delete p; p = 0; }
#define SAFE_DELETE_ARRAY(p) if(p) { delete [] p; p = 0; }

#endif