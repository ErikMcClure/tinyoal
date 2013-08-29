// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudio.h"
#include "cAudioResource.h"
#include "openAL/loadoal.h"
#include "cTinyOAL.h"

using namespace TinyOAL;

cAudio::cAudio(void* data, unsigned int datalength, T_TINYOAL_FLAGS flags) : _nbuffers(cTinyOAL::Instance()->getDefaultBuffer()),
  _funcs(cTinyOAL::Instance()->getFunctions()), _looptime(0), _flags(flags), uiSource(-1)
{
  _construct(cAudioResource::CreateAudioReference(data, datalength, flags));
}

cAudio::cAudio(const char* file, T_TINYOAL_FLAGS flags) : _nbuffers(cTinyOAL::Instance()->getDefaultBuffer()),
  _funcs(cTinyOAL::Instance()->getFunctions()), _looptime(0), _flags(flags), uiSource(-1)
{
  _construct(cAudioResource::CreateAudioReference(file, flags));
}

cAudio::cAudio(_iobuf* file, unsigned int datalength, T_TINYOAL_FLAGS flags) : _nbuffers(cTinyOAL::Instance()->getDefaultBuffer()),
  _funcs(cTinyOAL::Instance()->getFunctions()), _looptime(0), _flags(flags), uiSource(-1)
{
  _construct(cAudioResource::CreateAudioReference(file, datalength, flags));
}
cAudio::cAudio(const cAudioRef& ref, T_TINYOAL_FLAGS addflags) : _nbuffers(cTinyOAL::Instance()->getDefaultBuffer()),
  _funcs(cTinyOAL::Instance()->getFunctions()), _looptime(0), _flags(ref._res->GetFlags()|addflags), uiSource(-1)
{
  if(ref._res) ref._res->Grab(); //Normally done by CreateAudioReference
  _construct(ref._res);
}

cAudio::~cAudio()
{
  Stop();
  
  if(_stream)
  {
    if(uiSource!=(unsigned int)-1)
    {
      _funcs->alSourcei(uiSource, AL_BUFFER, 0); // Detach buffer
	    _funcs->alDeleteSources(1, &uiSource);
    }

    if(pDecodeBuffer)
      free(pDecodeBuffer);

	  _funcs->alDeleteBuffers(_nbuffers, uiBuffers);
    _source->CloseStream(_stream);
  }

  delete [] uiBuffers;
  if(_source) _source->Drop();
}

bool cAudio::Play()
{
  if(!_stream) return false;
  _getsource();

  if((_flags & TINYOAL_NODOUBLEPLAY) != 0)
  {
    if(!_streaming())
      _funcs->alSourcePlay(uiSource);
  }
  else
  {
    if(_streaming())
    {
      _funcs->alSourceStop(uiSource);
      _source->Reset(_stream);
      _funcs->alSourcei(uiSource,AL_BUFFER,0); //detach buffer
      //std::unique_ptr<ALuint[]> bufhold(new ALuint[_nbuffers]);
      //_funcs->alSourceUnqueueBuffers(uiSource,_nbuffers,bufhold.get()); //dequeue all buffers
      _fillbuffers(); //Refill all buffers
      _queuebuffers();
    }

    _funcs->alSourcePlay(uiSource);
  }
  _playing = true; //After calling this function this will always happen
  cTinyOAL::Instance()->_addaudio(this); // Addaudio will make sure we aren't added more than once.

  return _streaming();
}

void cAudio::Stop()
{
  if(_streaming())
    _funcs->alSourceStop(uiSource);
  bool del = (_flags&TINYOAL_MANAGED)!=0; // Do this up here because _removeaudio can destroy this object.
  del=cTinyOAL::Instance()->_removeaudio(this) && del; // If remove returns false, we weren't active, so managed objects aren't destroyed
  if(del) return; //if we are managed we just got destroyed so return immediately
  if(uiSource!=-1) _funcs->alSourcei(uiSource,AL_BUFFER,0); //detach buffer
  if(_stream!=0) 
  {
    _source->Reset(_stream);
    _fillbuffers(); //Refill all buffers
  }
  _playing = false;
}

void cAudio::Pause()
{
  if(_streaming())
    _funcs->alSourcePause(uiSource);
  if(cTinyOAL::Instance()->_removeaudio(this)) 
    assert(!(_flags&TINYOAL_MANAGED));
  _playing = false;
}

bool cAudio::Skip(double seconds)
{
  if(!_stream) return false;
  return SkipSample(_source->ToSample(_stream,seconds));
}
bool cAudio::SkipSample(unsigned __int64 sample)
{
  if(!_stream) return false;
  return _source->Skip(_stream,sample);
}

