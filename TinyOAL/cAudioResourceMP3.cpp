// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifdef __INCLUDE_MP3
#include "cAudioResourceMP3.h"
#include "cMp3Functions.h"
#include "cTinyOAL.h"

using namespace TinyOAL;

//////////////////
// Constructors //
//////////////////

cAudioResourceMP3::cAudioResourceMP3(const cAudioResourceMP3 &copy) : cAudioResource(copy), _filebuf(0)
{
  _functions = cTinyOAL::Instance()->getMP3();
}

// "Reading from an already opened file for x number of bytes"
// Constructor that takes a file pointer, a length of data to copy, and a set of flags.
// It appears to pull the data off the file and then calls a constructor with a flag to load the file into memory.
// The constructor it calls then loads the file into memory regardless of whether that flag is set or not.
cAudioResourceMP3::cAudioResourceMP3(_iobuf *file, unsigned int datalength, TINYOAL_FLAG flags, unsigned __int64 loop) : cAudioResource(file, datalength, flags|TINYOAL_LOADINTOMEMORY), _filebuf(0)
{
  _functions = cTinyOAL::Instance()->getMP3();
}

// "Reading from a file path"
// Constructor takes a file name and flags. Most of the work is done in the base class's constructor.
cAudioResourceMP3::cAudioResourceMP3(const char *file, TINYOAL_FLAG flags) : cAudioResource(file, flags), _filebuf(0)
{
  _functions = cTinyOAL::Instance()->getMP3();
}

// "Reading from an arbitrary point in memory for x number of bytes."
// Constructor that takes a data pointer, a length of data, and flags.
cAudioResourceMP3::cAudioResourceMP3(void* data, unsigned int datalength, TINYOAL_FLAG flags) : cAudioResource(data, datalength, flags), _filebuf(0)
{
  _functions = cTinyOAL::Instance()->getMP3();
}

// Destructor that doesn't do anything
cAudioResourceMP3::~cAudioResourceMP3()
{
  _destruct();
}

//////////////////////////
//Method Implementations//
//////////////////////////

struct id3v2header
{
 char id[3];
 unsigned char majVer;
 unsigned char minVer;
 unsigned char flags;
 unsigned char flags2;
 unsigned char flags3;
 unsigned char size;
};

// This returns an AUDIOSTREAM on success, or NULL on failure 
AUDIOSTREAM* cAudioResourceMP3::OpenStream()
{
  AUDIOSTREAM* retval = cAudioResource::OpenStream();
	if(!retval || !_functions->fn_lameDecode1Headers) return 0;

  short left[256];
  short right[256];
  char head[4000];
  id3v2header id3head;

  _readstream(retval, (char*)&id3head, sizeof(id3v2header)); //first we must get past the ID3 header
  
  if(retval->isfile)
    fseek(retval->file, id3head.size, SEEK_SET); //7th byte in the ID3 header tells us howbig it is
  else
    retval->data->seek(id3head.size, SEEK_SET);

  _readstream(retval, (char*)head, 4000);
  //_functions->fn_lameDecode1HeadersB((unsigned char*)&head, 4, left, right, &_header, &_enc_delay, &_enc_padding);
  _functions->fn_lameDecodeHeaders((unsigned char*)head, 4000, left, right, &_header);
  Reset(retval); //And go back so we don't miss the first frame

  if(_header.header_parsed!=1)
  {
    CloseStream(retval);
    return 0;
  }

  ulFormat = _header.mode;
  ulChannels = _header.stereo;
  ulFrequency = _header.samplerate;
  _framesize= (unsigned long)((144*_header.bitrate) / (double)(ulFrequency + (head[2]&(1<<6)))); //this gets the 22nd bit in the header, which should be the padding bit
  ulBufferSize = (sizeof(short) * ulChannels) * _header.framesize; //Each sample is 2 bytes multiplied by the number of channels (because technically there's only one sample per channel, but mp3 treats all channels as one sample becuase its stupid)
  
  if(_filebuf) delete [] _filebuf;
  _filebuf = (unsigned char*)malloc(_framesize);
  return 0;
}
// This reads the next chunk of MP3 specific data. pDecodeBuffer must be at least GetBufSize() long 
unsigned long cAudioResourceMP3::Read(AUDIOSTREAM* stream, char* pDecodeBuffer)
{
  short* left=0; //This will cause a runtime error but since we dont' even get this far yet i don't care
  short* right=0;
  short* sDecodeBuf=(short*)pDecodeBuffer; //this lets us treat the decode buffer as an array because we know it has to be the right size
  unsigned long retval;

  retval=_readstream(stream, (char*)_filebuf, _framesize);
  if((retval=_functions->fn_lameDecode1(_filebuf, retval, left, right)) <= 0)
    return 0; //Decoding error

  if(ulChannels==2)
  {
    unsigned long counter=-1;
    for(unsigned long i = 0; i < retval; ++i)
    {
      sDecodeBuf[++counter] = left[i];
      sDecodeBuf[++counter] = right[i];
    }
  }
  else if(ulChannels==1)
  {
    for(unsigned long i = 0; i < retval; ++i)
      sDecodeBuf[i] = left[i];
  }

  return retval;
}
// This resets the stream to the beginning. 
bool cAudioResourceMP3::Reset(AUDIOSTREAM* stream)
{
  if(stream->isfile)
    fseek(stream->file, 0, SEEK_SET);
  else
    stream->data->seek(0, SEEK_SET);

  return false;
}
// This closes a stream and destroys any associated data (not the actual audio source itself) 
void cAudioResourceMP3::CloseStream(AUDIOSTREAM* stream)
{
  cAudioResource::CloseStream(stream);
}

unsigned __int64 cAudioResourceMP3::ToSample(AUDIOSTREAM* stream, double seconds)
{
  return (unsigned __int64)(seconds*_header.samplerate);
}

bool cAudioResourceMP3::Skip(AUDIOSTREAM* stream, unsigned __int64 samples)
{
  long bytes = (long)(samples*(sizeof(short) * ulChannels));
  if(stream->isfile)
    fseek(stream->file, bytes, SEEK_SET);
  else
    stream->data->seek(bytes, SEEK_SET);
  return true;
}

////////////////////
// Helper Methods //
////////////////////

unsigned int cAudioResourceMP3::_readstream(AUDIOSTREAM* stream, char* buf, int num)
{
  if(stream->isfile)
    return fread(buf, 1, num, stream->file);
  else
    return stream->data->read(buf, 1, num);
}

#endif