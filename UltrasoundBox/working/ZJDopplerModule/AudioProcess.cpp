#include "stdafx.h"
#include "AudioProcess.h"
#include <math.h>

//wav头的结构如下所示：
typedef   struct
{
	char     fccID[4];
	unsigned   long      dwSize;
	char     fccType[4];
}HEADER; //RIFF WAVE Chunk
typedef   struct
{
	char    fccID[4];
	unsigned   long		dwSize;
	unsigned   short    wFormatTag;
	unsigned   short    wChannels;
	unsigned   long     dwSamplesPerSec;
	unsigned   long     dwAvgBytesPerSec;
	unsigned   short    wBlockAlign;
	unsigned   short    uiBitsPerSample;
}FMT; //Format Chunk

typedef   struct
{
	char    fccID[4];
	unsigned   long     dwSize;
}DATA; //Data Chunk
//以上是wav头文件


CAudioProcess::CAudioProcess()
{
	MakePhasedCo();
}


CAudioProcess::~CAudioProcess()
{
}

int CAudioProcess::MakePhasedCo()
{
	int ret = 0;
	for (int i = 0; i < 31;i++)
	{
		if (i==15)
		{
			m_H_Phased[i] = 0;
			continue;
		}
		int k = i - 15;
		m_H_Phased[i] = 2.0 / (k*M_PI)*(sin((k*M_PI) / 2)*sin((k*M_PI) / 2));
	}
	return 0;
}

int CAudioProcess::MakeAudioCH(unsigned int* pIQ, int len, unsigned int* pAudio)
{
	if (!pIQ || !pAudio || len<15)
	{
		return -1;
	}
	memset(pAudio, 0, sizeof(unsigned int)*len);
	short *psIQ = (short*)pIQ;
	short *psAudio = (short*)pAudio;
	for (int i = 15; i < len - 15;i++)
	{
		short x = psIQ[i * 2];
		short y = psIQ[i * 2 + 1];
		double phased_y = 0;
		for (int j = 0; j < 31; j++)
		{
			phased_y += (psIQ[(i - 15 + j) * 2 + 1] * m_H_Phased[j]);
		}
		psAudio[i * 2] = (x - phased_y) / 2;
		psAudio[i * 2 + 1] = (x + phased_y) / 2;
	}
	return 0;
}

double lastIQ[64];
int persavedlen = 60;

