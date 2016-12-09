#include "stdafx.h"
#include "WaveOut.h"
#include "AudioProcess.h"
#include <stdio.h>
#include <Windows.h>
#include <afxmt.h>

#pragma comment(lib, "winmm.lib")
#define MAXIMUM_VOLUME 0xFFFFFFFF
#define AUDIO_SAMPLERATE 48000

#define DATASIZE 7300 //分次截取数据大小
FILE*			pcmfile=NULL;  //音频文件
HWAVEOUT        hwo=NULL;

CCriticalSection g_wavecs;
#ifdef _DEBUG
unsigned char tosavedata[655350] = { 0 };
int curlen = 0;
#endif // _DEBUG


void CALLBACK WaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwInstance, DWORD dw1, DWORD dw2)//回调函数
{
	switch (uMsg)
	{
		case WOM_DONE://上次缓存播放完成,触发该事件
		{
			if (pcmfile)
			{
				static int k = 0;
				LPWAVEHDR pWaveHeader = (LPWAVEHDR)dw1;
				pWaveHeader->dwBufferLength = 0;
				//if ((k++)%2==0)
				//{
					pWaveHeader->dwBufferLength = fread(pWaveHeader->lpData, 1, DATASIZE, pcmfile);;
				//}
				waveOutPrepareHeader(hwo, pWaveHeader, sizeof(WAVEHDR));
				waveOutWrite(hWave, pWaveHeader, sizeof(WAVEHDR));
			}

			break;
		}
		case WOM_CLOSE:
		{
			LPWAVEHDR pWaveHeader = (LPWAVEHDR)dw1;
			if (pWaveHeader)
			{
				while (waveOutUnprepareHeader(hWave, pWaveHeader, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
					Sleep(100);//清理数据						  
			}
			 break;
		}
	}
}



void CALLBACK WaveCallback2(HWAVEOUT hWave, UINT uMsg, DWORD dwInstance, DWORD dw1, DWORD dw2)//回调函数
{
	g_wavecs.Lock();
	//TRACE1("WaveCallback2 TrheadID:%d\n", GetCurrentThreadId());
	
	switch (uMsg)
	{
		case WOM_DONE://上次缓存播放完成,触发该事件
		{
				CWaveOut *pwo = (CWaveOut *)dwInstance;
				LPWAVEHDR pWaveHeader = (LPWAVEHDR)dw1;
				if (pwo && pWaveHeader)
				{
					int len = 0;
				
					len = pwo->GetPRF();
					BYTE *pData = (BYTE *)pwo->m_Queue.Get(len);
					if (len>0&&pData)
					{

						memcpy(pWaveHeader->lpData, pData, len * 2);
						
						short B1[16] = {
							//77, 199, 554, 1226, 2180, 3248, 4178, 4721, 4721,\
							4178, 3248, 2180, 1226, 554, 199, 77
							//0, 105, 519, 1300, 2348, 3419, 4208, 4485, 4485,\
							4208, 3419, 2348, 1300, 519, 105, 0
							//0, 1, 1, 1, 1, 1, 1, 1, 1, \
							1, 1, 1, 1, 1, 1, 0

							//3132, 844, -3802, 313, 4011, -1619, -3669, 2819, 2819,\
							-3669, -1619, 4011, 313, -3802, 844, 3132
							- 251, -1349, -2612, -2980, -1690, 1131, 4384, 6550, 6550,\
							4384, 1131, -1690, -2980, -2612, -1349, -251
						};
						short B2[15] = {
							812, 1535, 0, -2311, -1907, 2863, 9268, 12248, 9268,
							2863, -1907, -2311, 0, 1535, 812
						};

						short B3[16] = {
							2823, -896, 1277, -1670, 2044, -2362, 2595, -2717, 2717,
							-2595, 2362, -2044, 1670, -1277, 896, -2823
						};

						//short B4[5] = {
						//	1,2,4,2,1
						//};
						//for (int i = 0; i < 128; i++)
						//{
						//	pData[i] *= i / 128.0;
						//	pData[len * 2 - 1 - i] *= i / 128.0;
						//}
						//CWaveOut::LPFitler(pData, (BYTE*)pWaveHeader->lpData, len * 2, B4, 5);

						

						//memcpy(pWaveHeader->lpData, pData, len * 2);
						//pWaveHeader->dwBufferLength = len * 2;
						
					}	
					else
					{
						len = 0;
						//memcpy(pWaveHeader->lpData, lastsound, len*2);
#ifdef _DEBUG
						SYSTEMTIME tm;
						GetLocalTime(&tm);
						TRACE("Dump %02d:%02d.%03d\n", tm.wMinute, tm.wSecond, tm.wMilliseconds);
#endif // _DEBUG
						
					}
					//pWaveHeader->dwBufferLength = len * 2;
					//waveOutPrepareHeader(hWave, pWaveHeader, sizeof(WAVEHDR));
					//waveOutWrite(hWave, pWaveHeader, sizeof(WAVEHDR));
					pWaveHeader->dwBufferLength = len * 2;
					waveOutPrepareHeader(hWave, pWaveHeader, sizeof(WAVEHDR));
					waveOutWrite(hWave, pWaveHeader, sizeof(WAVEHDR));
					
				}

				

			break;
		}
		case WOM_CLOSE:
		{
			//LPWAVEHDR pWaveHeader = (LPWAVEHDR)dw1;
			//if (pWaveHeader)
			//{
			//	while (waveOutUnprepareHeader(hWave, pWaveHeader, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
			//		Sleep(100);//清理数据						  
			//}
			CWaveOut *pwo = (CWaveOut *)dwInstance;
			if (pwo)
			{
				pwo->UnPrepare();
				//pwo->Clear();
			}
			break;
		}
	}
	g_wavecs.Unlock();
}

short lastsound[2] = { 0,0 };
void CALLBACK WaveCallback3(HWAVEOUT hWave, UINT uMsg, DWORD dwInstance, DWORD dw1, DWORD dw2)//回调函数
{
	g_wavecs.Lock();
	//TRACE1("WaveCallback2 TrheadID:%d\n", GetCurrentThreadId());

	switch (uMsg)
	{
	case WOM_DONE://上次缓存播放完成,触发该事件
	{
		CWaveOut *pwo = (CWaveOut *)dwInstance;
		LPWAVEHDR pWaveHeader = (LPWAVEHDR)dw1;
		if (pwo && pWaveHeader)
		{
			int len = 0;

			len = pwo->GetPRF();
			
			BYTE *pData = (BYTE *)pwo->m_Queue_s.Get(len);
			//TRACE1("播放数据长度:%d\n", len);
			if (len > 0 && pData)
			{

				memcpy(pWaveHeader->lpData, pData, len * 4);
				lastsound[0] = pData[len * 2 - 2];
				lastsound[1] = pData[len * 2 - 1];
				short B1[16] = {
					//77, 199, 554, 1226, 2180, 3248, 4178, 4721, 4721,\
												4178, 3248, 2180, 1226, 554, 199, 77
//0, 105, 519, 1300, 2348, 3419, 4208, 4485, 4485,\
							4208, 3419, 2348, 1300, 519, 105, 0
//0, 1, 1, 1, 1, 1, 1, 1, 1, \
							1, 1, 1, 1, 1, 1, 0

//3132, 844, -3802, 313, 4011, -1619, -3669, 2819, 2819,\
							-3669, -1619, 4011, 313, -3802, 844, 3132
					-251, -1349, -2612, -2980, -1690, 1131, 4384, 6550, 6550,\
					4384, 1131, -1690, -2980, -2612, -1349, -251
				};
				short B2[15] = {
					812, 1535, 0, -2311, -1907, 2863, 9268, 12248, 9268,
					2863, -1907, -2311, 0, 1535, 812
				};

				short B3[16] = {
					2823, -896, 1277, -1670, 2044, -2362, 2595, -2717, 2717,
					-2595, 2362, -2044, 1670, -1277, 896, -2823
				};

				//short B4[5] = {
				//	1,2,4,2,1
				//};
				//for (int i = 0; i < 128; i++)
				//{
				//	pData[i] *= i / 128.0;
				//	pData[len * 2 - 1 - i] *= i / 128.0;
				//}
				//CWaveOut::LPFitler(pData, (BYTE*)pWaveHeader->lpData, len * 2, B4, 5);



				//memcpy(pWaveHeader->lpData, pData, len * 2);
				//pWaveHeader->dwBufferLength = len * 2;

			}
			else
			{
				len = 1;
				memcpy(pWaveHeader->lpData, lastsound, len*4);
//#ifdef _DEBUG
//				SYSTEMTIME tm;
//				GetLocalTime(&tm);
//				TRACE("Dump %02d:%02d.%03d\n", tm.wMinute, tm.wSecond, tm.wMilliseconds);
//#endif // _DEBUG

			}
			//pWaveHeader->dwBufferLength = len * 2;
			//waveOutPrepareHeader(hWave, pWaveHeader, sizeof(WAVEHDR));
			//waveOutWrite(hWave, pWaveHeader, sizeof(WAVEHDR));
			pWaveHeader->dwBufferLength = len * 4;
			waveOutPrepareHeader(hWave, pWaveHeader, sizeof(WAVEHDR));
			waveOutWrite(hWave, pWaveHeader, sizeof(WAVEHDR));

		}



		break;
	}
	case WOM_CLOSE:
	{
		//LPWAVEHDR pWaveHeader = (LPWAVEHDR)dw1;
		//if (pWaveHeader)
		//{
		//	while (waveOutUnprepareHeader(hWave, pWaveHeader, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		//		Sleep(100);//清理数据						  
		//}
		CWaveOut *pwo = (CWaveOut *)dwInstance;
		if (pwo)
		{
			pwo->UnPrepare();
			//pwo->Clear();
		}
		break;
	}
	}
	g_wavecs.Unlock();
}

/*
void CWaveOut::PlayDemo()
{
	int             cnt;
	WAVEHDR         wh1 = { 0 };
	WAVEHDR         wh2 = { 0 };
	WAVEFORMATEX    wfx = { 0 };

	fopen_s(&pcmfile, "PWAudio.wav", "rb");//打开文件
	fseek(pcmfile, 44, SEEK_SET);
	wfx.wFormatTag = WAVE_FORMAT_PCM;//设置波形声音的格式
	wfx.nChannels = 2;//设置音频文件的通道数量
	wfx.nSamplesPerSec = 7300;//设置每个声道播放和记录时的样本频率
	wfx.nAvgBytesPerSec = 14600;//设置请求的平均数据传输率,单位byte/s。这个值对于创建缓冲大小是很有用的
	wfx.nBlockAlign = 2;//以字节为单位设置块对齐
	wfx.wBitsPerSample = 8;
	wfx.cbSize = 0;//额外信息的大小

	waveOutOpen(&hwo, WAVE_MAPPER, &wfx, (DWORD)WaveCallback, (DWORD_PTR)this, CALLBACK_FUNCTION);//打开一个给定的波形音频输出装置来进行声音播放，方式为回调函数方式。如果是对话框程序，可以将第五个参数改为(DWORD)this，操作跟本Demo程序相似

	wh1.dwLoops = 0L;//播放区一
	wh1.lpData = new char[DATASIZE];
	wh1.dwBufferLength = DATASIZE;
	fread(wh1.lpData, 1, DATASIZE, pcmfile);
	wh1.dwFlags = 0L;
	waveOutPrepareHeader(hwo, &wh1, sizeof(WAVEHDR));//准备一个波形数据块用于播放
	waveOutWrite(hwo, &wh1, sizeof(WAVEHDR));//在音频媒体中播放第二个参数指定的数据，也相当于开启一个播放区的意思

	wh2.dwLoops = 0L;//播放区二，基本同上
	wh2.lpData = new char[DATASIZE];
	wh2.dwBufferLength = DATASIZE;
	fread(wh2.lpData, 1, DATASIZE, pcmfile);
	wh2.dwFlags = 0L;
	waveOutPrepareHeader(hwo, &wh2, sizeof(WAVEHDR));
	waveOutWrite(hwo, &wh2, sizeof(WAVEHDR));

	while (wh1.dwBufferLength > 0  || wh2.dwBufferLength > 0)//如果文件还在没播放完则等待500ms
	{
		Sleep(500);
	}
	fclose(pcmfile);//关闭文件
	pcmfile = NULL;

	while (waveOutUnprepareHeader(hwo, &wh1, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		Sleep(100);//清理数据
	while (waveOutUnprepareHeader(hwo, &wh2, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		Sleep(100);//清理数据
	waveOutClose(hwo);
	
	delete[]wh1.lpData;
	delete[]wh2.lpData;
	

	//fopen_s(&pcmfile, "PWAudio.wav", "rb");//打开文件
	//fseek(pcmfile, 44, SEEK_SET);
	//m_wfx.wFormatTag = WAVE_FORMAT_PCM;//设置波形声音的格式
	//m_wfx.nChannels = 2;//设置音频文件的通道数量
	//m_wfx.nSamplesPerSec = 7300;//设置每个声道播放和记录时的样本频率
	//m_wfx.nAvgBytesPerSec = 14600;//设置请求的平均数据传输率,单位byte/s。这个值对于创建缓冲大小是很有用的
	//m_wfx.nBlockAlign = 2;//以字节为单位设置块对齐
	//m_wfx.wBitsPerSample = 8;
	//m_wfx.cbSize = 0;//额外信息的大小

	//waveOutOpen(&hwo, WAVE_MAPPER, &m_wfx, (DWORD)WaveCallback, 0L, CALLBACK_FUNCTION);//打开一个给定的波形音频输出装置来进行声音播放，方式为回调函数方式。如果是对话框程序，可以将第五个参数改为(DWORD)this，操作跟本Demo程序相似

	//m_wh1.dwLoops = 0L;//播放区一
	//m_wh1.lpData = new char[DATASIZE];
	//m_wh1.dwBufferLength = 0;
	////m_wh1.dwBufferLength = DATASIZE;
	////fread(m_wh1.lpData, 1, DATASIZE, pcmfile);
	//m_wh1.dwFlags = 0L;
	//waveOutPrepareHeader(hwo, &m_wh1, sizeof(WAVEHDR));//准备一个波形数据块用于播放
	//waveOutWrite(hwo, &m_wh1, sizeof(WAVEHDR));//在音频媒体中播放第二个参数指定的数据，也相当于开启一个播放区的意思

	//m_wh2.dwLoops = 0L;//播放区二，基本同上
	//m_wh2.lpData = new char[DATASIZE];
	//m_wh2.dwBufferLength = 0;
	////m_wh2.dwBufferLength = DATASIZE;
	////fread(m_wh2.lpData, 1, DATASIZE, pcmfile);
	//m_wh2.dwFlags = 0L;
	//waveOutPrepareHeader(hwo, &m_wh2, sizeof(WAVEHDR));
	//waveOutWrite(hwo, &m_wh2, sizeof(WAVEHDR));

	//waveOutClose(hwo);
	return;
}
*/


CWaveOut::CWaveOut()
{
	m_BuffTime=1000;
	m_nChannel=2;
	m_prf=0;
	m_hdprf = 0;
	m_BitsPerSample=8;
	memset(&m_wh1, 0, sizeof(m_wh1));
	memset(&m_wh2, 0, sizeof(m_wh2));
	memset(&m_wfx, 0, sizeof(m_wfx));
	m_hwo = 0;
	m_playBuff[0] = NULL;
	m_playBuff[1] = NULL;
	m_MaxVolume = 255;
	m_finterval = 1.0f;
	m_pTmp = NULL;
	m_pTmp_s = NULL;
}


CWaveOut::~CWaveOut()
{

#ifdef _DEBUG
	//CAudioProcess::WriteWave(tosavedata, curlen, m_prf, ".\\lastpw.wav");
	//CAudioProcess::WriteWave((unsigned short*)tosavedata, curlen / 2, m_prf, ".\\lastpw.wav");
	
	CAudioProcess::WriteWave((unsigned short*)tosavedata, curlen/2, AUDIO_SAMPLERATE, ".\\lastpw.wav");
#endif
	if (m_hwo)
	{
		waveOutPause(m_hwo);
		waveOutClose(m_hwo);
		//while (waveOutUnprepareHeader(m_hwo, &m_wh1, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		//	Sleep(100);//清理数据
		//while (waveOutUnprepareHeader(m_hwo, &m_wh2, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		//	Sleep(100);//清理数据
		//waveOutClose(m_hwo);
		m_hwo = NULL;
	}
	

	Clear();
}

//int CWaveOut::CreateWave(int BuffTime, int Channel, int prf, int BitsPerSample, int maxVolume)
//{
//	m_BuffTime = BuffTime;
//	m_nChannel = Channel;
//	m_BitsPerSample = BitsPerSample;
//	m_AudioBuffsz = BuffTime*prf*Channel/1000;
//	if (maxVolume>0)
//	{
//		m_MaxVolume = maxVolume;
//	}
//	
//	return SetPRF(prf);
//
//}
//
//int CWaveOut::SetPRF(int prf)
//{
//	if (prf<100)
//	{
//		return -1;
//	}
//	if (prf==m_prf)
//	{
//		return 1;
//	}
//	if (m_hwo)
//	{
//		waveOutPause(m_hwo);
//		waveOutClose(m_hwo);
//		//while (waveOutUnprepareHeader(m_hwo, &m_wh1, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
//		//	Sleep(100);//清理数据
//		//while (waveOutUnprepareHeader(m_hwo, &m_wh2, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
//		//	Sleep(100);//清理数据
//		
//		m_hwo = 0;
//	}
//	Clear();
//	
//	m_prf = prf-10;
//	m_AudioBuffsz = m_BuffTime*m_prf*m_nChannel / 1000;
//	m_Queue.Create(m_AudioBuffsz, m_prf,0x80);
//	
//	memset(&m_wh1, 0, sizeof(m_wh1));
//	memset(&m_wh2, 0, sizeof(m_wh2));
//	memset(&m_wfx, 0, sizeof(m_wfx));
//
//	m_playbufsz = m_prf * 2;
//
//	m_playBuff[0] = new char[m_playbufsz+1000];
//	m_playBuff[1] = new char[m_playbufsz+1000];
//	memset(m_playBuff[0], 0x80, m_playbufsz+1000);
//	memset(m_playBuff[1], 0x80, m_playbufsz+1000);
//
//	m_wfx.wFormatTag = WAVE_FORMAT_PCM;//设置波形声音的格式
//	m_wfx.nChannels = m_nChannel;//设置音频文件的通道数量
//	m_wfx.nSamplesPerSec = m_prf-100;//设置每个声道播放和记录时的样本频率
//	m_wfx.nAvgBytesPerSec = (m_prf)*m_nChannel;//设置请求的平均数据传输率,单位byte/s。这个值对于创建缓冲大小是很有用的
//	m_wfx.nBlockAlign = 2;//以字节为单位设置块对齐
//	m_wfx.wBitsPerSample = m_BitsPerSample;
//	m_wfx.cbSize = 0;//额外信息的大小
//
//	waveOutOpen(&m_hwo, WAVE_MAPPER, &m_wfx, (DWORD)WaveCallback2, (DWORD_PTR) this, CALLBACK_FUNCTION);//打开一个给定的波形音频输出装置来进行声音播放，方式为回调函数方式。如果是对话框程序，可以将第五个参数改为(DWORD)this，操作跟本Demo程序相似
//
//	m_wh1.dwLoops = 0L;//播放区一
//	m_wh1.lpData = m_playBuff[0];
//	m_wh1.dwBufferLength = 0;
//	m_wh1.dwFlags = 0L;
//	waveOutPrepareHeader(m_hwo, &m_wh1, sizeof(WAVEHDR));//准备一个波形数据块用于播放
//	waveOutWrite(m_hwo, &m_wh1, sizeof(WAVEHDR));//在音频媒体中播放第二个参数指定的数据，也相当于开启一个播放区的意思
//
//	m_wh2.dwLoops = 0L;//播放区二，基本同上
//	m_wh2.lpData = m_playBuff[1];
//	m_wh2.dwBufferLength = 0;
//	m_wh2.dwFlags = 0L;
//	waveOutPrepareHeader(m_hwo, &m_wh2, sizeof(WAVEHDR));
//	waveOutWrite(m_hwo, &m_wh2, sizeof(WAVEHDR));
//
//	return 0;
//}


int CWaveOut::CreateWave(int BuffTime, int Channel, int prf, int BitsPerSample, int maxVolume)
{
	g_wavecs.Lock();
	m_BuffTime = BuffTime;
	m_nChannel = Channel;
	m_BitsPerSample = BitsPerSample;
	m_AudioBuffsz = BuffTime*prf*Channel / 1000;
	if (maxVolume > 0)
	{
		m_MaxVolume = maxVolume;
	}

	//TRACE1("1 TrheadID:%d\n", GetCurrentThreadId());
	if (prf < 100)
	{
		//TRACE0("2\n");
		g_wavecs.Unlock();
		return -1;
	}
	//TRACE0("3\n");
	if (m_hwo)
	{
		
		//TRACE0("4\n");
		waveOutPause(m_hwo);
		//TRACE0("5\n");
		waveOutClose(m_hwo);
		//TRACE0("6\n");
		//while (waveOutUnprepareHeader(m_hwo, &m_wh1, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		//	Sleep(100);//清理数据
		//while (waveOutUnprepareHeader(m_hwo, &m_wh2, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		//	Sleep(100);//清理数据

		m_hwo = 0;
	}
	Clear();
	//TRACE0("7\n");

	m_AudioBuffsz = m_BuffTime*AUDIO_SAMPLERATE*m_nChannel / 1000;
	m_Queue.Create(m_AudioBuffsz, AUDIO_SAMPLERATE, 0x80);
	m_Queue_s.Create(m_AudioBuffsz, AUDIO_SAMPLERATE, 0x80);
	//TRACE0("8\n");
	m_pTmp = new char[m_AudioBuffsz * 2];
	memset(m_pTmp, 0, m_AudioBuffsz * 2 * sizeof(char));
	m_pTmp_s = new short[m_AudioBuffsz * 2];
	memset(m_pTmp_s, 0, m_AudioBuffsz * 2 * sizeof(short));

	memset(&m_wh1, 0, sizeof(m_wh1));
	memset(&m_wh2, 0, sizeof(m_wh2));
	memset(&m_wfx, 0, sizeof(m_wfx));

	m_playbufsz = AUDIO_SAMPLERATE * 4;

	m_playBuff[0] = new char[m_playbufsz + 1000];
	m_playBuff[1] = new char[m_playbufsz + 1000];
	memset(m_playBuff[0], 0x80, m_playbufsz + 1000);
	memset(m_playBuff[1], 0x80, m_playbufsz + 1000);
	//TRACE0("9\n");
	m_wfx.wFormatTag = WAVE_FORMAT_PCM;//设置波形声音的格式
	m_wfx.nChannels = m_nChannel;//设置音频文件的通道数量
	m_wfx.nSamplesPerSec = AUDIO_SAMPLERATE;//设置每个声道播放和记录时的样本频率
	m_wfx.nAvgBytesPerSec = (AUDIO_SAMPLERATE)*m_nChannel*m_BitsPerSample/8;//设置请求的平均数据传输率,单位byte/s。这个值对于创建缓冲大小是很有用的
	m_wfx.nBlockAlign = m_nChannel*m_BitsPerSample / 8;//以字节为单位设置块对齐
	m_wfx.wBitsPerSample = m_BitsPerSample;
	m_wfx.cbSize = 0;//额外信息的大小

	//重复开关waveout设备会使设备阻塞，导致程序崩溃，所以只能初始化一次，创建一个巨大的波特率
	waveOutOpen(&m_hwo, WAVE_MAPPER, &m_wfx, (DWORD)WaveCallback3, (DWORD_PTR) this, CALLBACK_FUNCTION);//打开一个给定的波形音频输出装置来进行声音播放，方式为回调函数方式。如果是对话框程序，可以将第五个参数改为(DWORD)this，操作跟本Demo程序相似
	//TRACE0("10\n");
	m_wh1.dwLoops = 0L;//播放区一
	m_wh1.lpData = m_playBuff[0];
	m_wh1.dwBufferLength = 0;
	m_wh1.dwFlags = 0L;
	waveOutPrepareHeader(m_hwo, &m_wh1, sizeof(WAVEHDR));//准备一个波形数据块用于播放
	waveOutWrite(m_hwo, &m_wh1, sizeof(WAVEHDR));//在音频媒体中播放第二个参数指定的数据，也相当于开启一个播放区的意思
	//TRACE0("11\n");
	m_wh2.dwLoops = 0L;//播放区二，基本同上
	m_wh2.lpData = m_playBuff[1];
	m_wh2.dwBufferLength = 0;
	m_wh2.dwFlags = 0L;
	waveOutPrepareHeader(m_hwo, &m_wh2, sizeof(WAVEHDR));
	waveOutWrite(m_hwo, &m_wh2, sizeof(WAVEHDR));
	//TRACE0("12\n");
	g_wavecs.Unlock();
	return SetPRF(prf);
}

int CWaveOut::SetPRF(int prf)
{
	g_wavecs.Lock();
	//TRACE1("SetPrf TrheadID:%d PRF:%d\n", GetCurrentThreadId(), prf);

	if (prf<100)
	{
		g_wavecs.Unlock();
		return -7;
	}
	if (prf>20)
	{
		
		m_prf = prf;
		m_hdprf = m_prf;
		m_finterval = AUDIO_SAMPLERATE * 1.0 / m_prf;
	}
	g_wavecs.Unlock();
	return 0;
}

void CWaveOut::Clear()
{
	m_wh1.lpData = NULL;
	m_wh2.lpData = NULL;
	m_wh1.dwBufferLength = 0;
	m_wh2.dwBufferLength = 0;
	if (m_playBuff[0])
	{
		delete[] m_playBuff[0];
		m_playBuff[0] = NULL;
	}
	if (m_playBuff[1])
	{
		delete[] m_playBuff[1];
		m_playBuff[1] = NULL;
	}
	if (m_pTmp)
	{
		delete[] m_pTmp;
		m_pTmp = NULL;
	}
	if (m_pTmp_s)
	{
		delete[] m_pTmp_s;
		m_pTmp_s = NULL;
	}
	m_Queue.Clear();
	m_Queue_s.Clear();
}

clock_t g_tm = 0;
int CWaveOut::AddDatasToBuff(BYTE *pData, DWORD length, WORD Flag)
{
	static char lastData[2] = { 0,0 };
	g_wavecs.Lock();
	int tm = clock();
	int prf = length / 2.0 / (tm - g_tm) * 1000;
	
	if (prf>0 && g_tm!=0)
	{
		m_prf = prf-10;
		m_finterval = AUDIO_SAMPLERATE * 1.0 / m_prf;
	}
	g_tm = tm;

	////TRACE1("AddDatasToBuff TrheadID:%d\n", GetCurrentThreadId());
//#ifdef _DEBUG
//	if (length + curlen < 655350)
//	{
//		memcpy(tosavedata + curlen, pData, length);
//		curlen += length;
//	}
//#endif // _DEBUG



	int ret = 0;
	//线性插值，插成需要的波特率
	float interval = m_finterval;
	int totallen = length * interval / 2;
	if (totallen > m_AudioBuffsz)
	{
		totallen = m_AudioBuffsz;
	}
	else if (totallen <= 0)
	{
		g_wavecs.Unlock();
		return -1;
	}


	BYTE *psdata = (BYTE*)pData;

	//低通滤波
	short B[15] = {
		//37, -128, -525, -667, 687, 4080, 8012, 9779, 8012, 4080, 687, -667, -525, -128, 37
		//- 120, -188, -166, 428, 1992, 4311, 6459, 7337, 6459, 4311, 1992, 428, -166, -188, -120
		//250, 629, 1199, 1933, 2745, 3494, 4025, 4218, 4025, 3494, 2745, 1933, 1199, 629, 250
		//767, 1144, 1604, 2114, 2621, 3056, 3350, 3455, 3350, 3056, 2621, 2114, 1604, 1144, 767
	};
		short B2[9] = {
			-1891, -1793, 2972, 10213, 13766, 10213, 2972, -1793, -1891	//Gauss alpha = 0.8 wc = 0.4
	//		1303, 2816, 4231, 5235, 5598, 5235, 4231, 2816, 1303	//Gauss alpha = 0.1 wc = 0.2
	//		166, 962, 3629, 7187, 8879, 7187, 3629, 962, 166
		};
	short B3[7] = {
		2087, 4455, 6325, 7034, 6325, 4455, 2087 //Gauss alpha = 0.2 wc = 0.25
	};

	short B1[16] = {
		77, 199, 554, 1226, 2180, 3248, 4178, 4721, 4721,
		4178, 3248, 2180, 1226, 554, 199, 77
	};

	short B4[9] = {
		277,   2272,   4421,   6070,   6688,   6070,   4421,   2272,    277
	};

	short B5[7] = {	// LP Gauss alpha = 0.1 wc = 0.25
		2096,   4460,   6317,   11021,   6317,   4460,   2096
	};
	short B6[7] = {	// LP Gauss alpha = 0.1 wc = 0.25
		0,   1,   2,   4,   2,   1,   0
	};

	BYTE *paudiodata = new BYTE[length * 2];
	memcpy(paudiodata, pData, length * 2 * sizeof(char));
	//LPFitler(paudiodata, psdata, length * 2, B6, 7);
	delete[] paudiodata;

//#ifdef _DEBUG
//	if (length + curlen < 655350)
//	{
//		memcpy(tosavedata + curlen, psdata, length);
//		curlen += length;
//	}
//#endif // _DEBUG


	//线性插值

	m_pTmp[0] = lastData[0]/2+psdata[0]/2;
	m_pTmp[1] = lastData[1]/2+psdata[1]/2;
	int tmp1 = 0, tmp2 = 0;
	for (int i = 1; i < totallen; i++)
	{
		double fpos = i / interval;
		int npos = fpos;
		double alpha = fpos - npos;
		tmp1 = psdata[npos * 2] * (1 - alpha) + psdata[npos * 2 + 2] * alpha;
		tmp2 = psdata[npos * 2 + 1] * (1 - alpha) + psdata[npos * 2 + 3] * alpha;
		//if (tmp1>127)
		//{
		//	tmp1 = 127;
		//}
		//else if (tmp1<-127)
		//{
		//	tmp1 = -127;
		//}
		//if (tmp2 > 127)
		//{
		//	tmp2 = 127;
		//}
		//else if (tmp2 < -127)
		//{
		//	tmp2 = -127;
		//}
		m_pTmp[2 * i] = tmp1;
		m_pTmp[2 * i + 1] = tmp2;

		//m_pTmp[2 * i] = psdata[npos * 2] * (1 - alpha) + psdata[npos * 2 + 2] * alpha;
		//m_pTmp[2 * i + 1] = psdata[npos * 2 + 1] * (1 - alpha) + psdata[npos * 2 + 3] * alpha;

	}
	lastData[0] = psdata[length - 2];
	lastData[1] = psdata[length - 1];


	//for (int i = 0; i < totallen; i+=4)
	//{
	//	if (m_pTmp[2 * i]==0&&m_pTmp[2*i+1]==0 && 
	//		m_pTmp[2 * i + 2] == 0 && m_pTmp[2 * i + 3]==0)
	//	{
	//		TRACE1("Here %d\n",i);
	//	}

	//}
	

	m_Queue.Add((unsigned short*)m_pTmp, totallen);

#ifdef _DEBUG
	if (totallen * 2 + curlen < 655350)
	{
		memcpy(tosavedata + curlen, m_pTmp, totallen*2);
		curlen += totallen * 2;
	}
#endif // _DEBUG
	//m_Queue.Add((unsigned short*)pData, length / 2);
	g_wavecs.Unlock();
	
	return 0;
}

int CWaveOut::AddDatasToBuff(short *pData, DWORD length, WORD Flag)
{
	static short lastData[2] = { 0,0 };
	g_wavecs.Lock();
	clock_t tm = clock();
	long resttm = (tm - g_tm);

	if (resttm <= 0)
	{
		resttm = 1;
	}
	int prf = length / 2.0 / resttm * 1000;

	if (prf > 20 && g_tm != 0 && m_hdprf!=0)
	{
		if(prf<m_hdprf*0.7)
		{
			prf = m_hdprf*0.7;
		}else if(prf>m_hdprf*1.3)
		{
			prf = m_hdprf*1.3;
		}
		m_prf = prf-1;
		m_finterval = AUDIO_SAMPLERATE * 1.0 / m_prf;
		//HDC hdc = GetDC(0);
		//char tmp[256];
		//sprintf_s(tmp, "标称PRF：%d Hz  实际PRF: %d Hz",m_hdprf, m_prf);
		//TextOut(hdc, 10, 10, tmp, strlen(tmp)+1);
	}
	g_tm = tm;
	//TRACE1("接受PRF:%d\n", m_prf);
	////TRACE1("AddDatasToBuff TrheadID:%d\n", GetCurrentThreadId());
	//#ifdef _DEBUG
	//	if (length + curlen < 655350)
	//	{
	//		memcpy(tosavedata + curlen, pData, length*2);
	//		curlen += length*2;
	//	}
	//#endif // _DEBUG



	int ret = 0;
	//线性插值，插成需要的波特率
	double interval = m_finterval;
	int totallen = (length+1) * interval / 2;
	if (totallen > m_AudioBuffsz)
	{
		totallen = m_AudioBuffsz;
	}
	else if (totallen <= 0)
	{
		g_wavecs.Unlock();
		return -1;
	}


	short *psdata = (short*)pData;

	//低通滤波
	short B[15] = {
		37, -128, -525, -667, 687, 4080, 8012, 9779, 8012, 4080, 687, -667, -525, -128, 37
		//- 120, -188, -166, 428, 1992, 4311, 6459, 7337, 6459, 4311, 1992, 428, -166, -188, -120
		//250, 629, 1199, 1933, 2745, 3494, 4025, 4218, 4025, 3494, 2745, 1933, 1199, 629, 250
		//767, 1144, 1604, 2114, 2621, 3056, 3350, 3455, 3350, 3056, 2621, 2114, 1604, 1144, 767
	};
	short B2[9] = {
		-1891, -1793, 2972, 10213, 13766, 10213, 2972, -1793, -1891	//Gauss alpha = 0.8 wc = 0.4
																	//		1303, 2816, 4231, 5235, 5598, 5235, 4231, 2816, 1303	//Gauss alpha = 0.1 wc = 0.2
																	//		166, 962, 3629, 7187, 8879, 7187, 3629, 962, 166
	};
	short B3[7] = {
		2087, 4455, 6325, 7034, 6325, 4455, 2087 //Gauss alpha = 0.2 wc = 0.25
	};

	short B1[16] = {
		77, 199, 554, 1226, 2180, 3248, 4178, 4721, 4721,
		4178, 3248, 2180, 1226, 554, 199, 77
	};

	short B4[9] = {
		277,   2272,   4421,   6070,   6688,   6070,   4421,   2272,    277
	};

	short B5[7] = {	// LP Gauss alpha = 0.1 wc = 0.25
		2096,   4460,   6317,   11021,   6317,   4460,   2096
	};
	short B6[9] = {	// LP Gauss alpha = 0.1 wc = 0.25
		0,   0,   0,   0,   1,   0,   0,	0,	0
	};
	short B7[9] = { // LP Gauss alpha = 0.8 wc = 0.8
		-1117,   2782,  -4624,   6079,  26529,   6079,  -4624,   2782,  -1117
	};
	short B8[9] = { // LP Gauss alpha = 0.6 wc = 0.6
		2004,  -1787,  -2835,   9491,  19023,   9491,  -2835,  -1787,   2004
	};

	short B9[3] = { // LP Gauss alpha = 0.6 wc = 0.6
		1,   1,   1
	};
	short *paudiodata = new short[(length+6) * 2];
	
	memcpy(paudiodata, psdata, (length) * 2 * sizeof(short));
	LPFitler(paudiodata, psdata,  length * 2, B8, 9);
	//FILE *fp1 = NULL; 
	//FILE *fp2 = NULL; 
	//fopen_s(&fp1, "d:\\l.txt", "w");
	//fopen_s(&fp2, "d:\\2.txt", "w");
	//for (int i = 0; i < length * 2;i++)
	//{
	//	fprintf(fp1, "%d ", paudiodata[i]);
	//	fprintf(fp2, "%d ", psdata[i]);
	//}
	//fprintf(fp1, "\n=====================================\n");
	//fprintf(fp2, "\n=====================================\n");
	//fclose(fp1);
	//fclose(fp2);
	paudiodata[0] = lastData[0];
	paudiodata[1] = lastData[1];
	memcpy(paudiodata + 2, psdata, length * 2 * sizeof(short));

	//#ifdef _DEBUG
	//	if (length + curlen < 655350)
	//	{
	//		memcpy(tosavedata + curlen, psdata, length*2);
	//		curlen += length*2;
	//	}
	//#endif // _DEBUG


	//线性插值


	int tmp1 = 0, tmp2 = 0;
	double fpos = 0,alpha = 0;
	int npos = 0;
	for (int i = 0; i < totallen; i++)
	{
		fpos = i / interval;
		npos = fpos;
		alpha = fpos - npos;
		tmp1 = paudiodata[npos * 2] * (1 - alpha) + paudiodata[npos * 2 + 2] * alpha;
		tmp2 = paudiodata[npos * 2 + 1] * (1 - alpha) + paudiodata[npos * 2 + 3] * alpha;

		m_pTmp_s[2 * i] = tmp1;
		m_pTmp_s[2 * i + 1] = tmp2;


	}
	lastData[0] = psdata[length - 2];
	lastData[1] = psdata[length - 1];

	delete[] paudiodata;

	//for (int i = 0; i < totallen; i+=4)
	//{
	//	if (m_pTmp[2 * i]==0&&m_pTmp[2*i+1]==0 && 
	//		m_pTmp[2 * i + 2] == 0 && m_pTmp[2 * i + 3]==0)
	//	{
	//		TRACE1("Here %d\n",i);
	//	}

	//}
	for (int j = 0; j < 10;j++)
	{
		for (int i = 0; i < totallen - 1; i++)
		{
			m_pTmp_s[2 * i] = (m_pTmp_s[2 * i] + m_pTmp_s[2 * (i + 1)]) / 2;
			m_pTmp_s[2 * i + 1] = (m_pTmp_s[2 * i + 1] + m_pTmp_s[2 * (i + 1) + 1]) / 2;
		}
		for (int i = totallen - 1; i > 0; i--)
		{
			m_pTmp_s[2 * i] = (m_pTmp_s[2 * i] + m_pTmp_s[2 * (i - 1)]) / 2;
			m_pTmp_s[2 * i + 1] = (m_pTmp_s[2 * i + 1] + m_pTmp_s[2 * (i - 1) + 1]) / 2;
		}
	}
	m_Queue_s.Add((unsigned int*)m_pTmp_s, totallen);
	//TRACE1("TOTALEN:%d\n", totallen);

#ifdef _DEBUG
	if (totallen * 4 + curlen < 655350)
	{
		memcpy(tosavedata + curlen, m_pTmp_s, totallen * 4);
		curlen += totallen * 4;
	}
#endif // _DEBUG
	//m_Queue.Add((unsigned short*)pData, length / 2);
	g_wavecs.Unlock();

	return 0;
}

int CWaveOut::GetVolume()
{
	g_wavecs.Lock();
	unsigned int  vol = 0;
	waveOutGetVolume(0, (LPDWORD)&vol);
	vol = 1.0 * vol * m_MaxVolume / MAXIMUM_VOLUME;
	g_wavecs.Unlock();
	return vol;
}

int CWaveOut::SetVolume(unsigned int vol)
{
	g_wavecs.Lock();
	if (vol>m_MaxVolume)
	{
		vol = m_MaxVolume;
	}
	int ret = waveOutSetVolume(0, 1.0* MAXIMUM_VOLUME / m_MaxVolume * vol);
	if (ret == MMSYSERR_NOERROR)
	{
		g_wavecs.Unlock();
		return 0;
	}
	else
	{
		g_wavecs.Unlock();
		return ret;
	}
}

void CWaveOut::UnPrepare()
{
	while (waveOutUnprepareHeader(m_hwo, &m_wh1, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		Sleep(100);//清理数据
	while (waveOutUnprepareHeader(m_hwo, &m_wh2, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		Sleep(100);//清理数据
}

void CWaveOut::Play()
{
	if (m_hwo)
	{
		waveOutRestart(m_hwo);
	}
}

void CWaveOut::Pause()
{
	if (m_hwo)
	{
		waveOutPause(m_hwo);
	}
}

int CWaveOut::LPFitler(BYTE *src, BYTE *dest, int len, short *filter, int order)
{
	int ret = 0;
	if (len<order)
	{
		memcpy(dest, src, sizeof(unsigned short)*len);
		return -1;
	}
	int horder = order >> 1;
	int tmp = order & 1;//order%2;取余


	double sum1;
	double sum2;

	int m;
	int n;

	int filtersum = 0;
	for (int i = 0; i < order;i++)
	{
		filtersum += filter[i];
	}
	if (filtersum==0)
	{
		filtersum = 1;
	}

	m = 0;
	n = 0;

	if (tmp == 0)//偶数
	{
		if (len > order)//filter长度小于src长度
		{
			for (int i = 0; i < len; i += 2)
			{
				sum1 = 0;
				sum2 = 0;
				n = i - 2 * horder;
				for (int j = 0; j < order; j++)
				{
					m = n + 2 * j;
					if ((m >= 0) && (m < len - horder))
					{
						sum1 += (src[m] + src[m + 2])*filter[j];// /2;
						sum2 += ((src[m + 1] + src[m + 3])*filter[j]);// /2;
					}
					else if ((m >= len - horder) && m < len)//超出数组范围的数置0
					{
						sum1 += src[m] * filter[j];// /2;
						sum2 += src[m + 1] * filter[j];// /2;
					}
					
				}
				dest[i] = sum1 / filtersum;
				dest[i + 1] = sum2 / filtersum;
			}

		}
	}
	else//奇数
	{

		if (len > order)//filter长度小于src长度
		{
			for (int i = 0; i < len; i += 2)
			{
				sum1 = 0;
				sum2 = 0;
				n = i - 2 * horder;
				filtersum = 0;
				for (int j = 0; j < order; j++)
				{
					int m = n + 2 * j;//i-2*horder+2*j;//数组src中与filter[j]进行卷积的点
					if (m>0 && m < len)
					{
						sum1 += src[m] * filter[j];
						sum2 += src[m + 1] * filter[j];
						filtersum += filter[j];
					}
					
				}
				if (filtersum==0)
				{
					dest[i] = src[m];
					dest[i + 1] = src[m + 1];
				}
				else
				{
					dest[i] = sum1 / filtersum;
					dest[i + 1] = sum2 / filtersum;
				}
			}

		}

	}
	return ret;

}

int CWaveOut::LPFitler(short *src, short *dest, int len, short *filter, int order)
{
	int ret = 0;
	if (len < order)
	{
		memcpy(dest, src, sizeof(unsigned short)*len);
		return -1;
	}
	int horder = order >> 1;
	int tmp = order & 1;//order%2;取余


	double sum1;
	double sum2;

	int m;
	int n;

	int filtersum = 0;
	for (int i = 0; i < order; i++)
	{
		filtersum += filter[i];
	}
	if (filtersum == 0)
	{
		filtersum = 1;
	}

	m = 0;
	n = 0;

	if (tmp == 0)//偶数
	{
		if (len > order)//filter长度小于src长度
		{
			for (int i = 0; i < len; i += 2)
			{
				sum1 = 0;
				sum2 = 0;
				n = i - 2 * horder;
				for (int j = 0; j < order; j++)
				{
					m = n + 2 * j;
					if ((m >= 0) && (m < len - horder))
					{
						sum1 += (src[m] + src[m + 2])*filter[j];// /2;
						sum2 += ((src[m + 1] + src[m + 3])*filter[j]);// /2;
					}
					else if ((m >= len - horder) && m < len)//超出数组范围的数置0
					{
						sum1 += src[m] * filter[j];// /2;
						sum2 += src[m + 1] * filter[j];// /2;
					}

				}
				dest[i] = sum1 / filtersum;
				dest[i + 1] = sum2 / filtersum;
			}

		}
	}
	else//奇数
	{

		if (len > order)//filter长度小于src长度
		{
			for (int i = 0; i < len; i += 2)
			{
				sum1 = 0;
				sum2 = 0;
				n = i - 2 * horder;
				filtersum = 0;
				for (int j = 0; j < order; j++)
				{
					int m = n + 2 * j;//i-2*horder+2*j;//数组src中与filter[j]进行卷积的点
					if (m >= 0 && m < len)
					{
						sum1 += src[m] * filter[j];
						sum2 += src[m + 1] * filter[j];
						filtersum += filter[j];
					}


				}
				if (filtersum == 0)
				{
					dest[i] = src[m];
					dest[i + 1] = src[m + 1];
				}
				else
				{
					dest[i] = sum1 / filtersum;
					dest[i + 1] = sum2 / filtersum;
				}
			}

		}

	}
	return ret;

}
/*
short lastfilterdata[64]={0};
int CWaveOut::LPFitler(short *src, short *dest, int len, short *filter, int order)
{
	int ret = 0;
	if (len < order)
	{
		memcpy(dest, src, sizeof(unsigned short)*len);
		return -1;
	}
	int horder = order >> 1;
	int tmp = order & 1;//order%2;取余


	int sum1;
	int sum2;

	int m;
	int n;

	int filtersum = 0;
	for (int i = 0; i < order; i++)
	{
		filtersum += filter[i];
	}
	if (filtersum == 0)
	{
		filtersum = 1;
	}

	m = 0;
	n = 0;

	short *pPredata = new short[len + order*2];
	memcpy(pPredata, lastfilterdata, (order - 1) * 2*sizeof(short));
	memcpy(pPredata + (order - 1) * 2, src, len*sizeof(short));

	if (tmp == 0)//偶数
	{
		if (len > order)//filter长度小于src长度
		{
			for (int i = 0; i < len; i += 2)
			{
				sum1 = 0;
				sum2 = 0;
				filtersum = 0;
				for (int j = 0; j < order; j++)
				{
					
						sum1 += pPredata[i + j*2] * filter[j];// /2;
						sum2 += pPredata[i + j*2 + 1] * filter[j];// /2;
						filtersum += filter[j];
				}
				dest[i] = sum1 / filtersum;
				dest[i + 1] = sum2 / filtersum;
			}

		}
	}
	else//奇数
	{

		if (len > order)//filter长度小于src长度
		{
			for (int i = 0; i < len; i +=2)
			{
				sum1 = 0;
				sum2 = 0;
				for (int j = 0; j < order; j++)
				{
					sum1 += pPredata[i + j*2] * (int)filter[j];// /2;
					sum2 += pPredata[i + j*2 + 1] * (int)filter[j];// /2;
				}
				dest[i] = sum1 / filtersum;
				dest[i + 1] = sum2 / filtersum;

			}

		}

	}
	memcpy(lastfilterdata, src+len- (order - 1) * 2, (order - 1) * 2 * sizeof(short));
	delete[] pPredata;
	pPredata = NULL;
	return ret;

}
*/