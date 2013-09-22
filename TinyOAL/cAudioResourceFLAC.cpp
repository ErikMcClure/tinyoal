// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudioResourceFLAC.h"
#include "cTinyOAL.h"
#include "cWaveFunctions.h"
#include "openAL/loadoal.h"

using namespace TinyOAL;

DatStreamEx* cAudioResourceFLAC::_freelist=0;

cAudioResourceFLAC::cAudioResourceFLAC(const cAudioResourceFLAC& copy) : cAudioResource(copy) {}
cAudioResourceFLAC::cAudioResourceFLAC(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned __int64 loop) : cAudioResource(data, datalength, flags, loop)
{
  auto fn = cTinyOAL::Instance()->flacFuncs;
  DatStreamEx* ex = (DatStreamEx*)_openstream(true);
  if(!ex) return;
  if(!fn->fn_flac_process_single(ex->d)) // Lets us pick up all this metadata
    TINYOAL_LOGM("WARNING","Failed to preprocess first frame");
  _freq=fn->fn_flac_get_sample_rate(ex->d);
  _channels=fn->fn_flac_get_channels(ex->d);
  _samplebits=fn->fn_flac_get_bits_per_sample(ex->d);
  if(_samplebits==24) _samplebits=32;
  _bufsize=fn->fn_flac_get_block_size(ex->d)*(_samplebits>>3)*_channels*2; // Allocate enough space for 2 blocks
  _format=cTinyOAL::GetFormat(_channels,_samplebits,false);
  CloseStream(ex);
}
cAudioResourceFLAC::~cAudioResourceFLAC()
{
  DatStreamEx* h;
  while(h=_freelist)
  {
    _freelist=h->next;
    cTinyOAL::Instance()->flacFuncs->fn_flac_delete(h->d);
    delete h;
  }

  _destruct();
}
void* cAudioResourceFLAC::OpenStream()
{
  return _openstream(false);
}
void* cAudioResourceFLAC::_openstream(bool empty)
{
  auto fn = cTinyOAL::Instance()->flacFuncs;
  if(!fn) { TINYOAL_LOGM("ERROR","flacFuncs is NULL, cannot open stream"); return 0; }
  DatStreamEx* stream = _getstream();
  stream->p=this;
  stream->cursample=0;
  FLAC__StreamDecoderInitStatus err;
  if(_flags&TINYOAL_ISFILE) {
    fseek((FILE*)_data,0,SEEK_SET);
    stream->f=(FILE*)_data;
    err=fn->fn_flac_init_stream(stream->d,&_cbfread,&_cbfseek,&_cbftell,&_cbflength,&_cbfeof,empty?&_cbemptywrite:&_cbwrite,&_cbmeta,&_cberror,stream);
  } else {
    stream->stream.data=stream->stream.streampos=(const char*)_data;
    stream->stream.datalength=_datalength;
    err=fn->fn_flac_init_stream(stream->d,&_cbread,&_cbseek,&_cbtell,&_cblength,&_cbeof,empty?&_cbemptywrite:&_cbwrite,&_cbmeta,&_cberror,stream);
  }
  if(err!=0) { TINYOAL_LOG("WARNING") << "fn_flac_init_stream failed with error code " << err << std::endl; CloseStream(stream); return 0; }
  if(!fn->fn_flac_process_until_metadata_end(stream->d))
    TINYOAL_LOGM("INFO","fn_flac_process_until_metadata_end failed in _openstream()");

  return stream;
}
DatStreamEx* cAudioResourceFLAC::_getstream()
{
  DatStreamEx* r;
  if(!_freelist)
  {
    r = new DatStreamEx();
    r->d = cTinyOAL::Instance()->flacFuncs->fn_flac_new();
  }
  else
  {
    r=_freelist;
    _freelist=_freelist->next;
  }
  return r;
}
void cAudioResourceFLAC::CloseStream(void* stream)
{
  DatStreamEx* ex = (DatStreamEx*)stream;
  cTinyOAL::Instance()->flacFuncs->fn_flac_finish(ex->d);
  ex->next=_freelist;
  _freelist=ex;
}
unsigned long cAudioResourceFLAC::Read(void* stream, char* buffer, unsigned int len, bool& eof)
{
  DatStreamEx* ex = (DatStreamEx*)stream;
  auto fn = cTinyOAL::Instance()->flacFuncs;

  _len=len;
  _buffer=buffer;
  _bytesread=0;
  while(_len>0 && !(eof=fn->fn_flac_get_state(ex->d)>=FLAC__STREAM_DECODER_END_OF_STREAM))
    fn->fn_flac_process_single(ex->d);
  _len=0;
  if(_cursample!=-1LL && !eof) //_cursample gets set by our write callback. If it's -1, then we didn't need to terminate early.
    if(!fn->fn_flac_seek(ex->d,_cursample))
      TINYOAL_LOG("INFO") << "fn_flac_seek failed to seek to " << _cursample << std::endl;
  return _bytesread;
}
bool cAudioResourceFLAC::Reset(void* stream)
{
  ((DatStreamEx*)stream)->cursample=0;
  if(cTinyOAL::Instance()->flacFuncs->fn_flac_reset(((DatStreamEx*)stream)->d)!=0)
    return true;
  TINYOAL_LOGM("WARNING","fn_flac_reset failed");
  return false;
}
bool cAudioResourceFLAC::Skip(void* stream, unsigned __int64 samples)
{
  _len=0; // Because FLAC was written by morons we have to make sure we don't go writing random shit willy-nilly
  if(!samples) return Reset(stream);
  ((DatStreamEx*)stream)->cursample=samples;
  if(cTinyOAL::Instance()->flacFuncs->fn_flac_seek(((DatStreamEx*)stream)->d,samples)!=0)
    return true;
  TINYOAL_LOG("WARNING") << "fn_flac_seek failed to seek to " << samples << std::endl;
  return false;
}
unsigned __int64 cAudioResourceFLAC::Tell(void* stream)
{
  return ((DatStreamEx*)stream)->cursample;
}
void cAudioResourceFLAC::_cberror(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
  const char* s="Unknown error";
  switch(status)
  {
  case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
    s="An error in the stream caused the decoder to lose synchronization.";
    break;
  case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
    s="The decoder encountered a corrupted frame header.";
    break;
  case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
    s="The frame's data did not match the CRC in the footer.";
    break;
  case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
    s="The decoder encountered reserved fields in use in the stream.";
    break;
  }
  TINYOAL_LOG("WARNING (FLAC CODEC ERROR)") << s << std::endl;
}
void cAudioResourceFLAC::_cbmeta(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{

}

template<typename T>
BSS_FORCEINLINE void r_flacread(T* target, const FLAC__int32* const buffer[], unsigned int num, unsigned int channels)
{
  for(unsigned int i = 0; i < num; i+=1)
  {
    for(unsigned int j=0; j < channels; ++j)
      target[j]=(T)buffer[j][i];
    target+=channels;
  }
}
template<>
BSS_FORCEINLINE void r_flacread<float>(float* target, const FLAC__int32* const buffer[], unsigned int num, unsigned int channels)
{
  for(unsigned int i = 0; i < num; i+=1)
  {
    for(unsigned int j=0; j < channels; ++j)
      target[j]=(float)buffer[j][i]/8388607.0f;
    target+=channels;
  }
}
FLAC__StreamDecoderWriteStatus cAudioResourceFLAC::_cbwrite(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
  cAudioResourceFLAC* self = *(cAudioResourceFLAC**)client_data;
  unsigned int channels = frame->header.channels;
  unsigned int num = frame->header.blocksize;
  if(num==1) num=192;
  else if(num>=2 && num<=5) num = 576 * (1<<(num-2));
  else if(num>=8 && num<=15) num = 256 * (1<<(num-8));

  unsigned int bits=frame->header.bits_per_sample==24?32:frame->header.bits_per_sample;
  unsigned int persample = channels*(bits>>3);
  unsigned int len = self->_len/persample;
  if(num>len) { self->_len=0; return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE; }
  unsigned int bytes=num*persample;
  switch(bits)
  {
  case 8: r_flacread<char>((char*)self->_buffer,buffer,num,channels); break;
  case 16: r_flacread<short>((short*)self->_buffer,buffer,num,channels); break;
  case 24: //24-bit gets converted to 32-bit
  case 32: r_flacread<float>((float*)self->_buffer,buffer,num,channels); break;
  }

  self->_bytesread+=bytes;
  self->_buffer+=bytes;
  self->_len-=bytes;
  ((DatStreamEx*)client_data)->cursample+=num;
  self->_cursample=(self->_len>0)?((DatStreamEx*)client_data)->cursample:(unsigned __int64)-1;

  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}
std::pair<void*,unsigned int> cAudioResourceFLAC::ToWave(void* data, unsigned int datalength, TINYOAL_FLAG flags)
{
  cAudioResourceFLAC res(data,datalength,flags,0);
  DatStreamEx* ex = (DatStreamEx*)res.OpenStream();
  if(!ex) return std::pair<void*,unsigned int>((void*)0,0);
  
  auto fn = cTinyOAL::Instance()->flacFuncs;
  unsigned __int64 total = fn->fn_flac_get_total_samples(ex->d);
  unsigned __int64 totalbytes = total*res._channels*(res._samplebits>>3);
  unsigned int header = cTinyOAL::Instance()->waveFuncs->WriteHeader(0,0,0,0,0);
  char* buffer = (char*)malloc(totalbytes+header);
  res.Reset(ex);
  res._len=totalbytes;
  res._buffer=buffer+header;
  res._bytesread=0;
  fn->fn_flac_process_until_stream_end(ex->d);
  cTinyOAL::Instance()->waveFuncs->WriteHeader(buffer,res._bytesread+header,res._channels,res._samplebits,res._freq);
  
  res.CloseStream(ex);
  return std::pair<void*,unsigned int>(buffer,res._bytesread+header);
}
FLAC__StreamDecoderWriteStatus cAudioResourceFLAC::_cbemptywrite(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{ // This function exists so we can yank the metadata out of the stream without actually putting it anywhere.
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

FLAC__StreamDecoderReadStatus cAudioResourceFLAC::_cbread(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
  if(*bytes > 0) {
    *bytes = dat_read_func(buffer, sizeof(FLAC__byte), *bytes, &((DatStreamEx*)client_data)->stream);
    if(*bytes == 0)
      return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    else
      return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
  }
  else
    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}
FLAC__StreamDecoderSeekStatus cAudioResourceFLAC::_cbseek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
   if(dat_seek_func(&((DatStreamEx*)client_data)->stream, (long long)absolute_byte_offset, SEEK_SET) < 0)
     return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
   else
     return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}
FLAC__StreamDecoderTellStatus cAudioResourceFLAC::_cbtell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
  long pos;
  if((pos = dat_tell_func(&((DatStreamEx*)client_data)->stream)) < 0)
    return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
  
  *absolute_byte_offset = (FLAC__uint64)pos;
  return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}
FLAC__StreamDecoderLengthStatus cAudioResourceFLAC::_cblength(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
  *stream_length = ((DatStreamEx*)client_data)->stream.datalength;
  return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}
FLAC__bool cAudioResourceFLAC::_cbeof(const FLAC__StreamDecoder *decoder, void *client_data)
{
  DatStreamEx* stream=(DatStreamEx*)client_data;
  return (((char*)stream->stream.data)+stream->stream.datalength)==stream->stream.streampos;
}
FLAC__StreamDecoderReadStatus cAudioResourceFLAC::_cbfread(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
  if(*bytes > 0) {
    *bytes = fread(buffer, sizeof(FLAC__byte), *bytes, ((DatStreamEx*)client_data)->f);
    if(*bytes == 0)
      return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    else
      return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
  }
  else
    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}
FLAC__StreamDecoderSeekStatus cAudioResourceFLAC::_cbfseek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
  if(fseek(((DatStreamEx*)client_data)->f, absolute_byte_offset, SEEK_SET) < 0)
     return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
   else
     return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}
FLAC__StreamDecoderTellStatus cAudioResourceFLAC::_cbftell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
  long pos;
  if((pos = ftell(((DatStreamEx*)client_data)->f)) < 0)
    return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
  
  *absolute_byte_offset = (FLAC__uint64)pos;
  return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}
FLAC__StreamDecoderLengthStatus cAudioResourceFLAC::_cbflength(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
  *stream_length = ((DatStreamEx*)client_data)->p->_datalength;
  return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}
FLAC__bool cAudioResourceFLAC::_cbfeof(const FLAC__StreamDecoder *decoder, void *client_data)
{
  return feof(((DatStreamEx*)client_data)->f);
}
