// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudioResourceOGG.h"
#include "cTinyOAL.h"
#include "cWaveFunctions.h"
#include "openAL/loadoal.h"

using namespace tinyoal;
bss_util::cBlockAlloc<OggVorbis_FileEx> cAudioResourceOGG::_allocogg(3);

// Constructor that takes a data pointer, a length of data, and flags.
cAudioResourceOGG::cAudioResourceOGG(void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop) : cAudioResource(data, datalength, flags, TINYOAL_FILETYPE_OGG, loop)
{
  _setcallbacks(_callbacks,(_flags&TINYOAL_ISFILE)!=0);
  // Open an initial stream and read in static information from the file
  OggVorbis_FileEx* f=(OggVorbis_FileEx*)OpenStream();
  if(!f) return;

	// Get some information about the file (Channels, Format, and Frequency)
  cOggFunctions* ogg = cTinyOAL::Instance()->oggFuncs;
  vorbis_info* psVorbisInfo = ogg->fn_ov_info(&f->ogg, -1);
	if (psVorbisInfo)
	{
		_freq = psVorbisInfo->rate;
		_channels = psVorbisInfo->channels;
    _samplebits=16; 
    _bufsize = (_freq * _channels * 2)>>2; // Sets buffer size to 250 ms, which is freq * 2 (bytes per sample) / 4 (quarter of a second) 
    _bufsize -= (_bufsize % (_channels*2));
    _format = cTinyOAL::GetFormat(_channels,_samplebits,false);
    _total = ogg->fn_ov_pcm_total(&f->ogg, -1);
	}

	// If the format never got set, error.
	if(!_format)
    TINYOAL_LOG(1,"Failed to find format information, or unsupported format");

  uint64_t fileloop = ogg->GetLoopStart(&f->ogg);
  if(fileloop!=-1LL) _loop=fileloop; // Only overwrite our loop point with the file loop point if it actually had one.
  CloseStream(f);
}

cAudioResourceOGG::~cAudioResourceOGG()
{
  _destruct();
}

void* cAudioResourceOGG::OpenStream()
{
  OggVorbis_FileEx* r = _allocogg.alloc(1);
  if(_openstream(r)) return r;
  _allocogg.dealloc(r);
  return 0;
}

bool cAudioResourceOGG::_openstream(OggVorbis_FileEx* r)
{
  if(_flags&TINYOAL_ISFILE) fseek((FILE*)_data,0,SEEK_SET); // If we're a file, reset the pointer
  else { r->stream.data=r->stream.streampos=(const char*)_data; r->stream.datalength=_datalength; } // Otherwise, set the data pointers in our Ex structure
  cOggFunctions* ogg = cTinyOAL::Instance()->oggFuncs;

  if(!ogg || !ogg->fn_ov_open_callbacks || ogg->fn_ov_open_callbacks((_flags&TINYOAL_ISFILE)?_data:&r->stream,&r->ogg,0,0,_callbacks)!=0) {
    TINYOAL_LOG(1,"Failed to create file stream");
    return false;
  }
  return true;
}

void cAudioResourceOGG::CloseStream(void* stream)
{
  OggVorbis_FileEx* data = (OggVorbis_FileEx*)stream;
  cTinyOAL::Instance()->oggFuncs->fn_ov_clear(&data->ogg);
  _allocogg.dealloc(data);
}

