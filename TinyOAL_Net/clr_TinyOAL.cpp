// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "clr_Audio.h"
#include "clr_TinyOAL.h"
#include "TinyOAL.h"
#include <ostream>
#include "TOCHAR.h"

using namespace TinyOAL_net;

clr_TinyOAL::clr_TinyOAL()
{
  _ref = new TinyOAL::cTinyOAL(4);
}

clr_TinyOAL::clr_TinyOAL(int defaultbuffers)
{
  _ref = new TinyOAL::cTinyOAL(defaultbuffers);
}

clr_TinyOAL::clr_TinyOAL(int defaultbuffers, System::String^ logfile)
{
  TOCHAR(logfile)
  _ref = new TinyOAL::cTinyOAL(pstr, defaultbuffers);
}

clr_TinyOAL::~clr_TinyOAL()
{
  if(_ref) delete _ref;
}

int clr_TinyOAL::getDefaultBuffer()
{
  return _ref->getDefaultBuffer();
}
void clr_TinyOAL::setDefaultBuffer(int defaultbuffer)
{
  _ref->setDefaultBuffer(defaultbuffer);
}

unsigned int clr_TinyOAL::Update()
{
  return _ref->Update();
}

clr_Audio^ clr_TinyOAL::ManagedLoad(clr_AudioRef^ ref)
{
  return gcnew clr_Audio(ref,TinyOAL::TINYOAL_MANAGED);
}
