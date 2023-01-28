#include "PCMtoWAV.h"
#include<iostream>
//char name[] = "tiankongzhicheng_44100_2ch_16bit.pcm";


void split_PCM(const char* PCMFlieName)
{
	unsigned char data[2];
	int size = DEFAULT_CHANNELS * DEFAULT_BITS_PER_SAMPLE * DEFAULT_SAMPLE_RATE / 8, num;

	wav_riff riff;
	wav_format format;
	wav_data data_;
	
	char* cpyFileName = (char*)malloc(1000);
	char* WAVFileName;

	strncpy(cpyFileName, PCMFlieName, strlen(PCMFlieName));

	WAVFileName = strtok(cpyFileName, ".");
	if (WAVFileName == NULL) {
	printf("subFileName:null");
		return;
	}
	printf("subFileName: %s\n", WAVFileName);

	strncat(WAVFileName, ".wav", 4);
	

	FILE *pcm = fopen(PCMFlieName, "rb+");
	FILE *wav = fopen(WAVFileName, "wb+");

	// 申请两个声道的数据
	unsigned char *buff = (unsigned char*)malloc(4);

	// 填充 wav_riff 头信息( ‘RIFF’ (0x52494646) 大端存储，先高位 )
	riff.id[0] = 'R';
	riff.id[1] = 'I';
	riff.id[2] = 'F';
	riff.id[3] = 'F';
	riff.size = sizeof(wav_riff) + sizeof(wav_format);		// 
	riff.type[0] = 'W';	// (WAVE  0x57415645)
	riff.type[1] = 'A';
	riff.type[2] = 'V';
	riff.type[3] = 'E';
	num = fwrite(&riff, sizeof(wav_riff), 1, wav);
	printf("写入wav_riff %d 个字节\n", sizeof(wav_riff));

	// 填充 wav_format 头信息  'fmt ’ (0x666D7420)
	format.id[0] = 'f';
	format.id[1] = 'm';
	format.id[2] = 't';
	format.id[3] = ' ';
	format.size = 16;
	format.audioformat = 1;	//pcm
	format.numchannels = DEFAULT_CHANNELS;
	format.samplerate = DEFAULT_SAMPLE_RATE;
	format.byterate = 4 * DEFAULT_SAMPLE_RATE;  //2 *  samplerate * 16 / 8;
	format.blockalign =  DEFAULT_CHANNELS * DEFAULT_BITS_PER_SAMPLE  / 8; 		 //ch * bit / 8;
	format.bitspersample = DEFAULT_BITS_PER_SAMPLE;
	num = fwrite(&format, sizeof(wav_format), 1, wav);
	printf("写入wav_format %d 个字节\n", sizeof(wav_format));

	//写入Data 区块  ‘data’ (0x64617461)
	data_.id[0] = 'd';
	data_.id[1] = 'a';
	data_.id[2] = 't';
	data_.id[3] = 'a';
	data_.size = 0;
	num = fwrite(&data_, sizeof(wav_data), 1, wav);
	printf("写入wav_data %d 个字节\n", sizeof(wav_data));

	fread(buff, 1, 4, pcm);
	while (!feof(pcm)) {
		//写入pcm 数据
		data_.size += fwrite(buff, 1, 4, wav);
		fread(buff, 1, 4, pcm);
	}
	printf("写入pcm  %d个字节\n", data_.size);

	//配置文件大小 
	riff.size = data_.size + sizeof(wav_riff) + sizeof(wav_format) + sizeof(wav_data);


	rewind(wav); 		// 设置文件位置为给定流 stream 的文件的开头

						// 重写 wav 头信息 
	num = fwrite(&riff, sizeof(wav_riff), 1, wav);
	printf("写入wav_riff %d 个字节\n", sizeof(wav_riff));

	num = fwrite(&format, sizeof(wav_format), 1, wav);
	printf("写入wav_format %d 个字节\n", sizeof(wav_format));

	num = fwrite(&data_, sizeof(wav_data), 1, wav);	// 注意，该重写很重要，如果播放时数据大小 data_.size=0,播放失败 
	printf("写入wav_data %d 个字节\n", sizeof(wav_data));


	free(buff);
	fclose(wav);
	fclose(pcm);
}


/*
int main(void)
{
	split_PCM(2, 16, 44100);

	printf("程序结束\n");
	return 0;
}
*/