void cAudio::SetLoopPointSeconds(double seconds)
{
  _looptime = _source->ToSample(_stream,seconds);
}

void cAudio::SetLoopPoint(unsigned __int64 samples)
{
  _looptime = samples;
}

void cAudio::_construct(cAudioResource* source, bool init)
{
  prev = 0;
  next = 0;
  pDecodeBuffer = 0;
	uiBuffers = new ALuint[_nbuffers];
  memset(uiBuffers,0,sizeof(ALuint)*_nbuffers);
  _stream=0;
  _source=0;
  if(init)
  {
    _pos[0] = 0.0f;
    _pos[1] = 1.0f;
    _pos[2] = 1.0f;
    _vol = 1.0f;
    _pitch = 1.0f;
  }
  if(!source) return;

  _source = source;
  _stream = source->OpenStream();
  if(!_stream) return; //bad source

  // Allocate a buffer to be used to store decoded data for all Buffers
  pDecodeBuffer = (char*)malloc(_source->GetBufSize());
	if (!pDecodeBuffer)
	{
    TINYOAL_LOGM("ERROR","Failed to allocate memory for decoded audio data");
    return;
  }

  // Generate some AL Buffers for streaming
  _funcs->alGenBuffers(_nbuffers, uiBuffers);
  _fillbuffers(); // Fill all the Buffers with decoded audio data

  if(_stream!=0 && _source!=0)
    _looptime=_source->GetLoopStart(_stream);

  if((_flags & TINYOAL_AUTOPLAY) != 0)
    Play();
}

void cAudio::Update()
{
  if(!_stream) //Do we have a valid stream
    return;

  _processbuffers(); //this must be first

  if(!_streaming() && _playing) // If we aren't playing but _playing is true uiSource *must* be valid because Play() was called.
  {
    ALint iQueuedBuffers;
    _funcs->alGetSourcei(uiSource, AL_BUFFERS_QUEUED, &iQueuedBuffers);
    if(!iQueuedBuffers)
    {
      Stop();
      return;
    }

    _funcs->alSourcePlay(uiSource); //The audio device was starved for data so we need to restart it
  }
}

void cAudio::_getsource()
{
  if(uiSource==(unsigned int)-1) // if uiSource is invalid we need to grab a new one.
  {
    _funcs->alGetError(); // Clear last error
    _funcs->alGenSources(1, &uiSource);
    if(_funcs->alGetError() != AL_NO_ERROR)
      TINYOAL_LOGM("ERROR","Failed to generate source!");
      //TODO steal source from other audio instead
    _applyall(); // Make sure we've applied everything
  }
  _queuebuffers(); // Make sure all our buffers are queued
}

void cAudio::_fillbuffers()
{
  _bufstart=0;
  _queuebuflen=0;
  unsigned long ulBytesWritten;
  for (ALint i = 0; i < _nbuffers; i++)
  {
    ulBytesWritten = _source->ReadNext(_stream, pDecodeBuffer);
	  if (ulBytesWritten)
      _funcs->alBufferData(uiBuffers[_queuebuflen++], _source->GetFormat(), pDecodeBuffer, ulBytesWritten, _source->GetFreq());
  }
}
void cAudio::_queuebuffers()
{
  _queuebuflen+=_bufstart;
  for(ALint i = _bufstart; i < _queuebuflen; ++i) // Queues all waiting buffers in the correct order.
    _funcs->alSourceQueueBuffers(uiSource, 1, &uiBuffers[i%_nbuffers]);
  _queuebuflen=0;
}

void cAudio::_processbuffers()
{
  // Request the number of OpenAL Buffers have been processed (played) on the Source
	ALint iBuffersProcessed = 0;
	_funcs->alGetSourcei(uiSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed);
  bool loop = (_flags & TINYOAL_LOOP) != 0;
  ALuint uiBuffer;
  unsigned long ulBytesWritten;

	// For each processed buffer, remove it from the Source Queue, read next chunk of audio
	// data from disk, fill buffer with new data, and add it to the Source Queue
	while (iBuffersProcessed)
	{
		// Remove the Buffer from the Queue.  (uiBuffer contains the Buffer ID for the unqueued Buffer)
		uiBuffer = 0;
		_funcs->alSourceUnqueueBuffers(uiSource, 1, &uiBuffer);

		// Read more audio data (if there is any)
		ulBytesWritten = _source->ReadNext(_stream, pDecodeBuffer);
    if(!ulBytesWritten && loop) //If we are looping we reset the stream and continue filling buffers
    {
      //_source->Reset(_stream);
      _source->Skip(_stream,_looptime); // Automatically falls back to resetting the stream if _looptime is 0.0 and the stream doesn't support skipping
		  ulBytesWritten = _source->ReadNext(_stream, pDecodeBuffer);
    }
    
		if(ulBytesWritten)
		{
      _funcs->alBufferData(uiBuffer, _source->GetFormat(), pDecodeBuffer, ulBytesWritten, _source->GetFreq());
			_funcs->alSourceQueueBuffers(uiSource, 1, &uiBuffer);
		}

    iBuffersProcessed--;
	}
}

