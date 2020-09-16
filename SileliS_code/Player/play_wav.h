/*
 * play_wav.h
 *
 *  Created on: 29 lip 2020
 *      Author: dbank
 */

#ifndef PLAYER_PLAY_WAV_H_
#define PLAYER_PLAY_WAV_H_
#include <stdbool.h>

enum WAV_audioPreProcessorTypeDef {

//	PCM Format
//	The first part of the Format chunk is used to describe PCM data.
//
//	For PCM data, the Format chunk in the header declares the number of bits/sample in each sample (wBitsPerSample). The original documentation (Revision 1) specified that the number of bits per sample is to be rounded up to the next multiple of 8 bits. This rounded-up value is the container size. This information is redundant in that the container size (in bytes) for each sample can also be determined from the block size divided by the number of channels (nBlockAlign / nChannels).
//	This redundancy has been appropriated to define new formats. For instance, Cool Edit uses a format which declares a sample size of 24 bits together with a container size of 4 bytes (32 bits) determined from the block size and number of channels. With this combination, the data is actually stored as 32-bit IEEE floats. The normalization (full scale 223) is however different from the standard float format.
//	PCM data is two's-complement except for resolutions of 1-8 bits, which are represented as offset binary.
//	Non-PCM Formats
//	An extended Format chunk is used for non-PCM data. The cbSize field gives the size of the extension.
//
//	For all formats other than PCM, the Format chunk must have an extended portion. The extension can be of zero length, but the size field (with value 0) must be present.
//	For float data, full scale is 1. The bits/sample would normally be 32 or 64.
//	For the log-PCM formats (µ-law and A-law), the Rev. 3 documentation indicates that the bits/sample field (wBitsPerSample) should be set to 8 bits.
//	The non-PCM formats must have a fact chunk.
//	Extensible Format
//	The WAVE_FORMAT_EXTENSIBLE format code indicates that there is an extension to the Format chunk. The extension has one field which declares the number of valid bits/sample (wValidBitsPerSample). Another field (dwChannelMask) contains bits which indicate the mapping from channels to loudspeaker positions. The last field (SubFormat) is a 16-byte globally unique identifier (GUID).
//
//	With the WAVE_FORMAT_EXTENSIBLE format, the original bits/sample field (wBitsPerSample) must match the container size (8 * nBlockAlign / nChannels). This means that wBitsPerSample must be a multiple of 8. Reduced precision within the container size is now specified by wValidBitsPerSample.
//	The number of valid bits (wValidBitsPerSample) is informational only. The data is correctly represented in the precision of the container size. The number of valid bits can be any value from 1 to the container size in bits.
//	The loudspeaker position mask uses 18 bits, each bit corresponding to a speaker position (e.g. Front Left or Top Back Right), to indicate the channel to speaker mapping. More details are in the document cited above. This field is informational. An all-zero field indicates that channels are mapped to outputs in order: first channel to first output, second channel to second output, etc.
//	The first two bytes of the GUID form the sub-code specifying the data format code, e.g. WAVE_FORMAT_PCM. The remaining 14 bytes contain a fixed string, \x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71.
//	The WAVE_FORMAT_EXTENSIBLE format should be used whenever:
//
//	PCM data has more than 16 bits/sample.
//	The number of channels is more than 2.
//	The actual number of bits/sample is not equal to the container size.
//	The mapping from channels to speakers needs to be specified.

	WAVE_FORMAT_PCM 		= 0x0001,
	WAVE_FORMAT_IEEE_FLOAT	= 0x0003,
	WAVE_FORMAT_ALAW		= 0x0006,
	WAVE_FORMAT_MULAW		= 0x0007,
	WAVE_FORMAT_EXTENSIBLE	= 0xFFFE
};

struct  WAV_HeaderTypeDef//wav_header_t
{
	//	 * The header for a WAV file looks like this:
	//	 *
	//	 * The "RIFF" chunk descriptor:
	//	 * 1 - 4	"RIFF". Marks the file as a riff file. Characters are each 1 byte long.
	//	 * 5 - 8	File size (integer)	Size of the overall file - 8 bytes, in bytes (32-bit integer).
	//	 * 9 -12	"WAVE"	File Type Header. For our purposes, it always equals "WAVE".
	//	 *
	//	 * The "fmt" sub-chunk:
	//	 * 13-16	"fmt "	Format chunk marker. Includes trailing null
	//	 * 17-20	16	Length of format data as listed above
	//	 * 21-22	1	Type of format (1 is PCM) - 2 byte integer
	//	 * 23-24	2	Number of Channels - 2 byte integer
	//	 * 25-28	44100	Sample Rate - 32 byte integer. CSample Rate = Number of Samples per second, or Hertz.
	//	 * 29-32	176400	(Sample Rate * BitsPerSample * Channels) / 8. Byte rate
	//	 * 33-34	4	(BitsPerSample * Channels) / 8.1 - 8 bit mono2 - 8 bit stereo/16 bit mono4 - 16 bit stereo
	//	 * 35-36	16	Bits per sample
	//	 *
	//	 * The "data" sub-chunk:
	//	 * 37-40	"data" Marks the beginning of the data section.
	//	 * 41-44	File size (data). Size of the data section.
	//	 * 44-..    Data

    char chunkID[4]; //"RIFF" = 0x46464952
    unsigned long chunkSize; //28 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes] + sum(sizeof(chunk.id) + sizeof(chunk.size) + chunk.size)
    char format[4]; //"WAVE" = 0x45564157
    char subchunk1ID[4]; //"fmt " = 0x20746D66
    unsigned long subchunk1Size; //16 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes]
    enum WAV_audioPreProcessorTypeDef audioFormat;	//unsigned short audioFormat;
    unsigned short numChannels;
    //enum audioChannelsTypeDef numChannels;	//unsigned short numChannels;
    unsigned long sampleRate;
    unsigned long byteRate;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
    char  _data_Marker [4];
    unsigned long file_size;
};


static int	WavInit(FIL *wavFile);
static bool WavDecode(uint8_t *inBuffer, uint16_t *decodeBuffer, uint32_t frameSize);
void WavProcess(FIL *audiFile);
#endif /* PLAYER_PLAY_WAV_H_ */
