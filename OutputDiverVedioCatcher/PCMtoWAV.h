#ifndef _PCMtoWAV
#define _PCMtoWAV

#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_SAMPLE_RATE 48000		 // 默认采样率:48kHz
#define DEFAULT_BITS_PER_SAMPLE 32		 // 默认位深:32bit
#define DEFAULT_CHANNELS  2				 // 默认音频通道数:2
#define DEFAULT_AUDIO_PACKET_INTERVAL 10 // 默认音频包发送间隔:10ms


typedef struct wav_riff {
	__int8 	id[4];
	__int32 size;
	__int8	type[4];
}wav_riff;

typedef struct wav_format {
	__int8 	id[4];
	__int32 size;
	__int16 audioformat;
	__int16 numchannels;
	__int32 samplerate;
	__int32 byterate;
	__int16 blockalign;
	__int16 bitspersample;
}wav_format;

typedef struct wav_data {
	__int8 id[4];
	__int32 size;
}wav_data;

void split_PCM(const char* PCMFlieName);

#endif
