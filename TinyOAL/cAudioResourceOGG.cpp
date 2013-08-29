// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudioResourceOGG.h"
#include "cDataStream.h"
#include "cTinyOAL.h"
#include "cOggFunctions.h"
#include "openAL/al.h"
#include "openAL/loadoal.h"

using namespace TinyOAL;

cAudioResourceOGG::cAudioResourceOGG(const cAudioResourceOGG &copy) : cAudioResource(copy), _loopstart(copy._loopstart)
{
  _readinfo = false;
  sCallbacks = copy.sCallbacks;
}

// Constructor that takes a file pointer, a length of data to copy, and a set of flags.
// It appears to pull the data off the file and then calls a constructor with a flag to load the file into memory.
// The constructor it calls then loads the file into memory regardless of whether that flag is set or not.
cAudioResourceOGG::cAudioResourceOGG(_iobuf *file, unsigned int datalength, T_TINYOAL_FLAGS flags) :
  cAudioResource(file, datalength, flags|TINYOAL_LOADINTOMEMORY), _loopstart(0)
{
  if(_data!=0)
    _buildstream(false);
  else
    TINYOAL_LOGM("WARNING", "Null data pointer in cAudioResourceOGG!");
}

// Constructor takes a file name and flags. Most of the work is done in the base class's constructor.
cAudioResourceOGG::cAudioResourceOGG(const char *file, T_TINYOAL_FLAGS flags) : cAudioResource(file, flags), _loopstart(0)
{
  if(!_data) // If there is not data, then it's still at the file. (What if the file's wrong?)
    _buildstream(true); // Build the stream and tell it that it is loading from a file.
  else // Otherwise, there is data. Meaning it was loaded into memory.
    _buildstream(false); // So build the stream in memory.
}

// Constructor that takes a data pointer, a length of data, and flags.
cAudioResourceOGG::cAudioResourceOGG(void* data, unsigned int datalength, T_TINYOAL_FLAGS flags) : cAudioResource(data, datalength, flags), _loopstart(0)
{
  if(_data!=0)
    _buildstream(false);
  else
    TINYOAL_LOGM("WARNING", "Null data pointer in cAudioResourceOGG!");
}

cAudioResourceOGG::~cAudioResourceOGG()
{

}

//8 functions - Four for parsing pure void*, and four for reading files
size_t dat_ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
  return ((cDataStream*)datasource)->read(ptr, size, nmemb);
}

int dat_ov_seek_func(void *datasource, __int64 offset, int whence) //Who the hell names a parameter "whence"?!
{
  return ((cDataStream*)datasource)->seek(offset, whence);
}

int dat_ov_close_func(void *datasource)
{
  return 0; //We manage file opening and closing.
  //return ((cDataStream*)datasource)->close();
}

long dat_ov_tell_func(void *datasource)
{
  return ((cDataStream*)datasource)->tell();
}

size_t file_ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return fread(ptr, size, nmemb, (FILE*)datasource);
}

int file_ov_seek_func(void *datasource, __int64 offset, int whence)
{
	return fseek((FILE*)datasource, (long)offset, whence);
}

int file_ov_close_func(void *datasource)
{
  return 0; //We manage file opening and closing.
  //return fclose((FILE*)datasource);
}

long file_ov_tell_func(void *datasource)
{
	return ftell((FILE*)datasource);
}

// This is what opens the actual audio stream. They way this works is that the OGG DLL loads up a bunch of information
// for us into a file struct called OggVorbis_File. We then store a pointer to this information in our stream, and use
// it when the audio instance asks for the next chunk of audio data, so we can tell the DLL what to decode and where,
// using the above file callbacks (read, seek, close, tell) for both actual files and for in-memory.

