#pragma once
class CAudioProcess
{
public:
	CAudioProcess();
	~CAudioProcess();
	int MakeAudioCH(unsigned int* pIQ, int len, unsigned int* pAudio);
	int MakeAudioCH(double* pIQ, int len, unsigned short* pAudio, int isInvert = 0);
	int MakeAudioCH(double* pIQ, int len, unsigned int* pAudio, int isInvert = 0);
	static int WriteWave(unsigned short* pAudio, int len, int prf, char *file);
	static int WriteWave(unsigned char* pAudio, int len, int prf, char *file);
private:
	double m_H_Phased[61];
	int MakePhasedCo();
};

