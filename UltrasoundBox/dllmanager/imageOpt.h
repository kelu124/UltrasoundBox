#ifndef DSCOPT_H
#define DSCOPT_H

typedef unsigned char BYTE ;
class ZJIF_ImageOpt
{
public:
	typedef enum _TagEnumLineDirection
	{
		Horizontal = 0,
		Vertical = 1
	}EnumLineDirection;
	typedef enum _TagEnumProcPara
	{
		PARA_IDLE = 0,
		B_Persistence,		//0不做，1~6档可调
		B_PostProcess,		//0不做，1~4档可调
		C_Persistence,		//0不做，1~100档可调
		C_PostProcess,		//0不做，1~4档可调
		M_PostProcess,		//0不做，1~3档可调
		B_PersistResetData,	//重置B的帧相关
		C_PersistResetData,	//重置C的帧相关
		PARA_DUMMY
	} EnumProcPara;
public:
	ZJIF_ImageOpt(){};
	virtual ~ZJIF_ImageOpt(){};
	virtual int Ver()=0;
	virtual int Init(int b_w, int b_h, int c_w, int c_h, int m_w, EnumLineDirection dir) = 0;
	virtual int Set(EnumProcPara para, int value) = 0;
	virtual int Get(EnumProcPara para) = 0;
	virtual int Proc_B(BYTE *pDatain, BYTE *pDataout)=0;
	virtual int Proc_C(char *pDatain, char *pDataout)=0;
	virtual int Proc_C(char *pCDatain, BYTE *pBDatain, char *pCDataout,int isLinearProbe = 0)=0;//该接口只能在BC大小一致的情况下调用
	virtual int Proc_M(BYTE *pDatain, int lines, BYTE *pDataout) = 0;
	virtual void End() = 0;
};


//////加载定义
////extern lpImgOptFun pImgOptFun;

////extern ZJIF_ImageOpt *pImageOpt;

////int LoadImgOptLib(ZJIF_ImageOpt *pImgopt);
////void UnLoadLib();
////ZJIF_ImageOpt *imageOpt();
////void optUpParams(int bw , int bh, int cw, int ch, int mw, ZJIF_ImageOpt::_TagEnumLineDirection dir = ZJIF_ImageOpt::Horizontal, int perb = 1, int posb = 1, int perc = 10, int posc = 1);
#endif // DSCOPT_H