bool cAudio::_streaming()
{
  if(!_funcs) return false;
  int iState=0;
	_funcs->alGetSourcei(uiSource, AL_SOURCE_STATE, &iState); // if uiSource is invalid or this fails for any reason, iState will remain at 0
	return iState == AL_PLAYING;
}

bool cAudio::IsPlaying()
{
  return _playing;
}

void cAudio::SetVolume(float range)
{
  if(range >= 0.0f)
    _vol = range;
  if(uiSource != -1) 
    _funcs->alSourcef(uiSource, AL_GAIN, _vol);
}

void cAudio::SetPitch(float range)
{
  if(range >= 0.5f && range <= 2.0f)
    _pitch = range;
  if(uiSource != -1) 
    _funcs->alSourcef(uiSource, AL_PITCH, _pitch);
}

void cAudio::SetPosition(float X, float Y, float Z)
{
  _pos[0] = X;
  _pos[1] = Y;
  _pos[2] = Z;
  if(uiSource != -1)
    _funcs->alSourcefv(uiSource, AL_POSITION, _pos);
}

void cAudio::_applyall()
{
  if(uiSource != -1)
  {
    _funcs->alSourcefv(uiSource, AL_POSITION, _pos);
    _funcs->alSourcef(uiSource, AL_PITCH, _pitch);
    _funcs->alSourcef(uiSource, AL_GAIN, _vol);
  }
}

void cAudio::Replace(void* data, unsigned int datalength, T_TINYOAL_FLAGS flags)
{
  cAudio::~cAudio();
  _looptime=0;
  _flags=flags;
  uiSource=-1;
  _construct(cAudioResource::CreateAudioReference(data, datalength, flags),false);
  _applyall();
}
void cAudio::Replace(const char* file, T_TINYOAL_FLAGS flags)
{
  cAudio::~cAudio();
  _looptime=0;
  _flags=flags;
  uiSource=-1;
  _construct(cAudioResource::CreateAudioReference(file, flags),false);
  _applyall();
}
void cAudio::Replace(_iobuf* file, unsigned int datalength, T_TINYOAL_FLAGS flags)
{
  cAudio::~cAudio();
  _looptime=0;
  _flags=flags;
  uiSource=-1;
  _construct(cAudioResource::CreateAudioReference(file, datalength, flags),false);
  _applyall();
}
void cAudio::Replace(const cAudioRef& ref, T_TINYOAL_FLAGS addflags)
{
  cAudio::~cAudio();
  _looptime=0;
  _flags=ref._res->GetFlags()|addflags;
  uiSource=-1;
  if(ref._res) ref._res->Grab(); //Normally done by CreateAudioReference
  _construct(ref._res,false);
}

cAudioRef::cAudioRef(cAudioRef&& mov) : _res(mov._res)
{
  mov._res=0;
}
cAudioRef::cAudioRef(const cAudioRef& copy) : _res(copy._res)
{
  _res->Grab();
}
cAudioRef::cAudioRef(const char* file, unsigned char flags) : _res(cAudioResource::CreateAudioReference(file, flags)) {}
cAudioRef::cAudioRef(void* data, unsigned int datalength, unsigned char flags) : _res(cAudioResource::CreateAudioReference(data, datalength, flags)) {}
cAudioRef::cAudioRef(_iobuf* file, unsigned int datalength, unsigned char flags) : _res(cAudioResource::CreateAudioReference(file, datalength, flags)) {}
cAudioRef::cAudioRef(cAudioResource* res) : _res(res) { if(_res) _res->Grab(); }
cAudioRef::~cAudioRef()
{
  if(_res) _res->Drop();
}
cAudioRef& cAudioRef::operator=(cAudioRef&& mov)
{
  if(_res) _res->Drop();
  _res=mov._res;
  mov._res=0;
  return *this;
}
cAudioRef& cAudioRef::operator=(const cAudioRef& copy)
{
  if(_res) _res->Drop();
  _res=copy._res;
  if(_res) _res->Grab();
  return *this;
}
