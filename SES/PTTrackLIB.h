// PlaceTokenTrack.h: interface for the CPlaceTokenMatch class.
//
//////////////////////////////////////////////////////////////////////

#ifndef  _PTTRACKLIB_H_
#define  _PTTRACKLIB_H_
#include <cv.h>
#include <highgui.h>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
typedef unsigned char BYTE;
typedef CvPoint2D32f CFPt2D;
// 二维点的坐标类型定义
//class CFPt2D
//{
//public:
//	double	x;		// x坐标
//	double	y;		// y坐标
//
//	CFPt2D( ) { x= y= 0; };
//	CFPt2D( double Ix, double Iy ) { x= Ix; y= Iy; };
//};


#define PT_NDIM              6
#define PT_EQUNUM            48
#define PT_MASKD             96	      // 72
#define PT_MASKSIZE          9216//9216	    // 5184
#define ITERNUM  	         3

extern  BYTE      pBTEMPMASK[PT_MASKSIZE];//[THREEMASK];
extern  short     pTMASK[3][PT_MASKSIZE];
extern  short     pCurMASK[PT_MASKSIZE];
extern  short     pTMask1 [PT_MASKSIZE];

extern  short     pSTEMPMASK[PT_MASKSIZE];
extern  short     pSTEMPMASK_2[PT_MASKSIZE];

extern  double    Boeff[3][PT_NDIM][PT_EQUNUM];
extern  double    pCoeff0[PT_EQUNUM][PT_NDIM];
extern  double    pCoeff1[PT_EQUNUM][PT_NDIM];
extern  double    pCoeff2[PT_EQUNUM][PT_NDIM];
extern  double    pCoeffT0[PT_NDIM][PT_EQUNUM];
extern  double    pCoeffT1[PT_NDIM][PT_EQUNUM];
extern  double    pCoeffT2[PT_NDIM][PT_EQUNUM];
extern  double    pTempA[PT_NDIM][PT_NDIM];
//*****************************************************************************
//**                   PlaceToken跟踪方法                                    **
//**        1、    建立一个  PlaceTokenTrack 对象                            **
//**        2、    礗nitiateTrack()输入第一幅图象中目标的个数和各自的位置    **
//**        3、    调用TrackOneImage()开始跟踪，给出当前图像指针以及         **
//**               由于相机运动造成的目标在图像上的运动，输出目标在当前      **
//**               图像中的位置.                                             **                                 
//**                                                                         **
//**                                                                     :)  **
//*****************************************************************************

class CTargetInfo  
{
public:
	CTargetInfo();
	virtual ~CTargetInfo();

	void    UpdateSelf(CFPt2D);
	CFPt2D  Forcast();

	CFPt2D    Rat;
	CFPt2D    Pos;
};

class CPlaceTokenMatch  
{
public:
	CPlaceTokenMatch();
	void        InitiateTrack(IplImage *im, CFPt2D  TargetPos);
//	bool        TrackOneImage(BYTE *pInImg, CFPt2D &TargetPos);
	bool        TrackOneImage(IplImage *im, CFPt2D *TargetPos);
	void        GetToken(short  *pInMask, double *pToken);
	void        ResetCoeff0(BYTE *pInImg, int x0, int y0);

	void        DAT_copy(void* pSrc, void* pDst, int datalen );
	void        Get_CImage_Rect(short *pMask, BYTE *pImage, short x, short y, short Magnitude);

	CTargetInfo m_Target;
	float       MaxCor;
	float       CurCor;
	float       PreCor;
	int			Unstable;
	double      GetCoe(short *Mask0, short *Mask1, int Height, int Width);
	int         PT_IMGW;
	int         PT_IMGH;
	bool        m_GaborOK;
private:	
	short       m_pGaborData[PT_MASKSIZE * PT_EQUNUM / 2];
	double      m_pAffineparm[PT_NDIM];
private:
	int  		m_UpdateNum;
	double      m_pToken1[PT_EQUNUM];
};
// nDim维向量vecIa与nDim维向量vecIb的内积
double GetCoe(short* Mask0, short* Mask1, int datanum);
void   MaskMinus(short* Mask0, short* Mask1, int datanum);
int    V_InnerProduct   (short *vecIa, short   *vecIb);
//void    Get_Gabor_7Mask(CM_Short  Mask[7], CGaborParm Parm, int FunMode);

// 一般实方阵的逆矩阵(全选主元高斯—约当法)
void M_Inversion(double MatIO[PT_NDIM][PT_NDIM]);
void M_Multiply1(double matIa[PT_NDIM][PT_EQUNUM], double matIb[PT_EQUNUM][PT_NDIM], double matO[PT_NDIM][PT_NDIM]);
void M_Multiply2(double matIa[PT_NDIM][PT_NDIM],   double matIb[PT_NDIM][PT_EQUNUM], double matO[PT_NDIM][PT_EQUNUM]);
void M_Multiply3(double matIa[PT_NDIM][PT_EQUNUM], double matIb[PT_EQUNUM],          double matO[PT_NDIM]);

#endif // !defined(AFX_PLACETOKENTRACK_H__2EC15455_886D_4358_9146_265271FEF850__INCLUDED_)
