#include "stdafx.h"
#include "GenDopplerProcess.h"
#include "WallFilterFIR.h"
#include "WindowedSignal.h"
#include <math.h>
#include <stdarg.h>

#pragma comment(lib, "FFTW\\libfftw3-3.lib" )
#pragma comment(lib, "FFTW\\libfftw3f-3.lib" )
#pragma comment(lib, "FFTW\\libfftw3l-3.lib" )

#define MAXLEN 655360

CGenDopplerProcess::CGenDopplerProcess()
{
	memset(&m_dParas, 0, sizeof(m_dParas));
	m_pComplexIQ = new _complex[MAXLEN];
	memset(m_pComplexIQ, 0, MAXLEN*sizeof(_complex));
	m_pAudioData = new unsigned int[MAXLEN];
	memset(m_pAudioData, 0x80, MAXLEN*sizeof(unsigned int));
	m_N = 256;
	m_pIQin = new _complex[m_N*4];
	memset(m_pIQin, 0, m_N*4*sizeof(_complex));
	m_pIQout = new _complex[m_N*4];
	memset(m_pIQout, 0, m_N*4*sizeof(_complex));
	m_nWinID = 0;
	m_fftPlan = NULL;
	m_CalN = 128;
	m_winlastlen = -1;
	m_winlastwinid = 0;
	
	m_nCurImageLine = 0;

	m_CalSetN[0] = 254;
	m_CalSetN[1] = 192;
	m_CalSetN[2] = 168;
	m_CalSetN[3] = 128;

	m_CalSetN[0] = 192;
	m_CalSetN[1] = 128;
	m_CalSetN[2] = 96;
	m_CalSetN[3] = 64;

	m_pLastImgData = NULL;

//	m_ImgQueue.Create(2000, 768, 0);
	m_ImgQueue.Create(768, 768, 0);

	//m_LogTabLen = 4096;
	//m_pLogTab = new unsigned char[m_LogTabLen];
	////for (int i = 0; i < m_LogTabLen;i++)
	////{
	////	m_pLogTab[i] = 240 * log(i / 350.0 + 1);
	////	if (m_pLogTab[i]<=0)
	////	{
	////		m_pLogTab[i] = 1;
	////	}
	////	else if (m_pLogTab[i] > 250)
	////	{
	////		m_pLogTab[i] = 250;
	////	}
	////}
	//memset(m_pLogTab, 1, m_LogTabLen);
	//for (int i = 40; i < m_LogTabLen; i++)
	//{
	//	//m_pLogTab[i] = 450 * log((i >> 2) / 300.0 + 1);
	//	int tmp = 280 * log(((i - 40) >> 2) / 200.0 + 1);
	//	
	//	if (tmp <= 0)
	//	{
	//		m_pLogTab[i] = 1;
	//	}
	//	else if (tmp > 250)
	//	{
	//		m_pLogTab[i] = 250;
	//	}
	//	else
	//		m_pLogTab[i] = tmp;
	//}

	m_LogTabLen = 65536;
	m_pLogTab = new unsigned char[m_LogTabLen];
	//for (int i = 0; i < m_LogTabLen;i++)
	//{
	//	m_pLogTab[i] = 240 * log(i / 350.0 + 1);
	//	if (m_pLogTab[i]<=0)
	//	{
	//		m_pLogTab[i] = 1;
	//	}
	//	else if (m_pLogTab[i] > 250)
	//	{
	//		m_pLogTab[i] = 250;
	//	}
	//}
	memset(m_pLogTab, 1, m_LogTabLen);
	for (int i = 50; i < m_LogTabLen; i++)
	{
		//m_pLogTab[i] = 450 * log((i >> 2) / 300.0 + 1);
		int tmp = 420 * log((i-50) / 2000.0 + 1);

		if (tmp <= 0)
		{
			m_pLogTab[i] = 1;
		}
		else if (tmp > 250)
		{
			m_pLogTab[i] = 250;
		}
		else
			m_pLogTab[i] = tmp;
	}
	//LoadLogTab(m_pLogTab, 65535, ".\\log1.dat");
	LoadLogTabByID(m_pLogTab, 65535, m_dParas.dyn_id);
	m_WaveOut.CreateWave(1000, 2, 4000, 16);//为适应DLLFrame添加，因为她没调DInit
}


CGenDopplerProcess::~CGenDopplerProcess()
{
	if (m_pComplexIQ)
	{
		delete[] m_pComplexIQ;
		m_pComplexIQ = NULL;
	}
	if (m_pAudioData)
	{
		delete[] m_pAudioData;
		m_pAudioData = NULL;
	}
	if (m_pIQin)
	{
		delete[] m_pIQin;
		m_pIQin = NULL;
	}
	if (m_pIQout)
	{
		delete[] m_pIQout;
		m_pIQout = NULL;
	}
	if (m_fftPlan)
	{
		fftw_destroy_plan(m_fftPlan);
		m_fftPlan = NULL;
	}
	if(m_pLogTab)
	{
		delete[] m_pLogTab;
		m_pLogTab = NULL;
	}
	if (m_pLastImgData)
	{
		delete[] m_pLastImgData;
		m_pLastImgData = NULL;
	}
}

int CGenDopplerProcess::DInit()
{
	m_nCurImageLine = 0;
	m_ImgQueue.Reset();
	//老徐居然没调这个函数，只有移植到构造函数中去了
	//m_WaveOut.CreateWave(1000, 2, 4000, 8);
	return -1;
}

