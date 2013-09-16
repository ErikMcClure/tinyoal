// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "clr_Audio.h"
#include "clr_TinyOAL.h"
#include "TinyOAL.h"
#include "TOCHAR.h"

using namespace TinyOAL_net;
using namespace TinyOAL;

clr_Audio::clr_Audio(System::String^ file, unsigned char flags) : _ref(new cAudio(TOCHAR(file),flags))
{
}

clr_Audio::clr_Audio(System::String^ data, unsigned char flags, bool isdata)
{
  if(isdata)
  {
    TOCHAR(data);
    _ref = new cAudio((void*)pstr, strlen(pstr), flags|TINYOAL_LOADINTOMEMORY);
  }
  else
    _ref = new cAudio(TOCHAR(data),flags);
}

clr_Audio::clr_Audio(clr_AudioRef^ ref, unsigned char addflags) : _ref(!ref?0:new cAudio(*ref->__getref(),addflags))
{
}
clr_Audio::clr_Audio(clr_AudioRef^ ref) : _ref(!ref?0:new cAudio(*ref->__getref()))
{
}
clr_Audio::~clr_Audio()
{
  if(_ref) delete _ref;
}

void clr_Audio::Update()
{
  _ref->Update();
}

bool clr_Audio::Play()
{
  return _ref->Play();
}

void clr_Audio::Stop()
{
  _ref->Stop();
}

void clr_Audio::Pause()
{
  _ref->Pause();
}

bool clr_Audio::IsPlaying()
{
  return _ref->IsPlaying();
}

bool clr_Audio::Skip(double seconds)
{
  return _ref->Skip(seconds);
}
bool clr_Audio::SkipSample(unsigned __int64 sample)
{
  return _ref->SkipSample(sample);
}

void clr_Audio::SetVolume(float range)
{
  _ref->SetVolume(range);
}

void clr_Audio::SetPitch(float range)
{
  _ref->SetPitch(range);
}

void clr_Audio::SetPosition(float X)
{
  _ref->SetPosition(X);
  
}
void clr_Audio::SetPosition(float X, float Y, float Z)
{
  _ref->SetPosition(X,Y,Z);
}

void clr_Audio::SetLoopPoint(double seconds)
{
  _ref->SetLoopPoint(seconds);
}

void clr_Audio::SetLoopPointSample(unsigned __int64 sample)
{
  _ref->SetLoopPointSample(sample);
}

int clr_Audio::GetFlags()
{ 
  return _ref->GetFlags();
}
clr_AudioRef^ clr_Audio::GetAudioRef()
{
  return gcnew clr_AudioRef(new cAudioRef(_ref->GetAudioRef()));
}

clr_AudioRef::clr_AudioRef(const clr_AudioRef% copy) : _ref(new cAudioRef(*copy._ref)) {}
clr_AudioRef::clr_AudioRef(TinyOAL::cAudioRef* ref) : _ref(ref) {}
clr_AudioRef::clr_AudioRef(System::String^ file, unsigned char flags) : _ref(new cAudioRef(TOCHAR(file),flags)) {}
clr_AudioRef::clr_AudioRef(System::String^ data, bool isdata, unsigned char flags)
{
  if(isdata)
  {
    TOCHAR(data);
    _ref = new cAudioRef((void*)pstr, strlen(pstr), flags|TINYOAL_LOADINTOMEMORY);
  }
  else
    _ref = new cAudioRef(TOCHAR(data),flags);
}
clr_AudioRef::~clr_AudioRef() { delete _ref; }
bool clr_AudioRef::IsValid() { return _ref->IsValid(); }
clr_AudioRef% clr_AudioRef::operator=(const clr_AudioRef% copy) { delete _ref; _ref = new cAudioRef(*copy._ref); return *this; }