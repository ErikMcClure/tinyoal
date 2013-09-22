// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "clr_TinyOAL.h"
#include "cAudio.h"

using namespace TinyOAL_net;
using namespace TinyOAL;

clr_Audio::clr_Audio(clr_Audio^ copy) : _ref(!copy->_ref?0:new cAudio(*copy->_ref))
{
  _managed=!_ref?false:((_ref->GetFlags()&TINYOAL_MANAGED)!=0);
}
clr_Audio::clr_Audio(clr_AudioResource^ ref, unsigned char addflags) : _ref(new cAudio(ref,addflags))
{ 
  _managed=(_ref->GetFlags()&TINYOAL_MANAGED)!=0;
}
clr_Audio::clr_Audio(clr_AudioResource^ ref) : _managed((ref->Flags&TINYOAL_MANAGED)!=0), _ref(new cAudio(ref))
{
}
clr_Audio::clr_Audio(cAudio* p) : _managed(!p?false:(p->GetFlags()&TINYOAL_MANAGED)!=0), _ref(p)
{
}
clr_Audio::~clr_Audio() { this->!clr_Audio(); }
clr_Audio::!clr_Audio() { if(!_managed) delete _ref; }
bool clr_Audio::Update() { return !_ref?false:_ref->Update(); }
bool clr_Audio::Play() { return !_ref?false:_ref->Play(); }
void clr_Audio::Stop() { if(_ref) _ref->Stop(); }
void clr_Audio::Pause() { if(_ref) _ref->Pause(); }
bool clr_Audio::Playing::get() { return !_ref?false:_ref->IsPlaying(); }
unsigned __int64 clr_Audio::Time::get() { return !_ref?0:_ref->IsWhere(); }
void clr_Audio::Time::set(unsigned __int64 sample) { if(_ref) _ref->Skip(sample); }
float clr_Audio::Volume::get() { return !_ref?0.0f:_ref->GetVolume(); }
void clr_Audio::Volume::set(float range) { if(_ref) _ref->SetVolume(range); }
float clr_Audio::Pitch::get() { return !_ref?0.0f:_ref->GetPitch(); }
void clr_Audio::Pitch::set(float range) { if(_ref) _ref->SetPitch(range); }
cli::array<float>^ clr_Audio::Position::get() { 
  cli::array<float>^ r = gcnew cli::array<float>(3); 
  if(!_ref) 
    return r; 
  const float* p=_ref->GetPosition();
  r[0]=p[0];
  r[1]=p[1];
  r[2]=p[2];
  return r;
}
void clr_Audio::Position::set(cli::array<float>^ range) { if(_ref) _ref->SetPosition(range[0],range[1],range[2]); }
unsigned __int64 clr_Audio::LoopPoint::get() { return !_ref?-1LL:_ref->GetLoopPoint(); }
void clr_Audio::LoopPoint::set(unsigned __int64 sample) { if(_ref) _ref->SetLoopPoint(sample); }
CLR_TINYOAL_FLAG clr_Audio::Flags::get() { return !_ref?0:_ref->GetFlags(); }

bool clr_Audio::SkipSeconds(double seconds) { return !_ref?false:_ref->SkipSeconds(seconds); }
void clr_Audio::SetLoopPointSeconds(double seconds) { _ref->SetLoopPointSeconds(seconds); }
clr_AudioResource^ clr_Audio::GetResource() { return gcnew clr_AudioResource(_ref->GetResource()); }