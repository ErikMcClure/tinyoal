// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "tinyoal/Audio.h"
#include "tinyoal/AudioResource.h"
#include "openAL/loadoal.h"
#include "tinyoal/TinyOAL.h"

using namespace tinyoal;

Audio::Audio(const Audio& copy)
{
  memcpy(this,&copy,sizeof(Audio));
  uiSource=-1;
  _flags-=TINYOAL_MANAGED; // Any copying can't be managed
  _stream=0;
  pDecodeBuffer=0;
  uiBuffers = (ALuint*)TinyOAL::Instance()->_bufalloc.alloc(1);
  unsigned char nbuffers=TinyOAL::Instance()->defNumBuf;
  memset(uiBuffers,0,sizeof(ALuint)*nbuffers);
  prev = 0;
  next = 0;

  if(!_source) { TINYOAL_LOG(2,"NULL AudioResource passed to Audio"); return; }
  _source->Grab();
  bss::LLAdd<Audio>(this,_source->_inactivelist);
  
  _stream = (!TinyOAL::Instance()->oalFuncs)?0:_source->OpenStream(); // If we don't have oalFuncs, force stream to 0
  if(_stream!=0)
  { // Allocate a buffer to be used to store decoded data for all Buffers
    pDecodeBuffer = TinyOAL::Instance()->_allocDecoder(_bufsize=_source->GetBufSize());
    Skip(copy.IsWhere());

	  if(pDecodeBuffer!=0)
    { // Generate some AL Buffers for streaming
      TinyOAL::Instance()->oalFuncs->alGenBuffers(nbuffers, uiBuffers);
      _fillBuffers(); // Fill all the Buffers with decoded audio data
    }
    else
      TINYOAL_LOG(1,"Failed to allocate memory for decoded audio data");
  }

  if(_flags&TINYOAL_ISPLAYING) {
    _flags-=TINYOAL_ISPLAYING;
    Play();
  }
}
Audio::Audio(Audio&& mov)
{
  memcpy(this,&mov,sizeof(Audio));
  uiBuffers = (ALuint*)TinyOAL::Instance()->_bufalloc.alloc(1);
  memcpy(uiBuffers,mov.uiBuffers,sizeof(ALuint)*TinyOAL::Instance()->defNumBuf);
  if(prev) prev->next=this;
  if(next) next->prev=this;
  mov.pDecodeBuffer=0;
  mov.uiSource=-1;
  mov._source=0;
  mov._stream=0;
  mov.ONDESTROY=0;
  mov._flags=mov._flags&TINYOAL_MANAGED;
  mov.Stop(); // This will destroy the source of the mov if it was managed (otherwise it'd never get destroyed)
  _flags-=TINYOAL_MANAGED; // Any moving can't be managed
}
Audio::Audio(AudioResource* ref, TINYOAL_FLAG addflags, void* _userdata) : _looptime(-1LL), _flags(addflags), _pitch(1.0f), _vol(1.0f),
  uiSource(-1), pDecodeBuffer(0),_stream(0),_source(ref), userdata(_userdata), ONDESTROY(0), _bufsize(0)
{
  _pos[0] = 0.0f;
  _pos[1] = 0.0f;
  _pos[2] = 0.5f;
  uiBuffers = (ALuint*)TinyOAL::Instance()->_bufalloc.alloc(1);
  unsigned char nbuffers=TinyOAL::Instance()->defNumBuf;
  memset(uiBuffers,0,sizeof(ALuint)*nbuffers);
  prev = 0;
  next = 0;

  if(!ref) { TINYOAL_LOG(2,"NULL AudioResource passed to Audio"); return; }
  ref->Grab();
  _looptime=ref->GetLoopPoint();
  _flags+=ref->GetFlags();
  bss::LLAdd<Audio>(this,ref->_inactivelist);

  _stream = (!TinyOAL::Instance()->oalFuncs)?0:ref->OpenStream(); // If we don't have oalFuncs, force stream to 0
  if(_stream!=0)
  { // Allocate a buffer to be used to store decoded data for all Buffers
    pDecodeBuffer = TinyOAL::Instance()->_allocDecoder(_bufsize=ref->GetBufSize());

	  if(pDecodeBuffer!=0)
    { // Generate some AL Buffers for streaming
      TinyOAL::Instance()->oalFuncs->alGenBuffers(nbuffers, uiBuffers);
      _fillBuffers(); // Fill all the Buffers with decoded audio data
    }
    else
      TINYOAL_LOG(1,"Failed to allocate memory for decoded audio data");
  }
  else
    TINYOAL_LOG(2,"Stream request failed");


  if(_flags&TINYOAL_ISPLAYING) {
    _flags-=TINYOAL_ISPLAYING;
    Play();
  }
}

