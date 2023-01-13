#include "VedioCatcher.h"

bool g_output_to_console_as_well = false;
bool g_loopFlag = false;
std::ofstream g_log;
WAVEFORMATEXTENSIBLE g_formatex;

void InitLog(bool output_to_console)
{
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[32] = { 0 };

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 32, "%Y-%m-%d_%H-%M-%S", timeinfo);
	std::string logName(buffer);
	logName = "logs/" + logName + ".log";

	g_log.open(logName, std::ios::app);
}

HRESULT CreateDeviceEnumerator(IMMDeviceEnumerator **enumerator)
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	return CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
		__uuidof(IMMDeviceEnumerator),
		reinterpret_cast<void **>(enumerator));
}

HRESULT CreateDevice(IMMDeviceEnumerator *enumerator, IMMDevice **device)
{
	EDataFlow enDataFlow = eRender;		// 表示获取扬声器的audio_endpoint
	ERole enRole = eConsole;
	return enumerator->GetDefaultAudioEndpoint(enDataFlow, enRole, device);
}

HRESULT CreateAudioClient(IMMDevice *device, IAudioClient **audioClient)
{
	return device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL,
		(void **)audioClient);
}

HRESULT IsFormatSupported(IAudioClient *audioClient)
{
	WAVEFORMATEX *format = &g_formatex.Format;
	format->nSamplesPerSec = DEFAULT_SAMPLE_RATE;
	format->wBitsPerSample = DEFAULT_BITS_PER_SAMPLE;
	format->nChannels = DEFAULT_CHANNELS;

	WAVEFORMATEX *closestMatch = nullptr;

	HRESULT hr = audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, format, &closestMatch);
	if (hr == AUDCLNT_E_UNSUPPORTED_FORMAT) // 0x88890008
	{
		if (closestMatch != nullptr) // 如果找不到最相近的格式，closestMatch可能为nullptr
		{
			logd() << "Epxected format: "
				<< "sample_rate[" << format->nSamplesPerSec << "] "
				<< "bits_per_sample[" << format->wBitsPerSample << "] "
				<< "channels[" << format->nChannels << "] "
				<< "\n"
				<< "Supported format: "
				<< "sample_rate[" << closestMatch->nSamplesPerSec << "] "
				<< "bits_per_sample[" << closestMatch->wBitsPerSample << "] "
				<< "channels[" << closestMatch->nChannels << "] "
				<< "\n";

			format->nSamplesPerSec = closestMatch->nSamplesPerSec;
			format->wBitsPerSample = closestMatch->wBitsPerSample;
			format->nChannels = closestMatch->nChannels;

			return S_OK;
		}
	}

	return hr;
}

HRESULT GetPreferFormat(IAudioClient *audioClient, WAVEFORMATEXTENSIBLE *formatex)
{
	WAVEFORMATEX *format = nullptr;
	HRESULT hr = audioClient->GetMixFormat(&format);
	if (FAILED(hr))
	{
		return hr;
	}

	logd() << "Prefer format: "
		<< "sample_rate[" << format->nSamplesPerSec << "] "
		<< "bits_per_sample[" << format->wBitsPerSample << "] "
		<< "channels[" << format->nChannels << "] "
		<< "\n";

	formatex->Format.nSamplesPerSec = format->nSamplesPerSec;
	formatex->Format.wBitsPerSample = format->wBitsPerSample;
	formatex->Format.nChannels = format->nChannels;

	return hr;
}

