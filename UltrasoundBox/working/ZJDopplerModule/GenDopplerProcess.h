#pragma once
#include "ZJIF_Doppler.h"
#include "WindowedSignal.h"
#include "AudioProcess.h"
#include "WaveOut.h"
#include "FFTW/fftw3.h"
class CGenDopplerProcess :
	public ZJIF_Doppler
{
public:
	CGenDopplerProcess();
	virtual ~CGenDopplerProcess();
	virtual int DInit();
	virtual int DLoadParas(DParas *pparas);
	virtual int DLoadParasFromFile(char *file);
	virtual int DSaveParasToFile(char *file);
	//virtual int LoadIQDataFromFile(char *file);
	virtual int DopplerProcess(ComplexIQ *pIQData, int nLen, unsigned char *pImageData);
	virtual int DopplerProcessByWord(ComplexIQ *pIQData, int nLen, unsigned char *pImageData);
	virtual void DClearData();
	virtual int DSetPara(EDopperParas dpara, ...);
	virtual int DGetPara(EDopperParas dpara, ...);
	virtual int DSetPara2(EDopperParas dpara, va_list argls);
	virtual int DGetPara2(EDopperParas dpara, va_list argls);

	virtual int DReset();
	virtual void DEnd();

protected:
	int m_winlastlen;
	int m_winlastwinid;
	DParas m_dParas;
	int m_LinePos;
	SIZE m_imgsz;
	_complex *m_pComplexIQ;
	CWindowedSignal m_WinSignal;
	CAudioProcess m_AudioProcess;
	CWaveOut m_WaveOut;
	unsigned int *m_pAudioData;
	_complex *m_pIQin;
	_complex *m_pIQout;
	int m_N;
	int m_CalN;
	fftw_plan m_fftPlan;
	unsigned char *m_pLogTab;
	int m_LogTabLen;
	int m_nCurImageLine;
	int m_CalSetN[4];
	BYTE *m_pLastImgData;
	typedef struct 
	{
		unsigned char data[256];
	}DLine;

	CArrayQueue<DLine> m_ImgQueue;

	int m_nWinID;
	int DopplerImage(_complex *pIQData_in, int len, unsigned char *pImageData);
	int DopplerAudio(_complex *pIQData_in, int len);

	int Converts2d(ComplexIQ *pIn, _complex *pOut, int len);
	int WindowedSignal(_complex *pIQData_in, int len, _complex *pIQData_Out, int nWinID = 0);
	//int GaussBlur(unsigned char *pData_in, int len, unsigned char *pData_out);

	int Trace(unsigned short *pData_int, int len, int *pMaxPos, int *pMeanPos);
	int InvertAndShiftBaseLine(unsigned char* pDatainOut, int w, int h);
	int Transpose(unsigned char* pDatain, int w, int h, unsigned char* pDataout);
//	int Transpose(unsigned char* pDatainOut, int w, int h);
	int DopplerImageLine(_complex *pIQData_in, int len, unsigned char *pImageData, int *pmaxpos, int *pmeanpos);
	int LoadLogTab(unsigned char *tab, int len, char *path);
	int LoadLogTabByID(unsigned char *tab, int len, int id);
	int BlackHoleFilter(unsigned short* pin, unsigned short *pout, int len, int theta_black, int theta_white, int isBigsize);
	
};

