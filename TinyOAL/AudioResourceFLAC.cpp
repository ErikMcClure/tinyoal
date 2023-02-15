// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "AudioResourceFLAC.h"
#include "tinyoal/TinyOAL.h"
#include "WaveFunctions.h"
#include "Engine.h"

using namespace tinyoal;

DatStreamEx* AudioResourceFLAC::_freelist = 0;

AudioResourceFLAC::AudioResourceFLAC(void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop) :
  AudioResource(data, datalength, flags, TINYOAL_FILETYPE_FLAC, loop)
{
  auto fn         = TinyOAL::Instance()->GetFlac();
  DatStreamEx* ex = (DatStreamEx*)_openstream(true);
  if(!ex)
    return;
  if(!fn->fn_flac_process_single(ex->d)) // Lets us pick up all this metadata
    TINYOAL_LOG(2, "Failed to preprocess first frame");
  _freq       = fn->fn_flac_get_sample_rate(ex->d);
  _channels   = fn->fn_flac_get_channels(ex->d);
  _samplebits = fn->fn_flac_get_bits_per_sample(ex->d);
  if(_samplebits == 24)
    _samplebits = 32;
  _bufsize = fn->fn_flac_get_block_size(ex->d) * (_samplebits >> 3) * _channels * 2; // Allocate enough space for 2 blocks
  _format  = TinyOAL::Instance()->GetEngine()->GetFormat(_channels, _samplebits, false);
  _total   = fn->fn_flac_get_total_samples(ex->d);
  CloseStream(ex);
}
AudioResourceFLAC::~AudioResourceFLAC() { _destruct(); }
void* AudioResourceFLAC::OpenStream() { return _openstream(false); }
void* AudioResourceFLAC::_openstream(bool empty)
{
  auto fn = TinyOAL::Instance()->GetFlac();
  if(!fn)
  {
    TINYOAL_LOG(1, "GetFlac() is NULL, cannot open stream");
    return 0;
  }

  DatStreamEx* stream = _getstream();
  stream->p           = &_internal;
  stream->cursample   = 0;
  FLAC__StreamDecoderInitStatus err;
  stream->stream.datalength = _datalength; // This still works, even for a file, because of DatStream's layout.

  if(_flags & TINYOAL_ISFILE)
  {
    fseek((FILE*)_data, 0, SEEK_SET);
    stream->f = (FILE*)_data;
    err       = fn->fn_flac_init_stream(stream->d, &_cbfread, &_cbfseek, &_cbftell, &_cblength, &_cbfeof,
                                  empty ? &_cbemptywrite : &_cbwrite, &_cbmeta, &_cberror, stream);
  }
  else
  {
    stream->stream.data = stream->stream.streampos = (const char*)_data;
    err = fn->fn_flac_init_stream(stream->d, &_cbread, &_cbseek, &_cbtell, &_cblength, &_cbeof,
                                  empty ? &_cbemptywrite : &_cbwrite, &_cbmeta, &_cberror, stream);
  }

  if(err != 0)
  {
    TINYOAL_LOG(2, "fn_flac_init_stream failed with error code %i", (int)err);
    CloseStream(stream);
    return 0;
  }

  if(!fn->fn_flac_process_until_metadata_end(stream->d))
    TINYOAL_LOG(4, "fn_flac_process_until_metadata_end failed in _openstream()");

  return stream;
}
DatStreamEx* AudioResourceFLAC::_getstream()
{
  DatStreamEx* r;
  if(!_freelist)
  {
    r    = new DatStreamEx();
    r->d = TinyOAL::Instance()->GetFlac()->fn_flac_new();
  }
  else
  {
    r         = _freelist;
    _freelist = _freelist->next;
  }
  return r;
}

