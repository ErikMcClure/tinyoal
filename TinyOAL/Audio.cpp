// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "tinyoal/Audio.h"
#include "tinyoal/AudioResource.h"
#include "loadoal.h"
#include "tinyoal/TinyOAL.h"
#include "Engine.h"

using namespace tinyoal;

Audio::Audio(const Audio& copy)
{
  memcpy(this, &copy, sizeof(Audio));
  _flags -= TINYOAL_MANAGED; // Any copying can't be managed
  _stream = nullptr;
  prev    = nullptr;
  next    = nullptr;
  _source = nullptr;

  if(!_resource)
  {
    TINYOAL_LOG(2, "NULL AudioResource passed to Audio");
    return;
  }

  _resource->Grab();
  bun::LLAdd<Audio>(this, _resource->_inactivelist);
  _source = TinyOAL::Instance()->GetEngine()->GenSource(&ReadBuffer, _resource->GetBufSize(), _resource->GetFormat(),
                                                        _resource->GetFreq());

  _stream = (!_source) ? nullptr : _resource->OpenStream(); // If we don't have oalFuncs, force stream to 0
  if(_stream != nullptr)
  { // Allocate a buffer to be used to store decoded data for all Buffers
    Skip(copy.IsWhere());

    // Fill all the Buffers with decoded audio data
    _source->FillBuffers(this);
  }

  if(_flags & TINYOAL_ISPLAYING)
  {
    _flags -= TINYOAL_ISPLAYING;
    Play();
  }
}
Audio::Audio(AudioResource* ref, TINYOAL_FLAG addflags, void* _userdata) :
  _looptime(-1LL),
  _flags(addflags),
  _pitch(1.0f),
  _vol(1.0f),
  _stream(nullptr),
  _source(nullptr),
  _resource(ref),
  userdata(_userdata)
{
  _pos[0] = 0.0f;
  _pos[1] = 0.0f;
  _pos[2] = 0.5f;
  prev    = nullptr;
  next    = nullptr;

  if(!ref)
  {
    TINYOAL_LOG(2, "NULL AudioResource passed to Audio");
    return;
  }

  ref->Grab();
  _looptime = ref->GetLoopPoint();
  _flags += ref->GetFlags();
  bun::LLAdd<Audio>(this, ref->_inactivelist);
  _source = TinyOAL::Instance()->GetEngine()->GenSource(&ReadBuffer, _resource->GetBufSize(), _resource->GetFormat(),
                                                        _resource->GetFreq());

  _stream = (!_source) ? nullptr : ref->OpenStream(); // If we don't have oalFuncs, force stream to 0
  if(_stream != nullptr)
  {
    // Fill all the Buffers with decoded audio data
    _source->FillBuffers(this);
  }
  else
    TINYOAL_LOG(2, "Stream request failed");

  if(_flags & TINYOAL_ISPLAYING)
  {
    _flags -= TINYOAL_ISPLAYING;
    Play();
  }
}

Audio::~Audio()
{
  _flags -= TINYOAL_MANAGED; // Remove the flag first, which prevents us from going into an infinite loop
  Stop();                    // Stop destroys the source for us and ensures we are in the inactive list

  if(_stream)
  {
    if(_resource)
      _resource->CloseStream(_stream);
  }

  if(_resource)
  {
    bun::LLRemove<Audio>(this, _resource->_inactivelist);
    _resource->Drop();
  }

  if(_source)
    TinyOAL::Instance()->GetEngine()->DestroySource(_source);
}

Audio& Audio::operator=(const Audio& copy)
{
  if(this != &copy)
  {
    this->~Audio();
    new(this) Audio(copy);
  }
  return *this;
}
Audio& Audio::operator=(Audio&& mov)
{
  if(this != &mov)
  {
    this->~Audio();
    new(this) Audio(std::move(mov));
  }
  return *this;
}

bool Audio::Play()
{
  if(!_stream)
    return false;

  if(_source)
    _source->Play(_vol, _pitch, _pos);

  if(!(_flags & TINYOAL_ISPLAYING) && _resource != 0)
    TinyOAL::Instance()->_addAudio(this, _resource);
  _flags += TINYOAL_ISPLAYING;

  return true;
}

