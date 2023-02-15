// Copyright (c)2023 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "OALEngine.h"
#include "tinyoal/TinyOAL.h"
#include "WaveFunctions.h"
#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

using namespace tinyoal;

typedef struct
{
  std::string strDeviceName;
  int iMajorVersion;
  int iMinorVersion;
  unsigned int _sourceCount;
  std::vector<std::string> pvstrExtensions;
  bool bSelected;
} ALDEVICEINFO, *LPALDEVICEINFO;

OALEngine::OALEngine(unsigned char bufferCount, const char* dllpath) :
  defNumBuf(bufferCount), _bufalloc(bufferCount * sizeof(ALuint), 5)
{
  // We do this instead of std::string because the pointer being NULL has meaning.
  if(dllpath)
  {
    auto len = strlen(dllpath) + 1;
    _dllpath.reset(new char[len]);
    MEMCPY(_dllpath.get(), len, dllpath, len);
  }
}

OALEngine::~OALEngine()
{
  ALCcontext* pContext = oalFuncs->alcGetCurrentContext();
  ALCdevice* pDevice   = oalFuncs->alcGetContextsDevice(pContext);

  oalFuncs->alcMakeContextCurrent(nullptr);
  oalFuncs->alcDestroyContext(pContext);
  oalFuncs->alcCloseDevice(pDevice);
  UnloadOAL10Library();
}
bool OALEngine::Init(const char* device)
{
  OPENALFNTABLE* functmp = new OPENALFNTABLE();

  // OpenAL initialization code
  const char* actualDeviceName;
  OPENALFNTABLE& ALFunction = *functmp;
  std::vector<ALDEVICEINFO> vDeviceInfo;
  size_t defaultDeviceIndex = 0;

  // grab function pointers for 1.0-API functions, and if successful proceed to enumerate all devices
  if(LoadOAL10Library(_dllpath.get(), &ALFunction) != 0)
  {
    if(ALFunction.alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT"))
    {
      char* devices                 = (char*)ALFunction.alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
      const char* defaultDeviceName = (char*)ALFunction.alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
      TINYOAL_LOG(4, "Default device name is: %s", defaultDeviceName);
      size_t index = 0;
      // go through device list (each device terminated with a single NULL, list terminated with double NULL)
      while((*devices) != '\0')
      {
        if(!strcmp(defaultDeviceName, devices))
          defaultDeviceIndex = index;

        ALCdevice* device = ALFunction.alcOpenDevice(devices);
        if(device)
        {
          ALCcontext* context = ALFunction.alcCreateContext(device, nullptr);
          if(context)
          {
            ALFunction.alcMakeContextCurrent(context);
            actualDeviceName = ALFunction.alcGetString(device, ALC_DEVICE_SPECIFIER);
            bool bNewName    = true;

            if(actualDeviceName)
            {
              for(size_t i = 0; i < vDeviceInfo.size(); i++)
              {
                if(!strcmp(vDeviceInfo[i].strDeviceName.c_str(), actualDeviceName))
                  bNewName = false;
              }
            }

            if((bNewName) && (actualDeviceName != nullptr) && (strlen(actualDeviceName) > 0))
            {
              vDeviceInfo.resize(vDeviceInfo.size() + 1);
              ALDEVICEINFO& ALDeviceInfo = vDeviceInfo.back();
              ALDeviceInfo.bSelected     = true;
              ALDeviceInfo.strDeviceName = actualDeviceName;
              ALFunction.alcGetIntegerv(device, ALC_MAJOR_VERSION, sizeof(int), &ALDeviceInfo.iMajorVersion);
              ALFunction.alcGetIntegerv(device, ALC_MINOR_VERSION, sizeof(int), &ALDeviceInfo.iMinorVersion);

              const char* exts[] = { "ALC_EXT_CAPTURE",
                                     "ALC_EXT_EFX",
                                     "AL_EXT_OFFSET",
                                     "AL_EXT_LINEAR_DISTANCE",
                                     "AL_EXT_EXPONENT_DISTANCE",
                                     "EAX2.0",
                                     "EAX3.0",
                                     "EAX4.0",
                                     "EAX5.0",
                                     "EAX-RAM" };

              // Check for Extensions
              for(int i = 0; i < sizeof(exts) / sizeof(const char*); ++i)
                if(ALFunction.alcIsExtensionPresent(device, exts[i]) == AL_TRUE)
                  ALDeviceInfo.pvstrExtensions.push_back(exts[i]);

              ALDeviceInfo._sourceCount = 0; // Get Source Count
            }

            ALFunction.alcMakeContextCurrent(nullptr);
            ALFunction.alcDestroyContext(context);
          }
          ALFunction.alcCloseDevice(device);
        }
        devices += strlen(devices) + 1;
        index += 1;
      }
    }
  }

  oalFuncs.reset(functmp);
  if(!vDeviceInfo.size())
    TINYOAL_LOG(1, "No devices in device list!");
  else if(SetDevice(vDeviceInfo[defaultDeviceIndex].strDeviceName.c_str()))
    functmp = nullptr;

  if(functmp) // If functmp is nonzero, something blew up, so delete it
  {
    oalFuncs = nullptr;
    delete functmp;
    return false;
  }

  return true;
}
bool OALEngine::SetDevice(const char* device)
{
  ALCdevice* pDevice = oalFuncs->alcOpenDevice(device);
  if(!pDevice)
  {
    TINYOAL_LOG(1, "Failed to open device: %s", device);
    return false;
  }
  ALCcontext* pContext = oalFuncs->alcCreateContext(pDevice, nullptr);
  if(pContext)
  {
    TINYOAL_LOG(4, "Opened Device: %s", device);
    oalFuncs->alcMakeContextCurrent(pContext);
    return true;
  }
  oalFuncs->alcCloseDevice(pDevice);
  TINYOAL_LOG(1, "Failed to create context for %s", device);
  return false;
}

unsigned int OALEngine::GetWaveFormat(WaveFileInfo& wave)
{
  unsigned short bits = wave.wfEXT.Format.wBitsPerSample;
  switch(wave.wfEXT.Format.wFormatTag)
  {
  case WAVE_FORMAT_PCM:
  case WAVE_FORMAT_IEEE_FLOAT:
    return GetFormat(wave.wfEXT.Format.nChannels, (bits == 24) ? 32 : bits,
                     false); // 24-bit gets converted to 32 bit
  case WAVE_FORMAT_IMA_ADPCM:
    if(!oalFuncs->alIsExtensionPresent("AL_LOKI_IMA_ADPCM_format"))
      break;
    switch(wave.wfEXT.Format.nChannels)
    {
    case 1: return AL_FORMAT_IMA_ADPCM_MONO16_EXT;
    case 2: return AL_FORMAT_IMA_ADPCM_STEREO16_EXT;
    }
    break;
  case WAVE_FORMAT_ALAW:
    if(!oalFuncs->alIsExtensionPresent("AL_EXT_ALAW"))
      break;
    switch(wave.wfEXT.Format.nChannels)
    {
    case 1: return AL_FORMAT_MONO_ALAW_EXT;
    case 2: return AL_FORMAT_STEREO_ALAW_EXT;
    }
    break;
  case WAVE_FORMAT_MULAW:
    if(!oalFuncs->alIsExtensionPresent("AL_EXT_MULAW"))
      break;
    switch(wave.wfEXT.Format.nChannels)
    {
    case 1: return AL_FORMAT_MONO_MULAW;
    case 2: return AL_FORMAT_STEREO_MULAW;
    case 4: return AL_FORMAT_QUAD_MULAW;
    case 6: return AL_FORMAT_51CHN_MULAW;
    case 7: return AL_FORMAT_61CHN_MULAW;
    case 8: return AL_FORMAT_71CHN_MULAW;
    }
    break;
  case WAVE_FORMAT_EXTENSIBLE:
    return GetFormat(wave.wfEXT.Format.nChannels, (bits == 24) ? 32 : bits,
                     wave.wfEXT.dwChannelMask == (SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT));
  }
  return 0;
}

const char* OALEngine::GetDefaultDevice() { return oalFuncs->alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER); }