Audio::~Audio()
{
  if(ONDESTROY) (*ONDESTROY)(this);
  _flags-=TINYOAL_MANAGED; // Remove the flag first, which prevents us from going into an infinite loop
  Stop(); // Stop destroys the source for us and ensures we are in the inactive list

  if(_stream) 
  {
    TinyOAL::Instance()->oalFuncs->alDeleteBuffers(TinyOAL::Instance()->defNumBuf, uiBuffers);
    _source->CloseStream(_stream);
  }

  if(pDecodeBuffer) TinyOAL::Instance()->_deallocDecoder(pDecodeBuffer, _bufsize);
  TinyOAL::Instance()->_bufalloc.dealloc(uiBuffers);
  if(_source)
  {
    bss::LLRemove<Audio>(this,_source->_inactivelist);
    _source->Drop();
  }
}

bool Audio::Play()
{
  if(!_stream) return false;
  _getSource();

  if(!_streaming())
    TinyOAL::Instance()->oalFuncs->alSourcePlay(uiSource);
  
  if(!(_flags&TINYOAL_ISPLAYING) && _source!=0)
    TinyOAL::Instance()->_addAudio(this,_source);
  _flags += TINYOAL_ISPLAYING;

  return _streaming();
}

void Audio::Stop()
{
  if(_streaming())
    TinyOAL::Instance()->oalFuncs->alSourceStop(uiSource);
  if(uiSource!=(unsigned int)-1)
  {
    TinyOAL::Instance()->oalFuncs->alSourcei(uiSource, AL_BUFFER, 0); // Detach buffer
	  TinyOAL::Instance()->oalFuncs->alDeleteSources(1, &uiSource);
    uiSource=(unsigned int)-1;
  }
  if(_stream!=0) 
  {
    _source->Reset(_stream);
    _fillBuffers(); //Refill all buffers
  }
  _stop();
  if(_flags&TINYOAL_MANAGED) { // If we're managed and we stopped playing, destroy ourselves.
    this->~Audio();
    if(_source) _source->_allocaudio.dealloc(this);
  }
}

void Audio::Pause()
{
  if(_streaming())
    TinyOAL::Instance()->oalFuncs->alSourcePause(uiSource);
  _stop();
}

bool Audio::SkipSeconds(double seconds)
{
  if(!_source) return false;
  return Skip(_source->ToSamples(seconds));
}
bool Audio::Skip(uint64_t sample)
{
  if(!_source || !_stream) return false;
  if(!_source->Skip(_stream,sample)) return false;
  if(uiSource!=(unsigned int)-1) // We have to check this instead of whether or not it's playing because it could be paused
  {
    TinyOAL::Instance()->oalFuncs->alSourceStop(uiSource); // Stop no matter what in case it's paused, because we have to reset it.
    TinyOAL::Instance()->oalFuncs->alSourcei(uiSource, AL_BUFFER, 0); // Detach buffer
    _fillBuffers(); //Refill all buffers
    _queueBuffers(); //requeue everything, which forces the audio to immediately skip.
    if(_flags&TINYOAL_ISPLAYING) TinyOAL::Instance()->oalFuncs->alSourcePlay(uiSource);
  }
  else
    _fillBuffers(); // If we don't have a source, just refill the buffers and don't do anything else

  return true;
}

