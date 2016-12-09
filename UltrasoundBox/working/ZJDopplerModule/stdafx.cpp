// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// ZJDopplerModule.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"

// TODO:  �� STDAFX.H ��
// �����κ�����ĸ���ͷ�ļ����������ڴ��ļ�������

int _WriteDoubleSignal(char *file, double *signal, int len)
{
#ifdef _DEBUG
	FILE *fp = nullptr;
	int wlen = len < 1024 ? len : 1024;
	fopen_s(&fp, file, "wb");
	if (fp)
	{
		fprintf_s(fp, "P5\n256 %d\n255\n", wlen);
		unsigned char tmp[256];
		memset(tmp, 0, 256);
		int pp = 0, np = 0;
		for (int i = 0; i < wlen; i++)
		{
			np = signal[i * 2] + 127;
			if (np < 0) np = 0;
			else if (np > 255)
				np = 255;

			tmp[pp] = 0;
			tmp[np] = 255;
			fwrite(tmp, 256, 1, fp);
			pp = np;
		}
		fclose(fp);
	}
	
	
#endif // _DEBUG
	return 0;
}