// Also builds the information struct, which is what it returns?
AUDIOSTREAM* cAudioResourceOGG::OpenStream()
{
	// Start by calling the base cAudioResource's OpenStream() function.
	// Everything else in this function has to do with formatting that stream.
	AUDIOSTREAM* retval = cAudioResource::OpenStream();
	if(!retval || !cTinyOAL::Instance()->getOGG()->fn_ov_open_callbacks) return 0;

	retval->oggfile = new OggVorbis_File();

	// Create an OggVorbis file stream
	if(cTinyOAL::Instance()->getOGG()->fn_ov_open_callbacks(retval->file, retval->oggfile, NULL, 0, sCallbacks) != 0) //doesn't matter if we call file or data here because its the same pointer
	{
    TINYOAL_LOGM("ERROR","Failed to create file stream");
		_altclearstream(retval); //clean up our mess
		return 0;
	}

	if(_readinfo)
		return retval;

	// Get some information about the file (Channels, Format, and Frequency)
	// Store this data into psVorbisInfo
	vorbis_info* psVorbisInfo = cTinyOAL::Instance()->getOGG()->fn_ov_info(retval->oggfile, -1);
	if (psVorbisInfo)
	{
		ulFrequency = psVorbisInfo->rate;
		ulChannels = psVorbisInfo->channels;
		
		// If the VorbisInfo tells us we only have one output channel...
		if (psVorbisInfo->channels == 1)
		{
			// We're in MONO output.
			ulFormat = AL_FORMAT_MONO16;
			// Set BufferSize to 250ms (Frequency * 2 (16bit) divided by 4 (quarter of a second))
			ulBufferSize = ulFrequency >> 1;
			// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
			ulBufferSize -= (ulBufferSize % 2);
		}
		// If we have two output channels
		else if (psVorbisInfo->channels == 2)
		{
			// We're in STEREO output
			ulFormat = AL_FORMAT_STEREO16;
			// Set BufferSize to 250ms (Frequency * 4 (16bit stereo) divided by 4 (quarter of a second))
			ulBufferSize = ulFrequency;
			// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
			ulBufferSize -= (ulBufferSize % 4);
		}
		// If we have four output channels
		else if (psVorbisInfo->channels == 4)
		{
			// We're in "Quad" output.
			ulFormat = cTinyOAL::Instance()->getFunctions()->alGetEnumValue("AL_FORMAT_QUAD16");
			// Set BufferSize to 250ms (Frequency * 8 (16bit 4-channel) divided by 4 (quarter of a second))
			ulBufferSize = ulFrequency * 2;
			// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
			ulBufferSize -= (ulBufferSize % 8);
		}
		// If we have six output channels
		else if (psVorbisInfo->channels == 6)
		{
			// We're (probably) in 5.1 output.
			ulFormat = cTinyOAL::Instance()->getFunctions()->alGetEnumValue("AL_FORMAT_51CHN16");
			// Set BufferSize to 250ms (Frequency * 12 (16bit 6-channel) divided by 4 (quarter of a second))
			ulBufferSize = ulFrequency * 3;
			// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
			ulBufferSize -= (ulBufferSize % 12);
		}
	}

	// If the format never got set, error.
	if(!ulFormat)
	{
    TINYOAL_LOGM("ERROR","Failed to find format information, or unsupported format");
		CloseStream(retval); //clean up our mess
		return 0;
	}

  if(_flags & TINYOAL_LOADINTOMEMORY) //If this was NOT loaded into memory, it could be changed on us at any time, so we reload it for every opened stream
    _readinfo = true; //otherwise we don't need to go through this process for each and every stream access request

  _loopstart=cTinyOAL::Instance()->getOGG()->GetLoopStart(retval->oggfile);

  return retval;
}

// We simply call the base close stream function after we delete some information that we created.
void cAudioResourceOGG::CloseStream(AUDIOSTREAM* stream)
{  
  cTinyOAL::Instance()->getOGG()->fn_ov_clear(stream->oggfile); // Close OggVorbis stream
  if(stream->oggfile) { delete stream->oggfile; stream->oggfile = 0; }
  cAudioResource::CloseStream(stream);
}

//Special function for when we're only cleaning up half of our mess
void cAudioResourceOGG::_altclearstream(AUDIOSTREAM* stream)
{
  delete stream->oggfile;
  stream->oggfile = 0;
  cAudioResource::CloseStream(stream);
}

//This is a very small function that we use to prep our OGG resource stream. It sets callbacks and nulls a few variables. It does NOT open an actual stream, that's what OpenStream does.
bool cAudioResourceOGG::_buildstream(bool file)
{
  _readinfo = false;
	
	ulFrequency = 0;
	ulChannels = 0;
	ulFormat = 0;

  // Prep ourselves for the creation of a stream
  //sOggVorbisFile = new OggVorbis_File();

  if(file)
  {
	  // If it's a file, call all the callback functions for file IO.
	  sCallbacks.read_func = file_ov_read_func;
	  sCallbacks.seek_func = file_ov_seek_func;
	  sCallbacks.close_func = file_ov_close_func;
	  //sCallbacks.close_func = NULL;
	  sCallbacks.tell_func = file_ov_tell_func;
  }
  else
  {
	  // If it's not a file, call all the callback functions for memory.
	  sCallbacks.read_func = dat_ov_read_func;
	  sCallbacks.seek_func = dat_ov_seek_func;
	  sCallbacks.close_func = dat_ov_close_func;
	  //sCallbacks.close_func = NULL;
	  sCallbacks.tell_func = dat_ov_tell_func;
  }

  if(!cTinyOAL::Instance()->getOGG()->fn_ov_open_callbacks) return false; //Something fucked up, so bail out (but only after we set those callbacks

  return true;
}