int CGenDopplerProcess::DLoadParas(DParas *pparas)
{
	if (!pparas)
	{
		return -1;

	}
	int ret = 0;
	static int s_img_w = -1;
	static int s_img_h = -1;
	static int s_audiovolume = -1;
	memcpy(&m_dParas, pparas, sizeof(m_dParas));
	m_CalN = m_CalSetN[m_dParas.resolution];
	//ret = m_WaveOut.CreateWave(1000, 2, m_dParas.prf, 8);
	ret = m_WaveOut.SetPRF(m_dParas.prf);
	if (ret < 0)
	{
		ret = -2;
	}
	//m_WaveOut.Pause();
	if (s_audiovolume != m_dParas.audiovolume)
	{
		ret = m_WaveOut.SetVolume(m_dParas.audiovolume);
		if (ret < 0)
		{
			ret = -3;
		}
		s_audiovolume != m_dParas.audiovolume;
	}

	m_nCurImageLine = 0;
	if (s_img_w != m_dParas.img_w || s_img_h != m_dParas.img_h ||!m_pLastImgData)
	{

		ret = m_ImgQueue.Create(m_dParas.img_w, m_dParas.img_w, 0);
		if (ret < 0)
		{
			ret = -4;
		}
		if (m_pLastImgData)
		{
			delete[] m_pLastImgData;
			m_pLastImgData = NULL;
		}
		m_pLastImgData = new BYTE[m_dParas.img_w*m_dParas.img_h];
		s_img_w = m_dParas.img_w;
		s_img_h = m_dParas.img_h;
	}
	m_ImgQueue.Reset();
	return ret;
}

int CGenDopplerProcess::DLoadParasFromFile(char *file)
{
	return -1;
}

int CGenDopplerProcess::DSaveParasToFile(char *file)
{
	return -1;
}

int CGenDopplerProcess::DopplerProcess(ComplexIQ *pIQData, int nLen, unsigned char *pImageData)
{
	if (!pIQData || !pImageData)
	{
		if (m_pLastImgData && pImageData)
		{
			memcpy(pImageData, m_pLastImgData, m_dParas.img_w*m_dParas.img_h);
		}
		return -1;
	}
	if (nLen<32)
	{
		if (m_pLastImgData)
		{
			memcpy(pImageData, m_pLastImgData, m_dParas.img_w*m_dParas.img_h);
		}
		return -5;
	}
	int ret = 0;
	if (nLen > MAXLEN/2)
	{
		nLen = MAXLEN/2;
	}
	ret = Converts2d(pIQData, m_pComplexIQ, nLen);
	if (ret<0)
	{
		if (m_pLastImgData)
		{
			memcpy(pImageData, m_pLastImgData, m_dParas.img_w*m_dParas.img_h);
		}
		return -2;
	}
	if (m_dParas.triplex)
	{
		ret = DopplerAudio(m_pComplexIQ, nLen);
		if (ret < 0)
		{
			if (m_pLastImgData)
			{
				memcpy(pImageData, m_pLastImgData, m_dParas.img_w*m_dParas.img_h);
			}
			return -3;
		}
	}
	
	ret = DopplerImage(m_pComplexIQ, nLen, pImageData);
	if (ret<0)
	{
		if (m_pLastImgData)
		{
			memcpy(pImageData, m_pLastImgData, m_dParas.img_w*m_dParas.img_h);
		}
		return -4;
	}
	if (m_pLastImgData)
	{
		memcpy(m_pLastImgData, pImageData, m_dParas.img_w*m_dParas.img_h);
	}
	return ret;
}

void CGenDopplerProcess::DClearData()
{
	m_nCurImageLine = 0;
	if (m_pLastImgData)
	{
		memset(m_pLastImgData, 0, m_dParas.img_w*m_dParas.img_h);
	}
	m_ImgQueue.Reset();
}

int CGenDopplerProcess::DSetPara(EDopperParas dpara, ...)
{
	va_list argptr;
	va_start(argptr, dpara); //初始化变元指针  

	return DSetPara2(dpara, argptr);


	int nValue = 0;
	float fValue = 0;
	RECT rcValue = { 0 };
	

	switch (dpara)
	{
	
	case ZJIF_Doppler::D_WALLFILTER:
	case ZJIF_Doppler::D_DYN:
	case ZJIF_Doppler::D_BASELINE:
	case ZJIF_Doppler::D_INVERT:
	case ZJIF_Doppler::D_PRF:
	case ZJIF_Doppler::D_ITOUCH:
	case ZJIF_Doppler::D_IMGOPT:
	case ZJIF_Doppler::D_RES:
	case ZJIF_Doppler::D_GREYMAP:
	case ZJIF_Doppler::D_TINTMAP:
	case ZJIF_Doppler::D_TRIPLEX:
	case ZJIF_Doppler::D_AUTOTRACE:
	case ZJIF_Doppler::D_AUDIOVOLUME:
		nValue = va_arg(argptr, int);
		break;
	
	
	case ZJIF_Doppler::D_STEER:
	case ZJIF_Doppler::D_FLOWANGLE:
	case ZJIF_Doppler::D_TIMEPERPIXEL:
	case ZJIF_Doppler::D_SPEED:
		fValue = va_arg(argptr, float);
		break;

	case ZJIF_Doppler::D_LAYOUT:
		rcValue = va_arg(argptr, RECT);
		break;

	case ZJIF_Doppler::D_DUMMY:
	case ZJIF_Doppler::D_IDEL:
	default:
		break;
	}

	va_end(argptr);

	switch (dpara)
	{
	case ZJIF_Doppler::D_IDEL:
		break;
	case ZJIF_Doppler::D_WALLFILTER:
		m_dParas.wallfilter_id = nValue;
		break;
	case ZJIF_Doppler::D_DYN:
		m_dParas.dyn_id = nValue;
		break;
	case ZJIF_Doppler::D_BASELINE:
		m_dParas.baseline = nValue;
		break;
	case ZJIF_Doppler::D_INVERT:
		m_dParas.invert = nValue;
		break;
	case ZJIF_Doppler::D_STEER:
		m_dParas.steer = fValue;
		break;
	case ZJIF_Doppler::D_FLOWANGLE:
		m_dParas.flowangle = fValue;
		break;
	case ZJIF_Doppler::D_PRF:
		m_dParas.prf = nValue;
		DClearData();
		m_WaveOut.SetPRF(nValue);
		m_ImgQueue.Reset();
		break;
	case ZJIF_Doppler::D_ITOUCH:
		break;
	case ZJIF_Doppler::D_IMGOPT:
		m_dParas.imgopt_id = nValue;
		break;
	case ZJIF_Doppler::D_SPEED:
		m_dParas.linepersec = fValue;
		break;
	case ZJIF_Doppler::D_RES:
		
		m_dParas.resolution = nValue;
		m_CalN = m_CalSetN[m_dParas.resolution];
		DClearData();
		break;
	case ZJIF_Doppler::D_GREYMAP:
		m_dParas.greymap_id = nValue;
		break;
	case ZJIF_Doppler::D_TINTMAP:
		m_dParas.tintmap_id = nValue;
		break;
	case ZJIF_Doppler::D_LAYOUT:
		m_dParas.pw_roi = rcValue;
		break;
	case ZJIF_Doppler::D_TRIPLEX:
		m_dParas.triplex = nValue;
		break;
	case ZJIF_Doppler::D_AUTOTRACE:
		m_dParas.autotrace = nValue;
		break;
	case ZJIF_Doppler::D_AUDIOVOLUME:
		m_WaveOut.SetVolume(nValue);
		break;
	case ZJIF_Doppler::D_TIMEPERPIXEL:
		break;
	case ZJIF_Doppler::D_DUMMY:
		break;
	default:
		break;
	}
	return 0;
}

