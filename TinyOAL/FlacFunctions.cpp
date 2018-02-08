// Copyright ©2018 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "FlacFunctions.h"
#include "bss-util/bss_util.h"
#include "tinyoal/TinyOAL.h"

#ifdef BSS_PLATFORM_WIN32
#include "bss-util/win32_includes.h"

#define FLAC_MODULE "libflac.dll"
#define FLAC_MODULE_ALT "flac.dll"
#else
#include <dlfcn.h>
#define FLAC_MODULE "libFLAC.so.8"
#endif

#define DYNFUNC(v,t,n) v = (t)GETDYNFUNC(_flacDLL, n); \
		if(!v) TINYOAL_LOG(1,"Could not load " TXT(n))

using namespace tinyoal;

FlacFunctions::FlacFunctions(const char* force)
{
  if(!force)
    force = FLAC_MODULE;

  bss::bssFill(*this, 0);
  _flacDLL = LOADDYNLIB(force);
  if(!_flacDLL)
    _flacDLL = LOADDYNLIB(FLAC_MODULE_ALT);

  if(_flacDLL)
  {
    DYNFUNC(fn_flac_new, LPFLACNEW, FLAC__stream_decoder_new);
    DYNFUNC(fn_flac_delete, LPFLACDELETE, FLAC__stream_decoder_delete);

    DYNFUNC(fn_flac_set_ogg_serial, LPFLACSETOGGSERIAL, FLAC__stream_decoder_set_ogg_serial_number);
    DYNFUNC(fn_flac_set_md5_checking, LPFLACSETMD5, FLAC__stream_decoder_set_md5_checking);
    DYNFUNC(fn_flac_set_metadata_respond, LPFLACSETMETARESPOND, FLAC__stream_decoder_set_metadata_respond);
    DYNFUNC(fn_flac_set_metadata_respond_app, LPFLACSETMETARESPONDAPP, FLAC__stream_decoder_set_metadata_respond_application);
    DYNFUNC(fn_flac_set_metadata_respond_all, LPFLACSETMETARESPONDALL, FLAC__stream_decoder_set_metadata_respond_all);
    DYNFUNC(fn_flac_set_metadata_ignore, LPFLACSETMETAIGNORE, FLAC__stream_decoder_set_metadata_ignore);
    DYNFUNC(fn_flac_set_metadata_ignore_app, LPFLACSETMETAIGNOREAPP, FLAC__stream_decoder_set_metadata_ignore_application);
    DYNFUNC(fn_flac_set_metadata_ignore_all, LPFLACSETMETAIGNOREALL, FLAC__stream_decoder_set_metadata_ignore_all);

    DYNFUNC(fn_flac_get_state, LPFLACGETSTATE, FLAC__stream_decoder_get_state);
    DYNFUNC(fn_flac_get_state_string, LPFLACGETSTATESTRING, FLAC__stream_decoder_get_resolved_state_string);
    DYNFUNC(fn_flac_get_md5, LPFLACGETMD5, FLAC__stream_decoder_get_md5_checking);
    DYNFUNC(fn_flac_get_total_samples, LPFLACGETTOTALSAMPLES, FLAC__stream_decoder_get_total_samples);
    DYNFUNC(fn_flac_get_channels, LPFLACGETCHANNELS, FLAC__stream_decoder_get_channels);
    DYNFUNC(fn_flac_get_channel_assignments, LPFLACGETCHANNELASSIGN, FLAC__stream_decoder_get_channel_assignment);
    DYNFUNC(fn_flac_get_bits_per_sample, LPFLACGETBITSPERSAMPLE, FLAC__stream_decoder_get_bits_per_sample);
    DYNFUNC(fn_flac_get_sample_rate, LPFLACGETSAMPLERATE, FLAC__stream_decoder_get_sample_rate);
    DYNFUNC(fn_flac_get_block_size, LPFLACGETBLOCKSIZE, FLAC__stream_decoder_get_blocksize);
    DYNFUNC(fn_flac_get_decoder_position, LPFLACGETDECODEPOS, FLAC__stream_decoder_get_decode_position);

    DYNFUNC(fn_flac_init_stream, LPFLACINITSTREAM, FLAC__stream_decoder_init_stream);
    DYNFUNC(fn_flac_init_ogg_stream, LPFLACINITOGGSTREAM, FLAC__stream_decoder_init_ogg_stream);
    DYNFUNC(fn_flac_init_file, LPFLACINITFILE, FLAC__stream_decoder_init_FILE);
    DYNFUNC(fn_flac_init_ogg_file, LPFLACINITOGGFILE, FLAC__stream_decoder_init_ogg_FILE);
    DYNFUNC(fn_flac_init_path, LPFLACINITPATH, FLAC__stream_decoder_init_file);
    DYNFUNC(fn_flac_init_ogg_path, LPFLACINITOGGPATH, FLAC__stream_decoder_init_ogg_file);

    DYNFUNC(fn_flac_finish, LPFLACFINISH, FLAC__stream_decoder_finish);
    DYNFUNC(fn_flac_flush, LPFLACFLUSH, FLAC__stream_decoder_flush);
    DYNFUNC(fn_flac_reset, LPFLACRESET, FLAC__stream_decoder_reset);
    DYNFUNC(fn_flac_process_single, LPFLACPROCESSSINGLE, FLAC__stream_decoder_process_single);
    DYNFUNC(fn_flac_process_until_metadata_end, LPFLACPROCESSMETA, FLAC__stream_decoder_process_until_end_of_metadata);
    DYNFUNC(fn_flac_process_until_stream_end, LPFLACPROCESSSTREAM, FLAC__stream_decoder_process_until_end_of_stream);
    DYNFUNC(fn_flac_skip, LPFLACSKIP, FLAC__stream_decoder_skip_single_frame);
    DYNFUNC(fn_flac_seek, LPFLACSEEK, FLAC__stream_decoder_seek_absolute);
  }
  else
    TINYOAL_LOG(1, "Could not find the libflac DLL (or it may be missing one of its dependencies)");
}

FlacFunctions::~FlacFunctions()
{
  if(_flacDLL)
    FREEDYNLIB(_flacDLL);
}