void AudioResourceFLAC::CloseStream(void* stream) { _closestream(stream); }
void AudioResourceFLAC::_closestream(void* stream)
{
  DatStreamEx* ex = (DatStreamEx*)stream;
  TinyOAL::Instance()->GetFlac()->fn_flac_finish(ex->d);
  ex->next  = _freelist;
  _freelist = ex;
}
unsigned long AudioResourceFLAC::Read(void* stream, char* buffer, unsigned int len, bool& eof)
{
  DatStreamEx* ex = (DatStreamEx*)stream;
  auto fn         = TinyOAL::Instance()->GetFlac();

  _internal._len       = len;
  _internal._buffer    = buffer;
  _internal._bytesread = 0;
  while(_internal._len > 0 && !(eof = (fn->fn_flac_get_state(ex->d) >= FLAC__STREAM_DECODER_END_OF_STREAM)))
    fn->fn_flac_process_single(ex->d);
  _internal._len = 0;

  if(_internal._cursample != -1LL &&
     !eof) //_cursample gets set by our write callback. If it's -1, then we didn't need to terminate early.
    if(!fn->fn_flac_seek(ex->d, _internal._cursample))
      TINYOAL_LOG(4, "fn_flac_seek failed to seek to %llu", _internal._cursample);
  return _internal._bytesread;
}
bool AudioResourceFLAC::Reset(void* stream)
{
  ((DatStreamEx*)stream)->cursample = 0;
  if(TinyOAL::Instance()->GetFlac()->fn_flac_reset(((DatStreamEx*)stream)->d) != 0)
    return true;
  TINYOAL_LOG(2, "fn_flac_reset failed");
  return false;
}
bool AudioResourceFLAC::Skip(void* stream, uint64_t samples)
{
  _internal._len = 0; // Because FLAC was written by morons we have to make sure we don't go writing random shit willy-nilly
  if(!samples)
    return Reset(stream);
  ((DatStreamEx*)stream)->cursample = samples;
  if(TinyOAL::Instance()->GetFlac()->fn_flac_seek(((DatStreamEx*)stream)->d, samples) != 0)
    return true;
  TINYOAL_LOG(2, "fn_flac_seek failed to seek to %llu", samples);
  if(TinyOAL::Instance()->GetFlac()->fn_flac_get_state(((DatStreamEx*)stream)->d) == FLAC__STREAM_DECODER_SEEK_ERROR)
    Reset(stream); // FLAC requires us to reset or flush the stream if seeking fails with FLAC__STREAM_DECODER_SEEK_ERROR
  return false;
}
uint64_t AudioResourceFLAC::Tell(void* stream) { return ((DatStreamEx*)stream)->cursample; }
void AudioResourceFLAC::_cberror(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status,
                                 void* client_data)
{
  switch(status)
  {
  case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
    TINYOAL_LOG(2, "(FLAC CODEC ERROR) An error in the stream caused the decoder to lose synchronization.");
    break;
  case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
    TINYOAL_LOG(2, "(FLAC CODEC ERROR) The decoder encountered a corrupted frame header.");
    break;
  case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
    TINYOAL_LOG(2, "(FLAC CODEC ERROR) The frame's data did not match the CRC in the footer.");
    break;
  case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
    TINYOAL_LOG(2, "(FLAC CODEC ERROR) The decoder encountered reserved fields in use in the stream.");
    break;
  default: TINYOAL_LOG(2, "Unknown FLAC CODEC ERROR %i", (int)status);
  }
}
void AudioResourceFLAC::_cbmeta(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data)
{}

