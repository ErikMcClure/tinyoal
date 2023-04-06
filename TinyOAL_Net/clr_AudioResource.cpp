// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "clr_TinyOAL.h"
#include "tinyoal/AudioResource.h"
#include "TOCHAR.h"

using namespace TinyOAL_net;
using namespace tinyoal;

clr_AudioResource::clr_AudioResource(AudioResource* p) : _ref(p) { if(p) p->Grab(); }
clr_AudioResource::clr_AudioResource(System::String^ file)
{
  TOCHAR(file);
  _ref=AudioResource::Create((char*)pstr);
}
clr_AudioResource::clr_AudioResource(System::String^ file, CLR_TINYOAL_FLAG flags)
{
  TOCHAR(file);
  _ref = AudioResource::Create((char*)pstr, flags);
}
clr_AudioResource::clr_AudioResource(cli::array<System::Byte>^ data, CLR_TINYOAL_FLAG flags)
{
  pin_ptr<const unsigned char> pstr = &data[0];
  _ref = AudioResource::Create((const void*)pstr, flags);
}
clr_AudioResource::clr_AudioResource(System::String^ file, CLR_TINYOAL_FLAG flags, CLR_TINYOAL_FILETYPE filetype)
{
  TOCHAR(file);
  _ref = AudioResource::Create((char*)pstr, flags, filetype);
}
clr_AudioResource::clr_AudioResource(cli::array<System::Byte>^ data, CLR_TINYOAL_FLAG flags, CLR_TINYOAL_FILETYPE filetype)
{
  pin_ptr<const unsigned char> pstr = &data[0];
  _ref = AudioResource::Create((const void*)pstr, flags, filetype);
}
clr_AudioResource::~clr_AudioResource() { this->!clr_AudioResource(); }
clr_AudioResource::!clr_AudioResource() { if(_ref) _ref->Drop(); }
uint64_t clr_AudioResource::ToSamples(double seconds) { return !_ref?0:_ref->ToSamples(seconds); }
uint64_t clr_AudioResource::LoopPoint::get() { return !_ref?-1LL:_ref->GetLoopPoint(); }
void clr_AudioResource::LoopPoint::set(uint64_t loop) { if(_ref) _ref->SetLoopPoint(loop); }
CLR_TINYOAL_FLAG clr_AudioResource::Flags::get() { return !_ref?0:_ref->GetFlags(); }
void clr_AudioResource::Flags::set(CLR_TINYOAL_FLAG flags) { if(_ref) _ref->SetFlags(flags); }
CLR_TINYOAL_FILETYPE clr_AudioResource::FileType::get() { return !_ref ? 0 : _ref->GetFileType(); }
unsigned int clr_AudioResource::Frequency::get() { return !_ref?0:_ref->GetFreq(); }
unsigned int clr_AudioResource::Channels::get() { return !_ref?0:_ref->GetChannels(); }
unsigned int clr_AudioResource::Format::get() { return !_ref?0:_ref->GetFormat(); }
unsigned int clr_AudioResource::GetBufSize() { return !_ref?0:_ref->GetBufSize(); }
unsigned int clr_AudioResource::NumActive::get() { return !_ref?0:_ref->GetNumActive(); }
unsigned int clr_AudioResource::MaxActive::get() { return !_ref?0:_ref->GetMaxActive(); }
void clr_AudioResource::MaxActive::set(unsigned int max) { if(_ref) _ref->SetMaxActive(max); }
clr_Audio^ clr_AudioResource::Play(CLR_TINYOAL_FLAG flags) { return gcnew clr_Audio(!_ref?0:_ref->Play(flags)); }
clr_Audio^ clr_AudioResource::Play() { return gcnew clr_Audio(!_ref?0:_ref->Play()); }
