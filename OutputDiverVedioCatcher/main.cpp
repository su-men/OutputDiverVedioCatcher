#include "VedioCatcher.h"


int main()
{
	// initialize the log
	InitLog();

	IMMDeviceEnumerator *pDeviceEnumerator = nullptr;
	IMMDevice *pDevice = nullptr;
	IAudioClient *pAudioClient = nullptr;
	IAudioCaptureClient *pAudioCaptureClient = nullptr;
	std::unique_ptr<std::thread> capture_thread = nullptr;

	std::string input_str;

	HRESULT hr;

	hr = CreateDeviceEnumerator(&pDeviceEnumerator);
	Process_if_failed(hr);

	hr = CreateDevice(pDeviceEnumerator, &pDevice);
	Process_if_failed(hr);

	hr = CreateAudioClient(pDevice, &pAudioClient);
	Process_if_failed(hr);

	hr = IsFormatSupported(pAudioClient);
	if (FAILED(hr))
	{
		hr = GetPreferFormat(pAudioClient, &g_formatex);
		if (FAILED(hr))
		{
			Process_if_failed(hr);
		}
	}

	hr = InitAudioClient(pAudioClient, &g_formatex);
	Process_if_failed(hr);

	hr = CreateAudioCaptureClient(pAudioClient, &pAudioCaptureClient);
	Process_if_failed(hr);

	g_loopFlag = true;
	capture_thread = std::make_unique<std::thread>(ThreadRun, pAudioClient, pAudioCaptureClient);

	do
	{
		std::cout << "Type \"quit\" to quit.... " << std::endl;
		std::cin >> input_str;
		if (input_str == "quit")
		{
			g_loopFlag = false;
			capture_thread->join();
			break;
		}
	} while (true);

Exit:
	SAFE_RELEASE(pAudioCaptureClient);
	if (pAudioClient != nullptr)
	{
		pAudioClient->Stop();
	}
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pDeviceEnumerator);

	if (g_log.is_open())
	{
		g_log.close();
	}

	split_PCM("echo_speaker-48000Hz_32b_2c.pcm", DEFAULT_CHANNELS, DEFAULT_BITS_PER_SAMPLE, DEFAULT_SAMPLE_RATE);

	return 0;
}

