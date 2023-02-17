/*
 * Copyright (c) 2006, Creative Labs Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and
 * 	     the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 * 	     and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of Creative Labs Inc. nor the names of its contributors may be used to endorse or
 * 	     promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "loadoal.h"
#include "tinyoal/TinyOAL.h"
#include <ostream>

void* g_hOpenALDLL = nullptr;

#ifdef TINYOAL_STATICLIB
  #define DEFAULT_OAL_DLLPATH ""
  #define LOADDYNLIB(s)       (void*)(~0)
  #define GETDYNFUNC(p, s)    (&s)
  #define FREEDYNLIB(p)       ((void)0)
#else
  #ifdef BSS_PLATFORM_WIN32
    #include "win32_includes.h"
    #define DEFAULT_OAL_DLLPATH "OpenAL32.dll"
  #else
    #include <dlfcn.h>
    #define DEFAULT_OAL_DLLPATH "libopenal.so.1"
  #endif
#endif

ALboolean LoadOAL10Library(const char* szOALFullPathName, OPENALFNTABLE* lpOALFnTable)
{
  if(!lpOALFnTable)
  {
    TINYOAL_LOG(1, "lpOALFnTable cannot be NULL");
    return AL_FALSE;
  }

  if(!szOALFullPathName)
    szOALFullPathName = DEFAULT_OAL_DLLPATH;
  g_hOpenALDLL = LOADDYNLIB(szOALFullPathName);

  if(!g_hOpenALDLL)
  {
    TINYOAL_LOG(1, "Failed to load %s library!", szOALFullPathName);
    return AL_FALSE;
  }

  memset(lpOALFnTable, 0, sizeof(OPENALFNTABLE));

  // Get function pointers
  lpOALFnTable->alEnable = (LPALENABLE)GETDYNFUNC(g_hOpenALDLL, alEnable);
  if(lpOALFnTable->alEnable == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alEnable' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alDisable = (LPALDISABLE)GETDYNFUNC(g_hOpenALDLL, alDisable);
  if(lpOALFnTable->alDisable == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alDisable' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alIsEnabled = (LPALISENABLED)GETDYNFUNC(g_hOpenALDLL, alIsEnabled);
  if(lpOALFnTable->alIsEnabled == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alIsEnabled' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetBoolean = (LPALGETBOOLEAN)GETDYNFUNC(g_hOpenALDLL, alGetBoolean);
  if(lpOALFnTable->alGetBoolean == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetBoolean' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetInteger = (LPALGETINTEGER)GETDYNFUNC(g_hOpenALDLL, alGetInteger);
  if(lpOALFnTable->alGetInteger == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetInteger' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetFloat = (LPALGETFLOAT)GETDYNFUNC(g_hOpenALDLL, alGetFloat);
  if(lpOALFnTable->alGetFloat == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetFloat' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetDouble = (LPALGETDOUBLE)GETDYNFUNC(g_hOpenALDLL, alGetDouble);
  if(lpOALFnTable->alGetDouble == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetDouble' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetBooleanv = (LPALGETBOOLEANV)GETDYNFUNC(g_hOpenALDLL, alGetBooleanv);
  if(lpOALFnTable->alGetBooleanv == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetBooleanv' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetIntegerv = (LPALGETINTEGERV)GETDYNFUNC(g_hOpenALDLL, alGetIntegerv);
  if(lpOALFnTable->alGetIntegerv == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetIntegerv' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetFloatv = (LPALGETFLOATV)GETDYNFUNC(g_hOpenALDLL, alGetFloatv);
  if(lpOALFnTable->alGetFloatv == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetFloatv' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetDoublev = (LPALGETDOUBLEV)GETDYNFUNC(g_hOpenALDLL, alGetDoublev);
  if(lpOALFnTable->alGetDoublev == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetDoublev' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetString = (LPALGETSTRING)GETDYNFUNC(g_hOpenALDLL, alGetString);
  if(lpOALFnTable->alGetString == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetString' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetError = (LPALGETERROR)GETDYNFUNC(g_hOpenALDLL, alGetError);
  if(lpOALFnTable->alGetError == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetError' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alIsExtensionPresent = (LPALISEXTENSIONPRESENT)GETDYNFUNC(g_hOpenALDLL, alIsExtensionPresent);
  if(lpOALFnTable->alIsExtensionPresent == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alIsExtensionPresent' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetProcAddress = (LPALGETPROCADDRESS)GETDYNFUNC(g_hOpenALDLL, alGetProcAddress);
  if(lpOALFnTable->alGetProcAddress == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetProcAddress' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetEnumValue = (LPALGETENUMVALUE)GETDYNFUNC(g_hOpenALDLL, alGetEnumValue);
  if(lpOALFnTable->alGetEnumValue == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetEnumValue' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alListeneri = (LPALLISTENERI)GETDYNFUNC(g_hOpenALDLL, alListeneri);
  if(lpOALFnTable->alListeneri == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alListeneri' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alListenerf = (LPALLISTENERF)GETDYNFUNC(g_hOpenALDLL, alListenerf);
  if(lpOALFnTable->alListenerf == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alListenerf' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alListener3f = (LPALLISTENER3F)GETDYNFUNC(g_hOpenALDLL, alListener3f);
  if(lpOALFnTable->alListener3f == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alListener3f' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alListenerfv = (LPALLISTENERFV)GETDYNFUNC(g_hOpenALDLL, alListenerfv);
  if(lpOALFnTable->alListenerfv == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alListenerfv' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetListeneri = (LPALGETLISTENERI)GETDYNFUNC(g_hOpenALDLL, alGetListeneri);
  if(lpOALFnTable->alGetListeneri == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetListeneri' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetListenerf = (LPALGETLISTENERF)GETDYNFUNC(g_hOpenALDLL, alGetListenerf);
  if(lpOALFnTable->alGetListenerf == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetListenerf' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetListener3f = (LPALGETLISTENER3F)GETDYNFUNC(g_hOpenALDLL, alGetListener3f);
  if(lpOALFnTable->alGetListener3f == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetListener3f' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetListenerfv = (LPALGETLISTENERFV)GETDYNFUNC(g_hOpenALDLL, alGetListenerfv);
  if(lpOALFnTable->alGetListenerfv == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetListenerfv' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGenSources = (LPALGENSOURCES)GETDYNFUNC(g_hOpenALDLL, alGenSources);
  if(lpOALFnTable->alGenSources == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGenSources' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alDeleteSources = (LPALDELETESOURCES)GETDYNFUNC(g_hOpenALDLL, alDeleteSources);
  if(lpOALFnTable->alDeleteSources == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alDeleteSources' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alIsSource = (LPALISSOURCE)GETDYNFUNC(g_hOpenALDLL, alIsSource);
  if(lpOALFnTable->alIsSource == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alIsSource' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alSourcei = (LPALSOURCEI)GETDYNFUNC(g_hOpenALDLL, alSourcei);
  if(lpOALFnTable->alSourcei == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alSourcei' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alSourcef = (LPALSOURCEF)GETDYNFUNC(g_hOpenALDLL, alSourcef);
  if(lpOALFnTable->alSourcef == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alSourcef' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alSource3f = (LPALSOURCE3F)GETDYNFUNC(g_hOpenALDLL, alSource3f);
  if(lpOALFnTable->alSource3f == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alSource3f' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alSourcefv = (LPALSOURCEFV)GETDYNFUNC(g_hOpenALDLL, alSourcefv);
  if(lpOALFnTable->alSourcefv == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alSourcefv' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetSourcei = (LPALGETSOURCEI)GETDYNFUNC(g_hOpenALDLL, alGetSourcei);
  if(lpOALFnTable->alGetSourcei == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetSourcei' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetSourcef = (LPALGETSOURCEF)GETDYNFUNC(g_hOpenALDLL, alGetSourcef);
  if(lpOALFnTable->alGetSourcef == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetSourcef' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetSourcefv = (LPALGETSOURCEFV)GETDYNFUNC(g_hOpenALDLL, alGetSourcefv);
  if(lpOALFnTable->alGetSourcefv == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetSourcefv' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alSourcePlayv = (LPALSOURCEPLAYV)GETDYNFUNC(g_hOpenALDLL, alSourcePlayv);
  if(lpOALFnTable->alSourcePlayv == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alSourcePlayv' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alSourceStopv = (LPALSOURCESTOPV)GETDYNFUNC(g_hOpenALDLL, alSourceStopv);
  if(lpOALFnTable->alSourceStopv == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alSourceStopv' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alSourcePlay = (LPALSOURCEPLAY)GETDYNFUNC(g_hOpenALDLL, alSourcePlay);
  if(lpOALFnTable->alSourcePlay == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alSourcePlay' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alSourcePause = (LPALSOURCEPAUSE)GETDYNFUNC(g_hOpenALDLL, alSourcePause);
  if(lpOALFnTable->alSourcePause == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alSourcePause' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alSourceStop = (LPALSOURCESTOP)GETDYNFUNC(g_hOpenALDLL, alSourceStop);
  if(lpOALFnTable->alSourceStop == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alSourceStop' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGenBuffers = (LPALGENBUFFERS)GETDYNFUNC(g_hOpenALDLL, alGenBuffers);
  if(lpOALFnTable->alGenBuffers == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGenBuffers' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alDeleteBuffers = (LPALDELETEBUFFERS)GETDYNFUNC(g_hOpenALDLL, alDeleteBuffers);
  if(lpOALFnTable->alDeleteBuffers == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alDeleteBuffers' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alIsBuffer = (LPALISBUFFER)GETDYNFUNC(g_hOpenALDLL, alIsBuffer);
  if(lpOALFnTable->alIsBuffer == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alIsBuffer' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alBufferData = (LPALBUFFERDATA)GETDYNFUNC(g_hOpenALDLL, alBufferData);
  if(lpOALFnTable->alBufferData == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alBufferData' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetBufferi = (LPALGETBUFFERI)GETDYNFUNC(g_hOpenALDLL, alGetBufferi);
  if(lpOALFnTable->alGetBufferi == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetBufferi' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alGetBufferf = (LPALGETBUFFERF)GETDYNFUNC(g_hOpenALDLL, alGetBufferf);
  if(lpOALFnTable->alGetBufferf == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alGetBufferf' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alSourceQueueBuffers = (LPALSOURCEQUEUEBUFFERS)GETDYNFUNC(g_hOpenALDLL, alSourceQueueBuffers);
  if(lpOALFnTable->alSourceQueueBuffers == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alSourceQueueBuffers' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alSourceUnqueueBuffers = (LPALSOURCEUNQUEUEBUFFERS)GETDYNFUNC(g_hOpenALDLL, alSourceUnqueueBuffers);
  if(lpOALFnTable->alSourceUnqueueBuffers == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alSourceUnqueueBuffers' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alDistanceModel = (LPALDISTANCEMODEL)GETDYNFUNC(g_hOpenALDLL, alDistanceModel);
  if(lpOALFnTable->alDistanceModel == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alDistanceModel' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alDopplerFactor = (LPALDOPPLERFACTOR)GETDYNFUNC(g_hOpenALDLL, alDopplerFactor);
  if(lpOALFnTable->alDopplerFactor == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alDopplerFactor' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alDopplerVelocity = (LPALDOPPLERVELOCITY)GETDYNFUNC(g_hOpenALDLL, alDopplerVelocity);
  if(lpOALFnTable->alDopplerVelocity == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alDopplerVelocity' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcGetString = (LPALCGETSTRING)GETDYNFUNC(g_hOpenALDLL, alcGetString);
  if(lpOALFnTable->alcGetString == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcGetString' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcGetIntegerv = (LPALCGETINTEGERV)GETDYNFUNC(g_hOpenALDLL, alcGetIntegerv);
  if(lpOALFnTable->alcGetIntegerv == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcGetIntegerv' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcOpenDevice = (LPALCOPENDEVICE)GETDYNFUNC(g_hOpenALDLL, alcOpenDevice);
  if(lpOALFnTable->alcOpenDevice == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcOpenDevice' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcCloseDevice = (LPALCCLOSEDEVICE)GETDYNFUNC(g_hOpenALDLL, alcCloseDevice);
  if(lpOALFnTable->alcCloseDevice == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcCloseDevice' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcCreateContext = (LPALCCREATECONTEXT)GETDYNFUNC(g_hOpenALDLL, alcCreateContext);
  if(lpOALFnTable->alcCreateContext == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcCreateContext' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcMakeContextCurrent = (LPALCMAKECONTEXTCURRENT)GETDYNFUNC(g_hOpenALDLL, alcMakeContextCurrent);
  if(lpOALFnTable->alcMakeContextCurrent == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcMakeContextCurrent' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcProcessContext = (LPALCPROCESSCONTEXT)GETDYNFUNC(g_hOpenALDLL, alcProcessContext);
  if(lpOALFnTable->alcProcessContext == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcProcessContext' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcGetCurrentContext = (LPALCGETCURRENTCONTEXT)GETDYNFUNC(g_hOpenALDLL, alcGetCurrentContext);
  if(lpOALFnTable->alcGetCurrentContext == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcGetCurrentContext' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcGetContextsDevice = (LPALCGETCONTEXTSDEVICE)GETDYNFUNC(g_hOpenALDLL, alcGetContextsDevice);
  if(lpOALFnTable->alcGetContextsDevice == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcGetContextsDevice' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcSuspendContext = (LPALCSUSPENDCONTEXT)GETDYNFUNC(g_hOpenALDLL, alcSuspendContext);
  if(lpOALFnTable->alcSuspendContext == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcSuspendContext' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcDestroyContext = (LPALCDESTROYCONTEXT)GETDYNFUNC(g_hOpenALDLL, alcDestroyContext);
  if(lpOALFnTable->alcDestroyContext == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcDestroyContext' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcGetError = (LPALCGETERROR)GETDYNFUNC(g_hOpenALDLL, alcGetError);
  if(lpOALFnTable->alcGetError == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcGetError' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcIsExtensionPresent = (LPALCISEXTENSIONPRESENT)GETDYNFUNC(g_hOpenALDLL, alcIsExtensionPresent);
  if(lpOALFnTable->alcIsExtensionPresent == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcIsExtensionPresent' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcGetProcAddress = (LPALCGETPROCADDRESS)GETDYNFUNC(g_hOpenALDLL, alcGetProcAddress);
  if(lpOALFnTable->alcGetProcAddress == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcGetProcAddress' function address");
    return AL_FALSE;
  }
  lpOALFnTable->alcGetEnumValue = (LPALCGETENUMVALUE)GETDYNFUNC(g_hOpenALDLL, alcGetEnumValue);
  if(lpOALFnTable->alcGetEnumValue == nullptr)
  {
    TINYOAL_LOG(2, "Failed to retrieve 'alcGetEnumValue' function address");
    return AL_FALSE;
  }

  return AL_TRUE;
}

ALvoid UnloadOAL10Library()
{
  // Unload the dll
  if(g_hOpenALDLL)
  {
    FREEDYNLIB(g_hOpenALDLL);
    g_hOpenALDLL = nullptr;
  }
}
