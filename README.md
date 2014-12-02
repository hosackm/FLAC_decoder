FLAC_decoder
===========

Simple FLAC decoder as an exercise to learn libFLAC in more detail.
The program takes an input flac file and outputs a wav file.  For example:
```
FLAC_decoder input.flac output.wav
```

####Purpose

I've been investigating the different audio codecs available today.  The
purpose of this program is to build a minimal decoder in order to
understand libFLAC.  libFLAC was chosen because it is free and
open source.  Also, the audiophile in me thinks it's a lot cooler
than lossy codecs.

####Building
The program depends on libFLAC for decoding (duh!) and libsndfile for
writing the output wav file.  So you need to link to these libraries
and make sure their header files are in your search path.


####Limitations
* The file is assumed to be a stereo 16 bit 44.1 kHz sample rate signal