uint64_t Audio::IsWhere() const
{
  if(!_source || !_stream) return 0;
  ALint offset=0;
  TinyOAL::Instance()->oalFuncs->alGetSourcei(uiSource, AL_SAMPLE_OFFSET, &offset);
  return _source->Tell(_stream)-(TinyOAL::Instance()->defNumBuf*(_bufsize/(_source->GetChannels()*(_source->GetBitsPerSample()>>3))))+offset;
}

void Audio::SetLoopPointSeconds(double seconds)
{
  if(_source!=0)
    _looptime = _source->ToSamples(seconds);
}

void Audio::SetLoopPoint(uint64_t samples)
{
  _looptime = samples;
}

bool Audio::Update()
{
  if(!_source) //Do we have a valid stream
    return false;

  _processBuffers(); //this must be first

  if(!_streaming() && (_flags&TINYOAL_ISPLAYING)) // If we aren't playing but should be uiSource *must* be valid because Play() was called.
  {
    ALint iQueuedBuffers;
    TinyOAL::Instance()->oalFuncs->alGetSourcei(uiSource, AL_BUFFERS_QUEUED, &iQueuedBuffers);
    if(!iQueuedBuffers)
    {
      Stop();
      return false;
    }

    TinyOAL::Instance()->oalFuncs->alSourcePlay(uiSource); //The audio device was starved for data so we need to restart it
  }

  return true;
}
void Audio::Invalidate()
{
  if(_stream && _source) _source->CloseStream(_stream);
  _stream=0;
  _source=0;
}

void Audio::_stop()
{
  if((_flags&TINYOAL_ISPLAYING)!=0 && _source!=0)
    TinyOAL::Instance()->_removeAudio(this,_source);
  _flags -= TINYOAL_ISPLAYING;
}

void Audio::_getSource()
{
  if(uiSource==(unsigned int)-1) // if uiSource is invalid we need to grab a new one.
  {
    TinyOAL::Instance()->oalFuncs->alGetError(); // Clear last error
    TinyOAL::Instance()->oalFuncs->alGenSources(1, &uiSource);
    if(TinyOAL::Instance()->oalFuncs->alGetError() != AL_NO_ERROR)
    {
      TINYOAL_LOG(1,"Failed to generate source!");
      //TODO steal source from other audio instead
    }

    _applyAll(); // Make sure we've applied everything
    _queueBuffers();
  }
}

void Audio::_queueBuffers()
{
  unsigned char nbuffers=TinyOAL::Instance()->defNumBuf; // Queue everything
  _queuebuflen+=_bufstart;
  for(ALint i = _bufstart; i < _queuebuflen; ++i) // Queues all waiting buffers in the correct order.
    TinyOAL::Instance()->oalFuncs->alSourceQueueBuffers(uiSource, 1, &uiBuffers[i%nbuffers]);
  _queuebuflen=0;
}
void Audio::_fillBuffers()
{
  _bufstart=0;
  _queuebuflen=0;
  unsigned long ulBytesWritten;
  unsigned char nbuffers=TinyOAL::Instance()->defNumBuf;
  for (ALint i = 0; i < nbuffers; i++)
  {
    ulBytesWritten = _readBuffer();
	  if(ulBytesWritten)
      TinyOAL::Instance()->oalFuncs->alBufferData(uiBuffers[_queuebuflen++], _source->GetFormat(), pDecodeBuffer, ulBytesWritten, _source->GetFreq());
  }
}