int CGenDopplerProcess::DGetPara(EDopperParas dpara, ...)
{
	
	va_list argptr;
	va_start(argptr, dpara); //初始化变元指针  

	return DGetPara2(dpara, argptr);


	int ret = 0;
	int* nValue = 0;
	float* fValue = 0;
	RECT* rcValue = { 0 };
	

	switch (dpara)
	{

	case ZJIF_Doppler::D_WALLFILTER:
	case ZJIF_Doppler::D_DYN:
	case ZJIF_Doppler::D_BASELINE:
	case ZJIF_Doppler::D_INVERT:
	case ZJIF_Doppler::D_PRF:
	case ZJIF_Doppler::D_ITOUCH:
	case ZJIF_Doppler::D_IMGOPT:
	case ZJIF_Doppler::D_RES:
	case ZJIF_Doppler::D_GREYMAP:
	case ZJIF_Doppler::D_TINTMAP:
	case ZJIF_Doppler::D_TRIPLEX:
	case ZJIF_Doppler::D_AUTOTRACE:
	case ZJIF_Doppler::D_AUDIOVOLUME:
		nValue = va_arg(argptr, int*);
		if (!nValue)
		{
			ret = -1;
		}
		break;


	case ZJIF_Doppler::D_STEER:
	case ZJIF_Doppler::D_FLOWANGLE:
	case ZJIF_Doppler::D_TIMEPERPIXEL:
	case ZJIF_Doppler::D_SPEED:
		fValue = va_arg(argptr, float*);
		if (!fValue)
		{
			ret = -1;
		}
		break;

	case ZJIF_Doppler::D_LAYOUT:
		rcValue = va_arg(argptr, RECT*);
		if (!rcValue)
		{
			ret = -1;
		}
		break;

	case ZJIF_Doppler::D_DUMMY:
	case ZJIF_Doppler::D_IDEL:
	default:
		break;
	}

	va_end(argptr);

	if (ret<0)
	{
		return ret;
	}

	switch (dpara)
	{
	case ZJIF_Doppler::D_IDEL:
		break;
	case ZJIF_Doppler::D_WALLFILTER:
		*nValue = m_dParas.wallfilter_id;
		break;
	case ZJIF_Doppler::D_DYN:
		*nValue = m_dParas.dyn_id;
		break;
	case ZJIF_Doppler::D_BASELINE:
		*nValue = m_dParas.baseline;
		break;
	case ZJIF_Doppler::D_INVERT:
		*nValue = m_dParas.invert;
		break;
	case ZJIF_Doppler::D_STEER:
		*fValue = m_dParas.steer;
		break;
	case ZJIF_Doppler::D_FLOWANGLE:
		*fValue = m_dParas.flowangle;
		break;
	case ZJIF_Doppler::D_PRF:
		*nValue = m_dParas.prf;
		break;
	case ZJIF_Doppler::D_ITOUCH:
		break;
	case ZJIF_Doppler::D_IMGOPT:
		*nValue = m_dParas.imgopt_id;
		break;
	case ZJIF_Doppler::D_SPEED:
		*nValue = m_dParas.linepersec;
		break;
	case ZJIF_Doppler::D_RES:
		*nValue = m_dParas.resolution;
		break;
	case ZJIF_Doppler::D_GREYMAP:
		*nValue = m_dParas.greymap_id;
		break;
	case ZJIF_Doppler::D_TINTMAP:
		*nValue = m_dParas.tintmap_id;
		break;
	case ZJIF_Doppler::D_LAYOUT:
		*rcValue = m_dParas.pw_roi;
		break;
	case ZJIF_Doppler::D_TRIPLEX:
		*nValue = m_dParas.triplex;
		break;
	case ZJIF_Doppler::D_AUTOTRACE:
		*nValue = m_dParas.autotrace;
		break;
	case ZJIF_Doppler::D_AUDIOVOLUME:
		*nValue = m_WaveOut.GetVolume();
		break;
	case ZJIF_Doppler::D_TIMEPERPIXEL:
		break;
	case ZJIF_Doppler::D_DUMMY:
		break;
	default:
		break;
	}
	return 0;
}

