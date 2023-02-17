// Copyright (c)2023 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "WASEngine.h"
#include "WaveFunctions.h"

#ifdef BUN_PLATFORM_WIN32

using namespace tinyoal;

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator    = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient           = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient     = __uuidof(IAudioRenderClient);
const IID IID_IAudioStreamVolume     = __uuidof(IAudioStreamVolume);

WASEngine::WASEngine(bool exclusive) : _exclusive(exclusive), _enumerator(nullptr), _device(nullptr)
{
  CoInitializeEx(NULL, COINIT_MULTITHREADED);
}
WASEngine::~WASEngine() { CoUninitialize(); }
bool WASEngine::Init(const char* device)
{
  IMMDeviceEnumerator* pEnum = nullptr;
  HRESULT hr;
  if(FAILED(hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnum)))
    return false;
  _enumerator.reset(pEnum);
  if(!SetDevice(nullptr))
    return false;
  return true;
}
bool WASEngine::SetDevice(const char* device)
{
  IMMDevice* newDevice = nullptr;
  if(!device)
  {
    if(FAILED(_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &newDevice)))
      return false;
  }
  else
  {
    if(FAILED(_enumerator->GetDevice(bun::StrW(device).c_str(), &newDevice)))
      return false;
  }

  _device.reset(newDevice);
  return true;
}
size_t WASEngine::GetDefaultDevice(char* out, size_t len)
{
  IMMDevice* pDevice;
  if(FAILED(_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice)))
    return 0;
  std::unique_ptr<IMMDevice, IUnknownDeleter> defaultDevice(pDevice);
  LPWSTR id;
  size_t sz = 0;
  if(SUCCEEDED(defaultDevice->GetId(&id)))
  {
    bun::Str convert(id);

    sz = convert.size() + 1;
    if(sz > len)
      sz = len;
    if(out)
      MEMCPY(out, len, convert.c_str(), sz);

    CoTaskMemFree(id);
  }

  return sz;
}

uint32_t WASEngine::GetFormat(uint16_t channels, uint16_t bits, bool rear)
{
  return bits | (channels << 16) | (uint32_t)rear << 31;
}
uint32_t WASEngine::GetWaveFormat(WaveFileInfo& wave)
{
  uint16_t bits = wave.wfEXT.Format.wBitsPerSample;
  return GetFormat(wave.wfEXT.Format.nChannels, bits, wave.wfEXT.dwChannelMask == (SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT));
}
Source* WASEngine::GenSource(Source::LoadBuffer loadBuffer, size_t bufsize, int format, uint32_t freq)
{
  return new WASSource(_device.get(), loadBuffer, _exclusive, format, freq);
}
void WASEngine::DestroySource(Source* source) { delete source; }

  // REFERENCE_TIME time units per second and per millisecond
  #define REFTIMES_PER_SEC      10000000
  #define REFTIMES_PER_MILLISEC 10000