unsigned long Audio::_readBuffer()
{
  bool eof;
  unsigned long hold;
	unsigned long ulBytesWritten = _source->Read(_stream, pDecodeBuffer, _bufsize, eof);
  if(eof && _looptime!=(uint64_t)-1)
  {
    while(eof && ulBytesWritten<_bufsize) // If we didn't completely fill up our buffer, we hit the end, so if we're looping, reset.
    { // We reset the stream here for every loop, because we will only loop if we hit the end of the stream.
      _source->Skip(_stream,_looptime); // This is because if we didn't hit the end, Read will return _bufsize-ulBytesWritten,
      hold=_source->Read(_stream, pDecodeBuffer+ulBytesWritten, _bufsize-ulBytesWritten, eof); // which will make ulBytesWritten==_bufsize.
      if(!hold) break; // If Read returns 0 AFTER we attempted to go back to the loop location, something is wrong, so bail out.
      ulBytesWritten += hold;
    }
  }
  return ulBytesWritten;
}

void Audio::_processBuffers()
{
  // Request the number of OpenAL Buffers have been processed (played) on the Source
	ALint iBuffersProcessed = 0;
	TinyOAL::Instance()->oalFuncs->alGetSourcei(uiSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed);
  ALuint uiBuffer;
  unsigned long ulBytesWritten;

	// For each processed buffer, remove it from the Source Queue, read next chunk of audio
	// data from disk, fill buffer with new data, and add it to the Source Queue
	while (iBuffersProcessed)
	{
		// Remove the Buffer from the Queue.  (uiBuffer contains the Buffer ID for the unqueued Buffer)
		uiBuffer = 0;
		TinyOAL::Instance()->oalFuncs->alSourceUnqueueBuffers(uiSource, 1, &uiBuffer);

		ulBytesWritten=_readBuffer(); // Read more audio data (if there is any)
    
		if(ulBytesWritten)
		{
      TinyOAL::Instance()->oalFuncs->alBufferData(uiBuffer, _source->GetFormat(), pDecodeBuffer, ulBytesWritten, _source->GetFreq());
			TinyOAL::Instance()->oalFuncs->alSourceQueueBuffers(uiSource, 1, &uiBuffer);
		}

    iBuffersProcessed--;
	}
}

bool Audio::_streaming() const
{
  if(!TinyOAL::Instance()->oalFuncs || uiSource==(unsigned int)-1) return false;
  int iState=0;
	TinyOAL::Instance()->oalFuncs->alGetSourcei(uiSource, AL_SOURCE_STATE, &iState); // if uiSource is invalid or this fails for any reason, iState will remain at 0
	return iState == AL_PLAYING;
}

bool Audio::IsPlaying() const
{
  return (_flags&TINYOAL_ISPLAYING)!=0;
}

void Audio::SetVolume(float range)
{
  if(range >= 0.0f)
    _vol = range;
  if(uiSource != (unsigned int)-1) 
    TinyOAL::Instance()->oalFuncs->alSourcef(uiSource, AL_GAIN, _vol);
}

void Audio::SetPitch(float range)
{
  if(range >= 0.5f && range <= 2.0f)
    _pitch = range;
  if(uiSource != -1) 
    TinyOAL::Instance()->oalFuncs->alSourcef(uiSource, AL_PITCH, _pitch);
}

void Audio::SetPosition(float X, float Y, float Z)
{
  _pos[0] = X;
  _pos[1] = Y;
  _pos[2] = Z;
  if(uiSource != (unsigned int)-1)
    TinyOAL::Instance()->oalFuncs->alSourcefv(uiSource, AL_POSITION, _pos);
}

void Audio::_applyAll()
{
  if(uiSource != (unsigned int)-1)
  {
    TinyOAL::Instance()->oalFuncs->alSourcefv(uiSource, AL_POSITION, _pos);
    TinyOAL::Instance()->oalFuncs->alSourcef(uiSource, AL_PITCH, _pitch);
    TinyOAL::Instance()->oalFuncs->alSourcef(uiSource, AL_GAIN, _vol);
  }
}