int CGenDopplerProcess::DSetPara2(EDopperParas dpara, va_list argptr)
{
	int ret = 0;
	int nValue = 0;
	float fValue = 0;
	RECT rcValue = { 0 };
	//va_start(argptr, dpara); //初始化变元指针  

	switch (dpara)
	{

	case ZJIF_Doppler::D_WALLFILTER:
	case ZJIF_Doppler::D_DYN:
	case ZJIF_Doppler::D_BASELINE:
	case ZJIF_Doppler::D_INVERT:
	case ZJIF_Doppler::D_PRF:
	case ZJIF_Doppler::D_ITOUCH:
	case ZJIF_Doppler::D_IMGOPT:
	case ZJIF_Doppler::D_RES:
	case ZJIF_Doppler::D_GREYMAP:
	case ZJIF_Doppler::D_TINTMAP:
	case ZJIF_Doppler::D_TRIPLEX:
	case ZJIF_Doppler::D_AUTOTRACE:
	case ZJIF_Doppler::D_AUDIOVOLUME:
		nValue = va_arg(argptr, int);
		break;


	case ZJIF_Doppler::D_STEER:
	case ZJIF_Doppler::D_FLOWANGLE:
	case ZJIF_Doppler::D_TIMEPERPIXEL:
	case ZJIF_Doppler::D_SPEED:
		fValue = va_arg(argptr, double);
		break;

	case ZJIF_Doppler::D_LAYOUT:
		rcValue = va_arg(argptr, RECT);
		break;

	case ZJIF_Doppler::D_DUMMY:
	case ZJIF_Doppler::D_IDEL:
	default:
		break;
	}

	va_end(argptr);

	switch (dpara)
	{
	case ZJIF_Doppler::D_IDEL:
		break;
	case ZJIF_Doppler::D_WALLFILTER:
		m_dParas.wallfilter_id = nValue;
		break;
	case ZJIF_Doppler::D_DYN:
		m_dParas.dyn_id = nValue;
		LoadLogTabByID(m_pLogTab, 65535, m_dParas.dyn_id);
		break;
	case ZJIF_Doppler::D_BASELINE:
		m_dParas.baseline = nValue;
		break;
	case ZJIF_Doppler::D_INVERT:
		m_dParas.invert = nValue;
		break;
	case ZJIF_Doppler::D_STEER:
		m_dParas.steer = fValue;
		break;
	case ZJIF_Doppler::D_FLOWANGLE:
		m_dParas.flowangle = fValue;
		break;
	case ZJIF_Doppler::D_PRF:
		if (nValue<100)
		{
			ret = -2;
			break;
		}
		m_dParas.prf = nValue;
		DClearData();
		m_WaveOut.SetPRF(nValue);
		m_ImgQueue.Reset();
		break;
	case ZJIF_Doppler::D_ITOUCH:
		break;
	case ZJIF_Doppler::D_IMGOPT:
		m_dParas.imgopt_id = nValue;
		break;
	case ZJIF_Doppler::D_SPEED:
		m_dParas.linepersec = fValue;
		break;
	case ZJIF_Doppler::D_RES:
		m_dParas.resolution = nValue;
		m_CalN = m_CalSetN[m_dParas.resolution];
		DClearData();
		break;
	case ZJIF_Doppler::D_GREYMAP:
		m_dParas.greymap_id = nValue;
		break;
	case ZJIF_Doppler::D_TINTMAP:
		m_dParas.tintmap_id = nValue;
		break;
	case ZJIF_Doppler::D_LAYOUT:
		m_dParas.pw_roi = rcValue;
		break;
	case ZJIF_Doppler::D_TRIPLEX:
		m_dParas.triplex = nValue;
		break;
	case ZJIF_Doppler::D_AUTOTRACE:
		m_dParas.autotrace = nValue;
		break;
	case ZJIF_Doppler::D_AUDIOVOLUME:
		m_WaveOut.SetVolume(nValue);
		break;
	case ZJIF_Doppler::D_TIMEPERPIXEL:
		break;
	case ZJIF_Doppler::D_DUMMY:
		break;
	default:
		break;
	}
	return ret;
}

int CGenDopplerProcess::DGetPara2(EDopperParas dpara, va_list argptr)
{
	int ret = 0;
	int* nValue = 0;
	float* fValue = 0;
	RECT* rcValue = { 0 };
	//va_start(argptr, dpara); //初始化变元指针  

	switch (dpara)
	{

	case ZJIF_Doppler::D_WALLFILTER:
	case ZJIF_Doppler::D_DYN:
	case ZJIF_Doppler::D_BASELINE:
	case ZJIF_Doppler::D_INVERT:
	case ZJIF_Doppler::D_PRF:
	case ZJIF_Doppler::D_ITOUCH:
	case ZJIF_Doppler::D_IMGOPT:
	case ZJIF_Doppler::D_RES:
	case ZJIF_Doppler::D_GREYMAP:
	case ZJIF_Doppler::D_TINTMAP:
	case ZJIF_Doppler::D_TRIPLEX:
	case ZJIF_Doppler::D_AUTOTRACE:
	case ZJIF_Doppler::D_AUDIOVOLUME:
		nValue = va_arg(argptr, int*);
		if (!nValue)
		{
			ret = -1;
		}
		break;


	case ZJIF_Doppler::D_STEER:
	case ZJIF_Doppler::D_FLOWANGLE:
	case ZJIF_Doppler::D_TIMEPERPIXEL:
	case ZJIF_Doppler::D_SPEED:
		fValue = (float*)va_arg(argptr, float*);
		if (!fValue)
		{
			ret = -1;
		}
		break;

	case ZJIF_Doppler::D_LAYOUT:
		rcValue = va_arg(argptr, RECT*);
		if (!rcValue)
		{
			ret = -1;
		}
		break;

	case ZJIF_Doppler::D_DUMMY:
	case ZJIF_Doppler::D_IDEL:
	default:
		break;
	}

	va_end(argptr);

	if (ret < 0)
	{
		return ret;
	}

	switch (dpara)
	{
	case ZJIF_Doppler::D_IDEL:
		break;
	case ZJIF_Doppler::D_WALLFILTER:
		*nValue = m_dParas.wallfilter_id;
		break;
	case ZJIF_Doppler::D_DYN:
		*nValue = m_dParas.dyn_id;
		break;
	case ZJIF_Doppler::D_BASELINE:
		*nValue = m_dParas.baseline;
		break;
	case ZJIF_Doppler::D_INVERT:
		*nValue = m_dParas.invert;
		break;
	case ZJIF_Doppler::D_STEER:
		*fValue = m_dParas.steer;
		break;
	case ZJIF_Doppler::D_FLOWANGLE:
		*fValue = m_dParas.flowangle;
		break;
	case ZJIF_Doppler::D_PRF:
		*nValue = m_dParas.prf;
		break;
	case ZJIF_Doppler::D_ITOUCH:
		break;
	case ZJIF_Doppler::D_IMGOPT:
		*nValue = m_dParas.imgopt_id;
		break;
	case ZJIF_Doppler::D_SPEED:
		*fValue = m_dParas.linepersec;
		break;
	case ZJIF_Doppler::D_RES:
		*nValue = m_dParas.resolution;
		break;
	case ZJIF_Doppler::D_GREYMAP:
		*nValue = m_dParas.greymap_id;
		break;
	case ZJIF_Doppler::D_TINTMAP:
		*nValue = m_dParas.tintmap_id;
		break;
	case ZJIF_Doppler::D_LAYOUT:
		*rcValue = m_dParas.pw_roi;
		break;
	case ZJIF_Doppler::D_TRIPLEX:
		*nValue = m_dParas.triplex;
		break;
	case ZJIF_Doppler::D_AUTOTRACE:
		*nValue = m_dParas.autotrace;
		break;
	case ZJIF_Doppler::D_AUDIOVOLUME:
		*nValue = m_WaveOut.GetVolume();
		break;
	case ZJIF_Doppler::D_TIMEPERPIXEL:
		break;
	case ZJIF_Doppler::D_DUMMY:
		break;
	default:
		break;
	}
	return ret;
}

