// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudioResourceM4A.h"
#include "cTinyOAL.h"
#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <Propvarutil.h>

using namespace tinyoal;

#pragma comment(lib, "Mfuuid.lib")
#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "Shlwapi.lib")

cAudioResourceM4A::cAudioResourceM4A(void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop) : cAudioResource(data, datalength, flags, TINYOAL_FILETYPE_M4A, loop)
{
  if(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
    TINYOAL_LOGM("ERROR", "SSMF: failed to initialize COM");
  if(FAILED(MFStartup(MF_VERSION)))
    TINYOAL_LOGM("ERROR", "SSMF: failed to initialize Media Foundation");
}
cAudioResourceM4A::~cAudioResourceM4A()
{
  MFShutdown();
  CoUninitialize();
  _destruct();
}
void* cAudioResourceM4A::OpenStream()
{
  sMP4* r = new sMP4();
  IStream* stream = SHCreateMemStream((const BYTE*)_data, _datalength);
  IMFByteStream* mfstream;
  MFCreateMFByteStreamOnStream(stream, &mfstream);
  IMFSourceReader* reader;
  HRESULT hr = MFCreateSourceReaderFromByteStream(mfstream, NULL, &reader);
  if(FAILED(hr)) {
    TINYOAL_LOG("ERROR") << "SSMF: Error opening input file with error: " << HRESULT_CODE(hr) << std::endl;
    return 0;
  }

  reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false);
  reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true);

  IMFMediaType* type;
  if(FAILED(reader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &type)))
  {
    TINYOAL_LOG("ERROR") << "SSMF: failed to retrieve native media type: " << HRESULT_CODE(hr) << std::endl;
    return 0;
  }

  UINT32 bitsPerSample = 0;
  UINT32 channels = 0;
  UINT32 samplesPerSecond = 0;
  hr = type->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);
  hr = type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
  hr = type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);

  _channels = channels;
  _samplebits = bitsPerSample;
  _freq = samplesPerSecond;
  _total = 0;

  if(!_samplebits)
    _samplebits = 16;

  _format = cTinyOAL::GetFormat(_channels, _samplebits, false);

  if(FAILED(MFCreateMediaType(&type))) {
    TINYOAL_LOG("ERROR") << "SSMF: failed to create media type: " << HRESULT_CODE(hr) << std::endl;
    return 0;
  }

  hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
  hr = type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
  hr = type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
  hr = type->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, true);
  hr = type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
  hr = type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, channels * (bitsPerSample / 8));
  
  //MediaFoundation will not convert between mono and stereo or between samplerates without a transform in the pipeline
  hr = type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channels);
  hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, samplesPerSecond);

  // Set this type on the source reader. The source reader will load the necessary decoder.
  hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, type);
  type->Release();
  hr = reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true);

  // Get the complete uncompressed format.
  hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &type);

  PROPVARIANT prop;
  hr = reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &prop); //Get the duration, provided as a 64-bit integer of 100-nanosecond units
  _bufsize = (_freq * _channels * 2) >> 2;
  _total = (prop.hVal.QuadPart*_freq) / 1e7;
  PropVariantClear(&prop);
  type->Release();

  r->reader = reader;
  Skip(r, 0); // skips headers
  return r;
}
void cAudioResourceM4A::CloseStream(void* stream)
{
  sMP4* s = (sMP4*)stream;
  s->reader->Release();
  delete s;
}
unsigned long cAudioResourceM4A::Read(void* stream, char* buffer, unsigned int len, bool& eof)
{
  sMP4* s = (sMP4*)stream;

  unsigned long read = bssmin(len, s->buf.size());
  if(read > 0)
  {
    memcpy(buffer, s->buf.data(), read);
    s->buf.resize(0);
  }

  while(len > read) {
    DWORD dwFlags = 0;
    IMFSample* sample = NULL;
    
    HRESULT hr = s->reader->ReadSample(
      MF_SOURCE_READER_FIRST_AUDIO_STREAM, // [in] DWORD dwStreamIndex,
      0,                                   // [in] DWORD dwControlFlags,
      NULL,                                // [out] DWORD *pdwActualStreamIndex,
      &dwFlags,                            // [out] DWORD *pdwStreamFlags,
      &s->cur,                          // [out] LONGLONG *pllTimestamp,
      &sample);                           // [out] IMFSample **ppSample
    if(FAILED(hr) || !sample || (dwFlags&(MF_SOURCE_READERF_ERROR | MF_SOURCE_READERF_ENDOFSTREAM)))
      return read;

    IMFMediaBuffer* mfbuf;
    DWORD count = 0;
    DWORD maxlength = 0;
    DWORD curlength = 0;
    sample->GetBufferCount(&count);
    BYTE* buf;

    for(DWORD i = 0; i < count; ++i)
    {
      if(FAILED(sample->GetBufferByIndex(i, &mfbuf)))
        continue;
      mfbuf->Lock(&buf, &maxlength, &curlength);
      DWORD split = bssmin(curlength, len - read);
      memcpy(buffer + read, buf, split);
      read += split;
      if(curlength > split)
      {
        DWORD extra = curlength - split;
        DWORD pos = s->buf.size();
        s->buf.resize(pos + extra);
        char* begin = s->buf.data() + pos; // have to do this after resize
        memcpy(begin, buf + split, extra);
      }
      mfbuf->Unlock();
      mfbuf->Release();
    }
    sample->Release();
  }

  return read;
}
bool cAudioResourceM4A::Reset(void* stream)
{
  return Skip(stream, 0);
}
bool cAudioResourceM4A::Skip(void* stream, uint64_t samples)
{
  sMP4* s = (sMP4*)stream;
  s->buf.resize(0);

  if(FAILED(s->reader->Flush(MF_SOURCE_READER_FIRST_AUDIO_STREAM)))
    TINYOAL_LOGM("WARNING", "SSMF: failed to flush before seek");

  //int64_t seekTarget = samples / _channels;
  s->cur = static_cast<int64_t>((static_cast<double>(samples) / _freq) * 1e7);

  PROPVARIANT prop;
  InitPropVariantFromInt64(s->cur, &prop);
  if(FAILED(s->reader->SetCurrentPosition(GUID_NULL, prop)))
    return false;
  PropVariantClear(&prop);
  return true;
}
uint64_t cAudioResourceM4A::Tell(void* stream)
{
  sMP4* s = (sMP4*)stream;
  return (s->cur*_freq)/1e7;
}

size_t cAudioResourceM4A::Construct(void* p, void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop)
{
  if(p) new(p) cAudioResourceM4A(data, datalength, flags, loop);
  return sizeof(cAudioResourceM4A);
}
bool cAudioResourceM4A::ScanHeader(const char* fileheader)
{
  return !strncmp(fileheader + 4, "ftyp", 4);
}

std::pair<void*, unsigned int> cAudioResourceM4A::ToWave(void* data, unsigned int datalength, TINYOAL_FLAG flags)
{
  std::pair<void*, unsigned int> d(0, 0);
  return d;
}