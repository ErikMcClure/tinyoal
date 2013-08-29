// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudioResource.h"
#include "cAudioResourceOGG.h"
#include "cAudioResourceWAV.h"
#include "cAudioResourceMP3.h"
#include "cTinyOAL.h"
#include "cDataStream.h"
#include <stdio.h>
#include <string.h>

using namespace TinyOAL;
bss_util::cKhash_StringIns<cAudioResource*> cAudioResource::_audiolist;

AUDIOSTREAM::AUDIOSTREAM(const AUDIOSTREAM& copy)
{
  isfile = copy.isfile;
  if(isfile)
    file = copy.file;
  else
    data = new cDataStream(*copy.data);
}

AUDIOSTREAM::AUDIOSTREAM(FILE* _file)
{
  isfile = true;
  file = _file;
}

AUDIOSTREAM::AUDIOSTREAM(void* _data, size_t _datalength)
{
  isfile = false;
  data = new cDataStream(_data, _datalength);
}

AUDIOSTREAM::~AUDIOSTREAM()
{
  if(isfile)
    fclose(file);
  else
    delete data;
}

cAudioResource::cAudioResource(const cAudioResource& copy)
{
  _f = copy._f;
  _path = copy._path;
  _datalength = copy._datalength;
  _externdata = copy._externdata;
  if(!_externdata)
  {
  _data = malloc(_datalength);
  memcpy(_data, copy._data, _datalength);
  }
  else
    _data = copy._data;
}

// Constructor to build from void* data.
cAudioResource::cAudioResource(void* data, unsigned int datalength, T_TINYOAL_FLAGS flags)
{
  _flags = flags;
  _f = 0;
  _datalength = datalength;
  _data = 0;
  // Are we loading from external data XOR told to load into memory?
  _externdata = !(_flags & TINYOAL_LOADINTOMEMORY);
  if(_datalength > 4) // If datalength is less than 4, forget it.
  {
    if(_externdata) // If we're loading from external data and were not told to load it into memory
      _data = data; // Save our data pointer
    else // If we're not loading from external data but were told to load it into memory
    {
      _data = malloc(_datalength); // Allocate a block of memory to store the data
      memcpy_s(_data, _datalength, data, _datalength); // Copy data into _data.
    }
  }
  
  size_t length = sizeof(void*);
  _path.reserve(length+1);
  memcpy(_path.UnsafeString(), (const void*)&data, length); //We use the actual position of the data here for comparison purposes
  _path.UnsafeString()[length] = '\0';
}

//long funcFileSize(const char* sFileName) //C++ doesn't have a portable method for filesize :(
//{
//  std::ifstream f;
//  f.open(sFileName, std::ios_base::binary | std::ios_base::in);
//  if (!f.good() || f.eof() || !f.is_open()) { return 0; }
//  f.seekg(0, std::ios_base::beg);
//  std::ifstream::pos_type begin_pos = f.tellg();
//  f.seekg(0, std::ios_base::end);
//  return static_cast<long>(f.tellg() - begin_pos);
//}

// Constructor that takes a file name and flags.
cAudioResource::cAudioResource(const char* file, T_TINYOAL_FLAGS flags)
{
  _flags = flags;
  _f = 0;
  _datalength=0;
  _externdata = false;
  _data = 0;
  
  // If there is no file name, exit.
  if(!file)
    return;
  _path = file; // Assign the path to the file.

  if(_flags & TINYOAL_LOADINTOMEMORY) // If it's told to load it into memory
  {
    fopen_s(&_f, _path, "rb"); // Open the file to read with a buffer.
    _loadintomemory(); // Load the sound into memory
    fclose(_f); // Close the file
    _f = 0; // Clean up the iobuffer pointer.
  }
}

// This constructor will always load the file into memory.
//   It's given a datalength (to load?).
cAudioResource::cAudioResource(_iobuf* file, unsigned int datalength, T_TINYOAL_FLAGS flags)
{
	// Set the flag to load it into memory if it's not already set.
  _flags = flags|TINYOAL_LOADINTOMEMORY;
  _f = file;
  _datalength = datalength;
  _data = 0;
  _externdata = false;
  _loadintomemory();
  
  size_t length = sizeof(std::istream*);
  _path.reserve(length+1);
  memcpy(_path.UnsafeString(), (const void*)&_f, length);
  _path.UnsafeString()[length] = '\0';
}

cAudioResource::~cAudioResource()
{
  _removeref(this);
  if(_f)
    fclose(_f);
  if(!_externdata && _data)
    free(_data);
}

namespace TinyOAL {
	enum TINYOAL_FILETYPE : unsigned char
	{
		TINYOAL_FILETYPE_UNKNOWN,
		TINYOAL_FILETYPE_OGG,
		TINYOAL_FILETYPE_MP3,
		TINYOAL_FILETYPE_WAV
	};
}