int CGenDopplerProcess::DReset()
{
	return -1;
}

void CGenDopplerProcess::DEnd()
{
	if (m_pLastImgData)
	{
		delete[] m_pLastImgData;
		m_pLastImgData = NULL;
	}
	m_WaveOut.Pause();
	m_WaveOut.Clear();
	if (m_fftPlan)
	{
		fftw_destroy_plan(m_fftPlan);
		//fftw_cleanup();
		//fftw_cleanup_threads();
		m_fftPlan = NULL;
	}

	//m_nCurImageLine = 0;
	//m_ImgQueue.Reset();
	return ;
}

int CGenDopplerProcess::WindowedSignal(_complex *pIQData_in, int len, _complex *pIQData_Out, int nWinID/*=0*/)
{
	if (!pIQData_in || !pIQData_Out || len<=0)
	{
		return -1;
	}
	if (nWinID<0||nWinID>2)
	{
		return -2;
	}

	if (m_winlastlen!=len)
	{
		switch (nWinID)
		{
		case 0:
			m_WinSignal.MakeHammingWindow(len);
			break;
		case 1:
			m_WinSignal.MakeHanningWindow(len);
			break;
		case 2:
			m_WinSignal.MakeBlackmanWindow(len);
			break;
		default:
			break;
		}
		m_winlastlen = len;
	}
	if (m_winlastwinid!=nWinID)
	{ 
		switch (nWinID)
		{
		case 0:
			m_WinSignal.MakeHammingWindow(len);
			break;
		case 1:
			m_WinSignal.MakeHanningWindow(len);
			break;
		case 2:
			m_WinSignal.MakeBlackmanWindow(len);
			break;
		default:
			break;
		}
		m_winlastwinid = nWinID;
	}
	return m_WinSignal.DoWeight((double*)pIQData_in, (double*)pIQData_Out, len);

}

int CGenDopplerProcess::Converts2d(ComplexIQ *pIn, _complex *pOut, int len)
{
	if (!pIn || !pOut || len<=0 || len> MAXLEN)
	{
		return -1;
	}
#pragma loop(hint_parallel(CPUCORE))
	for (int i = 0; i < len; i++)
	{
		pOut[i].x = pIn[i].I;
		pOut[i].y = pIn[i].Q;
	}
	return 0;
}

int CGenDopplerProcess::Transpose(unsigned char* pDatain, int w, int h, unsigned char* pDataout)
{
	int ret = -1;
	if (pDatain && pDataout)
	{
		//#pragma omp parallel for
#pragma loop(hint_parallel(CPUCORE))
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				pDataout[j * h + i] = pDatain[i * w + j];
			}
		}
	}
	return 0;
}

//int CGenDopplerProcess::Transpose(unsigned char* pDatainOut, int w, int h)
//{
//	int ret = -1;
//	if (!pDatainOut)
//	{
//		//#pragma omp parallel for
//#pragma loop(hint_parallel(CPUCORE))
//		for (int i = 0; i < h; i++)
//		{
//			for (int j = 0; j < w; j++)
//			{
//				if (w>h && j<i)
//				{
//					continue;
//				}else if (w<=h && i<j)
//				{
//					continue;
//				}
//				unsigned char tmp = pDatainOut[j * h + i];
//				pDatainOut[j * h + i] = pDatainOut[i * w + j];
//				pDatainOut[i * w + j] = tmp;
//			}
//		}
//	}
//	return 0;
//}