unsigned int OALEngine::GetFormat(unsigned short channels, unsigned short bits, bool rear)
{
  unsigned short hash = (channels << 8) | bits;
  switch(hash)
  {
  case((1 << 8) | 8): return AL_FORMAT_MONO8;
  case((1 << 8) | 16): return AL_FORMAT_MONO16;
  case((1 << 8) | 32): return AL_FORMAT_MONO_FLOAT32;
  case((1 << 8) | 64): return AL_FORMAT_MONO_DOUBLE_EXT;
  case((2 << 8) | 8): return rear ? AL_FORMAT_REAR8 : AL_FORMAT_STEREO8;
  case((2 << 8) | 16): return rear ? AL_FORMAT_REAR16 : AL_FORMAT_STEREO16;
  case((2 << 8) | 32): return rear ? AL_FORMAT_REAR32 : AL_FORMAT_STEREO_FLOAT32;
  case((2 << 8) | 64): return AL_FORMAT_STEREO_DOUBLE_EXT;
  case((4 << 8) | 8): return AL_FORMAT_QUAD8;
  case((4 << 8) | 16): return AL_FORMAT_QUAD16;
  case((4 << 8) | 32): return AL_FORMAT_QUAD32;
  case((6 << 8) | 8): return AL_FORMAT_51CHN8;
  case((6 << 8) | 16): return AL_FORMAT_51CHN16;
  case((6 << 8) | 32): return AL_FORMAT_51CHN32;
  case((7 << 8) | 8): return AL_FORMAT_61CHN8;
  case((7 << 8) | 16): return AL_FORMAT_61CHN16;
  case((7 << 8) | 32): return AL_FORMAT_61CHN32;
  case((8 << 8) | 8): return AL_FORMAT_71CHN8;
  case((8 << 8) | 16): return AL_FORMAT_71CHN16;
  case((8 << 8) | 32): return AL_FORMAT_71CHN32;
  }
  return 0;
}

