// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "clr_TinyOAL.h"
#include "cTinyOAL.h"
#include <ostream>
#include "TOCHAR.h"

using namespace TinyOAL_net;
using namespace TinyOAL;

clr_TinyOAL::clr_TinyOAL() : _ref(new cTinyOAL()) {}
clr_TinyOAL::clr_TinyOAL(int defaultbuffers) : _ref(new cTinyOAL(defaultbuffers)) {}
clr_TinyOAL::clr_TinyOAL(int defaultbuffers, System::String^ logfile) { TOCHAR(logfile); _ref=new cTinyOAL((char*)pstr,defaultbuffers); }
clr_TinyOAL::~clr_TinyOAL() { this->!clr_TinyOAL(); }
clr_TinyOAL::!clr_TinyOAL() { delete _ref; }
unsigned int clr_TinyOAL::Update() { return _ref->Update(); }
System::String^ clr_TinyOAL::GetDefaultDevice() { return gcnew System::String(_ref->GetDefaultDevice()); }
bool clr_TinyOAL::SetDevice(System::String^ device) { TOCHAR(device); return _ref->SetDevice((char*)pstr); }
cli::array<System::String^>^ clr_TinyOAL::GetDevices()
{
  const char* cur = _ref->GetDevices();
  int sz=0;
  for(const char* i=cur; *i!=0; i+=(strlen(i)+1)) ++sz;
  cli::array<System::String^>^ r = gcnew cli::array<System::String^>(sz);
  for(int i = 0; i < sz; ++i)
  {
    r[i]=gcnew System::String(cur);
    cur+=strlen(cur)+1;
  }
  return r;
}