//int CGenDopplerProcess::LoadDataFromFile(char *file)
//{
//	return -1;
//}
_complex lastdata[256] = { 0 };
int lastminlen = 0;
int CGenDopplerProcess::DopplerImage(_complex *pIQData_in, int len, unsigned char *pImageData)
{
	if (len > MAXLEN/2)
	{
		len = MAXLEN/2;
	}

	_complex *pIQ = new _complex[len + m_N];
	memset(pIQ, 0, (len + m_N)*sizeof(_complex));
	memcpy(pIQ, lastdata, lastminlen*sizeof(_complex));
	memcpy(pIQ + lastminlen, pIQData_in, len*sizeof(_complex));

	int minlen = (len < m_CalN) ? len : m_CalN;
	memset(lastdata, 0, 256 * sizeof(_complex));
	memcpy(lastdata, pIQData_in + len - minlen, minlen*sizeof(_complex));
	lastminlen = minlen;

	float interval = m_dParas.prf / m_dParas.linepersec;
	int lines = (len) / interval;
	int buflen = ((lines > m_dParas.img_w) ? lines : m_dParas.img_w);
	unsigned char *buf = new unsigned char[buflen*m_dParas.img_h];
	
	memset(buf, 0, buflen*m_dParas.img_h);
	int *ptmp = new int[m_N];

	for (int i = 0; i < lines-1; i++)
	{
		memset(ptmp, 0, m_N*sizeof(int));
		//采用刘老师所说的方法，做4次求平均，提高信噪比
		for (int k = 0; k < 4;k++)
		{
			int pos = (int)(i*interval)+interval*k/4;
			DopplerImageLine(pIQ + pos, m_CalN, buf + i*m_N, NULL, NULL);
			for (int j = 0; j < m_N; j++)
			{
				ptmp[j] += buf[i*m_N + j];
			}
		}
		for (int j = 0; j < m_N; j++)
		{
			buf[i*m_N + j] = ptmp[j]/4;
		}
		//int pos = (int)(i*interval) + interval;
		//DopplerImageLine(pIQ + pos, m_CalN, buf + i*m_N, NULL, NULL);
	}
	delete[] ptmp;

	int pos = (int)((lines - 1)*interval);
	DopplerImageLine(pIQ + pos, m_CalN, buf + (lines - 1)*m_N, NULL, NULL);


	float oldalpha = 0.8;
	for (int i = 1; i < lines; i++)
	{
		for (int j = 0; j < m_N;j++)
		{
			buf[(i - 1)*m_N + j] = buf[(i - 1)*m_N + j]*oldalpha + buf[(i)*m_N + j]*(1-oldalpha);
		}
	}
	m_ImgQueue.Add((DLine*)buf, lines);

	//int n = m_dParas.img_w;
	//unsigned char *pimg = (unsigned char*)m_ImgQueue.GetLastest(n);
	//if (n<=0||!pimg)
	//{
	//	return -2;
	//}
	//memcpy(buf, pimg, n*m_dParas.img_h);
	//InvertAndShiftBaseLine(buf, m_dParas.img_h, m_dParas.img_w);
	//Transpose(buf, m_dParas.img_h, m_dParas.img_w,pImageData);


	int n = m_dParas.img_w;
	unsigned char *pimg = (unsigned char*)m_ImgQueue.GetFromPos(0,n);
	int curpos = m_ImgQueue.GetCurWritePos();
	

	memcpy(buf, pimg, n*m_dParas.img_h);
	memset(buf + m_dParas.img_h*(curpos%(m_dParas.img_w-4)), 0, m_dParas.img_h*4);
	//*(buf + m_dParas.img_h*(curpos % (m_dParas.img_w - 4)) + 128) = 255;
	//*(buf + m_dParas.img_h*(curpos % (m_dParas.img_w - 3)) + 128) = 255;
	//*(buf + m_dParas.img_h*(curpos % (m_dParas.img_w - 2)) + 128) = 255;
	//*(buf + m_dParas.img_h*(curpos % (m_dParas.img_w - 1)) + 128) = 255;
	
	InvertAndShiftBaseLine(buf, m_dParas.img_h, m_dParas.img_w);
	Transpose(buf, m_dParas.img_h, m_dParas.img_w,pImageData);

	m_nCurImageLine = (m_nCurImageLine + lines) % m_dParas.img_w;
	delete[] buf;
	delete[] pIQ;
	return 0;
}

int CGenDopplerProcess::DopplerAudio(_complex *pIQData_in, int len)
{
	if (len>MAXLEN/2)
	{
		len = MAXLEN/2;
	}
	m_AudioProcess.MakeAudioCH((double*)pIQData_in, len * 2, m_pAudioData,m_dParas.invert);
	m_WaveOut.AddDatasToBuff((short*)m_pAudioData, len * 2, 0);
	//static int flag = 0;
	//static BYTE *writebuffer = NULL;
	//static int writelen = 0;
	//if (!flag)
	//{
	//	writebuffer = new BYTE[65536*1024];
	//	memset(writebuffer, 0, 65536 * 1024);
	//	flag = 1;
	//}
	//if (writebuffer)
	//{
	//	memcpy(writebuffer+writelen, (BYTE*)m_pAudioData, len * 2);
	//	writelen += len * 2;
	//}

	//
	//if (writelen> 200000&&writebuffer)
	//{
	//	m_AudioProcess.WriteWave(writebuffer, writelen, m_dParas.prf, "d:\\pw.wav");
	//	delete[] writebuffer;
	//	writebuffer = NULL
	//	writelen = 0;
	//}
	return 0;
}