ALuint* OALEngine::_alloc()
{
  return (ALuint*)_bufalloc.Alloc();
}
void OALEngine::_dealloc(ALuint* buf)
{
  _bufalloc.Dealloc(buf);
}
OALEngine::OALSource::OALSource(OALEngine* engine, ReadBuffer readbuffer) :
  _source((unsigned int)-1),
  _engine(engine),
  uiBuffers(nullptr),
  _bufstart(0),
  _queuebuflen(0),
  _readBuffer(readbuffer)
{
  uiBuffers = _engine->_alloc();
  memset(uiBuffers, 0, sizeof(ALuint) * _engine->defNumBuf);
  _engine->oalFuncs->alGenBuffers(_engine->defNumBuf, uiBuffers);
}
OALEngine::OALSource::~OALSource()
{
  if(_source != (unsigned int)-1)
    _engine->oalFuncs->alDeleteSources(1, &_source);

  if(uiBuffers)
  {
    _engine->oalFuncs->alDeleteBuffers(_engine->defNumBuf, uiBuffers);
    _engine->_dealloc(uiBuffers);
  }
}
bool OALEngine::OALSource::Update(int format, uint32_t freq, void* context, bool isPlaying)
{
  _processBuffers((ALenum)format, (ALsizei)freq, context); // this must be first

  if(!IsStreaming() && isPlaying) // If we aren't playing but should be _source *must* be valid because Play() was called.
  {
    ALint iQueuedBuffers;
    _engine->oalFuncs->alGetSourcei(_source, AL_BUFFERS_QUEUED, &iQueuedBuffers);
    if(!iQueuedBuffers)
      return false;

    _engine->oalFuncs->alSourcePlay(_source); // The audio device was starved for data so we need to restart it
  }

  return true;
}
bool OALEngine::OALSource::Play(float volume, float pitch, float (&pos)[3])
{
  if(_source == (unsigned int)-1) // if _source is invalid we need to grab a new one.
  {
    _engine->oalFuncs->alGetError(); // Clear last error
    _engine->oalFuncs->alGenSources(1, &_source);
    if(_engine->oalFuncs->alGetError() != AL_NO_ERROR)
    {
      TINYOAL_LOG(1, "Failed to generate source!");
      // TODO steal source from other audio instead
    }

    // Make sure we've applied everything
    SetVolume(volume);
    SetPitch(pitch);
    SetPosition(pos);
    _queueBuffers();
  }

  if(!IsStreaming())
    _engine->oalFuncs->alSourcePlay(_source);

  return IsStreaming();
}
void OALEngine::OALSource::Stop()
{
  if(IsStreaming())
    _engine->oalFuncs->alSourceStop(_source);
  if(_source != (unsigned int)-1)
  {
    _engine->oalFuncs->alSourcei(_source, AL_BUFFER, 0); // Detach buffer
    _engine->oalFuncs->alDeleteSources(1, &_source);
    _source = (unsigned int)-1;
  }
}
void OALEngine::OALSource::Pause() { _engine->oalFuncs->alSourcePause(_source); }
bool OALEngine::OALSource::IsStreaming() const
{
  if(!_engine->oalFuncs || _source == (unsigned int)-1)
    return false;
  int iState = 0;
  // if _source is invalid or this fails for any reason, iState will remain at 0
  _engine->oalFuncs->alGetSourcei(_source, AL_SOURCE_STATE, &iState);
  return iState == AL_PLAYING;
}
bool OALEngine::OALSource::Skip(uint64_t sample, int format, uint32_t freq, void* context, bool isPlaying)
{
  if(_source != (unsigned int)-1) // We have to check this instead of whether or not it's playing because it could be paused
  {
    _engine->oalFuncs->alSourceStop(_source); // Stop no matter what in case it's paused, because we have to reset it.
    _engine->oalFuncs->alSourcei(_source, AL_BUFFER, 0); // Detach buffer
    _fillBuffers(format, freq, context);                 // Refill all buffers
    _queueBuffers();                                     // requeue everything, which forces the audio to immediately skip.
    if(isPlaying)
      _engine->oalFuncs->alSourcePlay(_source);
  }
  else
    _fillBuffers(format, freq, context); // If we don't have a source, just refill the buffers and don't do anything else

  return true;
}
void OALEngine::OALSource::FillBuffers(int format, uint32_t freq, void* context) { _fillBuffers(format, freq, context); }
uint64_t OALEngine::OALSource::GetOffset() const
{
  ALint offset;
  _engine->oalFuncs->alGetSourcei(_source, AL_SAMPLE_OFFSET, &offset);
  return offset;
}
void OALEngine::OALSource::SetVolume(float range)
{
  if(_source != (unsigned int)-1)
    _engine->oalFuncs->alSourcef(_source, AL_GAIN, range);
}
void OALEngine::OALSource::SetPitch(float range)
{
  if(_source != -1)
    _engine->oalFuncs->alSourcef(_source, AL_PITCH, range);
}
void OALEngine::OALSource::SetPosition(float (&pos)[3])
{
  if(_source != (unsigned int)-1)
    _engine->oalFuncs->alSourcefv(_source, AL_POSITION, pos);
}

