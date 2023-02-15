// Copyright (c)2023 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "WASEngine.h"

#ifdef BSS_PLATFORM_WIN32

using namespace tinyoal;

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator    = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient           = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient     = __uuidof(IAudioRenderClient);


WASEngine::WASEngine() {}
WASEngine::~WASEngine() {}
bool WASEngine::Init(const char* device) { return 0; }
bool WASEngine::SetDevice(const char* device) { return 0; }



#endif