int CGenDopplerProcess::DopplerImageLine(_complex *pIQData_in, int len, unsigned char *pImageData, int *pmaxpos, int *pmeanpos)
{
	if (!pIQData_in || len<1 || len>255 || !pImageData)
	{
		return -1;
	}
	static int lastlen = 0;
	m_N = 256;
	memset(m_pIQout, 0, m_N*sizeof(fftw_complex));
	memcpy(m_pIQout, pIQData_in, len  * sizeof(fftw_complex));
	WindowedSignal(m_pIQout, len, m_pIQin);
	if (m_fftPlan && lastlen != len)
	{
		fftw_destroy_plan(m_fftPlan);
		m_fftPlan = NULL;
		m_fftPlan = fftw_plan_dft_1d(m_N, (fftw_complex*)m_pIQin, (fftw_complex*)m_pIQout, FFTW_FORWARD, FFTW_MEASURE);// FFTW_MEASURE);//FFTW_ESTIMATE
		lastlen = len;
	}
	if (!m_fftPlan)
	{
		m_fftPlan = fftw_plan_dft_1d(m_N, (fftw_complex*)m_pIQin, (fftw_complex*)m_pIQout, FFTW_FORWARD, FFTW_MEASURE);// FFTW_MEASURE);//FFTW_ESTIMATE
		lastlen = len;
	}
	
	fftw_execute_dft(m_fftPlan, (fftw_complex*)m_pIQin, (fftw_complex*)m_pIQout);


	unsigned short *tmp = new unsigned short[m_N];
#pragma loop(hint_parallel(CPUCORE))
	for (int j = 0; j < m_N; j++)
	{
		tmp[j] = _cabs(*((_complex*)m_pIQout + j));
	}

	//m_N = 256;
	//for (int j = 0; j < m_N;j++)
	//{
	//	tmp[j] = tmp[j * 2] + tmp[j * 2 + 1];
	//	//tmp[j] = tmp[j * 4] + tmp[j * 4 + 1]+\
	//		tmp[j * 4 + 2] + tmp[j * 4 + 3];
	//	//tmp[j] /= 2;
	//}

	unsigned short *tmp4 = new unsigned short[m_N];
	//memset(tmp4, 0, m_N*sizeof(unsigned short));
	

	memcpy(tmp4, tmp, m_N*sizeof(unsigned short));
	
	for (int i = 0; i < 253; i++)
	{
		int interval = tmp4[i] - tmp4[i + 2];
		if (abs(interval) > 150)
		{
			int mid = tmp4[i] + interval*0.2;
			if (mid < 0)
				mid = 0;
			else if (mid > 4093)
				mid = 4093;
			tmp[i] = mid;
		}
		else
		{
			int mid = tmp4[i] - interval*0.2;
			if (mid < 0)
				mid = 0;
			else if (mid > 4093)
				mid = 4093;
			tmp[i] = mid;
		}

	}
	memcpy(tmp4, tmp, m_N*sizeof(unsigned short));
	BlackHoleFilter(tmp4, tmp, m_N, 20, 10000, 1);

	for (int j = 0; j < m_dParas.imgopt_id; j++)
	{

		tmp4[0] = (tmp[255] + tmp[0] + tmp[1]) / 3;
		for (int i = 1; i < 255; i++)
		{
			tmp4[i] = (tmp[i - 1] + tmp[i] + tmp[i + 1]) / 3;
		}
		tmp4[255] = (tmp[254] + tmp[255] + tmp[0]) / 3;

		tmp[0] = (tmp4[255] + tmp4[0] + tmp4[1]) / 3;
		for (int i = 1; i < 255; i++)
		{
			tmp[i] = (tmp4[i - 1] + tmp4[i] + tmp4[i + 1]) / 3;
		}
		tmp[255] = (tmp4[254] + tmp4[255] + tmp4[0]) / 3;
	}





	delete[] tmp4;


	Trace(tmp, m_N, pmaxpos, pmeanpos);


//	static int nmax = 0;

#pragma loop(hint_parallel(CPUCORE))	
	for (int j = 0; j < m_N; j++)
	{
		int tmp2 = tmp[j];
		
		//if (tmp2 > nmax)
		//{
		//	nmax = tmp2;
		//}
		if (tmp2>m_LogTabLen-1)
		{
			
			tmp2 = m_LogTabLen - 1;
		}
	
		tmp2 = m_pLogTab[tmp2];


		if (j < 128)
		{
			pImageData[127 - j] = tmp2;
		}
		else
		{
			pImageData[255 + 128 - j] = tmp2;
		}

	}
	


	unsigned char *tmp3=new unsigned char[m_N];
	memset(tmp3, 0, m_N);

	memcpy(tmp3, pImageData, m_N);

	//for (int i = 0; i < 253; i++)
	//{
	//	int interval = tmp3[i] - tmp3[i + 2];
	//	if (abs(interval)>20)
	//	{
	//		int mid = tmp3[i] + interval*0.2;
	//		if (mid < 0)
	//			mid = 0;
	//		else if (mid > 250)
	//			mid = 250;
	//		pImageData[i] = mid;
	//	}
	//}


	//for (int j = 0; j < m_dParas.imgopt_id+3; j++)
	//{

	//	tmp3[0] = (pImageData[255] + pImageData[0] + pImageData[1]) / 3;
	//	for (int i = 1; i < 255; i++)
	//	{
	//		tmp3[i] = (pImageData[i - 1] + pImageData[i] + pImageData[i + 1]) / 3;
	//	}
	//	tmp3[255] = (pImageData[254] + pImageData[255] + pImageData[0]) / 3;
	//	
	//	pImageData[0] = (tmp3[255] + tmp3[0] + tmp3[1]) / 3;
	//	for (int i = 1; i < 255; i++)
	//	{
	//		pImageData[i] = (tmp3[i - 1] + tmp3[i] + tmp3[i + 1]) / 3;
	//	}
	//	pImageData[255] = (tmp3[254] + tmp3[255] + tmp3[0]) / 3;
	//}

	
	
	

	delete[] tmp3;

	
	delete[] tmp;
	pImageData[128] = 255;
	return 0;
}

int CGenDopplerProcess::Trace(unsigned short *pData_int, int len, int *pMaxPos, int *pMeanPos)
{
	if (!pData_int)
	{
		return -1;
	}
	if (!pMaxPos && !pMeanPos)
	{
		return 0;
	}
	unsigned short *tmp1 = new unsigned short[len];
	unsigned short *tmp2 = new unsigned short[len];
	memcpy(tmp1, pData_int, len*sizeof(unsigned short));

	int sum[2]={0,0};
	int start = 0;
	int meanvalue = 0;
	for (int i = 0; i < len / 2; i++)
	{
		if (tmp1[i]>30)
		{
			sum[0] += tmp1[i];
		}
		
	}
	for (int i = len / 2; i < len; i++)
	{
		if (tmp1[i]>30)
		{ 
			sum[1] += tmp1[i];
		}
	}
	if (sum[1]>sum[0])
	{
		start = len / 2;
		meanvalue = sum[1];
	}
	else
	{
		meanvalue = sum[0];
	}
	int maxvalue = 0;
	int meanpos = 0;
	

	for (int i = start; i < start + len / 2; i++)
	{

		if (maxvalue < tmp1[i])
		{
			maxvalue = tmp1[i];
			meanpos = i;
		}

	}



	for (int i = 2; i < len / 2-2; i++)
	{
		int pos = i + start;
		tmp2[pos] = (tmp1[pos - 2] + tmp1[pos - 1] + tmp1[pos] + tmp1[pos + 1] + tmp1[pos + 2]) / 5;
	}
	for (int i = 2; i < len / 2 - 2; i++)
	{
		int pos = i + start;
		tmp1[pos] = (tmp2[pos - 2] + tmp2[pos - 1] + tmp2[pos] + tmp2[pos + 1] + tmp2[pos + 2]) / 5;
	}
	//for (int i = 1; i < len / 2 - 1; i++)
	//{
	//	int pos = i + start;
	//	tmp2[pos] = (tmp1[pos - 1] + tmp1[pos] + tmp1[pos + 1]) / 3;
	//}
	
	int maxgrant = 99999;
	int maxpos = 0;
	meanvalue = 1;
	
	for (int i = start; i < start + len / 2; i++)
	{
			meanvalue += tmp1[i];
	}
	int curvalue = meanvalue;
	for (int i = 5+start; i < start+len / 2-5; i++)
	{
		int cha = (int)tmp1[i - 2] - tmp1[i];
		curvalue -= tmp1[i - 2];
		if (curvalue*100/meanvalue>80)
		{
			continue;
		}
		if (maxgrant>=cha)
		{
			maxgrant = cha;
			maxpos = i-6;
		}
		if (maxpos<0)
		{
			maxpos = 0;
		}
		
	}

	delete[] tmp1;
	delete[] tmp2;

	meanvalue = meanvalue * 2 / len;
	if (pMaxPos)
		*pMaxPos = maxpos;
	if (pMeanPos)
		*pMeanPos = meanvalue;
//	pData_int[meanpos] = 1;
//	pData_int[maxpos] = 4095;
	return 0;
}

