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
    const char  *filename;
    SNDFILE     *sf;
}sndfile_data;

FLAC__StreamDecoderWriteStatus
write_callback      (const FLAC__StreamDecoder *dec
                    ,const FLAC__Frame *frame
                    ,const FLAC__int32 * const buffer[]
                    ,void *client_data
                    );

void error_callback (const FLAC__StreamDecoder *dec
                    ,FLAC__StreamDecoderErrorStatus status
                    ,void *client_data
                    );

int main(int argc, const char * argv[])
{
    SNDFILE             *sf;
    SF_INFO             info;
    FLAC__bool          ret;
    FLAC__StreamDecoder *decoder;
    sndfile_data        data;
    const char          *in_file = argv[1];
    const char          *out_file = argv[2];

    
    if (argc < 3)
    {
        printf("Must supply a filename and output name\n");
        return -1;
    }
    
    /*  We want to output a Stereo WAV file
     *  16 bit resolution
     *  44.1 kHz sample rate 
     */
    info.channels = 2;
    info.samplerate = 44100;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    /* Verify we have a valid format */
    if(sf_format_check(&info) < 0)
    {
        printf("File format not correct\n");
        return -1;
    }
    
    /* Open our output file and check that it worked */
    sf = sf_open(out_file, SFM_WRITE, &info);

    if (sf_error(sf) != SF_ERR_NO_ERROR) {
        printf("Could not open file: %s. error %d\n", out_file, sf_error(sf));
        printf("%s\n", sf_strerror(sf));
        return -1;
    }
    
    /* Point our data to the SNDFILE */
    data.sf         = sf;
    data.filename   = in_file;
    
    decoder = FLAC__stream_decoder_new();
    
    /* We won't be handling any metadata or md5 checking for now */
    FLAC__stream_decoder_set_metadata_ignore_all(decoder);
    FLAC__stream_decoder_set_md5_checking(decoder, false);
    
    /* Initialize the decoder and point to our callbacks */
    ret = FLAC__stream_decoder_init_file(decoder
                                         ,in_file
                                         ,write_callback
                                         ,NULL
                                         ,error_callback
                                         ,&data
                                         );
    
    if (ret != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        printf("Error initializing FLAC Stream Decoder\n");
        return -1;
    }
    
    /* Begin the decoding process */
    ret = FLAC__stream_decoder_process_until_end_of_stream(decoder);
    
    /* Check the process call's return value */
    if (ret == false)
    {
        printf("Fatal error during decoding process!\n");
    }
    
    
    /* Clean up */
    ret = FLAC__stream_decoder_finish(decoder);
    
    if (ret == false)
    {
        printf("Error finishing decode process\n");
    }
    
    FLAC__stream_decoder_delete(decoder);
    
    sf_close(sf);
    
    return 0;
}

FLAC__StreamDecoderWriteStatus
write_callback( const FLAC__StreamDecoder *dec
               ,const FLAC__Frame *frame
               ,const FLAC__int32 * const buffer[]
               ,void *client_data)
{
    FLAC__int16         *p_samples;         /* Buffer for holding interleaved samples */
    unsigned int        sample_idx;         /* Index of the current sample we are on  */
    unsigned int        channel_idx;        /* Index of the current channel we are on */
    sndfile_data      *data       = (sndfile_data*)client_data; /* Cast data provided by libFLAC */
    const unsigned int  channels    = frame->header.channels;       /* Number of channels decoded by libFLAC */
    const unsigned int  blocksize   = frame->header.blocksize;      /*  Number of samples decoded by libFLAC.
                                                                    *   Each sample contains a Left and Right channel
                                                                    *   because we are only handling stereo
                                                                    */

    
    /* Allocate memory for our interleaved buffer we will be populating */
    p_samples = (FLAC__int16*) malloc(sizeof(FLAC__int16) * channels * blocksize);
    /* Always a good idea to zero-out the whole thing before writing to it */
    memset(p_samples, 0, sizeof(FLAC__int16) * channels * blocksize );
    
    // Copy from the FLAC buffers into an interleaved array of samples
    for (sample_idx = 0;  sample_idx < blocksize ; ++sample_idx)
    {
        for(channel_idx = 0; channel_idx < channels; ++channel_idx)
        {
            *p_samples++ = buffer[channel_idx][sample_idx] ;
        }
    }
    
    /* Decrement our p_samples pointer to it's first sample */
    p_samples -= blocksize  * channels;
    
    /* Write to wavfile from interleaved buffer */
    sf_write_short(data->sf, p_samples, blocksize * channels);
    
    /* Free allocated memory to avoid a memory leak */
    free(p_samples);
    
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void error_callback(const FLAC__StreamDecoder *dec
                    ,FLAC__StreamDecoderErrorStatus status
                    ,void *client_data)
{
    sndfile_data *d = (sndfile_data *)client_data;
    printf("'Error Callback' called while decoding\
           %s but not handled very well...\n" ,d->filename);
}