// This is the important function. Using the stream given to us, we know that it must be an OGG stream, and thus will
// have the information we need contained in the pointer. We use this information to decode a chunk of the audio info
// and put it inside the given decodebuffer (which is the same for all audio formats, since its decoded). It then
// returns how many bytes were read.
unsigned long cAudioResourceOGG::_read(void* stream, char* buffer, unsigned int len, bool& eof, char bytes, unsigned int channels)
{
  if(!stream) return 0;
  int current_section;
	long lDecodeSize=1;
	unsigned long ulSamples;
	short *pSamples;
  eof=false;

	unsigned long ulBytesDone = 0;
	while(lDecodeSize > 0)
	{
    lDecodeSize = cTinyOAL::Instance()->oggFuncs->fn_ov_read((OggVorbis_File*)stream, buffer + ulBytesDone, len - ulBytesDone, 0, bytes, 1, &current_section);
		if (lDecodeSize > 0)
		{
			ulBytesDone += lDecodeSize;
			if (ulBytesDone >= len) break;
		}
    else
      eof=true;
	}

	// Mono, Stereo and 4-Channel files decode into the same channel order as WAVEFORMATEXTENSIBLE,
	// however 6-Channels files need to be re-ordered
	if (channels == 6)
	{		
		pSamples = (short*)buffer;
		for (ulSamples = 0; ulSamples < (len/bytes); ulSamples+=6)
		{
			// WAVEFORMATEXTENSIBLE Order : FL, FR, FC, LFE, RL, RR
			// OggVorbis Order            : FL, FC, FR,  RL, RR, LFE
      bss_util::rswap(pSamples[ulSamples+1], pSamples[ulSamples+2]);
			bss_util::rswap(pSamples[ulSamples+3], pSamples[ulSamples+5]);
			bss_util::rswap(pSamples[ulSamples+4], pSamples[ulSamples+5]);
		}
	}

	return ulBytesDone;
}
unsigned long cAudioResourceOGG::Read(void* stream, char* buffer, unsigned int len, bool& eof)
{
  return _read(stream,buffer,len,eof,_samplebits>>3,_channels);
}
bool cAudioResourceOGG::Reset(void* stream)
{
  if(!stream) return false;
  if(cTinyOAL::Instance()->oggFuncs->fn_ov_pcm_seek((OggVorbis_File*)stream,0) != 0)
  { 
    cTinyOAL::Instance()->oggFuncs->fn_ov_clear((OggVorbis_File*)stream); // Close the stream, but don't delete it
    return _openstream((OggVorbis_FileEx*)stream); // Attempt to reopen it. 
  }
  return true;
}
bool cAudioResourceOGG::Skip(void* stream, uint64_t samples)
{
  if(!stream) return false;
  if(!cTinyOAL::Instance()->oggFuncs->fn_ov_pcm_seek((OggVorbis_File*)stream,(ogg_int64_t)samples)) return true;
  else TINYOAL_LOG(2, "Seek failed to skip to %llu", samples);
  if(!samples) return Reset(stream); // If we fail to seek, but we want to loop to the start, attempt to reset the stream instead.
  return false;
}

uint64_t cAudioResourceOGG::Tell(void* stream) // Gets what sample a stream is currently on
{
  if(!stream) return false;
  return cTinyOAL::Instance()->oggFuncs->fn_ov_pcm_tell((OggVorbis_File*)stream);
}
void cAudioResourceOGG::_setcallbacks(ov_callbacks& callbacks, bool isfile)
{
  if(isfile) {
    callbacks.read_func = file_read_func;
	  callbacks.seek_func = file_seek_func;
	  callbacks.close_func = file_close_func;
	  callbacks.tell_func = file_tell_func;
  } else {
	  callbacks.read_func = dat_read_func;
	  callbacks.seek_func = dat_seek_func;
	  callbacks.close_func = dat_close_func;
	  callbacks.tell_func = dat_tell_func;
  }
}

size_t cAudioResourceOGG::Construct(void* p, void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop)
{
  if(p) new(p) cAudioResourceOGG(data, datalength, flags, loop);
  return sizeof(cAudioResourceOGG);
}
bool cAudioResourceOGG::ScanHeader(const char* fileheader)
{
  return !strncmp(fileheader, "OggS", 4);
}

std::pair<void*,unsigned int> cAudioResourceOGG::ToWave(void* data, unsigned int datalength, TINYOAL_FLAG flags)
{
  ov_callbacks callbacks;
  _setcallbacks(callbacks,(flags&TINYOAL_ISFILE)!=0);
  
  OggVorbis_FileEx r;
  if(!(flags&TINYOAL_ISFILE))
  { 
    r.stream.data=r.stream.streampos=(const char*)data;
    r.stream.datalength=datalength;
  }
  cOggFunctions* ogg = cTinyOAL::Instance()->oggFuncs;

  if(!ogg || !ogg->fn_ov_open_callbacks || ogg->fn_ov_open_callbacks((flags&TINYOAL_ISFILE)?data:&r.stream,&r.ogg,0,0,callbacks)!=0) {
    TINYOAL_LOG(1,"Failed to create file stream");
    return std::pair<void*,unsigned int>((void*)0,0); 
  }

	// Get some information about the file (Channels, Format, and Frequency)
  vorbis_info* psVorbisInfo = ogg->fn_ov_info(&r.ogg, -1);
  if(!psVorbisInfo) { ogg->fn_ov_clear(&r.ogg); return std::pair<void*,unsigned int>((void*)0,0);  }

  uint64_t total = ogg->fn_ov_pcm_total(&r.ogg,-1); // Get total number of samples
	long freq = psVorbisInfo->rate;
	int channels = psVorbisInfo->channels;
  short samplebits=16; 
  uint64_t totalbytes = total*channels*(samplebits>>3);
  unsigned int header = cTinyOAL::Instance()->waveFuncs->WriteHeader(0,0,0,0,0);
  char* buffer = (char*)malloc(totalbytes+header);
  bool eof;
  totalbytes = _read(&r,buffer+header,totalbytes,eof,samplebits>>3,channels);
  cTinyOAL::Instance()->waveFuncs->WriteHeader(buffer,totalbytes+header,channels,samplebits,freq);
  ogg->fn_ov_clear(&r.ogg);
  return std::pair<void*,unsigned int>(buffer,totalbytes+header);
}
