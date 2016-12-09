#pragma once
class CWallFilterFIR
{
public:
	CWallFilterFIR();
	~CWallFilterFIR();
	int ConvComplex1D(double *src, double *filter, double *dest, int len, int order);
	int ConvComplex1D(double *src,  double *dest, int len, int filternum);
	int ConvComplex1D2(double *src, double *filter, double *dest, int len, int order);
private:
	struct FILTERGROUP
	{
		double *filter;
		int order;
	}m_filter[10];
};

