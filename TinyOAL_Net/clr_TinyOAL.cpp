// Copyright ©2013 Black Sphere Studios
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
clr_TinyOAL::clr_TinyOAL(int defaultbuffers, System::String^ logfile, System::String^ forceOAL, System::String^ forceOGG, System::String^ forceFLAC, System::String^ forceMP3)
{
  pin_ptr<const unsigned char> plog = &System::Text::Encoding::UTF8->GetBytes(logfile)[0];
  pin_ptr<const unsigned char> poal = &System::Text::Encoding::UTF8->GetBytes(forceOAL)[0];
  pin_ptr<const unsigned char> pogg = &System::Text::Encoding::UTF8->GetBytes(forceOGG)[0];
  pin_ptr<const unsigned char> pflac = &System::Text::Encoding::UTF8->GetBytes(forceFLAC)[0];
  pin_ptr<const unsigned char> pmp3 = &System::Text::Encoding::UTF8->GetBytes(forceMP3)[0];
  _ref=new cTinyOAL((char*)plog,defaultbuffers,(char*)poal,(char*)pogg,(char*)pflac,(char*)pmp3);
}
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
unsigned int clr_TinyOAL::GetFormat(unsigned short channels, unsigned short bits, bool rear) { return cTinyOAL::GetFormat(channels,bits,rear); }
// Given a file or stream, creates or overwrites the openal config file in the proper magical location (%APPDATA% on windows)
void clr_TinyOAL::SetSettings(System::String^ file)
{
  TOCHAR(file);
  cTinyOAL::SetSettings((const char*)pstr);
}
void clr_TinyOAL::SetSettingsStream(System::String^ data)
{
  TOCHAR(data);
  cTinyOAL::SetSettingsStream((const char*)pstr);
}