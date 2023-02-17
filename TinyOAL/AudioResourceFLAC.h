// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef TOAL__AUDIO_RESOURCE_FLAC_H
#define TOAL__AUDIO_RESOURCE_FLAC_H

#include "tinyoal/AudioResource.h"
#include "FlacFunctions.h"

namespace tinyoal {
  struct DatStreamEx;

  // This is a resource class for OGG files, and handles all the IO operations from the given buffer
  class AudioResourceFLAC : public AudioResource
  {
  public:
    AudioResourceFLAC(const AudioResourceFLAC& copy);
    AudioResourceFLAC(void* data, uint32_t datalength, TINYOAL_FLAG flags, uint64_t loop);
    ~AudioResourceFLAC();
    virtual void* OpenStream();             // This returns a pointer to the internal stream on success, or NULL on failure
    virtual void CloseStream(void* stream); // This closes an AUDIOSTREAM pointer
    virtual unsigned long Read(void* stream, char* buffer, uint32_t len,
                               bool& eof); // Reads next chunk of data - buffer must be at least GetBufSize() long
    virtual bool Reset(void* stream);      // This resets a stream to the beginning
    virtual bool Skip(void* stream, uint64_t samples); // Sets a stream to given sample
    virtual uint64_t Tell(void* stream);               // Gets what sample a stream is currently on

    static size_t Construct(void* p, void* data, uint32_t datalength, TINYOAL_FLAG flags, uint64_t loop);
    static bool ScanHeader(const char* fileheader);
    static std::pair<void*, uint32_t> ToWave(void* data, uint32_t datalength, TINYOAL_FLAG flags);

  protected:
    static void _cberror(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data);
    static void _cbmeta(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data);
    static FLAC__StreamDecoderWriteStatus _cbwrite(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame,
                                                   const FLAC__int32* const buffer[], void* client_data);
    static FLAC__StreamDecoderWriteStatus _cbemptywrite(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame,
                                                        const FLAC__int32* const buffer[], void* client_data);
    static FLAC__StreamDecoderReadStatus _cbread(const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes,
                                                 void* client_data);
    static FLAC__StreamDecoderSeekStatus _cbseek(const FLAC__StreamDecoder* decoder, FLAC__uint64 absolute_byte_offset,
                                                 void* client_data);
    static FLAC__StreamDecoderTellStatus _cbtell(const FLAC__StreamDecoder* decoder, FLAC__uint64* absolute_byte_offset,
                                                 void* client_data);
    static FLAC__StreamDecoderLengthStatus _cblength(const FLAC__StreamDecoder* decoder, FLAC__uint64* stream_length,
                                                     void* client_data);
    static FLAC__bool _cbeof(const FLAC__StreamDecoder* decoder, void* client_data);
    static FLAC__StreamDecoderReadStatus _cbfread(const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes,
                                                  void* client_data);
    static FLAC__StreamDecoderSeekStatus _cbfseek(const FLAC__StreamDecoder* decoder, FLAC__uint64 absolute_byte_offset,
                                                  void* client_data);
    static FLAC__StreamDecoderSeekStatus _cbfseekoffset(const FLAC__StreamDecoder* decoder,
                                                        FLAC__uint64 absolute_byte_offset, void* client_data);
    static FLAC__StreamDecoderTellStatus _cbftell(const FLAC__StreamDecoder* decoder, FLAC__uint64* absolute_byte_offset,
                                                  void* client_data);
    static FLAC__bool _cbfeof(const FLAC__StreamDecoder* decoder, void* client_data);
    void* _openstream(bool empty);
    static DatStreamEx* _getstream();
    static void _closestream(void* stream);

    struct INTERNAL
    {
      char* _buffer;
      uint32_t _bytesread;
      uint32_t _len;
      uint64_t _cursample;
    } _internal;
    static DatStreamEx* _freelist;
    static size_t __flac_fseek_offset;
  };

  struct DatStreamEx
  {
    void* p; // INTERNAL* pointer
    FLAC__StreamDecoder* d;
    uint64_t cursample;
    union
    {
      DatStream stream;
      FILE* f;
      DatStreamEx* next;
    };
  };
}

#endif