//This function does NOT check to see if fileheader is 4 characters long
unsigned char __fastcall cAudioResource::_getfiletype(const char* fileheader)
{  //This is NOT wchar_t because these are really just bytes, not actual characters, but happen to map to ASCII nicely.
	if(!strncmp(fileheader, "OggS", 4)) return TINYOAL_FILETYPE_OGG;
  //if(!strncmp(fileheader, "ID3", 3)) return TINYOAL_FILETYPE_MP3; //MP3 not supported as of v.0.9.8
  if(!strncmp(fileheader, "RIFF", 4) || !strncmp(fileheader, "WAVE", 4)) return TINYOAL_FILETYPE_WAV;

	return TINYOAL_FILETYPE_UNKNOWN;
}

cAudioResource* cAudioResource::CreateAudioReference(void *data, unsigned int datalength, T_TINYOAL_FLAGS flags)
{
  if(!cTinyOAL::Instance()->getFunctions()) return 0;

  cAudioResource* retval = FindExists(data, 0, flags);
  if(retval!=0) { retval->Grab(); return retval; }

  if(!data || datalength < 4)
  {
    if(!data) TINYOAL_LOGM("WARNING", "NULL data pointer passed in to CreateAudioReference()");
    else TINYOAL_LOGM("WARNING", "datalength less than 4 passed to CreateAudioReference()");
    return 0;
  }

	switch(_getfiletype((const char*)data))
	{
	case TINYOAL_FILETYPE_OGG:
    TINYOAL_LOG("INFO") << "Loading " << (void*)data << " as OGG" << std::endl;
    return _addref(new cAudioResourceOGG(data, datalength, flags));
#ifdef __INCLUDE_MP3
	case TINYOAL_FILETYPE_MP3:
    TINYOAL_LOG("INFO") << "Loading " << (void*)data << " as MP3" << std::endl;
    return _addref(new cAudioResourceMP3(data, datalength,flags));
#endif
	case TINYOAL_FILETYPE_WAV:
    TINYOAL_LOG("INFO") << "Loading " << (void*)data << " as WAVE" << std::endl;
    return _addref(new cAudioResourceWAV(data,datalength, flags)); //Assume its a wav
	}

  TINYOAL_LOG("WARNING") << data << " is using an unknown or unrecognized format, or may be corrupt." << std::endl;
  return 0; //Unknown format
}

cAudioResource* cAudioResource::CreateAudioReference(_iobuf* file, unsigned int datalength, T_TINYOAL_FLAGS flags)
{
  if(!cTinyOAL::Instance()->getFunctions()) return 0;

  if(!file || !datalength) //bad file pointer
  {
    if(!file) TINYOAL_LOGM("WARNING", "NULL file pointer passed in to CreateAudioReference()");
    else TINYOAL_LOGM("WARNING", "datalength equal to 0 in CreateAudioReference()");
    return 0;
  }

  cAudioResource* retval = FindExists(&file, 0, flags);
  if(retval!=0) { retval->Grab(); return retval; }

  cStr fileheader(4);
  //file.read(fileheader.UnsafeString(),4);
  fread(fileheader.UnsafeString(), 1, 4,file);
  //file.seekg(std::ios_base::cur,-4);
  fseek(file, -4, SEEK_CUR); //reset file pointer

	switch(_getfiletype((const char*)fileheader))
	{
	case TINYOAL_FILETYPE_OGG:
    TINYOAL_LOG("INFO") << "Loading " << (void*)file << " as OGG" << std::endl;
    return _addref(new cAudioResourceOGG(file, datalength, flags));
#ifdef __INCLUDE_MP3
	case TINYOAL_FILETYPE_MP3:
    TINYOAL_LOG("INFO") << "Loading " << (void*)file << " as MP3" << std::endl;
    return _addref(new cAudioResourceMP3(file, datalength, flags));
#endif
	case TINYOAL_FILETYPE_WAV:
    TINYOAL_LOG("INFO") << "Loading " << (void*)file << " as WAVE" << std::endl;
    return _addref(new cAudioResourceWAV(file, datalength, flags));
	}

  TINYOAL_LOG("WARNING") << (void*)file << " is using an unknown or unrecognized format, or may be corrupt." << std::endl;
  return 0; //Unknown format
}

cAudioResource* cAudioResource::CreateAudioReference(const char *file, T_TINYOAL_FLAGS flags)
{
  if(!cTinyOAL::Instance()->getFunctions()) return 0;

  if(!file)
  {
    TINYOAL_LOGM("WARNING", "NULL file pointer passed in to CreateAudioReference()");
    return 0;
  }

  cAudioResource* retval = FindExists(0, file, flags);
  if(retval!=0) { retval->Grab(); return retval; }

  cStr fileheader(4);
  FILE* f;
  WFOPEN(f, cStrW(file), L"rb");
  if(!f) { TINYOAL_LOG("WARNING") << "Could not open " << file << std::endl; return 0; } //bad file
  fread(fileheader.UnsafeString(), 1, 4,f);
  fclose(f);

	switch(_getfiletype((const char*)fileheader))
	{
	case TINYOAL_FILETYPE_OGG:
    TINYOAL_LOG("INFO") << "Loading " << file << " as OGG" << std::endl;
    return _addref(new cAudioResourceOGG(file, flags));
#ifdef __INCLUDE_MP3
	case TINYOAL_FILETYPE_MP3:
    TINYOAL_LOG("INFO") << "Loading " << file << " as MP3" << std::endl;
    return _addref(new cAudioResourceMP3(file, flags));
#endif
	case TINYOAL_FILETYPE_WAV:
    TINYOAL_LOG("INFO") << "Loading " << file << " as WAVE" << std::endl;
    return _addref(new cAudioResourceWAV(file,flags)); 
	}

  TINYOAL_LOG("WARNING") << file << " is using an unknown or unrecognized format, or may be corrupt." << std::endl;
  return 0; // Unknown format
}

