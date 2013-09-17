// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudio.h"
#include "cAudioResource.h"
#include "openAL/loadoal.h"
#include "cTinyOAL.h"

using namespace TinyOAL;

cAudio::cAudio(const cAudio& copy)
{
}
cAudio::cAudio(cAudio&& mov)
{
  memcpy(this,&mov,sizeof(cAudio));
  mov.uiSource=-1;
  mov._source=0;
}
cAudio::cAudio(cAudioResource* ref, TINYOAL_FLAG addflags, void* _userdata) : _looptime(-1LL), _flags(addflags), _pitch(1.0f), _vol(1.0f),
  uiSource(-1), pDecodeBuffer(0),_stream(0),_source(ref), userdata(_userdata), ONDESTROY(0), _bufsize(0)
{
  _pos[0] = 0.0f;
  _pos[1] = 1.0f;
  _pos[2] = 1.0f;
  uiBuffers = (ALuint*)cTinyOAL::Instance()->_bufalloc.alloc(1);
  unsigned char nbuffers=cTinyOAL::Instance()->defNumBuf;
  memset(uiBuffers,0,sizeof(ALuint)*nbuffers);
  prev = 0;
  next = 0;

  if(!ref) return;
  ref->Grab();
  _looptime=ref->GetLoopPoint();
  _flags+=ref->GetFlags();
  bss_util::LLAdd<cAudio>(this,ref->_inactivelist);

  _stream = (!cTinyOAL::Instance()->oalFuncs)?0:ref->OpenStream(); // If we don't have oalFuncs, force stream to 0
  if(_stream!=0)
  { // Allocate a buffer to be used to store decoded data for all Buffers
    pDecodeBuffer = cTinyOAL::Instance()->_allocdecoder(_bufsize=ref->GetBufSize());

	  if(pDecodeBuffer!=0)
    { // Generate some AL Buffers for streaming
      cTinyOAL::Instance()->oalFuncs->alGenBuffers(nbuffers, uiBuffers);
      _fillbuffers(); // Fill all the Buffers with decoded audio data
    }
    else
      TINYOAL_LOGM("ERROR","Failed to allocate memory for decoded audio data");
  }

  if(_flags&TINYOAL_ISPLAYING) {
    _flags-=TINYOAL_ISPLAYING;
    Play();
  }
}

cAudio::~cAudio()
{
  if(ONDESTROY) (*ONDESTROY)(this);
  _flags-=TINYOAL_MANAGED; // Remove the flag first, which prevents us from going into an infinite loop
  Stop(); // Stop destroys the source for us and ensures we are in the inactive list

  if(_stream) 
  {
    cTinyOAL::Instance()->oalFuncs->alDeleteBuffers(cTinyOAL::Instance()->defNumBuf, uiBuffers);
    _source->CloseStream(_stream);
  }

  if(pDecodeBuffer) cTinyOAL::Instance()->_deallocdecoder(pDecodeBuffer, _bufsize);
  cTinyOAL::Instance()->_bufalloc.dealloc(uiBuffers);
  if(_source)
  {
    bss_util::LLRemove<cAudio>(this,_source->_inactivelist);
    _source->Drop();
  }
}

bool cAudio::Play()
{
  if(!_stream) return false;
  _getsource();

  if(!_streaming())
    cTinyOAL::Instance()->oalFuncs->alSourcePlay(uiSource);
  
  if(!(_flags&TINYOAL_ISPLAYING) && _source!=0)
    cTinyOAL::Instance()->_addaudio(this,_source);
  _flags += TINYOAL_ISPLAYING;

  return _streaming();
}

