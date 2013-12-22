// Copyright ©2013 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __C_AUDIO_RESOURCE_FLAC_H__TOAL__
#define __C_AUDIO_RESOURCE_FLAC_H__TOAL__

#include "cAudioResource.h"
#include "cFlacFunctions.h"

namespace TinyOAL {
  struct DatStreamEx;

	// This is a resource class for OGG files, and handles all the IO operations from the given buffer 
  class cAudioResourceFLAC : public cAudioResource
  {
  public:
    cAudioResourceFLAC(const cAudioResourceFLAC& copy);
    cAudioResourceFLAC(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned __int64 loop);
    ~cAudioResourceFLAC();
    virtual void* OpenStream(); // This returns a pointer to the internal stream on success, or NULL on failure 
    virtual void CloseStream(void* stream); //This closes an AUDIOSTREAM pointer
    virtual unsigned long Read(void* stream, char* buffer, unsigned int len, bool& eof); // Reads next chunk of data - buffer must be at least GetBufSize() long 
    virtual bool Reset(void* stream); // This resets a stream to the beginning 
    virtual bool Skip(void* stream, unsigned __int64 samples); // Sets a stream to given sample 
    virtual unsigned __int64 Tell(void* stream); // Gets what sample a stream is currently on

    static std::pair<void*,unsigned int> ToWave(void* data, unsigned int datalength, TINYOAL_FLAG flags);

  protected:
    static void _cberror(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);
    static void _cbmeta(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
    static FLAC__StreamDecoderWriteStatus _cbwrite(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
    static FLAC__StreamDecoderWriteStatus _cbemptywrite(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
    static FLAC__StreamDecoderReadStatus _cbread(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data);
    static FLAC__StreamDecoderSeekStatus _cbseek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data);
    static FLAC__StreamDecoderTellStatus _cbtell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data);
    static FLAC__StreamDecoderLengthStatus _cblength(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data);
    static FLAC__bool _cbeof(const FLAC__StreamDecoder *decoder, void *client_data);
    static FLAC__StreamDecoderReadStatus _cbfread(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data);
    static FLAC__StreamDecoderSeekStatus _cbfseek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data);
    static FLAC__StreamDecoderSeekStatus _cbfseekoffset(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data);
    static FLAC__StreamDecoderTellStatus _cbftell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data);
    static FLAC__bool _cbfeof(const FLAC__StreamDecoder *decoder, void *client_data);
    void* _openstream(bool empty);
    static DatStreamEx* _getstream();
    static void _closestream(void* stream);

    struct INTERNAL {
      char* _buffer;
      unsigned int _bytesread;
      unsigned int _len;
      unsigned __int64 _cursample;
    } _internal;
    static DatStreamEx* _freelist;
    static size_t __flac_fseek_offset;
  };

  struct DatStreamEx {
    void* p; //INTERNAL* pointer
    FLAC__StreamDecoder* d;
    unsigned __int64 cursample;
    union {
      DatStream stream;
      FILE* f;
      DatStreamEx* next;
    };
  };
}

#endif