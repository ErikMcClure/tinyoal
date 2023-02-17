// Copyright (c)2023 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef TOAL__WASENGINE_H
#define TOAL__WASENGINE_H

#include "buntils/compiler.h"
#include "Engine.h"

#ifdef BUN_PLATFORM_WIN32
#include "win32_includes.h"
  #include <audioclient.h>
  #include <mmdeviceapi.h>

namespace tinyoal {
  struct IUnknownDeleter
  {
    void operator()(IUnknown* p) const
    {
      if(p)
        p->Release();
    }
  };

  // Very questionable WASAPI "implementation" of the audio engine, intended for testing purposes only
  class WASEngine : public Engine
  {
    // This is actually a TERRIBLE idea given that this audio engine was intended to 
    // be used with a bunch of short-lived sound effects, but it is convenient for 
    // testing how WASAPI works. A real implementation would do software mixing down
    // to just a few clients.
    class WASSource : public Source
    {
    public:
      WASSource(IMMDevice* device, LoadBuffer loadBuffer, bool exclusive, int format, uint32_t freq);
      ~WASSource();
      virtual bool Update(void* context, bool isPlaying) override;
      virtual bool Play(float volume, float pitch, float (&pos)[3]) override;
      virtual void Stop() override;
      virtual void Pause() override;
      virtual bool IsStreaming() const override;
      virtual bool Skip(void* context) override;
      virtual void FillBuffers(void* context) override;
      virtual uint64_t GetOffset() const override;
      virtual void SetVolume(float range) override;
      virtual void SetPitch(float range) override;
      virtual void SetPosition(float (&pos)[3]) override;

    private:
      void _queueBuffers();
      inline uint32_t _frameBytes() const { return _format->nChannels * (_format->wBitsPerSample >> 3); }

      std::unique_ptr<IMMDevice, IUnknownDeleter>  _device;
      LoadBuffer _loadBuffer;
      std::unique_ptr<IAudioClient, IUnknownDeleter> _client;
      WAVEFORMATEX* _format;
      REFERENCE_TIME _duration;
      const AUDCLNT_SHAREMODE _exclusive;
      bool _isPlaying;
    };

  public:
    WASEngine(bool exclusive);
    ~WASEngine();
    bool Init(const char* device = nullptr) override;
    bool SetDevice(const char* device) override;
    virtual ENGINE_TYPE GetType() override { return _exclusive ? ENGINE_WASAPI_EXCLUSIVE : ENGINE_WASAPI_SHARED; }
    virtual size_t GetDefaultDevice(char* out, size_t len) override;
    virtual uint32_t GetFormat(uint16_t channels, uint16_t bits, bool rear) override;
    virtual uint32_t GetWaveFormat(WaveFileInfo& wave) override;
    virtual Source* GenSource(Source::LoadBuffer loadBuffer, size_t bufsize, int format, uint32_t freq) override;
    virtual void DestroySource(Source* source) override;

    std::unique_ptr<IMMDeviceEnumerator, IUnknownDeleter> _enumerator;
    std::unique_ptr<IMMDevice, IUnknownDeleter> _device;
    const bool _exclusive;
  };
}

#endif
#endif