int CAudioProcess::MakeAudioCH(double* pIQ, int len, unsigned short* pAudio, int isInvert)
{
	if (!pIQ || !pAudio || len < 62)
	{
		return -1;
	}
	memset(pAudio, 0, sizeof(unsigned short)*len);
	double *psIQ =  new double[len*2+64];
	memcpy(psIQ, lastIQ, persavedlen * sizeof(double));
	memcpy(psIQ+ persavedlen, pIQ, len*2 * sizeof(double));
	memcpy(lastIQ, pIQ + len*2 - persavedlen, persavedlen * sizeof(double));
	//unsigned char *psAudio = (unsigned char *)pAudio;

	BYTE *psAudio = new BYTE[(len+5) * 2];
	memset(psAudio, 0, sizeof(unsigned char)*(len+5) * 2);
	static int nmax = 0;
#define AudioShift 1
#pragma loop(hint_parallel(CPUCORE))
	for (int i = 15; i < len+15; i++)
	{
		double x = psIQ[i * 2];
		double y = psIQ[i * 2 + 1];

		//psAudio[i * 2] = x;
		//psAudio[i * 2 + 1] = y;

		double phased_y = 0;
		for (int j = 0; j < 31; j++)
		{
			phased_y += (psIQ[(i - 15 + j) * 2 + 1] * m_H_Phased[j]);
		}
		//if (abs(x-phased_y)>nmax)
		//{
		//	nmax = abs(x - phased_y);
		//}
		//if (abs(x + phased_y) > nmax)
		//{
		//	nmax = abs(x + phased_y);
		//}
		int tmp1 = (x - phased_y) / AudioShift;
		int tmp2 = (x + phased_y) / AudioShift;
		//if (tmp1 > 127)
		//	tmp1 = 127;
		//else if (tmp1 < -127)
		//	tmp1 = -127;
		//if (tmp2 > 127)
		//	tmp2 = 127;
		//else if (tmp2 < -127)
		//	tmp2 = -127;
		if (tmp1 > 255)
		{
			tmp1 = 255;
		}
		else if (tmp1 < 0)
		{
			tmp1 = 0;
		}
		if (tmp2 > 255)
		{
			tmp2 = 255;
		}
		else if (tmp2 < 0)
		{
			tmp2 = 0;
		}
		if (isInvert)
		{
			psAudio[(i - 15) * 2] = tmp1;
			psAudio[(i - 15) * 2 + 1] = tmp2;
		}
		else
		{
			psAudio[(i - 15) * 2 + 1] = tmp1;
			psAudio[(i - 15) * 2] = tmp2;
		}
	}
	memcpy(pAudio, psAudio, len*sizeof(unsigned short));
	delete[] psAudio;
	delete[] psIQ;

	return 0;



	//做平均
	unsigned char *pAudioOut = (unsigned char *)pAudio;
	int fltlen = 1;

#pragma loop(hint_parallel(CPUCORE))
	for (int i = 1; i < len+2; i++)
	{
		int sum0=0, sum1=0;
		for (int j = -fltlen; j <= fltlen; j++)
		{
			sum0 += psAudio[(i + j) * 2];
			sum1 += psAudio[(i + j) * 2+1];
		}

		pAudioOut[(i - 1) * 2 + 1] = sum1 / (2 * fltlen + 1);
		pAudioOut[(i - 1) * 2] = sum0 / (2 * fltlen + 1);
		
	}

	delete[] psAudio;
	delete[] psIQ;
	//for (int i = 15; i < len - 15; i++)
	//{
	//	
	//	psAudio[i * 2] = (x - phased_y) / 2;
	//	psAudio[i * 2 + 1] = (x + phased_y) / 2;
	//}




	return 0;
}

int CAudioProcess::MakeAudioCH(double* pIQ, int len, unsigned int* pAudio, int isInvert)
{
	if (!pIQ || !pAudio || len < 62)
	{
		return -1;
	}
	memset(pAudio, 0, sizeof(unsigned short)*len);
	double *psIQ = new double[len + 64];
	memcpy(psIQ, lastIQ, persavedlen * sizeof(double));
	memcpy(psIQ + persavedlen, pIQ, len * sizeof(double));
	memcpy(lastIQ, pIQ + len - persavedlen, persavedlen * sizeof(double));

	short *psAudio = new short[len + 60];
	memset(psAudio, 0, sizeof(short)*(len + 60));

#define AudioShift 1*256
#pragma loop(hint_parallel(CPUCORE))
	for (int i = 15; i < len/2 + 15; i++)
	{
		double x = psIQ[i * 2];
		double y = psIQ[i * 2 + 1];

		//psAudio[i * 2] = x;
		//psAudio[i * 2 + 1] = y;

		double phased_y = 0;
		for (int j = 0; j < 31; j++)
		{
			phased_y += (psIQ[(i - 15 + j) * 2 + 1] * m_H_Phased[j]);
		}

		short tmp1 = (x - phased_y) / AudioShift;
		short tmp2 = (x + phased_y) / AudioShift;

		if (isInvert)
		{
			psAudio[(i - 15) * 2] = tmp1;
			psAudio[(i - 15) * 2 + 1] = tmp2;
		}
		else
		{
			psAudio[(i - 15) * 2 + 1] = tmp1;
			psAudio[(i - 15) * 2] = tmp2;
		}
	}
	memcpy(pAudio, psAudio, len*sizeof(unsigned short)+60);
	//for (int i = 0; i < len; i++)
	//{
	//	TRACE3("%d: %f %d\n", i, pIQ[i], psAudio[i]);
	//}
	delete[] psAudio;
	delete[] psIQ;

	
	return 0;



	//做平均
	unsigned char *pAudioOut = (unsigned char *)pAudio;
	int fltlen = 1;

#pragma loop(hint_parallel(CPUCORE))
	for (int i = 1; i < len + 2; i++)
	{
		int sum0 = 0, sum1 = 0;
		for (int j = -fltlen; j <= fltlen; j++)
		{
			sum0 += psAudio[(i + j) * 2];
			sum1 += psAudio[(i + j) * 2 + 1];
		}

		pAudioOut[(i - 1) * 2 + 1] = sum1 / (2 * fltlen + 1);
		pAudioOut[(i - 1) * 2] = sum0 / (2 * fltlen + 1);

	}

	delete[] psAudio;
	delete[] psIQ;
	//for (int i = 15; i < len - 15; i++)
	//{
	//	
	//	psAudio[i * 2] = (x - phased_y) / 2;
	//	psAudio[i * 2 + 1] = (x + phased_y) / 2;
	//}




	return 0;
}

