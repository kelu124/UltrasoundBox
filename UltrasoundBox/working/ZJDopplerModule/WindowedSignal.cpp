#include "stdafx.h"
#include <math.h>
#include "WindowedSignal.h"

//#define MAXWINFILTER 512

CWindowedSignal::CWindowedSignal()
{
	m_windowfilter = NULL;

	//m_windowfilter = new double[MAXWINFILTER];
	//memset(m_windowfilter, 0, MAXWINFILTER*sizeof(double));
	m_filterlen = 0;
}


CWindowedSignal::~CWindowedSignal()
{
	Clear();
}

void CWindowedSignal::Clear()
{
	if (m_windowfilter)
	{
		delete[] m_windowfilter;
		m_windowfilter = NULL;
	}
	m_filterlen = 0;
}

int CWindowedSignal::MakeHammingWindow(int len)
{
	int ret = 0;
	Clear();
	m_windowfilter = new double[len];
	m_filterlen = len;
	for (int i = 0; i < len;i++)
	{
		m_windowfilter[i] = 0.54 - 0.46 * cos(2 * i * M_PI / (len - 1));
	}
	return ret;
}

int CWindowedSignal::MakeHanningWindow(int len)
{
	int ret = 0;
	Clear();
	m_windowfilter = new double[len];
	m_filterlen = len;
	for (int i = 0; i < len; i++)
	{
		m_windowfilter[i] = 0.5 - 0.5 * cos(2 * i * M_PI / (len - 1));
	}
	return ret;
}

int CWindowedSignal::MakeBlackmanWindow(int len)
{
	int ret = 0;
	Clear();
	m_windowfilter = new double[len];
	m_filterlen = len;
	for (int i = 0; i < len; i++)
	{
		m_windowfilter[i] = 0.42 - 0.5 * cos(2 * i * M_PI / (len - 1)) + 0.08 * cos(4 * i * M_PI / (len - 1));
	}
	return ret;
}

int CWindowedSignal::DoWeight(double * pdata, int len)
{
	if (!m_windowfilter || !pdata)
	{
		return -1;
	}
	int nlen = (len < m_filterlen) ? len : m_filterlen;
	for (int i = 0; i < nlen; i++)
	{
		pdata[2*i] *= m_windowfilter[i];
		pdata[2*i+1] *= m_windowfilter[i];
	}
	return 0;
}

int CWindowedSignal::DoWeight(double * psrc, double * pdest, int len)
{
	if (! psrc || !pdest)
	{
		return -1;
	}
	if(!m_windowfilter)	//解决接口删除后，没法恢复现场的bug
	{
		MakeHammingWindow(len);
	}
	int nlen = (len < m_filterlen) ? len : m_filterlen;
	for (int i = 0; i < nlen; i++)
	{
		pdest[2 * i] = m_windowfilter[i]*psrc[2 * i];
		pdest[2 * i + 1] = m_windowfilter[i] * psrc[2 * i + 1];
	}
	return 0;
}