void cAudio::Stop()
{
  if(_streaming())
    cTinyOAL::Instance()->oalFuncs->alSourceStop(uiSource);
  if(uiSource!=(unsigned int)-1)
  {
    cTinyOAL::Instance()->oalFuncs->alSourcei(uiSource, AL_BUFFER, 0); // Detach buffer
	  cTinyOAL::Instance()->oalFuncs->alDeleteSources(1, &uiSource);
    uiSource=(unsigned int)-1;
  }
  if(_stream!=0) 
  {
    _source->Reset(_stream);
    _fillbuffers(); //Refill all buffers
  }
  _stop();
  if(_flags&TINYOAL_MANAGED) { // If we're managed and we stopped playing, destroy ourselves.
    this->~cAudio();
    if(_source) _source->_allocaudio.dealloc(this);
  }
}

void cAudio::Pause()
{
  if(_streaming())
    cTinyOAL::Instance()->oalFuncs->alSourcePause(uiSource);
  _stop();
}

bool cAudio::SkipSeconds(double seconds)
{
  if(!_source) return false;
  return Skip(_source->ToSample(seconds));
}
bool cAudio::Skip(unsigned __int64 sample)
{
  if(!_source || !_stream) return false;
  if(!_source->Skip(_stream,sample)) return false;
  if(_streaming())
  {
    cTinyOAL::Instance()->oalFuncs->alSourceStop(uiSource);
    cTinyOAL::Instance()->oalFuncs->alSourcei(uiSource, AL_BUFFER, 0); // Detach buffer
    _fillbuffers(); //Refill all buffers
    _queuebuffers(); //requeue everything, which forces the audio to immediately skip.
    cTinyOAL::Instance()->oalFuncs->alSourcePlay(uiSource);
  }
  return true;
}

void cAudio::SetLoopPointSeconds(double seconds)
{
  if(_source!=0)
    _looptime = _source->ToSample(seconds);
}

void cAudio::SetLoopPoint(unsigned __int64 samples)
{
  _looptime = samples;
}

bool cAudio::Update()
{
  if(!_source) //Do we have a valid stream
    return false;

  _processbuffers(); //this must be first

  if(!_streaming() && (_flags&TINYOAL_ISPLAYING)) // If we aren't playing but should be uiSource *must* be valid because Play() was called.
  {
    ALint iQueuedBuffers;
    cTinyOAL::Instance()->oalFuncs->alGetSourcei(uiSource, AL_BUFFERS_QUEUED, &iQueuedBuffers);
    if(!iQueuedBuffers)
    {
      Stop();
      return false;
    }

    cTinyOAL::Instance()->oalFuncs->alSourcePlay(uiSource); //The audio device was starved for data so we need to restart it
  }

  return true;
}
void cAudio::Invalidate()
{
  if(_stream && _source) _source->CloseStream(_stream);
  _stream=0;
  _source=0;
}

void cAudio::_stop()
{
  if((_flags&TINYOAL_ISPLAYING)!=0 && _source!=0)
    cTinyOAL::Instance()->_removeaudio(this,_source);
  _flags -= TINYOAL_ISPLAYING;
}

void cAudio::_getsource()
{
  if(uiSource==(unsigned int)-1) // if uiSource is invalid we need to grab a new one.
  {
    cTinyOAL::Instance()->oalFuncs->alGetError(); // Clear last error
    cTinyOAL::Instance()->oalFuncs->alGenSources(1, &uiSource);
    if(cTinyOAL::Instance()->oalFuncs->alGetError() != AL_NO_ERROR)
    {
      TINYOAL_LOGM("ERROR","Failed to generate source!");
      //TODO steal source from other audio instead
    }

    _applyall(); // Make sure we've applied everything
    _queuebuffers();
  }
}

void cAudio::_queuebuffers()
{
  unsigned char nbuffers=cTinyOAL::Instance()->defNumBuf; // Queue everything
  _queuebuflen+=_bufstart;
  for(ALint i = _bufstart; i < _queuebuflen; ++i) // Queues all waiting buffers in the correct order.
    cTinyOAL::Instance()->oalFuncs->alSourceQueueBuffers(uiSource, 1, &uiBuffers[i%nbuffers]);
  _queuebuflen=0;
}
void cAudio::_fillbuffers()
{
  _bufstart=0;
  _queuebuflen=0;
  unsigned long ulBytesWritten;
  unsigned char nbuffers=cTinyOAL::Instance()->defNumBuf;
  for (ALint i = 0; i < nbuffers; i++)
  {
    ulBytesWritten = _source->Read(_stream, pDecodeBuffer);
	  if (ulBytesWritten)
      cTinyOAL::Instance()->oalFuncs->alBufferData(uiBuffers[_queuebuflen++], _source->GetFormat(), pDecodeBuffer, ulBytesWritten, _source->GetFreq());
  }
}

