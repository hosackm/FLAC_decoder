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

typedef struct
{
    SNDFILE *f;
}my_client_data;

/****************************************************************************
************************* CALLBACK PROTOS ***********************************
****************************************************************************/
FLAC__StreamDecoderWriteStatus write_callback(  const FLAC__StreamDecoder *dec
                                                ,const FLAC__Frame *frame
                                                ,const FLAC__int32 * const buffer[]
                                                ,void *client_data);

void error_callback(const FLAC__StreamDecoder *dec
                    ,FLAC__StreamDecoderErrorStatus status
                    ,void *client_data);
/***************************************************************************/

int main(int argc, const char * argv[])
{
    FLAC__bool ret;
    FLAC__StreamDecoder *decoder;
    
    SNDFILE *f;
    SF_INFO info;
    
    my_client_data data;
    
    if (argc < 3)
    {
        printf("Must supply a filename and output name\n");
        return -1;
    }
    
    info.channels = 2;
    info.samplerate = 44100;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
    
    if(sf_format_check(&info) < 0)
    {
        printf("File format not correct\n");
        return -1;
    }
    
    f = sf_open(argv[2], SFM_WRITE, &info);
    
    if (sf_error(f) != SF_ERR_NO_ERROR) {
        printf("Could not open file: %s. error %d\n", argv[2], sf_error(f));
        printf("%s\n", sf_strerror(f));
        return -1;
    }
    
    data.f = f;
    
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
    
    sf_write_sync(f);
    
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
    
    sf_close(f);
    
    return 0;
}

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *dec
                                              ,const FLAC__Frame *frame
                                              ,const FLAC__int32 * const buffer[]
                                              ,void *client_data)
{
    FLAC__int32 *p_samples, *p_read;
    unsigned int samp, ch;
    const unsigned int length = frame->header.blocksize;
    const unsigned int channels = frame->header.channels;
    my_client_data *data = (my_client_data*)client_data;

    p_samples = (FLAC__int32*) malloc(sizeof(FLAC__int32) * channels *length);
    memset(p_samples, 0.0, sizeof(FLAC__int32) * channels * length);
    
    p_read = p_samples;
    for (samp = 0; samp < length; ++samp) {
        for(ch = 0; ch < channels; ++ch)
        {
            *p_read++ = (buffer[ch][samp] << 16);
        }
    }
    
    // Write from interleaved buffer
    sf_write_int(data->f, p_samples, length * channels);
    free(p_samples);
    
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void error_callback(const FLAC__StreamDecoder *dec
                    ,FLAC__StreamDecoderErrorStatus status
                    ,void *client_data)
{
    printf("Error Callback called but not handled very well...\n");
}