WASEngine::WASSource::WASSource(IMMDevice* device, LoadBuffer loadBuffer, bool exclusive, int format, uint32_t freq) :
  _exclusive(exclusive ? AUDCLNT_SHAREMODE_EXCLUSIVE : AUDCLNT_SHAREMODE_SHARED),
  _client(nullptr),
  _format(nullptr),
  _isPlaying(false),
  _loadBuffer(loadBuffer),
  _device(device),
  _duration(0)
{
  IAudioClient* pClient = nullptr;
  if(FAILED(_device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pClient)))
    return;
  _client.reset(pClient);

  WAVEFORMATEX* mixformat;
  if(FAILED(_client->GetMixFormat(&mixformat)))
    return;

  WAVEFORMATEX trueformat    = *mixformat;
  uint16_t bits              = format & 0xFFFF;
  uint16_t channels          = (format >> 16) & 0x7FFF;
  trueformat.wFormatTag      = WAVE_FORMAT_PCM;
  trueformat.nChannels       = channels;
  trueformat.wBitsPerSample  = bits;
  trueformat.nSamplesPerSec  = freq;
  trueformat.nAvgBytesPerSec = (freq * bits * channels) >> 3;
  trueformat.nBlockAlign     = (bits * channels) >> 3;
  trueformat.cbSize          = 0;
  CoTaskMemFree(mixformat);

  HRESULT hr;
  if(FAILED(hr = _client->IsFormatSupported(_exclusive, &trueformat, &mixformat)))
    return;

  if(hr == S_FALSE)
  {
    _format = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX) + mixformat->cbSize);
    MEMCPY(_format, sizeof(WAVEFORMATEX) + mixformat->cbSize, mixformat, sizeof(WAVEFORMATEX) + mixformat->cbSize);
    CoTaskMemFree(mixformat);
  }
  else
  {
    _format  = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
    *_format = trueformat;
  }

  DWORD flags = AUDCLNT_STREAMFLAGS_NOPERSIST;
  if(_exclusive == AUDCLNT_SHAREMODE_EXCLUSIVE)
  {
    if(FAILED(_client->GetDevicePeriod(NULL, &_duration)))
      return;
  }
  else
  {
    flags |= AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;
    if(FAILED(hr = _client->GetDevicePeriod(&_duration, NULL)))
      return;
  }
  // Using event based buffer notifications would probably be better for latency, but our engine doesn't work like that
  hr = _client->Initialize(_exclusive, flags, _duration, _duration, _format, NULL);

  if(hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED)
  {
    // Align the buffer if needed, see IAudioClient::Initialize() documentation
    UINT32 nFrames = 0;
    if(SUCCEEDED(_client->GetBufferSize(&nFrames)))
    {
      _duration = (REFERENCE_TIME)((double)REFTIMES_PER_SEC / _format->nSamplesPerSec * nFrames + 0.5);
      _client->Initialize(_exclusive, flags, _duration, _duration, _format, NULL);
    }
  }
}
WASEngine::WASSource::~WASSource()
{
  if(_format)
    free(_format);
}
bool WASEngine::WASSource::Update(void* context, bool isPlaying)
{
  IAudioRenderClient* pRenderClient;
  if(FAILED(_client->GetService(IID_IAudioRenderClient, (void**)&pRenderClient)))
    return false;

  std::unique_ptr<IAudioRenderClient, IUnknownDeleter> render(pRenderClient);
  UINT32 numFramesPadding;
  UINT32 bufferFrameCount;

  // See how much buffer space is available.
  if(FAILED(_client->GetCurrentPadding(&numFramesPadding)))
    return false;
  if(FAILED(_client->GetBufferSize(&bufferFrameCount)))
    return false;

  UINT32
  numFramesAvailable = bufferFrameCount - numFramesPadding;

  BYTE* buffer;
  // Grab all the available space in the shared buffer.
  if(FAILED(render->GetBuffer(numFramesAvailable, &buffer)))
    return false;

  // Get next 1/2-second of data from the audio source.
  const auto framebytes = _frameBytes();
  auto numFramesWritten    = (*_loadBuffer)(numFramesAvailable * framebytes, (char*)buffer, context) / framebytes;

  if(FAILED(render->ReleaseBuffer(numFramesWritten, 0)))
    return false;

  // if we are out of data, mark the source as exhausted, so that we know to set _isPlaying to false when we run out of
  // buffer.
  if(numFramesAvailable == bufferFrameCount && numFramesWritten == 0)
    return false;

  return true;
}
bool WASEngine::WASSource::Play(float volume, float pitch, float (&pos)[3])
{
  return (_isPlaying = SUCCEEDED(_client->Start()));
}
void WASEngine::WASSource::Stop()
{
  if(SUCCEEDED(_client->Stop()))
    _client->Reset();
  _isPlaying = false;
}
void WASEngine::WASSource::Pause()
{
  _client->Stop();
  _isPlaying = false;
}
bool WASEngine::WASSource::IsStreaming() const { return _isPlaying; }
bool WASEngine::WASSource::Skip(void* context)
{
  _client->Stop();
  _client->Reset();
  FillBuffers(context);
  return true;
}

void WASEngine::WASSource::FillBuffers(void* context)
{
  UINT32 bufferFrameCount;
  if(FAILED(_client->GetBufferSize(&bufferFrameCount)))
    return;

  IAudioRenderClient* pRenderClient;
  if(SUCCEEDED(_client->GetService(IID_IAudioRenderClient, (void**)&pRenderClient)))
  {
    std::unique_ptr<IAudioRenderClient, IUnknownDeleter> render(pRenderClient);
    BYTE* buffer;
    // Grab the entire buffer for the initial fill operation.
    if(FAILED(render->GetBuffer(bufferFrameCount, &buffer)))
      return;

    // Load the initial data into the shared buffer.
    const auto framebytes = _frameBytes();
    bufferFrameCount      = (*_loadBuffer)(bufferFrameCount * framebytes, (char*)buffer, context) / framebytes;

    render->ReleaseBuffer(bufferFrameCount, 0);
  }
}
uint64_t WASEngine::WASSource::GetOffset() const
{
  UINT32 numFramesPadding;
  if(FAILED(_client->GetCurrentPadding(&numFramesPadding)))
    return 0;

  return numFramesPadding;
  // return numFramesPadding - (2 * (_bufsize / _frameBytes()));
}
void WASEngine::WASSource::SetVolume(float range)
{
  IAudioStreamVolume* iVolume;
  if(FAILED(_client->GetService(IID_IAudioStreamVolume, (void**)&iVolume)))
    return;

  UINT32 count;
  if(FAILED(iVolume->GetChannelCount(&count)))
    return;

  float* volumes = (float*)alloca(sizeof(float) * count);
  for(UINT32 i = 0; i < count; ++i)
    volumes[i] = range;

  iVolume->SetAllVolumes(count, volumes);
}
void WASEngine::WASSource::SetPitch(float range) {}
void WASEngine::WASSource::SetPosition(float (&pos)[3]) {}

#endif