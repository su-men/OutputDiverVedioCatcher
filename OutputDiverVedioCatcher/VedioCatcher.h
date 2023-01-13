#ifndef _VedioCatcher
#define _VedioCatcher

#include <combaseapi.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <memory>
#include <string>
#include <exception>
#include <stdint.h>
#include <ctime>

#include "PCMtoWAV.h"

extern bool g_output_to_console_as_wel;
extern bool g_loopFlag;
extern WAVEFORMATEXTENSIBLE g_formatex;
extern std::ofstream g_log;

#define log() g_log << "[Lay] "
#define logd() std::cout << "[Lay] "

#define SAFE_RELEASE(p)     \
	do                      \
	{                       \
		if ((p))            \
		{                   \
			(p)->Release(); \
			(p) = NULL;     \
		}                   \
	} while (0)


#define Process_if_failed(hr)                                                \
	do                                                                       \
	{                                                                        \
		if (FAILED(hr))                                                      \
		{                                                                    \
			std::cout << __LINE__ << ": errCode[" << hr << "]" << std::endl; \
			goto Exit;                                                       \
		}                                                                    \
	} while (false);


void InitLog(bool output_to_console = true);
HRESULT CreateDeviceEnumerator(IMMDeviceEnumerator **enumerator);
HRESULT CreateDevice(IMMDeviceEnumerator *enumerator, IMMDevice **device);
HRESULT CreateAudioClient(IMMDevice *device, IAudioClient **audioClient);
HRESULT IsFormatSupported(IAudioClient *audioClient);
HRESULT GetPreferFormat(IAudioClient *audioClient, WAVEFORMATEXTENSIBLE *formatex);
HRESULT InitAudioClient(IAudioClient *audioClient, WAVEFORMATEXTENSIBLE *formatex);
HRESULT CreateAudioCaptureClient(IAudioClient *audioClient, IAudioCaptureClient **audioCaptureClient);

void savePCM(const char *name,
	void *data,
	int bitsPerSample,
	int channels,
	int frames);


void ThreadRun(IAudioClient *audio_client, IAudioCaptureClient *audio_capture_client);


#endif