// This function returns an actual AUDIOSTREAM, that is, a stream that reads from the data that is stored inside this resource.
// This stream is first processed by whatever file format it is in, and then handed off to cAudio, which is unaware of what
//  format the audio stream is actually in, because it doesn't matter. 
AUDIOSTREAM* cAudioResource::OpenStream()
{
  if(!_data || !_datalength) // If there is no data we fall back to the file (There can be a file AND a data pointer, but if that is true, the file pointer is merely a marker and shouldn't be used.
  {
    if(_path[0] == '\0' || _f != 0)
    { // Either there's nothing to open or the file is already open.
      TINYOAL_LOGM("WARNING", "_path and _f empty, no stream opened");
      return 0;
    }
    fopen_s(&_f, _path, "rb");
    return new AUDIOSTREAM(_f);
  }
  return new AUDIOSTREAM(_data, _datalength); //otherwise data never fails
}

// This closes a stream. It is called by a cAudio instance when it is being destroyed.
void cAudioResource::CloseStream(AUDIOSTREAM* stream)
{
  if(stream->isfile && stream->file == _f) // if it doesn't match its not ours
  {
    delete stream;
    _f = 0;
  }
  else if(stream->data->begin() == _data) //if it doesn't match its not ours
    delete stream;
}

cAudioResource* cAudioResource::FindExists(void* filedata, const char* path, T_TINYOAL_FLAGS flags)
{
  cStr tmp;
  if(!path)
  {
    _encodeptr(filedata, tmp);
    path=tmp;
  }
  else
    tmp=path;
  tmp+=flags;
  return _audiolist.GetKeyPtrOnly(tmp.c_str());
}

void __fastcall cAudioResource::_loadintomemory()
{
  if(!_datalength) // If we don't know how long _datalength is...
  {
    char buffer[CHUNKSIZE]; // assign a buffer of CHUNKSIZE
    size_t read=0; // Declare a couple ints
    size_t nlength=0;
    while(!feof(_f)) // Do the following until we hit the end of the file
    {
      read=fread(buffer, 1, CHUNKSIZE, _f); // Read the file into a character buffer.
	  // Save a "read" that is the length of what got read
      if(!read) break; // If we didn't read anything, stop!
      nlength = _datalength+read; // Otherwise, save nlength (newlength) as datalength plus
									// the length of what we just read.
      char* ndata = (char*)malloc(nlength);  // Allocate memory for the nlength.
      memcpy_s(ndata, nlength, _data, _datalength); // Copy _data into nlength.
      free(_data); // Free up _data
      memcpy_s(ndata+_datalength, nlength-_datalength, buffer, read); // Copy the reading buffer into nlength.
      _data = ndata; // set _data to ndata (newdata).
      _datalength = nlength; // Set _datalength to the newlength.
    }
  }
  else // If we DO know how long it is...
  {
    _data = malloc(_datalength); // Allocate space for the data.
    fread(_data, 1, _datalength, _f); // Read the data into that space.
  }
}

cAudioResource* __fastcall cAudioResource::_addref(cAudioResource* target)
{
  target->_listpos=_audiolist.Insert(target->_path, target);
  target->Grab(); //gotta grab the thing
  return target;
}

void __fastcall cAudioResource::_removeref(cAudioResource* target)
{
  if(!target->_path.empty())
    _audiolist.Remove(target->_path);
}

void cAudioResource::DeleteAll()
{
  TINYOAL_LOGM("INFO", "DeleteAll() called");
  for(bss_util::cKhash_StringIns<cAudioResource*>::cKhash_Iter hold(_audiolist); hold.IsValid(); ++hold)
  {
    auto p=_audiolist[*hold];
    p->_path.clear();
    delete p;
  }

  _audiolist.Clear();
}

void cAudioResource::Drop()
{
  cRefCounter::Drop(); //Done to ensure this is deleted in the correct DLL
}

bool cAudioResource::Reset(AUDIOSTREAM* stream)
{
  if(!stream->file) return false;
  if(stream->isfile)
    fseek(stream->file, 0, SEEK_SET);
  else
    stream->data->seek(0, SEEK_SET);
  return true;
}

void __fastcall cAudioResource::_encodeptr(void* ptr, cStr& buf)
{
  buf=cStrF("%p", ptr);
}

unsigned __int64 cAudioResource::GetLoopStart(AUDIOSTREAM* stream)
{
  return 0;
}