void Audio::Stop()
{
  if(_source)
    _source->Stop();

  if(_stream != 0)
  {
    _resource->Reset(_stream);
    _source->FillBuffers(this); // Refill all buffers
  }

  _stop();

  if(_flags & TINYOAL_MANAGED)
  { // If we're managed and we stopped playing, destroy ourselves.
    this->~Audio();
    TinyOAL::Instance()->_allocaudio.deallocate(this, 1);
  }
}

void Audio::Pause()
{
  if(_source->IsStreaming())
    _source->Pause();
  _stop();
}

bool Audio::SkipSeconds(double seconds)
{
  if(!_resource)
    return false;
  return Skip(_resource->ToSamples(seconds));
}
bool Audio::Skip(uint64_t sample)
{
  if(!_resource || !_stream)
    return false;
  if(!_resource->Skip(_stream, sample))
    return false;

  _source->Skip(this);
  if(_flags & TINYOAL_ISPLAYING)
    _source->Play(_vol, _pitch, _pos);
  return true;
}

uint64_t Audio::IsWhere() const
{
  if(!_resource || !_stream)
    return 0;
  return _resource->Tell(_stream);
}

void Audio::SetLoopPointSeconds(double seconds)
{
  if(_resource != 0)
    _looptime = _resource->ToSamples(seconds);
}

void Audio::SetLoopPoint(uint64_t samples) { _looptime = samples; }

bool Audio::Update()
{
  if(!_resource || !_source) // Do we have a valid stream
    return false;

  if(_source->Update(this, _flags & TINYOAL_ISPLAYING))
    return true;

  Stop();
  return false;
}
void Audio::Invalidate()
{
  if(_stream && _resource)
    _resource->CloseStream(_stream);
  _stream   = 0;
  _resource = 0;
}

void Audio::_stop()
{
  if((_flags & TINYOAL_ISPLAYING) != 0 && _resource != 0)
    TinyOAL::Instance()->_removeAudio(this, _resource);
  _flags -= TINYOAL_ISPLAYING;
}

bool Audio::IsPlaying() const { return (_flags & TINYOAL_ISPLAYING) != 0; }

void Audio::SetVolume(float range)
{
  if(range >= 0.0f)
    _vol = range;
  if(_source)
    _source->SetVolume(_vol);
}

void Audio::SetPitch(float range)
{
  if(range >= 0.5f && range <= 2.0f)
    _pitch = range;
  if(_source)
    _source->SetPitch(_pitch);
}

void Audio::SetPosition(float X, float Y, float Z)
{
  _pos[0] = X;
  _pos[1] = Y;
  _pos[2] = Z;
  if(_source)
    _source->SetPosition(_pos);
}

unsigned long Audio::_readBuffer(unsigned long bufsize, char* buffer)
{
  bool eof;
  unsigned long hold;
  unsigned long ulBytesWritten = _resource->Read(_stream, buffer, bufsize, eof);
  if(eof && _looptime != (uint64_t)-1)
  {
    while(eof && ulBytesWritten <
                   bufsize) // If we didn't completely fill up our buffer, we hit the end, so if we're looping, reset.
    { // We reset the stream here for every loop, because we will only loop if we hit the end of the stream.
      _resource->Skip(_stream,
                      _looptime); // This is because if we didn't hit the end, Read will return _bufsize-ulBytesWritten,
      hold = _resource->Read(_stream, buffer + ulBytesWritten, bufsize - ulBytesWritten,
                             eof); // which will make ulBytesWritten==_bufsize.
      if(!hold)
        break; // If Read returns 0 AFTER we attempted to go back to the loop location, something is wrong, so bail out.
      ulBytesWritten += hold;
    }
  }
  return ulBytesWritten;
}

unsigned long Audio::ReadBuffer(unsigned long bufsize, char* buffer, void* context)
{
  auto audio = (Audio*)context;
  return audio->_readBuffer(bufsize, buffer);
}