int CGenDopplerProcess::InvertAndShiftBaseLine(unsigned char* pDatainOut, int w, int h)
{
	if (!pDatainOut)
	{
		return -1;
	}
//	int startpos = 128;
	if (m_dParas.invert)
	{
		int tmp;
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w / 2;j++)
			{
				tmp = pDatainOut[i*w + j];
				pDatainOut[i*w + j] = pDatainOut[i*w + w - 1 - j];
				pDatainOut[i*w + w - 1 - j] = tmp;
			}
		}
	}
	if (m_dParas.baseline>0)
	{
//		startpos = w - m_dParas.baseline;
		unsigned char *tmp = new unsigned char[w];
		for (int i = 0; i < h;i++)
		{
			memcpy(tmp, pDatainOut + i*w, m_dParas.baseline);
			memcpy(pDatainOut + i*w, pDatainOut + i*w+m_dParas.baseline, w-m_dParas.baseline);
			memcpy(pDatainOut + i*w + w - m_dParas.baseline, tmp, m_dParas.baseline);
		}
		delete[] tmp;
	}else if (m_dParas.baseline<0)
	{
//		startpos = - m_dParas.baseline;
		unsigned char *tmp = new unsigned char[w];
		for (int i = 0; i < h; i++)
		{
			memcpy(tmp, pDatainOut + i*w, w + m_dParas.baseline);
			memcpy(pDatainOut + i*w, pDatainOut + i*w + w + m_dParas.baseline, - m_dParas.baseline);
			memcpy(pDatainOut + i*w - m_dParas.baseline, tmp, w + m_dParas.baseline);
		}
		delete[] tmp;
	}

	//if (m_dParas.baseline != 0 && (m_dParas.baseline>-125 && m_dParas.baseline<125))
	//{
	//	unsigned char tmp[15];
	//	startpos -= 2;
	//	for (int i = 0; i < h; i++)
	//	{
	//		
	//		int sum = pDatainOut[i*w + startpos]
	//			+ pDatainOut[i*w + startpos + 1]
	//			+ pDatainOut[i*w + startpos + 2]
	//			+ pDatainOut[i*w + startpos + 3]
	//			+ pDatainOut[i*w + startpos + 4];
	//		tmp[0] = sum / 5;
	//		for (int j = 1; j < 5;j++)
	//		{
	//			sum = sum - pDatainOut[i*w + startpos +j] + pDatainOut[i*w + startpos + j + 4];
	//			tmp[j] = sum / 5;
	//		}

	//		memcpy(pDatainOut + i*w + startpos + 2, tmp, 5);
	//	}
	//}
	return 0;
}

int CGenDopplerProcess::DopplerProcessByWord(ComplexIQ *pIQData, int nLen, unsigned char *pImageData)
{
	int ret = -1;
	unsigned char *tmp = new unsigned char[m_dParas.img_w*m_dParas.img_h];
	memset(tmp, 0, m_dParas.img_w*m_dParas.img_h);
	ret = DopplerProcess(pIQData, nLen, tmp);
	//if (ret>=0)
	//{
		for (int i = 0; i < m_dParas.img_w*m_dParas.img_h;i++)
		{
			pImageData[i * 2] = tmp[i];
			pImageData[i * 2 + 1] = 0;
		}
	//}
	delete[] tmp;
	return ret;
}

int CGenDopplerProcess::LoadLogTab(unsigned char *tab, int len, char *path)
{
	if (!tab||len<4096||!path)
	{
		return -1;
	}
	FILE *fp = NULL;
	fopen_s(&fp,path, "rb");
	if (fp)
	{
		fread_s(tab,len, 4096, 1, fp);
		fclose(fp);
	}
	return 0;
}

int CGenDopplerProcess::LoadLogTabByID(unsigned char *tab, int len, int id)
{
	char path[256];
	sprintf_s(path, ".\\log%d.dat", id + 1);
	return LoadLogTab(tab, len, path);
}


int CGenDopplerProcess::BlackHoleFilter(unsigned short* pin, unsigned short *pout, int len, int theta_black,int theta_white, int isBigsize)
{
	if (!pin||!pout)
	{
		return -3;
	}
	if (isBigsize)
	{

		//int theta = 20;
		int winlen = 2;
		for (int i = winlen; i < len - winlen; i++)
		{
			int p = i;
			int aver = pin[p - 2] + pin[p - 1] + pin[p + 1] + pin[p + 2];
			aver = aver >> 2;
			if (pin[i] + theta_black < aver || pin[i] - theta_white > aver)
			{
				pout[i] = aver;
			}
			else
			{
				pout[i] = pin[i];
			}
		}
	}
	else
	{
		int winlen = 1;
		for (int i = winlen; i < len - winlen; i++)
		{
			int p = i;
			int aver = pin[p - 1] + pin[p + 1];
			aver = aver >> 1;
			if (pin[i] + theta_black < aver || pin[i] - theta_white > aver)
			{
				pout[i] = aver;
			}
			else
			{
				pout[i] = pin[i];
			}
		}
	}
	return 0;
}