int CAudioProcess::WriteWave(unsigned short* pAudio, int len, int prf, char *file)
{
	if (!file || prf<1 || !pAudio || len < 15)
	{
		return -1;
	}
	HEADER  pcmHEADER;
	FMT  pcmFMT;
	DATA pcmDATA;

	memcpy(pcmHEADER.fccID, "RIFF",4);
	pcmHEADER.dwSize = len*sizeof(unsigned short) + 44 - 8;
	memcpy(pcmHEADER.fccType, "WAVE", 4);

	memcpy(pcmFMT.fccID, "fmt ", 4);
	pcmFMT.dwSize = 0x10;
	pcmFMT.wFormatTag = 0x01;
	pcmFMT.wChannels = 0x02;
	pcmFMT.uiBitsPerSample = 0x10;
	pcmFMT.dwSamplesPerSec = prf;
	pcmFMT.dwAvgBytesPerSec = prf * pcmFMT.wChannels *pcmFMT.uiBitsPerSample / 8;
	pcmFMT.wBlockAlign = pcmFMT.uiBitsPerSample*pcmFMT.wChannels / 8;

	memcpy(pcmDATA.fccID, "data", 4);
	pcmDATA.dwSize = len*sizeof(unsigned short)+44;

	FILE *fp = NULL;
	fopen_s(&fp,file, "wb");
	if (fp)
	{
		fwrite(&pcmHEADER, sizeof(HEADER), 1, fp);
		fwrite(&pcmFMT, sizeof(FMT), 1, fp);
		fwrite(&pcmDATA, sizeof(DATA), 1, fp);
		fwrite(pAudio, len*sizeof(unsigned short), 1, fp);
		fclose(fp);
		return 0;
	}
	return -1;

}

int CAudioProcess::WriteWave(unsigned char* pAudio, int len, int prf, char *file)
{
	if (!file || prf < 1 || !pAudio || len < 15)
	{
		return -1;
	}
	HEADER  pcmHEADER;
	FMT  pcmFMT;
	DATA pcmDATA;

	memcpy(pcmHEADER.fccID, "RIFF", 4);
	pcmHEADER.dwSize = len*sizeof(unsigned char) + 44 - 8;
	memcpy(pcmHEADER.fccType, "WAVE", 4);

	memcpy(pcmFMT.fccID, "fmt ", 4);
	pcmFMT.dwSize = 0x10;
	pcmFMT.wFormatTag = 0x01;
	pcmFMT.wChannels = 0x02;
	pcmFMT.uiBitsPerSample = 0x08;
	pcmFMT.dwSamplesPerSec = prf;
	pcmFMT.dwAvgBytesPerSec = prf * pcmFMT.wChannels * pcmFMT.uiBitsPerSample / 8;
	pcmFMT.wBlockAlign = pcmFMT.uiBitsPerSample * pcmFMT.wChannels / 8;

	memcpy(pcmDATA.fccID, "data", 4);
	pcmDATA.dwSize = len*sizeof(unsigned char) + 44;

	FILE *fp = NULL;
	fopen_s(&fp, file, "wb");
	if (fp)
	{
		fwrite(&pcmHEADER, sizeof(HEADER), 1, fp);
		fwrite(&pcmFMT, sizeof(FMT), 1, fp);
		fwrite(&pcmDATA, sizeof(DATA), 1, fp);
		fwrite(pAudio, len*sizeof(unsigned char), 1, fp);
		fclose(fp);
		return 0;
	}
	return -1;

}

