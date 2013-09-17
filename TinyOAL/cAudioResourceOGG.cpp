// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudioResourceOGG.h"
#include "cTinyOAL.h"
#include "cOggFunctions.h"
#include "openAL/al.h"
#include "openAL/loadoal.h"

using namespace TinyOAL;
bss_util::cFixedAlloc<OggVorbis_FileEx> cAudioResourceOGG::_allocogg(3);

cAudioResourceOGG::cAudioResourceOGG(const cAudioResourceOGG &copy) : cAudioResource(copy) {}

// Constructor that takes a data pointer, a length of data, and flags.
cAudioResourceOGG::cAudioResourceOGG(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned __int64 loop) : cAudioResource(data, datalength, flags, loop)
{
  if(_flags&TINYOAL_ISFILE) {
    _callbacks.read_func = file_read_func;
	  _callbacks.seek_func = file_seek_func;
	  _callbacks.close_func = file_close_func;
	  _callbacks.tell_func = file_tell_func;
  } else {
	  _callbacks.read_func = dat_read_func;
	  _callbacks.seek_func = dat_seek_func;
	  _callbacks.close_func = dat_close_func;
	  _callbacks.tell_func = dat_tell_func;
  }

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
		
    
    switch(psVorbisInfo->channels)
    {
    case 1:
			_format = AL_FORMAT_MONO16; // mono output
			_bufsize = _freq >> 1; // Set BufferSize to 250ms (Frequency * 2 (16bit) divided by 4 (quarter of a second))
			_bufsize -= (_bufsize % 2);
      break;
    case 2:
			_format = AL_FORMAT_STEREO16; //stereo output
			_bufsize = _freq; // Set BufferSize to 250ms (Frequency * 4 (16bit stereo) divided by 4 (quarter of a second))
			_bufsize -= (_bufsize % 4);
      break;
    case 4: // "quad" output
			if(cTinyOAL::Instance()->oalFuncs!=0) 
        _format = cTinyOAL::Instance()->oalFuncs->alGetEnumValue("AL_FORMAT_QUAD16");
			_bufsize = _freq * 2; // Set BufferSize to 250ms (Frequency * 8 (16bit 4-channel) divided by 4 (quarter of a second))
			_bufsize -= (_bufsize % 8);
      break;
    case 6: // 5.1 output (probably)
      if(cTinyOAL::Instance()->oalFuncs!=0) 
        _format = cTinyOAL::Instance()->oalFuncs->alGetEnumValue("AL_FORMAT_51CHN16");
			_bufsize = _freq * 3; // Set BufferSize to 250ms (Frequency * 12 (16bit 6-channel) divided by 4 (quarter of a second))
			_bufsize -= (_bufsize % 12);
      break;
    }
	}

	// If the format never got set, error.
	if(!_format)
    TINYOAL_LOGM("ERROR","Failed to find format information, or unsupported format");

  _loop=ogg->GetLoopStart(&f->ogg);
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
    TINYOAL_LOGM("ERROR","Failed to create file stream");
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
unsigned long cAudioResourceOGG::Read(void* stream, char* buffer)
{
  if(!stream) return 0;
  int current_section;
	long lDecodeSize;
	unsigned long ulSamples;
	short *pSamples;

	unsigned long ulBytesDone = 0;
	while(1)
	{
    lDecodeSize = cTinyOAL::Instance()->oggFuncs->fn_ov_read((OggVorbis_File*)stream, buffer + ulBytesDone, _bufsize - ulBytesDone, 0, 2, 1, &current_section);
		if (lDecodeSize > 0)
		{
			ulBytesDone += lDecodeSize;
			if (ulBytesDone >= _bufsize) break;
		}
		else
      break;
	}

	// Mono, Stereo and 4-Channel files decode into the same channel order as WAVEFORMATEXTENSIBLE,
	// however 6-Channels files need to be re-ordered
	if (_channels == 6)
	{		
		pSamples = (short*)buffer;
		for (ulSamples = 0; ulSamples < (_bufsize>>1); ulSamples+=6)
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
bool cAudioResourceOGG::Reset(void* stream)
{
  if(!stream) return false;
  if(cTinyOAL::Instance()->oggFuncs->fn_ov_pcm_seek((OggVorbis_File*)stream,0) != 0)
  //if(ogg->fn_ov_time_seek_page(stream->oggfile,0.0) != 0) 
  { 
    cTinyOAL::Instance()->oggFuncs->fn_ov_clear((OggVorbis_File*)stream); // Close the stream, but don't delete it
    return _openstream((OggVorbis_FileEx*)stream); // Attempt to reopen it. 
  }
  return true;
}
bool cAudioResourceOGG::Skip(void* stream, unsigned __int64 samples)
{
  if(!stream) return false;
  if(!cTinyOAL::Instance()->oggFuncs->fn_ov_pcm_seek((OggVorbis_File*)stream,(ogg_int64_t)samples)) return true;
  if(!samples) return Reset(stream); // If we fail to seek, but we want to loop to the start, attempt to reset the stream instead.
  return false;
}