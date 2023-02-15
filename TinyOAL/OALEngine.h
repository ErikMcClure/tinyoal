// Copyright (c)2023 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef TOAL__OALENGINE_H
#define TOAL__OALENGINE_H

#include "bss-util/compiler.h"
#include "loadoal.h"
#include "Engine.h"
#include "bss-util/BlockAlloc.h"
#include <memory>

namespace tinyoal {
  class OALEngine : public Engine
  {
    class OALSource : public Source
    {
    public:
      OALSource(OALEngine* engine, ReadBuffer readBuffer);
      ~OALSource();
      virtual bool Update(int format, uint32_t freq, void* context, bool isPlaying) override;
      virtual bool Play(float volume, float pitch, float (&pos)[3]) override;
      virtual void Stop() override;
      virtual void Pause() override;
      virtual bool IsStreaming() const override;
      virtual bool Skip(uint64_t sample, int format, uint32_t freq, void* context, bool isPlaying) override;
      virtual void FillBuffers(int format, uint32_t freq, void* context) override;
      virtual uint64_t GetOffset() const override;
      virtual void SetVolume(float range) override;
      virtual void SetPitch(float range) override;
      virtual void SetPosition(float (&pos)[3]) override;

    private:
      void _processBuffers(ALenum format, ALsizei freq, void* context);
      void _fillBuffers(ALenum format, ALsizei freq, void* context);
      void _queueBuffers();

      ALuint _source;
      ALuint* uiBuffers;
      char _bufstart;
      char _queuebuflen;
      OALEngine* _engine;
      ReadBuffer _readBuffer;
    };

  public:
    OALEngine(unsigned char bufferCount, const char* dllpath);
    ~OALEngine();
    virtual bool Init(const char* device = nullptr);
    virtual bool SetDevice(const char* device);
    virtual ENGINE_TYPE GetType() override { return ENGINE_TINYOAL; }
    virtual const char* GetDefaultDevice() override;
    virtual uint32_t GetNumBuffers() override { return defNumBuf; }
    virtual unsigned int GetFormat(unsigned short channels, unsigned short bits, bool rear) override;
    virtual unsigned int GetWaveFormat(WaveFileInfo& wave) override;
    virtual Source* GenSource(Source::ReadBuffer readBuffer) override;
    virtual void DestroySource(Source* source) override;

  private:
    ALuint* _alloc();
    void _dealloc(ALuint* buf);

    const unsigned char defNumBuf;
    std::unique_ptr<OPENALFNTABLE> oalFuncs;
    std::unique_ptr<char[]> _dllpath;
    bss::BlockAlloc _bufalloc;
  };
}

#endif
