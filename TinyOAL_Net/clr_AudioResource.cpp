// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "clr_TinyOAL.h"
#include "cAudioResource.h"
#include "TOCHAR.h"

using namespace TinyOAL_net;
using namespace TinyOAL;

clr_AudioResource::clr_AudioResource(cAudioResource* p) : _ref(p) { p->Grab(); }
clr_AudioResource::clr_AudioResource(System::String^ file)
{
  TOCHAR(file);
  _ref=cAudioResource::Create((char*)pstr);
}
clr_AudioResource::clr_AudioResource(System::String^ file, CLR_TINYOAL_FLAG flags)
{
  TOCHAR(file);
  _ref=cAudioResource::Create((char*)pstr,flags);
}
clr_AudioResource::clr_AudioResource(cli::array<System::Byte>^ data, CLR_TINYOAL_FLAG flags)
{
  pin_ptr<const unsigned char> pstr = &data[0];
  _ref=cAudioResource::Create((const void*)pstr,flags);
}
clr_AudioResource::~clr_AudioResource() { this->!clr_AudioResource(); }
clr_AudioResource::!clr_AudioResource() { _ref->Drop(); }
unsigned __int64 clr_AudioResource::ToSample(double seconds) { return _ref->ToSample(seconds); }
unsigned __int64 clr_AudioResource::GetLoopPoint() { return _ref->GetLoopPoint(); }
void clr_AudioResource::SetLoopPoint(unsigned __int64 loop) { _ref->SetLoopPoint(loop); }
CLR_TINYOAL_FLAG clr_AudioResource::GetFlags() { return _ref->GetFlags(); }
void clr_AudioResource::SetFlags(CLR_TINYOAL_FLAG flags) { _ref->SetFlags(flags); }
unsigned int clr_AudioResource::GetFreq() { return _ref->GetFreq(); }
unsigned int clr_AudioResource::GetChannels() { return _ref->GetChannels(); }
unsigned int clr_AudioResource::GetFormat() { return _ref->GetFormat(); }
unsigned int clr_AudioResource::GetBufSize() { return _ref->GetBufSize(); }
unsigned int clr_AudioResource::GetNumActive() { return _ref->GetNumActive(); }
unsigned int clr_AudioResource::GetMaxActive() { return _ref->GetMaxActive(); }
void clr_AudioResource::SetMaxActive(unsigned int max) { _ref->SetMaxActive(max); }
clr_Audio^ clr_AudioResource::Play(CLR_TINYOAL_FLAG flags) { return gcnew clr_Audio(_ref->Play(flags)); }
clr_Audio^ clr_AudioResource::Play() { return gcnew clr_Audio(_ref->Play()); }
