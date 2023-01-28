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

	// ������������������
	unsigned char *buff = (unsigned char*)malloc(4);

	// ��� wav_riff ͷ��Ϣ( ��RIFF�� (0x52494646) ��˴洢���ȸ�λ )
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
	printf("д��wav_riff %d ���ֽ�\n", sizeof(wav_riff));

	// ��� wav_format ͷ��Ϣ  'fmt �� (0x666D7420)
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
	printf("д��wav_format %d ���ֽ�\n", sizeof(wav_format));

	//д��Data ����  ��data�� (0x64617461)
	data_.id[0] = 'd';
	data_.id[1] = 'a';
	data_.id[2] = 't';
	data_.id[3] = 'a';
	data_.size = 0;
	num = fwrite(&data_, sizeof(wav_data), 1, wav);
	printf("д��wav_data %d ���ֽ�\n", sizeof(wav_data));

	fread(buff, 1, 4, pcm);
	while (!feof(pcm)) {
		//д��pcm ����
		data_.size += fwrite(buff, 1, 4, wav);
		fread(buff, 1, 4, pcm);
	}
	printf("д��pcm  %d���ֽ�\n", data_.size);

	//�����ļ���С 
	riff.size = data_.size + sizeof(wav_riff) + sizeof(wav_format) + sizeof(wav_data);


	rewind(wav); 		// �����ļ�λ��Ϊ������ stream ���ļ��Ŀ�ͷ

						// ��д wav ͷ��Ϣ 
	num = fwrite(&riff, sizeof(wav_riff), 1, wav);
	printf("д��wav_riff %d ���ֽ�\n", sizeof(wav_riff));

	num = fwrite(&format, sizeof(wav_format), 1, wav);
	printf("д��wav_format %d ���ֽ�\n", sizeof(wav_format));

	num = fwrite(&data_, sizeof(wav_data), 1, wav);	// ע�⣬����д����Ҫ���������ʱ���ݴ�С data_.size=0,����ʧ�� 
	printf("д��wav_data %d ���ֽ�\n", sizeof(wav_data));


	free(buff);
	fclose(wav);
	fclose(pcm);
}


/*
int main(void)
{
	split_PCM(2, 16, 44100);

	printf("�������\n");
	return 0;
}
*/