void OALEngine::OALSource::_processBuffers(ALenum format, ALsizei freq, void* context)
{
  // Request the number of OpenAL Buffers have been processed (played) on the Source
  ALint iBuffersProcessed = 0;
  _engine->oalFuncs->alGetSourcei(_source, AL_BUFFERS_PROCESSED, &iBuffersProcessed);

  // For each processed buffer, remove it from the Source Queue, read next chunk of audio
  // data from disk, fill buffer with new data, and add it to the Source Queue
  while(iBuffersProcessed)
  {
    // Remove the Buffer from the Queue.  (uiBuffer contains the Buffer ID for the unqueued Buffer)
    ALuint uiBuffer = 0;
    _engine->oalFuncs->alSourceUnqueueBuffers(_source, 1, &uiBuffer);

    unsigned long ulBytesWritten;
    auto buffer = (*_readBuffer)(ulBytesWritten, context); // Read more audio data (if there is any)

    if(ulBytesWritten)
    {
      _engine->oalFuncs->alBufferData(uiBuffer, format, buffer, ulBytesWritten, freq);
      _engine->oalFuncs->alSourceQueueBuffers(_source, 1, &uiBuffer);
    }

    iBuffersProcessed--;
  }
}
void OALEngine::OALSource::_fillBuffers(ALenum format, ALsizei freq, void* context)
{
  _bufstart    = 0;
  _queuebuflen = 0;
  for(ALint i = 0; i < _engine->defNumBuf; i++)
  {
    unsigned long ulBytesWritten;
    auto buffer = (*_readBuffer)(ulBytesWritten, context);
    if(ulBytesWritten)
      _engine->oalFuncs->alBufferData(uiBuffers[_queuebuflen++], format, buffer, ulBytesWritten, freq);
  }
}
void OALEngine::OALSource::_queueBuffers()
{
  unsigned char nbuffers = _engine->defNumBuf; // Queue everything
  _queuebuflen += _bufstart;
  for(ALint i = _bufstart; i < _queuebuflen; ++i) // Queues all waiting buffers in the correct order.
    _engine->oalFuncs->alSourceQueueBuffers(_source, 1, &uiBuffers[i % nbuffers]);
  _queuebuflen = 0;
}
Source* OALEngine::GenSource(Source::ReadBuffer readBuffer)
{
  if(!oalFuncs)
    return nullptr;
  return new OALSource(this, readBuffer);
}
void OALEngine::DestroySource(Source* source) { delete source; }