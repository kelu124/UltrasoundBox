#pragma once
#include <mmsystem.h>
#include "ArrayQueue.h"
class CWaveOut
{
public:
	CWaveOut();
	~CWaveOut();
	//void PlayDemo();
	int CreateWave(int BuffTime, int Channel, int prf, int BitsPerSample, int maxVolume=255);
	int SetPRF(int prf);
	int GetPRF( ){ return m_prf; }
	void Clear();
	void UnPrepare();
	int AddDatasToBuff(BYTE *pData, DWORD length, WORD Flag);
	int AddDatasToBuff(short *pData, DWORD length, WORD Flag);
	int GetVolume();
	int SetVolume(unsigned int vol);
	void Play();
	void Pause();
	CArrayQueue<unsigned short> m_Queue;
	CArrayQueue<unsigned int> m_Queue_s;
	static int LPFitler(BYTE *pDatain, BYTE *PDataout, int len, short *filter, int filterlen);
	static int LPFitler(short *src, short *dest, int len, short *filter, int order);
private:
	int m_BuffTime;
	int m_nChannel;
	int m_prf;
	int m_hdprf;
	int m_BitsPerSample;
	WAVEHDR         m_wh1;
	WAVEHDR         m_wh2;
	WAVEFORMATEX    m_wfx;
	HWAVEOUT        m_hwo;
	char *m_playBuff[2];
	int m_playbufsz;
	int m_AudioBuffsz;
	int m_MaxVolume;
	double m_finterval;
	char *m_pTmp;
	short *m_pTmp_s;
};

