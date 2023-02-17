// Copyright (c)2023 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef TOAL__ENGINE_H
#define TOAL__ENGINE_H

#include "tinyoal/TinyOAL.h"

namespace tinyoal {
  struct WaveFileInfo;

  class Source
  {
  public:
    using LoadBuffer = unsigned long (*)(unsigned long, char*, void*);

    virtual ~Source() {}
    virtual bool Update(void* context, bool isPlaying)            = 0;
    virtual bool Play(float volume, float pitch, float (&pos)[3]) = 0;
    virtual void Stop()                                           = 0;
    virtual void Pause()                                          = 0;
    virtual bool IsStreaming() const                              = 0;
    virtual bool Skip(void* context)                              = 0;
    virtual void FillBuffers(void* context)                       = 0;
    virtual uint64_t GetOffset() const                            = 0;
    virtual void SetVolume(float range)                           = 0;
    virtual void SetPitch(float range)                            = 0;
    virtual void SetPosition(float (&pos)[3])                     = 0;
  };

  class Engine
  {
  public:
    virtual ~Engine() {}
    virtual bool Init(const char* device = nullptr)                                                     = 0;
    virtual bool SetDevice(const char* device)                                                          = 0;
    virtual size_t GetDefaultDevice(char* out, size_t len)                                              = 0;
    virtual ENGINE_TYPE GetType()                                                                       = 0;
    virtual Source* GenSource(Source::LoadBuffer loadBuffer, size_t bufsize, int format, uint32_t freq) = 0;
    virtual void DestroySource(Source* source)                                                          = 0;
    virtual uint32_t GetFormat(uint16_t channels, uint16_t bits, bool rear)                             = 0;
    virtual uint32_t GetWaveFormat(WaveFileInfo& wave)                                                  = 0;
  };
}

#endif