void cAudio::_processbuffers()
{
  // Request the number of OpenAL Buffers have been processed (played) on the Source
	ALint iBuffersProcessed = 0;
	cTinyOAL::Instance()->oalFuncs->alGetSourcei(uiSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed);
  bool loop = _looptime!=(unsigned __int64)-1;
  ALuint uiBuffer;
  unsigned long ulBytesWritten;

	// For each processed buffer, remove it from the Source Queue, read next chunk of audio
	// data from disk, fill buffer with new data, and add it to the Source Queue
	while (iBuffersProcessed)
	{
		// Remove the Buffer from the Queue.  (uiBuffer contains the Buffer ID for the unqueued Buffer)
		uiBuffer = 0;
		cTinyOAL::Instance()->oalFuncs->alSourceUnqueueBuffers(uiSource, 1, &uiBuffer);

		// Read more audio data (if there is any)
		ulBytesWritten = _source->Read(_stream, pDecodeBuffer);
    if(!ulBytesWritten && loop) //If we are looping we reset the stream and continue filling buffers
    {
      _source->Skip(_stream,_looptime); // Automatically falls back to resetting the stream if _looptime is 0.0 and the stream doesn't support skipping
		  ulBytesWritten = _source->Read(_stream, pDecodeBuffer);
    }
    
		if(ulBytesWritten)
		{
      cTinyOAL::Instance()->oalFuncs->alBufferData(uiBuffer, _source->GetFormat(), pDecodeBuffer, ulBytesWritten, _source->GetFreq());
			cTinyOAL::Instance()->oalFuncs->alSourceQueueBuffers(uiSource, 1, &uiBuffer);
		}

    iBuffersProcessed--;
	}
}

bool cAudio::_streaming()
{
  if(!cTinyOAL::Instance()->oalFuncs) return false;
  int iState=0;
	cTinyOAL::Instance()->oalFuncs->alGetSourcei(uiSource, AL_SOURCE_STATE, &iState); // if uiSource is invalid or this fails for any reason, iState will remain at 0
	return iState == AL_PLAYING;
}

bool cAudio::IsPlaying()
{
  return (_flags&TINYOAL_ISPLAYING)!=0;
}

void cAudio::SetVolume(float range)
{
  if(range >= 0.0f)
    _vol = range;
  if(uiSource != (unsigned int)-1) 
    cTinyOAL::Instance()->oalFuncs->alSourcef(uiSource, AL_GAIN, _vol);
}

void cAudio::SetPitch(float range)
{
  if(range >= 0.5f && range <= 2.0f)
    _pitch = range;
  if(uiSource != -1) 
    cTinyOAL::Instance()->oalFuncs->alSourcef(uiSource, AL_PITCH, _pitch);
}

void cAudio::SetPosition(float X, float Y, float Z)
{
  _pos[0] = X;
  _pos[1] = Y;
  _pos[2] = Z;
  if(uiSource != (unsigned int)-1)
    cTinyOAL::Instance()->oalFuncs->alSourcefv(uiSource, AL_POSITION, _pos);
}

void cAudio::_applyall()
{
  if(uiSource != (unsigned int)-1)
  {
    cTinyOAL::Instance()->oalFuncs->alSourcefv(uiSource, AL_POSITION, _pos);
    cTinyOAL::Instance()->oalFuncs->alSourcef(uiSource, AL_PITCH, _pitch);
    cTinyOAL::Instance()->oalFuncs->alSourcef(uiSource, AL_GAIN, _vol);
  }
}