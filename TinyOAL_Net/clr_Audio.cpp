// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "clr_TinyOAL.h"
#include "cAudio.h"

using namespace TinyOAL_net;
using namespace TinyOAL;

clr_Audio::clr_Audio(clr_Audio^ copy) : _ref(new cAudio(*copy->_ref))
{
  _managed=(_ref->GetFlags()&TINYOAL_MANAGED)!=0;
}
clr_Audio::clr_Audio(clr_AudioResource^ ref, unsigned char addflags) : _ref(new cAudio(ref,addflags))
{ 
  _managed=(_ref->GetFlags()&TINYOAL_MANAGED)!=0;
}
clr_Audio::clr_Audio(clr_AudioResource^ ref) : _managed((ref->GetFlags()&TINYOAL_MANAGED)!=0), _ref(new cAudio(ref))
{
}
clr_Audio::clr_Audio(cAudio* p) : _managed((p->GetFlags()&TINYOAL_MANAGED)!=0), _ref(p)
{
}
clr_Audio::~clr_Audio() { this->!clr_Audio(); }
clr_Audio::!clr_Audio() { if(!_managed) delete _ref; }
bool clr_Audio::Update() { return _ref->Update(); }
bool clr_Audio::Play() { return _ref->Play(); }
void clr_Audio::Stop() { _ref->Stop(); }
void clr_Audio::Pause() { _ref->Pause(); }
bool clr_Audio::IsPlaying() { return _ref->IsPlaying(); }
bool clr_Audio::SkipSeconds(double seconds) { return _ref->SkipSeconds(seconds); }
bool clr_Audio::Skip(unsigned __int64 sample) { return _ref->Skip(sample); }
unsigned __int64 clr_Audio::IsWhere() { return _ref->IsWhere(); }
void clr_Audio::SetVolume(float range) { _ref->SetVolume(range); }
void clr_Audio::SetPitch(float range) { _ref->SetPitch(range); }
void clr_Audio::SetPosition(float X, float Y, float Z) { _ref->SetPosition(X,Y,Z); }
void clr_Audio::SetLoopPointSeconds(double seconds) { _ref->SetLoopPointSeconds(seconds); }
void clr_Audio::SetLoopPoint(unsigned __int64 sample) { _ref->SetLoopPoint(sample); }
CLR_TINYOAL_FLAG clr_Audio::GetFlags() { return _ref->GetFlags(); }
clr_AudioResource^ clr_Audio::GetResource() { return gcnew clr_AudioResource(_ref->GetResource()); }