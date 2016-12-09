// == [6/25/2015 AlexHuang]
// ����SetPara2��GetPara2�ӿڣ���Ϊ�����δ��ݹ����鷳
#pragma once

// ͼ����ֵ��ʽ 0����δɨ�鵽������ 1Ϊ��ЧƵ���е����ֵ��250Ϊ��ЧƵ���е����ֵ��255Ϊ����ֵ��254ΪMaxTrace��ǣ�253ΪMeanTrace���ֵ
#include <stdarg.h>

class ZJIF_Doppler
{
public:
	enum EDopperParas
	{
		D_IDEL=0,
		D_WALLFILTER,	//���˲���������0~3����ϵͳ����FPGA���ƣ����� ��
		D_DYN,			//Ƶ�׵Ķ�̬��Χ����0~10������ҪLOG1.dat�ļ�����û���ļ��̶�Ϊ0
		D_BASELINE,		//���ߵ��ڣ���������Ϊ0����Χ��-127~127��
		D_INVERT,		//Ƶ�׷�ת����0~1��
		D_STEER,		//ƫתɨ�裬Ŀǰ���� ��
		D_FLOWANGLE,	//Ѫ���Ƕȣ�Ŀǰ���� ��
		D_PRF,			//�ظ�����Ƶ��,prf>100����λHz
		D_ITOUCH,		//Ƶ�׵�һ���Ż���Ŀǰ���� ��
		D_IMGOPT,		//Ƶ�׵�ͼ����0~3��
		D_SPEED,		//��1������Ҫ����������pw�ߣ�float����1~250��
		D_RES,			//Ƶ�׷ֱ��ʡ�0~3����Ŀǰ�̶�Ϊ0
		D_GREYMAP,		//Ƶ�׵Ļҽ�����Ŀǰ���� ��
		D_TINTMAP,		//Ƶ�׵�α������Ŀǰ���� ��
		D_LAYOUT,		//Ƶ�׵Ĳ��֣�Ŀǰ���� ��
		D_TRIPLEX,		//��ͬ����BD��BCD��ģʽ����0~1����0Ϊͬ����1Ϊ��D��Ŀǰ���� ��
		D_AUTOTRACE,	//�Զ����磬Ŀǰ���� ��
		D_AUDIOVOLUME,	//Ƶ��������С����0~255����0Ϊ�ر�Ƶ������
		D_TIMEPERPIXEL,	//OnlyGet, ms per pixel������λ�����м��㣬Ŀǰ���� ��
		D_DUMMY
	};

	typedef struct _TagComplexIQ
	{
		short I;
		short Q;
	}ComplexIQ;

	typedef struct _TagDParas
	{
		int Ver;	//�汾�š�1~��
		int prf;	//�����ظ�Ƶ��,prf>100����λHz
		int wallfilter_id; //���˲�ID Ŀǰ�̶�Ϊ0 ��
		int invert;	//Ƶ�׷�ת����0~1��
		int baseline;	//���ߵ��ڣ���������Ϊ0����Χ��-127~127��
		int dyn_id;		//Ƶ�׵Ķ�̬��Χ����0~15����Ŀǰ�̶�Ϊ0 ��
		int triplex;	//��ͬ����BD��BCD��ģʽ����0~1����0Ϊͬ����1Ϊ��D
		int audiovolume;//Ƶ��������С����0~255����0Ϊ�ر�Ƶ��
		int resolution;	//Ƶ�׷ֱ��ʣ���0~4��Ŀǰ�̶�Ϊ0 ��
		int imgopt_id;	//Ƶ�׵�ͼ����0~3��
		float linepersec;//��1������Ҫ����������pw�߶�ӦD_SPEED����1~250��
		int greymap_id;	//Ƶ�׵Ļҽ�����Ŀǰ���á�0-7��=0 ��
		int tintmap_id;	//Ƶ�׵�α������Ŀǰ���á�0-7��=0 ��
		int autotrace;	//�Զ����磬Ŀǰ���á�0~1�� ��
		float steer;	//ƫתɨ�裬Ŀǰ���� ��
		float flowangle;//Ѫ���Ƕȣ�Ŀǰ���� ��
		int img_w;		//ͼ�������768
		int img_h;		//ͼ��ߣ�����256
		RECT pw_roi;	//PW��������Ŀǰ���� ��
		int Reversed[64];
	}DParas,*LPDParas;

	ZJIF_Doppler(){};
	virtual ~ZJIF_Doppler(){};
	virtual int DInit() = 0;	//��ʼ��Dģʽ
	virtual int DLoadParas(DParas *pparas) = 0;	//����Dģʽ����
	virtual int DLoadParasFromFile(char *file) = 0;	//���ļ�����Dģʽ����
	virtual int DSaveParasToFile(char *file) = 0;	//��Dģʽ�������浽�ļ���
	//virtual int LoadIQDataFromFile(char *file) = 0;
	//nLen>=64
	//���������IQ�źŵõ��Ҷ�����
	virtual int DopplerProcess(ComplexIQ *pIQData, int nLen, unsigned char *pImageData) = 0;
	//nLen>=64
	//���������IQ�źŵõ�16λ���ݣ���24ͨ�����ݼ��ݣ�
	virtual int DopplerProcessByWord(ComplexIQ *pIQData, int nLen, unsigned char *pImageData) = 0;
	//������������ݣ�ģʽ�л�ʱ���ı�PRFʱ����
	virtual void DClearData() = 0;
	//��ò��������ز���
	virtual int DSetPara(EDopperParas dpara, ...) = 0;
	virtual int DGetPara(EDopperParas dpara, ...) = 0;
	//������APIһ�£����������װ
	virtual int DSetPara2(EDopperParas dpara, va_list argls) = 0;
	virtual int DGetPara2(EDopperParas dpara, va_list argls) = 0;
	//����Dģʽ
	virtual int DReset() = 0;
	//�ر�Dģʽ
	virtual void DEnd() = 0;
};