// This is the important function. Using the stream given to us, we know that it must be an OGG stream, and thus will
// have the information we need contained in the pointer. We use this information to decode a chunk of the audio info
// and put it inside the given decodebuffer (which is the same for all audio formats, since its decoded). It then
// returns how many bytes were read.
unsigned long cAudioResourceOGG::ReadNext(AUDIOSTREAM* stream, char* pDecodeBuffer)
{
  if(!stream->oggfile) return 0;
  int current_section;
	long lDecodeSize;
	unsigned long ulSamples;
	short *pSamples;

	unsigned long ulBytesDone = 0;
	while(1)
	{
    lDecodeSize = cTinyOAL::Instance()->getOGG()->fn_ov_read(stream->oggfile, pDecodeBuffer + ulBytesDone, ulBufferSize - ulBytesDone, 0, 2, 1, &current_section);
		if (lDecodeSize > 0)
		{
			ulBytesDone += lDecodeSize;

			if (ulBytesDone >= ulBufferSize)
				break;
		}
		else
      break;
	}

	// Mono, Stereo and 4-Channel files decode into the same channel order as WAVEFORMATEXTENSIBLE,
	// however 6-Channels files need to be re-ordered
	if (ulChannels == 6)
	{		
		pSamples = (short*)pDecodeBuffer;
		for (ulSamples = 0; ulSamples < (ulBufferSize>>1); ulSamples+=6)
		{
			// WAVEFORMATEXTENSIBLE Order : FL, FR, FC, LFE, RL, RR
			// OggVorbis Order            : FL, FC, FR,  RL, RR, LFE
			Swap(pSamples[ulSamples+1], pSamples[ulSamples+2]);
			Swap(pSamples[ulSamples+3], pSamples[ulSamples+5]);
			Swap(pSamples[ulSamples+4], pSamples[ulSamples+5]);
		}
	}

	return ulBytesDone;
}

//swap function. This should actually be optimized by the compiler and turned into a single CPU command.
void cAudioResourceOGG::Swap(short &s1, short &s2)
{
	short sTemp = s1;
	s1 = s2;
	s2 = sTemp;
}

//This is called when you need to reset a stream to the beginning of the file or in-memory equivelent.
bool cAudioResourceOGG::Reset(AUDIOSTREAM* stream)
{
  if(!stream->oggfile || !stream->file) return false;
  if(cTinyOAL::Instance()->getOGG()->fn_ov_raw_seek(stream->oggfile,0) != 0)
  //if(cTinyOAL::Instance()->getOGG()->fn_ov_time_seek_page(stream->oggfile,0.0) != 0) 
  { 
    if(stream->isfile) //OGG seek operation failed, so fall back to rebuilding the stream
      fseek(stream->file, 0, SEEK_SET);
    else
      stream->data->seek(0, SEEK_SET);

    cTinyOAL::Instance()->getOGG()->fn_ov_clear(stream->oggfile);
    if(cTinyOAL::Instance()->getOGG()->fn_ov_open_callbacks(stream->file, stream->oggfile, NULL, 0, sCallbacks) != 0) //Rebuilding the stream failed
    {
      delete stream->oggfile;
      stream->oggfile = 0;
      return false;
    }
  }
  return true;
}

bool cAudioResourceOGG::Skip(AUDIOSTREAM* stream, unsigned __int64 samples)
{
  if(!stream->oggfile || !stream->file) return false;
  return (cTinyOAL::Instance()->getOGG()->fn_ov_pcm_seek(stream->oggfile,(ogg_int64_t)samples) != 0);
}

unsigned __int64 cAudioResourceOGG::ToSample(AUDIOSTREAM* stream, double seconds)
{
  if(!stream->oggfile || !stream->file) return false;
  ogg_int64_t prev = cTinyOAL::Instance()->getOGG()->fn_ov_raw_tell(stream->oggfile);
  unsigned __int64 ret=0;

  if(cTinyOAL::Instance()->getOGG()->fn_ov_time_seek(stream->oggfile,seconds) == 0)
    ret = (unsigned __int64)cTinyOAL::Instance()->getOGG()->fn_ov_pcm_tell(stream->oggfile);

  cTinyOAL::Instance()->getOGG()->fn_ov_raw_seek(stream->oggfile,prev);
  return ret;
}

unsigned __int64 cAudioResourceOGG::GetLoopStart(AUDIOSTREAM* stream)
{
  return _loopstart;
}
//bool cAudioResourceOGG::SkipToTime(AUDIOSTREAM* stream, double seconds)
//{
//  if(!stream->oggfile || !stream->file) return false;
//  return (cTinyOAL::Instance()->getOGG()->fn_ov_time_seek(stream->oggfile,seconds) != 0);
//}
