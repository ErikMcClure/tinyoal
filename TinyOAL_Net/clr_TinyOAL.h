// Copyright ©2008-2009 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __CLR_TINYOAL_H__
#define __CLR_TINYOAL_H__

namespace TinyOAL {
  class cTinyOAL;
}

namespace TinyOAL_net {
  ref class clr_Audio;

	/* Managed wrapper for cTinyOAL class */
  public ref class clr_TinyOAL
  {
  public:
		/* Constructor - Default */
    clr_TinyOAL();
		/* Constructor - takes a defaultbuffers value. The default value for this argument is 4 */
    clr_TinyOAL(int defaultbuffers);
		/* Constructor - takes a defaultbuffers value and a logfile. */
    clr_TinyOAL(int defaultbuffers, System::String^ logfile);
		/* Destructor */
    ~clr_TinyOAL();
    int getDefaultBuffer();
    void setDefaultBuffer(int defaultbuffer);
		/* This updates any currently playing samples and returns the number that are still playing after the update. The time between calls to this update function can never exceed the length of a buffer, or the sound will cut out. */
    unsigned int Update();
    /* Creates an audio sample that is automatically deleted after it finishes playing */
    clr_Audio^ ManagedLoad(clr_AudioRef^ ref);

  private:
    TinyOAL::cTinyOAL* _ref; //pointer to unmanaged instance
  };
}

#endif