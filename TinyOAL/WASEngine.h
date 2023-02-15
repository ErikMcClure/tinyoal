// Copyright (c)2023 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef TOAL__WASENGINE_H
#define TOAL__WASENGINE_H

#include "bss-util/compiler.h"
#include "Engine.h"

#ifdef BSS_PLATFORM_WIN32
  #include <audioclient.h>
  #include <mmdeviceapi.h>

namespace tinyoal {
  class WASEngine : Engine
  {
    WASEngine();
    ~WASEngine();
    bool Init(const char* device = nullptr) override;
    bool SetDevice(const char* device) override;
    virtual ENGINE_TYPE GetType() override { return ENGINE_WASAPI; }

    IMMDeviceEnumerator* _enumerator;
    IMMDevice* _device;
    IAudioClient* _client;
    WAVEFORMATEX* _streamformat;
    WAVEFORMATEX* _mixformat;
    REFERENCE_TIME _duration;
  };
}

#endif
#endif