// Copyright (c)2023 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef TOAL__ENGINE_H
#define TOAL__ENGINE_H

#include <stdint.h>

namespace tinyoal {
  struct WaveFileInfo;

  class Source
  {
  public:
    using ReadBuffer = char* (*)(unsigned long&, void*);

    virtual ~Source() {}
    virtual bool Update(int format, uint32_t freq, void* context, bool isPlaying)                = 0;
    virtual bool Play(float volume, float pitch, float (&pos)[3])                                = 0;
    virtual void Stop()                                                                          = 0;
    virtual void Pause()                                                                         = 0;
    virtual bool IsStreaming() const                                                             = 0;
    virtual bool Skip(uint64_t sample, int format, uint32_t freq, void* context, bool isPlaying) = 0;
    virtual void FillBuffers(int format, uint32_t freq, void* context)                           = 0;
    virtual uint64_t GetOffset() const                                                           = 0;
    virtual void SetVolume(float range)                                                          = 0;
    virtual void SetPitch(float range)                                                           = 0;
    virtual void SetPosition(float (&pos)[3])                                                    = 0;
  };

  class Engine
  {
  public:
    enum ENGINE_TYPE
    {
      ENGINE_TINYOAL = 0,
      ENGINE_WASAPI
    };

    virtual ~Engine() {}
    virtual bool Init(const char* device = nullptr)                                         = 0;
    virtual bool SetDevice(const char* device)                                              = 0;
    virtual const char* GetDefaultDevice()                                                  = 0;
    virtual ENGINE_TYPE GetType()                                                           = 0;
    virtual Source* GenSource(Source::ReadBuffer readBuffer)                                = 0;
    virtual void DestroySource(Source* source)                                              = 0;
    virtual uint32_t GetNumBuffers()                                                        = 0;
    virtual unsigned int GetFormat(unsigned short channels, unsigned short bits, bool rear) = 0;
    virtual unsigned int GetWaveFormat(WaveFileInfo& wave)                                  = 0;
  };
}

#endif