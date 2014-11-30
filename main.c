//
//  main.c
//  FLAC_Decoder
//
//  Created by Matthew Hosack on 11/28/14.
//  Copyright (c) 2014 Matthew Hosack. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "FLAC/stream_decoder.h"
#include "sndfile.h"

//#define BIT32

typedef struct
{
    const char  *filename;
    SNDFILE     *sf;
}my_client_data;

FLAC__StreamDecoderWriteStatus write_callback(  const FLAC__StreamDecoder *dec
                                                ,const FLAC__Frame *frame
                                                ,const FLAC__int32 * const buffer[]
                                                ,void *client_data);

void error_callback(const FLAC__StreamDecoder *dec
                    ,FLAC__StreamDecoderErrorStatus status
                    ,void *client_data);

int main(int argc, const char * argv[])
{
    SNDFILE             *sf;
    SF_INFO             info;
    FLAC__bool          ret;
    FLAC__StreamDecoder *decoder;
    my_client_data      data;
    
    if (argc < 3)
    {
        printf("Must supply a filename and output name\n");
        return -1;
    }
    
    /*  We want to output a Stereo WAV File
     *  32 bit depth
     *  44.1 kHz sample rate */
    info.channels = 2;
    info.samplerate = 44100;
#ifdef BIT32
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
#else
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
#endif
    /* Verify we have a valid format */
    if(sf_format_check(&info) < 0)
    {
        printf("File format not correct\n");
        return -1;
    }
    
    /* Open our output file and check that it worked */
    sf = sf_open(argv[2], SFM_WRITE, &info);

    if (sf_error(sf) != SF_ERR_NO_ERROR) {
        printf("Could not open file: %s. error %d\n", argv[2], sf_error(sf));
        printf("%s\n", sf_strerror(sf));
        return -1;
    }
    
    /* Point our data to the SNDFILE */
    data.sf         = sf;
    data.filename   = argv[1];
    
    decoder = FLAC__stream_decoder_new();
    
    FLAC__stream_decoder_set_metadata_ignore_all(decoder);
    
    FLAC__stream_decoder_set_md5_checking(decoder, false);
    
    ret = FLAC__stream_decoder_init_file(decoder
                                         ,argv[1]
                                         ,write_callback
                                         ,NULL
                                         ,error_callback
                                         ,&data);
    
    if (ret != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        printf("Error initializing FLAC Stream Decoder\n");
        return -1;
    }
    
    sf_write_sync(sf);
    
    ret = FLAC__stream_decoder_process_until_end_of_stream(decoder);
    
    if (ret == false)
    {
        printf("Fatal error during decoding process!\n");
    }
    
    ret = FLAC__stream_decoder_finish(decoder);
    
    if (ret == false)
    {
        printf("Error finishing decode process\n");
    }
    
    FLAC__stream_decoder_delete(decoder);
    
    sf_close(sf);
    
    return 0;
}

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *dec
                                              ,const FLAC__Frame *frame
                                              ,const FLAC__int32 * const buffer[]
                                              ,void *client_data)
{
#ifdef BIT32
    FLAC__int32         *p_samples;
#else
    FLAC__int16         *p_samples;
#endif
    unsigned int        sample_idx;
    unsigned int        channel_idx;
    my_client_data      *data       = (my_client_data*)client_data;
    const unsigned int  channels    = frame->header.channels;
    const unsigned int  blocksize   = frame->header.blocksize;

#ifdef BIT32
    p_samples = (FLAC__int32*) malloc(sizeof(FLAC__int32) * channels * blocksize );
    memset(p_samples, 0, sizeof(FLAC__int32) * channels * blocksize );
#else
    p_samples = (FLAC__int16*) malloc(sizeof(FLAC__int16) * channels * blocksize );
    memset(p_samples, 0, sizeof(FLAC__int16) * channels * blocksize );
#endif
    // Copy from the FLAC buffers into an interleaved array of samples
#ifdef BIT32
    for ( sample_idx = 0;  sample_idx < blocksize ; ++sample_idx )
    {
        for(channel_idx = 0; channel_idx < channels; ++channel_idx )
        {
            *p_samples++ = buffer[channel_idx][sample_idx] << 16;
        }
    }
    p_samples -= blocksize  * channels;
#else
    for ( sample_idx = 0;  sample_idx < blocksize ; ++sample_idx )
    {
        for(channel_idx = 0; channel_idx < channels; ++channel_idx )
        {
            *p_samples++ = /*(FLAC__int16)*/ buffer[channel_idx][sample_idx] ;
        }
    }
    p_samples -= blocksize  * channels;
#endif
    // Write from interleaved buffer
#ifdef BIT32
    sf_write_int(data->sf, p_samples, blocksize  * channels);
#else
    sf_write_short(data->sf, p_samples, blocksize * channels);
#endif
    free(p_samples);
    
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void error_callback(const FLAC__StreamDecoder *dec
                    ,FLAC__StreamDecoderErrorStatus status
                    ,void *client_data)
{
    my_client_data *d = (my_client_data *)client_data;
    printf("'Error Callback' called while decoding\
           %s but not handled very well...\n" ,d->filename);
}
