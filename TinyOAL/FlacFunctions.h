// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __FLAC_FUNCTIONS_H__
#define __FLAC_FUNCTIONS_H__

#include "flac/stream_decoder.h"

namespace tinyoal {
  typedef FLAC__StreamDecoder* (*LPFLACNEW)(void);
  typedef void (*LPFLACDELETE)(FLAC__StreamDecoder*);

  typedef FLAC__bool (*LPFLACSETOGGSERIAL)(FLAC__StreamDecoder*,long);
  typedef FLAC__bool (*LPFLACSETMD5)(FLAC__StreamDecoder*,FLAC__bool);
  typedef FLAC__bool (*LPFLACSETMETARESPOND)(FLAC__StreamDecoder*,FLAC__MetadataType);
  typedef FLAC__bool (*LPFLACSETMETARESPONDAPP)(FLAC__StreamDecoder*,const FLAC__byte id[4]);
  typedef FLAC__bool (*LPFLACSETMETARESPONDALL)(FLAC__StreamDecoder*);
  typedef FLAC__bool (*LPFLACSETMETAIGNORE)(FLAC__StreamDecoder*,FLAC__MetadataType);
  typedef FLAC__bool (*LPFLACSETMETAIGNOREAPP)(FLAC__StreamDecoder*, const FLAC__byte id[4]);
  typedef FLAC__bool (*LPFLACSETMETAIGNOREALL)(FLAC__StreamDecoder*);
  
  typedef FLAC__StreamDecoderState (*LPFLACGETSTATE)(const FLAC__StreamDecoder*);
  typedef const char* (*LPFLACGETSTATESTRING)(const FLAC__StreamDecoder*);
  typedef FLAC__bool (*LPFLACGETMD5)(const FLAC__StreamDecoder*);
  typedef FLAC__uint64 (*LPFLACGETTOTALSAMPLES)(const FLAC__StreamDecoder*);
  typedef unsigned (*LPFLACGETCHANNELS)(const FLAC__StreamDecoder*);
  typedef FLAC__ChannelAssignment (*LPFLACGETCHANNELASSIGN)(const FLAC__StreamDecoder*);
  typedef unsigned (*LPFLACGETBITSPERSAMPLE)(const FLAC__StreamDecoder*);
  typedef unsigned (*LPFLACGETSAMPLERATE)(const FLAC__StreamDecoder*);
  typedef unsigned (*LPFLACGETBLOCKSIZE)(const FLAC__StreamDecoder*);
  typedef FLAC__bool (*LPFLACGETDECODEPOS)(const FLAC__StreamDecoder*, FLAC__uint64*);

  typedef FLAC__StreamDecoderInitStatus (*LPFLACINITSTREAM)(FLAC__StreamDecoder*,	FLAC__StreamDecoderReadCallback,FLAC__StreamDecoderSeekCallback,FLAC__StreamDecoderTellCallback,FLAC__StreamDecoderLengthCallback,FLAC__StreamDecoderEofCallback,FLAC__StreamDecoderWriteCallback,FLAC__StreamDecoderMetadataCallback,FLAC__StreamDecoderErrorCallback,void*);
  typedef FLAC__StreamDecoderInitStatus (*LPFLACINITOGGSTREAM)(FLAC__StreamDecoder*,FLAC__StreamDecoderReadCallback,FLAC__StreamDecoderSeekCallback,FLAC__StreamDecoderTellCallback,FLAC__StreamDecoderLengthCallback,FLAC__StreamDecoderEofCallback,FLAC__StreamDecoderWriteCallback,FLAC__StreamDecoderMetadataCallback,FLAC__StreamDecoderErrorCallback,void*);
  typedef FLAC__StreamDecoderInitStatus (*LPFLACINITFILE)(FLAC__StreamDecoder*,FILE*,FLAC__StreamDecoderWriteCallback,FLAC__StreamDecoderMetadataCallback,FLAC__StreamDecoderErrorCallback,void*);
  typedef FLAC__StreamDecoderInitStatus (*LPFLACINITOGGFILE)(FLAC__StreamDecoder*,FILE*,FLAC__StreamDecoderWriteCallback,FLAC__StreamDecoderMetadataCallback,FLAC__StreamDecoderErrorCallback,void*);
  typedef FLAC__StreamDecoderInitStatus (*LPFLACINITPATH)(FLAC__StreamDecoder*,const char*,FLAC__StreamDecoderWriteCallback,FLAC__StreamDecoderMetadataCallback,FLAC__StreamDecoderErrorCallback,void*);
  typedef FLAC__StreamDecoderInitStatus (*LPFLACINITOGGPATH)(FLAC__StreamDecoder*,const char*,FLAC__StreamDecoderWriteCallback,FLAC__StreamDecoderMetadataCallback,FLAC__StreamDecoderErrorCallback,void*);
  
