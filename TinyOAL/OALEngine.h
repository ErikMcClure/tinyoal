// Copyright (c)2023 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef TOAL__OALENGINE_H
#define TOAL__OALENGINE_H

#include "buntils/compiler.h"
#include "loadoal.h"
#include "Engine.h"
#include "buntils/BlockAlloc.h"
#include <memory>

namespace tinyoal {
  class OALEngine : public Engine
  {
    class OALSource : public Source
    {
    public:
      OALSource(OALEngine* engine, LoadBuffer loadBuffer, int format, uint32_t freq, size_t bufsize);
      ~OALSource();
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
      void _processBuffers(void* context);
      void _fillBuffers(void* context);
      void _queueBuffers();

      ALuint _source;
      ALuint* uiBuffers;
      char _bufstart;
      char _queuebuflen;
      OALEngine* _engine;
      LoadBuffer _loadBuffer;
      const size_t _bufsize;
      const int _format; 
      const uint32_t _freq;
      char* _buffer;
    };

  public:
    OALEngine(unsigned char bufferCount, const char* dllpath);
    ~OALEngine();
    virtual bool Init(const char* device = nullptr);
    virtual bool SetDevice(const char* device);
    virtual ENGINE_TYPE GetType() override { return ENGINE_OPENAL; }
    virtual size_t GetDefaultDevice(char* out, size_t len) override;
    virtual uint32_t GetFormat(uint16_t channels, uint16_t bits, bool rear) override;
    virtual uint32_t GetWaveFormat(WaveFileInfo& wave) override;
    virtual Source* GenSource(Source::LoadBuffer loadBuffer, size_t bufsize, int format, uint32_t freq) override;
    virtual void DestroySource(Source* source) override;

    static std::pair<uint16_t, uint16_t> ExtractFormat(uint32_t format);

  private:
    ALuint* _alloc();
    void _dealloc(ALuint* buf);

    const unsigned char defNumBuf;
    std::unique_ptr<OPENALFNTABLE> oalFuncs;
    std::unique_ptr<char[]> _dllpath;
    bun::BlockAlloc _bufalloc;
  };
}

#endif