HRESULT InitAudioClient(IAudioClient *audioClient, WAVEFORMATEXTENSIBLE *formatex)
{
	AUDCLNT_SHAREMODE shareMode =
		AUDCLNT_SHAREMODE_SHARED;					  // share Audio Engine with other applications
	DWORD streamFlags = AUDCLNT_STREAMFLAGS_LOOPBACK; // loopback speaker
													  //	streamFlags |=
													  //		AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM; // A channel matrixer and a sample
													  // rate converter are inserted
													  //	streamFlags |=
													  //		AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY; // a sample rate converter
													  // with better quality than
													  // the default conversion but
													  // with a higher performance
													  // cost is used
	REFERENCE_TIME hnsBufferDuration = 0;
	WAVEFORMATEX *format = &formatex->Format;
	format->wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	format->nBlockAlign = (format->wBitsPerSample >> 3) * format->nChannels;
	format->nAvgBytesPerSec = format->nBlockAlign * format->nSamplesPerSec;
	format->cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
	formatex->Samples.wValidBitsPerSample = format->wBitsPerSample;
	formatex->dwChannelMask =
		format->nChannels == 1 ? KSAUDIO_SPEAKER_MONO : KSAUDIO_SPEAKER_STEREO;
	formatex->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	return audioClient->Initialize(shareMode, streamFlags, hnsBufferDuration, 0,
		format, nullptr);
}

HRESULT CreateAudioCaptureClient(IAudioClient *audioClient, IAudioCaptureClient **audioCaptureClient)
{
	HRESULT hr = audioClient->GetService(IID_PPV_ARGS(audioCaptureClient));
	if (FAILED(hr))
	{
		*audioCaptureClient = nullptr;
	}
	return hr;
}

void ThreadRun(IAudioClient *audio_client, IAudioCaptureClient *audio_capture_client)
{
	HRESULT hr = S_OK;
	UINT32 num_success = 0;
	char pcmName[128] = { 0 };
	sprintf(pcmName, "echo_speaker-%dHz_%db_%dc.pcm",
		g_formatex.Format.nSamplesPerSec,
		g_formatex.Format.wBitsPerSample,
		g_formatex.Format.nChannels);

	logd() << "pcmName: " << pcmName;

	BYTE *p_audio_data = nullptr;
	UINT32 num_frames_to_read = 0;
	DWORD dw_flag = 0;

	UINT32 num_frames_in_next_packet = 0;

	UINT32 num_loop = 0;

	audio_client->Start();

	while (g_loopFlag)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(0));

		while (true)
		{
			hr = audio_capture_client->GetNextPacketSize(&num_frames_in_next_packet);
			if (FAILED(hr))
			{
				throw std::exception();
			}
			if (num_frames_in_next_packet == 0)
			{
				break;
			}
			log() << "Loop[" << num_loop << "] "
				<< "GetNextPacketSize: "
				<< "num_frames_in_next_packet[" << num_frames_in_next_packet << "] "
				<< "\n";

			hr = audio_capture_client->GetBuffer(&p_audio_data, &num_frames_to_read, &dw_flag, nullptr, nullptr);
			if (FAILED(hr))
			{
				throw std::exception();
			}
			log() << "Loop[" << num_loop << "] "
				<< "GetBuffer: "
				<< "num_frames_to_read[" << num_frames_to_read << "] "
				<< "\n";

			savePCM(pcmName, p_audio_data, g_formatex.Format.wBitsPerSample, g_formatex.Format.nChannels, num_frames_to_read);
			log() << "Loop[" << num_loop << "] "
				<< "savePCM: "
				<< "bits_per_sample[" << g_formatex.Format.wBitsPerSample << "] "
				<< "channels[" << g_formatex.Format.nChannels << "] "
				<< "num_frames_to_read[" << num_frames_to_read << "] "
				<< "\n";

			if (num_success++ % 500 == 0)
			{
				std::cout << "Have already cpatured [" << num_success << "] times." << std::endl;
			}

			hr = audio_capture_client->ReleaseBuffer(num_frames_to_read);
			if (FAILED(hr))
			{
				throw std::exception();
			}

			num_loop++;
		}
	}

	audio_client->Stop();
}

void savePCM(const char *name,
	void *data,
	int bitsPerSample,
	int channels,
	int frames)
{
	FILE *fp = fopen(name, "ab+");
	fwrite(data, bitsPerSample >> 3, channels * frames, fp);
	fclose(fp);

	//	split_PCM(name, DEFAULT_CHANNELS, DEFAULT_BITS_PER_SAMPLE, DEFAULT_SAMPLE_RATE);
	fp = NULL;


}