  typedef FLAC__bool (*LPFLACFINISH)(FLAC__StreamDecoder*);
  typedef FLAC__bool (*LPFLACFLUSH)(FLAC__StreamDecoder*);
  typedef FLAC__bool (*LPFLACRESET)(FLAC__StreamDecoder*);
  typedef FLAC__bool (*LPFLACPROCESSSINGLE)(FLAC__StreamDecoder*);
  typedef FLAC__bool (*LPFLACPROCESSMETA)(FLAC__StreamDecoder*);
  typedef FLAC__bool (*LPFLACPROCESSSTREAM)(FLAC__StreamDecoder*);
  typedef FLAC__bool (*LPFLACSKIP)(FLAC__StreamDecoder*);
  typedef FLAC__bool (*LPFLACSEEK)(FLAC__StreamDecoder*, FLAC__uint64 sample);

	// This is a holder class for the FLAC DLL specific functions 
  class FlacFunctions
  {
  public:
    FlacFunctions(const char* force);
    ~FlacFunctions();
    inline bool Failure() { return _flacDLL==0; }

    LPFLACNEW fn_flac_new;
    LPFLACDELETE fn_flac_delete;

    LPFLACSETOGGSERIAL fn_flac_set_ogg_serial;
    LPFLACSETMD5 fn_flac_set_md5_checking;
    LPFLACSETMETARESPOND fn_flac_set_metadata_respond;
    LPFLACSETMETARESPONDAPP fn_flac_set_metadata_respond_app;
    LPFLACSETMETARESPONDALL fn_flac_set_metadata_respond_all;
    LPFLACSETMETAIGNORE fn_flac_set_metadata_ignore;
    LPFLACSETMETAIGNOREAPP fn_flac_set_metadata_ignore_app;
    LPFLACSETMETAIGNOREALL fn_flac_set_metadata_ignore_all;

    LPFLACGETSTATE fn_flac_get_state;
    LPFLACGETSTATESTRING fn_flac_get_state_string;
    LPFLACGETMD5 fn_flac_get_md5;
    LPFLACGETTOTALSAMPLES fn_flac_get_total_samples;
    LPFLACGETCHANNELS fn_flac_get_channels;
    LPFLACGETCHANNELASSIGN fn_flac_get_channel_assignments;
    LPFLACGETBITSPERSAMPLE fn_flac_get_bits_per_sample;
    LPFLACGETSAMPLERATE fn_flac_get_sample_rate;
    LPFLACGETBLOCKSIZE fn_flac_get_block_size;
    LPFLACGETDECODEPOS fn_flac_get_decoder_position;

    LPFLACINITSTREAM fn_flac_init_stream;
    LPFLACINITOGGSTREAM fn_flac_init_ogg_stream;
    LPFLACINITFILE fn_flac_init_file;
    LPFLACINITOGGFILE fn_flac_init_ogg_file;
    LPFLACINITPATH fn_flac_init_path;
    LPFLACINITOGGPATH fn_flac_init_ogg_path;

    LPFLACFINISH fn_flac_finish;
    LPFLACFLUSH fn_flac_flush;
    LPFLACRESET fn_flac_reset;
    LPFLACPROCESSSINGLE fn_flac_process_single;
    LPFLACPROCESSMETA fn_flac_process_until_metadata_end;
    LPFLACPROCESSSTREAM fn_flac_process_until_stream_end;
    LPFLACSKIP fn_flac_skip;
    LPFLACSEEK fn_flac_seek;

  protected:
    void* _flacDLL;
  };
}

#endif