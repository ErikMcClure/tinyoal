// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudioResourceWAV.h"
#include "cTinyOAL.h"
#define WINVER 0x0502
#define _WIN32_WINNT 0x0502    
#define WIN32_LEAN_AND_MEAN
#include "openAL/CWaves.h"
//#include "openAL/al.h"
#include "openAL/loadoal.h"

using namespace TinyOAL;

cAudioResourceWAV::cAudioResourceWAV(const cAudioResourceWAV &copy) : cAudioResource(copy)
{
  //We have nothing to copy
}

cAudioResourceWAV::cAudioResourceWAV(_iobuf *file, unsigned int datalength, T_TINYOAL_FLAGS flags) : cAudioResource(file, datalength, flags|TINYOAL_LOADINTOMEMORY), _waves(cTinyOAL::Instance()->GetWaveLoader())
{
  if(_data!=0)
    _buildstream(false);
}

cAudioResourceWAV::cAudioResourceWAV(const char *file, T_TINYOAL_FLAGS flags) : cAudioResource(file, flags), _waves(cTinyOAL::Instance()->GetWaveLoader())
{
  if(!_data)
    _buildstream(true);
  else
    _buildstream(false); //Otherwise it was loaded into memory
}

cAudioResourceWAV::cAudioResourceWAV(void* data, unsigned int datalength, T_TINYOAL_FLAGS flags) : cAudioResource(data, datalength, flags), _waves(cTinyOAL::Instance()->GetWaveLoader())
{
  if(_data!=0)
    _buildstream(false);
}

cAudioResourceWAV::~cAudioResourceWAV()
{
  if(_callbacks) delete _callbacks;
}

// Much like OGG format, this uses the information in AUDIOSTREAM to read a buffer. However, in this case, we are
// using a special WaveLoader class to do the work for us.
unsigned long cAudioResourceWAV::ReadNext(AUDIOSTREAM* stream, char* pDecodeBuffer)
{
  unsigned long retval;
  _waves->ReadWaveData(stream->WaveID, pDecodeBuffer, ulBufferSize, &retval);
  return retval;
}

//These are defined in cAudioResourceOGG
extern size_t dat_ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
extern int dat_ov_seek_func(void *datasource, __int64 offset, int whence);
extern int dat_ov_close_func(void *datasource);
extern long dat_ov_tell_func(void *datasource);
extern size_t file_ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
extern int file_ov_seek_func(void *datasource, __int64 offset, int whence);
extern int file_ov_close_func(void *datasource);
extern long file_ov_tell_func(void *datasource);

//Doing pretty much the same thing here as in OGG. This is basically a constructor.
bool cAudioResourceWAV::_buildstream(bool file)
{  
  _readinfo = false;
	ulFrequency = 0;
	ulChannels = 0;
	ulFormat = 0;

  _callbacks = new wav_callbacks();
  if(!_callbacks)
    return false;

  if(file)
  {
	  _callbacks->read_func = file_ov_read_func;
	  _callbacks->seek_func = file_ov_seek_func;
	  _callbacks->close_func = file_ov_close_func;
	  _callbacks->tell_func = file_ov_tell_func;
  }
  else
  {
	  _callbacks->read_func = dat_ov_read_func;
	  _callbacks->seek_func = dat_ov_seek_func;
	  _callbacks->close_func = dat_ov_close_func;
	  _callbacks->tell_func = dat_ov_tell_func;
  }

  return true;
}

// Again, like OGG, we are opening a stream here, however we are using a helper class to do it, so instead of storing a
// pointer to our information struct, we store an ID that the waveloader then uses to figure out the struct's location.
AUDIOSTREAM* cAudioResourceWAV::OpenStream()
{
  AUDIOSTREAM* retval = cAudioResource::OpenStream();
  if(!retval) return 0;

  if(_waves->OpenWaveFile(retval->file, &retval->WaveID, *_callbacks) < 0)
  {  
    TINYOAL_LOGM("ERROR","Could not find file");
    cAudioResource::CloseStream(retval);
    return 0;
  }
	
  _waves->SetWaveDataOffset(retval->WaveID, 0);

  if(_readinfo)
    return retval;

  unsigned long	ulDataSize = 0;

	_waves->GetWaveSize(retval->WaveID, &ulDataSize);
	_waves->GetWaveFrequency(retval->WaveID, &ulFrequency);
  _waves->GetWaveALBufferFormat(retval->WaveID, cTinyOAL::Instance()->getFunctions()->alGetEnumValue, &ulFormat);

	WAVEFORMATEX	wfex;
	// Queue 250ms of audio data
	_waves->GetWaveFormatExHeader(retval->WaveID, &wfex);
	ulBufferSize = wfex.nAvgBytesPerSec >> 2;

	// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
	ulBufferSize -= (ulBufferSize % wfex.nBlockAlign);

	if(!ulFormat)
  {
    TINYOAL_LOGM("ERROR","Failed to find format information, or unsupported format");
    CloseStream(retval); //clean up our mess
    return 0;
  }

  if(_flags & TINYOAL_LOADINTOMEMORY) //If this was NOT loaded into memory, it could be changed on us at any time, so we reload it for every opened stream
    _readinfo = true;

  return retval;
}

//Closes and cleans up the stream using the helper class.
void cAudioResourceWAV::CloseStream(TinyOAL::AUDIOSTREAM *stream)
{
  if(!stream) return;
  _waves->DeleteWaveFile(stream->WaveID);
  cAudioResource::CloseStream(stream);
}

//Resets the WAVE file to the beginning. Again, using a helper class.
bool cAudioResourceWAV::Reset(TinyOAL::AUDIOSTREAM *stream)
{
  if(!stream) return false;
  return _waves->SetWaveDataOffset(stream->WaveID, 0) >= 0;
}

bool cAudioResourceWAV::Skip(AUDIOSTREAM* stream, unsigned __int64 samples)
{
  unsigned short bits;
  if(_waves->GetWaveBitsPerSample(stream->WaveID, &bits) < 0) return false;
  return _waves->SetWaveDataOffset(stream->WaveID, (unsigned long)samples*(bits>>3)) >= 0;
}
unsigned __int64 cAudioResourceWAV::ToSample(AUDIOSTREAM* stream, double seconds)
{
  unsigned long freq;
  if(_waves->GetWaveFrequency(stream->WaveID, &freq) < 0) return 0;
  return (unsigned __int64)(freq*seconds); //frequency is number of samples per second, so we just multiply by time (which is in seconds anyway)
}