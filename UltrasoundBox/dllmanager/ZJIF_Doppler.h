#pragma once
//图像数值格式 0代表未扫查到的区域 1为有效频谱中的最低值，250为有效频谱中的最大值。255为基线值，254为MaxTrace标记，253为MeanTrace标记值
#include <stdarg.h>

class ZJIF_Doppler
{
public:
	enum EDopperParas
	{
		D_IDEL = 0,
		D_WALLFILTER,	//壁滤波器索引【0~3】，系统上由FPGA控制，不用 ×
		D_DYN,			//频谱的动态范围，【0~15】，目前固定为0 ×
		D_BASELINE,		//基线调节，基线正中为0，范围【-127~127】
		D_INVERT,		//频谱翻转，【0~1】
		D_STEER,		//偏转扫描，目前不用 ×
		D_FLOWANGLE,	//血流角度，目前不用 ×
		D_PRF,			//重复脉冲频率，单位Hz
		D_ITOUCH,		//频谱的一键优化，目前不用 ×
		D_IMGOPT,		//频谱的图像处理【0~3】
		D_SPEED,		//传1秒钟需要产生多少条pw线，float，【1~250】
		D_RES,			//频谱分辨率，目前固定为0
		D_GREYMAP,		//频谱的灰阶条，目前不用 ×
		D_TINTMAP,		//频谱的伪彩条，目前不用 ×
		D_LAYOUT,		//频谱的布局，目前不用 ×
		D_TRIPLEX,		//三同步（BD、BCD）模式，【0~1】，0为同步，1为单D
		D_AUTOTRACE,	//自动包络，目前不用 ×
		D_AUDIOVOLUME,	//频谱音量大小，【0~255】，0为关闭频谱
		D_TIMEPERPIXEL,	//OnlyGet, ms per pixel，由上位机自行计算，目前不用 ×
		D_DUMMY
	};

	typedef struct _TagComplexIQ
	{
		short I;
		short Q;
	} ComplexIQ;

	typedef struct _TagDParas
	{
		int Ver;	//版本号【1~】
		int prf;	//脉冲重复频率，单位Hz
		int wallfilter_id; //壁滤波ID 目前固定为0 ×
		int invert;	//频谱翻转，【0~1】
		int baseline;	//基线调节，基线正中为0，范围【-127~127】
		int dyn_id;		//频谱的动态范围，【0~15】，目前固定为0 ×
		int triplex;	//三同步（BD、BCD）模式，【0~1】，0为同步，1为单D
		int audiovolume;//频谱音量大小，【0~255】，0为关闭频谱
		int resolution;	//频谱分辨率，【0~4】目前固定为0 ×
		int imgopt_id;	//频谱的图像处理【0~3】
		float linepersec;//传1秒钟需要产生多少条pw线对应D_SPEED，【1~250】
		int greymap_id;	//频谱的灰阶条，目前不用【0-7】=0 ×
		int tintmap_id;	//频谱的伪彩条，目前不用【0-7】=0 ×
		int autotrace;	//自动包络，目前不用【0~1】 ×
		float steer;	//偏转扫描，目前不用 ×
		float flowangle;//血流角度，目前不用 ×
		int img_w;		//图像宽，建议768
		int img_h;		//图像高，建议256
//		RECT pw_roi;	//PW成像区域，目前不用 ×
		int Reversed[64];
	} DParas, *LPDParas;

	ZJIF_Doppler(){};
	virtual ~ZJIF_Doppler(){};
	virtual int DInit() = 0;
	virtual int DLoadParas(DParas *pparas) = 0;
	virtual int DLoadParasFromFile(char *file) = 0;
	virtual int DSaveParasToFile(char *file) = 0;
	//virtual int LoadIQDataFromFile(char *file) = 0;
	virtual int DopplerProcess(ComplexIQ *pIQData, int nLen, unsigned char *pImageData) = 0;
	virtual int DopplerProcessByWord(ComplexIQ *pIQData, int nLen, unsigned char *pImageData) = 0;
	virtual void DClearData() = 0;
	virtual int DSetPara(EDopperParas dpara, ...) = 0;
	virtual int DGetPara(EDopperParas dpara, ...) = 0;
	virtual int DSetPara2(EDopperParas dpara, va_list argls) = 0;
	virtual int DGetPara2(EDopperParas dpara, va_list argls) = 0;
	virtual int DReset() = 0;
	virtual void DEnd() = 0;
};