template<typename T>
BSS_FORCEINLINE void r_flacread(T* target, const FLAC__int32* const buffer[], unsigned int num, unsigned int channels)
{
  for(unsigned int i = 0; i < num; i += 1)
  {
    for(unsigned int j = 0; j < channels; ++j)
      target[j] = (T)buffer[j][i];
    target += channels;
  }
}
template<>
BSS_FORCEINLINE void r_flacread<float>(float* target, const FLAC__int32* const buffer[], unsigned int num,
                                       unsigned int channels)
{
  for(unsigned int i = 0; i < num; i += 1)
  {
    for(unsigned int j = 0; j < channels; ++j)
      target[j] = (float)buffer[j][i] / 8388607.0f;
    target += channels;
  }
}
FLAC__StreamDecoderWriteStatus AudioResourceFLAC::_cbwrite(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame,
                                                           const FLAC__int32* const buffer[], void* client_data)
{
  INTERNAL* self        = *(INTERNAL**)client_data;
  unsigned int channels = frame->header.channels;
  unsigned int num      = frame->header.blocksize;
  if(num == 1)
    num = 192;
  else if(num >= 2 && num <= 5)
    num = 576 * (1 << (num - 2));
  else if(num >= 8 && num <= 15)
    num = 256 * (1 << (num - 8));

  unsigned int bits      = frame->header.bits_per_sample == 24 ? 32 : frame->header.bits_per_sample;
  unsigned int persample = channels * (bits >> 3);
  unsigned int len       = self->_len / persample;

  if(num > len)
  {
    self->_len = 0;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
  }

  unsigned int bytes = num * persample;
  switch(bits)
  {
  case 8: r_flacread<char>(reinterpret_cast<char*>(self->_buffer), buffer, num, channels); break;
  case 16: r_flacread<short>(reinterpret_cast<short*>(self->_buffer), buffer, num, channels); break;
  case 24: // 24-bit gets converted to 32-bit
  case 32: r_flacread<float>(reinterpret_cast<float*>(self->_buffer), buffer, num, channels); break;
  }

  self->_bytesread += bytes;
  self->_buffer += bytes;
  self->_len -= bytes;
  ((DatStreamEx*)client_data)->cursample += num;
  self->_cursample = (self->_len > 0) ? ((DatStreamEx*)client_data)->cursample : (uint64_t)-1;

  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

size_t AudioResourceFLAC::__flac_fseek_offset =
  0; // Special seek function that properly deals with external FILE* handles we can get in ToWave
FLAC__StreamDecoderSeekStatus AudioResourceFLAC::_cbfseekoffset(const FLAC__StreamDecoder* decoder,
                                                                FLAC__uint64 absolute_byte_offset, void* client_data)
{
  if(fseek(((DatStreamEx*)client_data)->f, absolute_byte_offset + __flac_fseek_offset, SEEK_SET) < 0)
    return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
  else
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

size_t AudioResourceFLAC::Construct(void* p, void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop)
{
  if(p)
    new(p) AudioResourceFLAC(data, datalength, flags, loop);
  return sizeof(AudioResourceFLAC);
}
bool AudioResourceFLAC::ScanHeader(const char* fileheader) { return !strncmp(fileheader, "fLaC", 4); }

std::pair<void*, unsigned int> AudioResourceFLAC::ToWave(void* data, unsigned int datalength, TINYOAL_FLAG flags)
{
  static const std::pair<void*, unsigned int> NULLRET(nullptr, 0);
  auto fn = TinyOAL::Instance()->GetFlac();
  if(!fn)
  {
    TINYOAL_LOG(1, "GetFlac() is NULL, cannot open stream");
    return NULLRET;
  }

  DatStreamEx* stream = _getstream();
  if(!stream)
    return NULLRET;

  INTERNAL internal;
  stream->p         = &internal; // Note: This is safe because we close the stream before the end of the scope.
  stream->cursample = 0;
  FLAC__StreamDecoderInitStatus err;
  stream->stream.datalength = datalength;

  if(flags & TINYOAL_ISFILE)
  {
    __flac_fseek_offset = ftell((FILE*)data);
    stream->f           = (FILE*)data;
    err = fn->fn_flac_init_stream(stream->d, &_cbfread, &_cbfseekoffset, &_cbftell, &_cblength, &_cbfeof, &_cbwrite,
                                  &_cbmeta, &_cberror, stream);
  }
  else
  {
    stream->stream.data = stream->stream.streampos = (const char*)data;
    err = fn->fn_flac_init_stream(stream->d, &_cbread, &_cbseek, &_cbtell, &_cblength, &_cbeof, &_cbwrite, &_cbmeta,
                                  &_cberror, stream);
  }

  if(err != 0)
  {
    TINYOAL_LOG(2, "fn_flac_init_stream failed with error code %i", (int)err);
    _closestream(stream);
    return NULLRET;
  }

  if(!fn->fn_flac_process_until_metadata_end(stream->d))
    TINYOAL_LOG(4, "fn_flac_process_until_metadata_end failed in _openstream()");

  internal._len = 0;

  if(!fn->fn_flac_process_single(stream->d)) // Lets us pick up all this metadata
    TINYOAL_LOG(2, "Failed to preprocess first frame");

  unsigned int channels   = fn->fn_flac_get_channels(stream->d);
  unsigned int samplebits = fn->fn_flac_get_bits_per_sample(stream->d);
  if(samplebits == 24)
    samplebits = 32;
  unsigned int freq   = fn->fn_flac_get_sample_rate(stream->d);
  uint64_t total      = fn->fn_flac_get_total_samples(stream->d);
  uint64_t totalbytes = total * channels * (samplebits >> 3);
  unsigned int header = TinyOAL::Instance()->GetWave()->WriteHeader(0, 0, 0, 0, 0);
  char* buffer        = (char*)malloc(totalbytes + header);
  assert(buffer != 0);
  TinyOAL::Instance()->GetFlac()->fn_flac_reset(stream->d);
  internal._len       = totalbytes;
  internal._buffer    = buffer + header;
  internal._bytesread = 0;
  fn->fn_flac_process_until_stream_end(stream->d);
  TinyOAL::Instance()->GetWave()->WriteHeader(buffer, internal._bytesread + header, channels, samplebits, freq);

  _closestream(stream);
  return std::pair<void*, unsigned int>(buffer, internal._bytesread + header);
}
FLAC__StreamDecoderWriteStatus AudioResourceFLAC::_cbemptywrite(const FLAC__StreamDecoder* decoder,
                                                                const FLAC__Frame* frame, const FLAC__int32* const buffer[],
                                                                void* client_data)
{ // This function exists so we can yank the metadata out of the stream without actually putting it anywhere.
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

FLAC__StreamDecoderReadStatus AudioResourceFLAC::_cbread(const FLAC__StreamDecoder* decoder, FLAC__byte buffer[],
                                                         size_t* bytes, void* client_data)
{
  if(*bytes > 0)
  {
    *bytes = dat_read_func(buffer, sizeof(FLAC__byte), *bytes, &((DatStreamEx*)client_data)->stream);
    if(*bytes == 0)
      return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    else
      return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
  }
  else
    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}
FLAC__StreamDecoderSeekStatus AudioResourceFLAC::_cbseek(const FLAC__StreamDecoder* decoder,
                                                         FLAC__uint64 absolute_byte_offset, void* client_data)
{
  if(dat_seek_func(&((DatStreamEx*)client_data)->stream, (long long)absolute_byte_offset, SEEK_SET) < 0)
    return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
  else
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}
FLAC__StreamDecoderTellStatus AudioResourceFLAC::_cbtell(const FLAC__StreamDecoder* decoder,
                                                         FLAC__uint64* absolute_byte_offset, void* client_data)
{
  long pos;
  if((pos = dat_tell_func(&((DatStreamEx*)client_data)->stream)) < 0)
    return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;

  *absolute_byte_offset = (FLAC__uint64)pos;
  return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}
FLAC__StreamDecoderLengthStatus AudioResourceFLAC::_cblength(const FLAC__StreamDecoder* decoder,
                                                             FLAC__uint64* stream_length, void* client_data)
{
  *stream_length = ((DatStreamEx*)client_data)->stream.datalength;
  return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}
FLAC__bool AudioResourceFLAC::_cbeof(const FLAC__StreamDecoder* decoder, void* client_data)
{
  DatStreamEx* stream = (DatStreamEx*)client_data;
  return (((char*)stream->stream.data) + stream->stream.datalength) == stream->stream.streampos;
}
FLAC__StreamDecoderReadStatus AudioResourceFLAC::_cbfread(const FLAC__StreamDecoder* decoder, FLAC__byte buffer[],
                                                          size_t* bytes, void* client_data)
{
  if(*bytes > 0)
  {
    *bytes = fread(buffer, sizeof(FLAC__byte), *bytes, ((DatStreamEx*)client_data)->f);
    if(*bytes == 0)
      return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    else
      return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
  }
  else
    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}
FLAC__StreamDecoderSeekStatus AudioResourceFLAC::_cbfseek(const FLAC__StreamDecoder* decoder,
                                                          FLAC__uint64 absolute_byte_offset, void* client_data)
{
  if(fseek(((DatStreamEx*)client_data)->f, absolute_byte_offset, SEEK_SET) < 0)
    return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
  else
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}
FLAC__StreamDecoderTellStatus AudioResourceFLAC::_cbftell(const FLAC__StreamDecoder* decoder,
                                                          FLAC__uint64* absolute_byte_offset, void* client_data)
{
  long pos;
  if((pos = ftell(((DatStreamEx*)client_data)->f)) < 0)
    return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;

  *absolute_byte_offset = (FLAC__uint64)pos;
  return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}
FLAC__bool AudioResourceFLAC::_cbfeof(const FLAC__StreamDecoder* decoder, void* client_data)
{
  return feof(((DatStreamEx*)client_data)->f);
}
