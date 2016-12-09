#pragma once
class CWindowedSignal
{
public:
	CWindowedSignal();
	~CWindowedSignal();
	int MakeHammingWindow(int len);
	int MakeHanningWindow(int len);
	int MakeBlackmanWindow(int len);
	int DoWeight(double * pdata, int len);
	int DoWeight(double * psrc, double * pdest, int len);
private:
	double *m_windowfilter;
	int m_filterlen;
	void Clear();
	

};

