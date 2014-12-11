// PlaceTokenTrack.cpp: implementation of the CPlaceTokenMatch class.//
///////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "PTTrackLIB.h"
int      MagniC[ITERNUM] = {4, 2, 1};
int      TrackMag[3]     = {3, 1, 1};
BYTE    pBTEMPMASK[PT_MASKSIZE ];
short    pCurMASK[PT_MASKSIZE];
short    pTMask1 [PT_MASKSIZE];
short    pTMASK[3][PT_MASKSIZE];
short   pSTEMPMASK[PT_MASKSIZE];
short   pSTEMPMASK_2[PT_MASKSIZE];
double  Boeff[3][PT_NDIM][PT_EQUNUM];
double  pCoeff0[PT_EQUNUM][PT_NDIM];
double  pCoeff1[PT_EQUNUM][PT_NDIM];
double  pCoeff2[PT_EQUNUM][PT_NDIM];
double  pCoeffT0[PT_NDIM][PT_EQUNUM];
double  pCoeffT1[PT_NDIM][PT_EQUNUM];
double  pCoeffT2[PT_NDIM][PT_EQUNUM];
double  pTempA[PT_NDIM][PT_NDIM];
CTargetInfo::CTargetInfo()
{

}

CTargetInfo::~CTargetInfo()
{

}
/*******************************************************
   Zhuxianwei 

   状态修正
  *****************************************************/
void CTargetInfo::UpdateSelf(CFPt2D  NewPos)
{
	CFPt2D	*p  = &NewPos;
	Rat.x = 0;//(p->x - Pos.x + 1)/2;
	Rat.y = 0;//(p->y - Pos.y + 1)/2;
//	Rat.x = p0.Acc.x / 2.0 + p->x - p0.Pos.x - OffSet.x;
//	Rat.y = p0.Acc.y / 2.0 + p->y - p0.Pos.y - OffSet.y;
//	Acc.x = p0.Acc.x / 2.0 - p0.Rat.x + p->x - p0.Pos.x ;
//	Acc.y = p0.Acc.y / 2.0 - p0.Rat.y + p->y - p0.Pos.y ;
	Pos.x = p->x;
	Pos.y = p->y;
}
/*******************************************************
   Zhuxianwei

   位置预测
 *******************************************************/
CFPt2D  CTargetInfo::Forcast()
{
	CFPt2D  ForcastPos;
	ForcastPos.x = Pos.x + Rat.x;
	ForcastPos.y = Pos.y + Rat.y;
//	ForcastPos.x = Pos.x + Rat.x + OffSet.x;
//	ForcastPos.y = Pos.y + Rat.y + OffSet.y;	
//	ForcastPos.x = Pos.x + Rat.x + Acc.x / 2;
//	ForcastPos.y = Pos.y + Rat.y + Acc.y / 2;
	return      ForcastPos;
}

CPlaceTokenMatch::CPlaceTokenMatch()
{
	FILE *fid;
	fopen_s(&fid, "TURNHARDATA.HAR", "rb");
	if (!fid)
	{
		m_GaborOK= FALSE;
		return;
	}
	m_GaborOK= TRUE;
	fread(m_pGaborData,sizeof(short),PT_MASKSIZE * PT_EQUNUM / 2,fid);		
	fclose(fid);
	MaxCor   = 0;
	CurCor   = 0;
	PreCor   = 1;
	Unstable = 0;
}

//***********************************************************************
//            初始化：初始化第一幅图像中待跟踪目标的位置
//
//
//***********************************************************************
void  CPlaceTokenMatch::InitiateTrack(BYTE *pInImage,    //输入图像指针
							  	      CFPt2D  TargetPos)          //目标个数
{
	m_Target.Pos = TargetPos;
	m_Target.Rat = cvPoint2D32f(0, 0);
 	m_UpdateNum  = 0;

	Get_CImage_Rect(pTMask1, pInImage, int(TargetPos.x + 0.5), int(TargetPos.y + 0.5), 2);
	ResetCoeff0(pInImage, int(TargetPos.x), int(TargetPos.y));
}
void  CPlaceTokenMatch::InitiateTrack(cv::Mat matIm,    //输入图像指针
	CFPt2D  TargetPos)          //目标个数
{
	m_Target.Pos = TargetPos;
	m_Target.Rat = cvPoint2D32f(0, 0);
	m_UpdateNum  = 0;
	BYTE *pInImage= matIm.data;
	Get_CImage_Rect(pTMask1, pInImage, int(TargetPos.x + 0.5), int(TargetPos.y + 0.5), 2);
	ResetCoeff0(pInImage, int(TargetPos.x), int(TargetPos.y));
}
void  CPlaceTokenMatch::GetToken(short  *pInMask, double *pToken)
{
	int i, i_end = PT_EQUNUM / 2, istr, jstr, index;
	short *tmpSrc1;
	tmpSrc1 = m_pGaborData;
	for (i = 0; i < i_end; i++)
	{
		pToken[i] = V_InnerProduct(tmpSrc1, pInMask);
		tmpSrc1 += PT_MASKSIZE;
	}
/*
	short *G0 = new short[PT_MASKSIZE];
	tmpSrc1 = m_pGaborData;
	for (i = i_end; i < PT_EQUNUM; i++)
	{
		for(istr= 0; istr< PT_MASKD; istr++)
		{
			for(jstr= 0; jstr< PT_MASKD; jstr++)
			{
				index = istr * PT_MASKD + jstr;
				G0[index] = tmpSrc1[(PT_MASKD - 1 - jstr) * PT_MASKD + istr] ;		//生成Gabor函数独立源
			}
		}
		pToken[i] = V_InnerProduct(G0, pInMask);
		tmpSrc1 += PT_MASKSIZE;
	}
	delete[]G0;
*/

	//旋转图像模板,金字塔1，2，3层可仅做一次加以优化
	short *pRotateMask = new short[PT_MASKSIZE];
	int constant = PT_MASKD - 1;
	index = 0;
	for(istr = 0; istr < PT_MASKD; istr++)
	{
		for (jstr = 0; jstr < PT_MASKD; jstr++)
		{
			pRotateMask[index] = pInMask[jstr * PT_MASKD + constant - istr];
			index++;
		}
	}
	tmpSrc1 = m_pGaborData;
	for (i = i_end; i < PT_EQUNUM; i++)
	{
		pToken[i] = V_InnerProduct(tmpSrc1, pRotateMask);
		tmpSrc1 += PT_MASKSIZE;
	}
	delete[]pRotateMask;

// 	int i = 0;
// 	short *pGabD   = m_pGaborData + PT_EQUNUM * 6 * PT_MASKSIZE;
// 	short *tmpSrc1 = pSTEMPMASK;
// 	short *tmpSrc2 = pSTEMPMASK_2;
// 	Uint32 xId1 = DAT_copy(pGabD, tmpSrc1, PT_MASKSIZE<<1);
// 	pGabD += PT_MASKSIZE;
// 	for(; i< PT_EQUNUM-1; i+=2)
// 	{
// 		DAT_wait(xId1);
// 		xId1 = DAT_copy(pGabD, tmpSrc2, PT_MASKSIZE<<1);
// 		pGabD += PT_MASKSIZE;
// 		pToken[i] = V_InnerProduct(tmpSrc1, pInMask);
// 
// 		DAT_wait(xId1);
// 		xId1 = DAT_copy(pGabD, tmpSrc1, PT_MASKSIZE<<1);
// 		pGabD += PT_MASKSIZE;
// 		pToken[i+1] = V_InnerProduct(tmpSrc2, pInMask);
// 	}

}
void   MaskMinus(short* Mask0, short* Mask1, int datanum)
{
	int j = 0;
	for(; j< datanum; j++)
		Mask1[j] = Mask0[j] - Mask1[j];
}

//*************************************************************************
//           获得当前图像中目标的位置
//
//
//*************************************************************************
bool  CPlaceTokenMatch::TrackOneImage(BYTE *pInImage,    //输入图像指针
									  CFPt2D &TargetPos)  //存储获得的目标位置
{
	CFPt2D    CalcPos;
    int x0, y0, j;
	float     Men1 = 0;
	double	px = m_Target.Pos.x - (int)(m_Target.Pos.x + 0.5);
	double  py = m_Target.Pos.y - (int)(m_Target.Pos.y + 0.5);

//	CalcPos = m_Target.Forcast();
	CalcPos = m_Target.Pos;
    x0 = (int)(CalcPos.x + 0.5);
    y0 = (int)(CalcPos.y + 0.5);

    if(x0< 10 || y0< 10 || (PT_IMGW - x0)< 10 || (PT_IMGH - y0)< 10)
		return false;

	for(j= 0; j< 2; j++)
	{
		Get_CImage_Rect(pCurMASK, pInImage, x0, y0, TrackMag[j]);

		MaskMinus(pTMASK[j], pCurMASK, PT_MASKSIZE);
		GetToken(pCurMASK, m_pToken1);
		M_Multiply3(Boeff[j], m_pToken1, m_pAffineparm);
		CalcPos.x = x0 + (px * m_pAffineparm[0] + py * m_pAffineparm[1] + m_pAffineparm[4] + px) * TrackMag[j];
		CalcPos.y = y0 + (px * m_pAffineparm[2] + py * m_pAffineparm[3] + m_pAffineparm[5] + py) * TrackMag[j];	   
		x0 = (int)(CalcPos.x + 0.5);
		y0 = (int)(CalcPos.y + 0.5);
	}
	Get_CImage_Rect(pCurMASK, pInImage, x0, y0, TrackMag[2]);
	MaskMinus(pTMASK[1], pCurMASK, PT_MASKSIZE);
	GetToken(pCurMASK, m_pToken1);
	M_Multiply3(Boeff[1], m_pToken1, m_pAffineparm);
	CalcPos.x = x0 + (px * m_pAffineparm[0] + py * m_pAffineparm[1] + m_pAffineparm[4] + px) * TrackMag[j];
	CalcPos.y = y0 + (px * m_pAffineparm[2] + py * m_pAffineparm[3] + m_pAffineparm[5] + py) * TrackMag[j];	   
	x0 = (int)(CalcPos.x + 0.5);
	y0 = (int)(CalcPos.y + 0.5);

	Get_CImage_Rect(pCurMASK, pInImage, x0, y0, 2);

// 	GetMenSig(pCurMASK, &Men1, &Sig1, PT_DL0>>1);
// 	MaxCor = NormCor(pCurMASK, pTMask1, Men1, Sig1, PT_DL0>>1);

	MaxCor = GetCoe(pCurMASK, pTMask1, PT_MASKD, PT_MASKD);
	if((PreCor - MaxCor) < 0.38 || Unstable> 1)
	{
		Get_CImage_Rect(pTMask1, pInImage, x0, y0, 2);
		Unstable = 0;
		PreCor = MaxCor;
		m_Target.UpdateSelf(CalcPos);
		m_UpdateNum++;	
		TargetPos = CalcPos;
		ResetCoeff0(pInImage, x0, y0);
	}
	else if((PreCor - MaxCor)> 0.38)
	{
		Unstable++;		
	}

	return true;
}


void CPlaceTokenMatch::ResetCoeff0(BYTE *pInImg, int x0, int y0)//pInImg: cong DSP du ru de shu ju
{
	int istr, jstr, istr_end = PT_MASKD - 1, jstr_end = istr_end, i, i_end = PT_EQUNUM / 2;
	short *tmpSrc1, index, index2, TempConstant = PT_MASKD - 1;

	//short *Gx1 = new short[PT_MASKSIZE];
	//short *Gy1 = new short[PT_MASKSIZE];
	short *G0 = new short[PT_MASKSIZE];
	short *G1 = new short[PT_MASKSIZE];   //建议优化为4个for循环赋零
	short *G2 = new short[PT_MASKSIZE];
	short *G3 = new short[PT_MASKSIZE];
	short *G4 = new short[PT_MASKSIZE];
	short *G5 = new short[PT_MASKSIZE];
	short *G6 = new short[PT_MASKSIZE];
	short Gx1, Gy1, X0, Y0;
	short CenterX = PT_MASKD >> 1;
	short CenterY = PT_MASKD >> 1;;
	//memset(Gx1, 0, PT_MASKSIZE);   //建议优化为4个for循环赋零
	//memset(Gy1, 0, PT_MASKSIZE);
	memset(G0, 0, PT_MASKSIZE * sizeof(short));
	memset(G1, 0, PT_MASKSIZE * sizeof(short));
	memset(G2, 0, PT_MASKSIZE * sizeof(short));
	memset(G3, 0, PT_MASKSIZE * sizeof(short));
	memset(G4, 0, PT_MASKSIZE * sizeof(short));
	memset(G5, 0, PT_MASKSIZE * sizeof(short));
	memset(G6, 0, PT_MASKSIZE * sizeof(short));

	Get_CImage_Rect(pTMASK[0], pInImg, x0, y0, TrackMag[0]);
	Get_CImage_Rect(pTMASK[1], pInImg, x0, y0, TrackMag[1]);
	tmpSrc1 = m_pGaborData;
	for (i = 0; i < i_end; i++)
	{
		for (istr = 1; istr < istr_end; istr++)
		{
			Y0 = istr - CenterY;
			for (jstr = 1; jstr < jstr_end; jstr++)
			{
				X0 = jstr - CenterX;
				index = istr * PT_MASKD + jstr;
				Gx1 = (tmpSrc1[index + 1] - tmpSrc1[index - 1]) / 2;
				Gy1 = (tmpSrc1[index + PT_MASKD] - tmpSrc1[index - PT_MASKD]) / 2;
				G1[index] = short(-X0 * Gx1 - tmpSrc1[index]);
				G2[index] = short(-Y0 * Gx1);
				G3[index] = short(-X0 * Gy1);
				G4[index] = short(-Y0 * Gy1 - tmpSrc1[index]);
				G5[index] = short(-Gx1);
				G6[index] = short(-Gy1);

			}
		}

		pCoeffT0[0][i] = pCoeff0[i][0] = V_InnerProduct(G1, pTMASK[0]);
		pCoeffT1[0][i] = pCoeff1[i][0] = V_InnerProduct(G1, pTMASK[1]);

		pCoeffT0[1][i] = pCoeff0[i][1] = V_InnerProduct(G2, pTMASK[0]);	
		pCoeffT1[1][i] = pCoeff1[i][1] = V_InnerProduct(G2, pTMASK[1]);

		pCoeffT0[2][i] = pCoeff0[i][2] = V_InnerProduct(G3, pTMASK[0]);	
		pCoeffT1[2][i] = pCoeff1[i][2] = V_InnerProduct(G3, pTMASK[1]);

		pCoeffT0[3][i] = pCoeff0[i][3] = V_InnerProduct(G4, pTMASK[0]);	
		pCoeffT1[3][i] = pCoeff1[i][3] = V_InnerProduct(G4, pTMASK[1]);

		pCoeffT0[4][i] = pCoeff0[i][4] = V_InnerProduct(G5, pTMASK[0]);	
		pCoeffT1[4][i] = pCoeff1[i][4] = V_InnerProduct(G5, pTMASK[1]);

		pCoeffT0[5][i] = pCoeff0[i][5] = V_InnerProduct(G6, pTMASK[0]);	
		pCoeffT1[5][i] = pCoeff1[i][5] = V_InnerProduct(G6, pTMASK[1]);	

		tmpSrc1 += PT_MASKSIZE;
	}
/*
	tmpSrc1 = m_pGaborData;
	for (i = i_end; i < PT_EQUNUM; i++)
	{
		for(istr= 0; istr< PT_MASKD; istr++)
		{
			for(jstr= 0; jstr< PT_MASKD; jstr++)
			{
				index = istr * PT_MASKD + jstr;
				G0[index] = tmpSrc1[(TempConstant - jstr) * PT_MASKD + istr] ;		//生成Gabor函数独立源
			}
		}

		for (istr = 1; istr < istr_end; istr++)
		{
			Y0 = istr - CenterY;
			for (jstr = 1; jstr < jstr_end; jstr++)
			{
				X0 = jstr - CenterX;

				index = istr * PT_MASKD + jstr;

				Gx1 = (G0[index + 1] - G0[index - 1]) / 2;
				Gy1 = (G0[index + PT_MASKD] - G0[index - PT_MASKD]) / 2;

				G1[index] = short(-X0 * Gx1 - G0[index]);
				G2[index] = short(-Y0 * Gx1);
				G3[index] = short(-X0 * Gy1);
				G4[index] = short(-Y0 * Gy1 - G0[index]);
				G5[index] = short(-Gx1);
				G6[index] = short(-Gy1);

			}
		}

		pCoeffT0[0][i] = pCoeff0[i][0] = V_InnerProduct(G1, pTMASK[0]);
		pCoeffT1[0][i] = pCoeff1[i][0] = V_InnerProduct(G1, pTMASK[1]);

		pCoeffT0[1][i] = pCoeff0[i][1] = V_InnerProduct(G2, pTMASK[0]);	
		pCoeffT1[1][i] = pCoeff1[i][1] = V_InnerProduct(G2, pTMASK[1]);

		pCoeffT0[2][i] = pCoeff0[i][2] = V_InnerProduct(G3, pTMASK[0]);	
		pCoeffT1[2][i] = pCoeff1[i][2] = V_InnerProduct(G3, pTMASK[1]);

		pCoeffT0[3][i] = pCoeff0[i][3] = V_InnerProduct(G4, pTMASK[0]);	
		pCoeffT1[3][i] = pCoeff1[i][3] = V_InnerProduct(G4, pTMASK[1]);

		pCoeffT0[4][i] = pCoeff0[i][4] = V_InnerProduct(G5, pTMASK[0]);	
		pCoeffT1[4][i] = pCoeff1[i][4] = V_InnerProduct(G5, pTMASK[1]);

		pCoeffT0[5][i] = pCoeff0[i][5] = V_InnerProduct(G6, pTMASK[0]);	
		pCoeffT1[5][i] = pCoeff1[i][5] = V_InnerProduct(G6, pTMASK[1]);	

		tmpSrc1 += PT_MASKSIZE;
	}
*/

	tmpSrc1 = m_pGaborData;
	for (i = i_end; i < PT_EQUNUM; i++)
	{
		for (istr = 1; istr < istr_end; istr++)
		{
			Y0 = istr - CenterY;
			for (jstr = 1; jstr < jstr_end; jstr++)
			{
				X0 = jstr - CenterX;

				index = istr * PT_MASKD + jstr;
				index2 = (TempConstant - jstr) * PT_MASKD + istr;
				
				Gx1 = (tmpSrc1[index2 - PT_MASKD] - tmpSrc1[index2 + PT_MASKD]) / 2;
				Gy1 = (tmpSrc1[index2 + 1] - tmpSrc1[index2 - 1]) / 2;

				G1[index] = short(-X0 * Gx1 - tmpSrc1[index2]);
				G2[index] = short(-Y0 * Gx1);
				G3[index] = short(-X0 * Gy1);
				G4[index] = short(-Y0 * Gy1 - tmpSrc1[index2]);
				G5[index] = short(-Gx1);
				G6[index] = short(-Gy1);

			}
		}

		pCoeffT0[0][i] = pCoeff0[i][0] = V_InnerProduct(G1, pTMASK[0]);
		pCoeffT1[0][i] = pCoeff1[i][0] = V_InnerProduct(G1, pTMASK[1]);

		pCoeffT0[1][i] = pCoeff0[i][1] = V_InnerProduct(G2, pTMASK[0]);	
		pCoeffT1[1][i] = pCoeff1[i][1] = V_InnerProduct(G2, pTMASK[1]);

		pCoeffT0[2][i] = pCoeff0[i][2] = V_InnerProduct(G3, pTMASK[0]);	
		pCoeffT1[2][i] = pCoeff1[i][2] = V_InnerProduct(G3, pTMASK[1]);

		pCoeffT0[3][i] = pCoeff0[i][3] = V_InnerProduct(G4, pTMASK[0]);	
		pCoeffT1[3][i] = pCoeff1[i][3] = V_InnerProduct(G4, pTMASK[1]);

		pCoeffT0[4][i] = pCoeff0[i][4] = V_InnerProduct(G5, pTMASK[0]);	
		pCoeffT1[4][i] = pCoeff1[i][4] = V_InnerProduct(G5, pTMASK[1]);

		pCoeffT0[5][i] = pCoeff0[i][5] = V_InnerProduct(G6, pTMASK[0]);	
		pCoeffT1[5][i] = pCoeff1[i][5] = V_InnerProduct(G6, pTMASK[1]);	

		tmpSrc1 += PT_MASKSIZE;
	}

// 	int  i, xId1, ii;
// 
// 	short *tmpSrc1 = pSTEMPMASK_2;
// 	short *tmpSrc2 = pSTEMPMASK;
// 	short *pGabD   = m_pGaborData;//fu zhi wei cha zhao biao 
// 
// 	xId1 = DAT_copy(pGabD, tmpSrc1, PT_MASKSIZE<<1);
// 	pGabD += PT_MASKSIZE;
// 	Get_CImage_Rect(pTMASK[0], pInImg, x0, y0, TrackMag[0]);
// 	Get_CImage_Rect(pTMASK[1], pInImg, x0, y0, TrackMag[1]);
// 
// 	for( i= 0; i< PT_EQUNUM; i++)
// 	{
// 		DAT_wait(xId1);
// 		xId1 = DAT_copy(pGabD, tmpSrc2, PT_MASKSIZE<<1);
// 		pGabD += PT_MASKSIZE;
// 		pCoeffT0[0][i] = pCoeff0[i][0] = V_InnerProduct(tmpSrc1, pTMASK[0]);//cha zhao bian di yi hang	
// 		pCoeffT1[0][i] = pCoeff1[i][0] = V_InnerProduct(tmpSrc1, pTMASK[1]);	
// //		pCoeffT2[0][i] = pCoeff2[i][0] = V_InnerProduct(tmpSrc1, pTMASK[2]);	
// 		DAT_wait(xId1);
// 		xId1 = DAT_copy(pGabD, tmpSrc1, PT_MASKSIZE<<1);
// 		pGabD += PT_MASKSIZE;
// 		pCoeffT0[1][i] = pCoeff0[i][1] = V_InnerProduct(tmpSrc2, pTMASK[0]);	
// 		pCoeffT1[1][i] = pCoeff1[i][1] = V_InnerProduct(tmpSrc2, pTMASK[1]);	
// //		pCoeffT2[1][i] = pCoeff2[i][1] = V_InnerProduct(tmpSrc2, pTMASK[2]);	
// 		DAT_wait(xId1);
// 		xId1 = DAT_copy(pGabD, tmpSrc2, PT_MASKSIZE<<1);
// 		pGabD += PT_MASKSIZE;
// 		pCoeffT0[2][i] = pCoeff0[i][2] = V_InnerProduct(tmpSrc1, pTMASK[0]);	
// 		pCoeffT1[2][i] = pCoeff1[i][2] = V_InnerProduct(tmpSrc1, pTMASK[1]);	
// //		pCoeffT2[2][i] = pCoeff2[i][2] = V_InnerProduct(tmpSrc1, pTMASK[2]);	
// 		DAT_wait(xId1);
// 		xId1 = DAT_copy(pGabD, tmpSrc1, PT_MASKSIZE<<1);
// 		pGabD += PT_MASKSIZE;
// 		pCoeffT0[3][i] = pCoeff0[i][3] = V_InnerProduct(tmpSrc2, pTMASK[0]);	
// 		pCoeffT1[3][i] = pCoeff1[i][3] = V_InnerProduct(tmpSrc2, pTMASK[1]);	
// //		pCoeffT2[3][i] = pCoeff2[i][3] = V_InnerProduct(tmpSrc2, pTMASK[2]);	
// 		DAT_wait(xId1);
// 		xId1 = DAT_copy(pGabD, tmpSrc2, PT_MASKSIZE<<1);
// 		pGabD += PT_MASKSIZE;
// 		pCoeffT0[4][i] = pCoeff0[i][4] = V_InnerProduct(tmpSrc1, pTMASK[0]);	
// 		pCoeffT1[4][i] = pCoeff1[i][4] = V_InnerProduct(tmpSrc1, pTMASK[1]);	
// //		pCoeffT2[4][i] = pCoeff2[i][4] = V_InnerProduct(tmpSrc1, pTMASK[2]);	
// 		DAT_wait(xId1);
// 		xId1 = DAT_copy(pGabD, tmpSrc1, PT_MASKSIZE<<1);
// 		pGabD += PT_MASKSIZE;
// 		pCoeffT0[5][i] = pCoeff0[i][5] = V_InnerProduct(tmpSrc2, pTMASK[0]);	
// 		pCoeffT1[5][i] = pCoeff1[i][5] = V_InnerProduct(tmpSrc2, pTMASK[1]);	
// //		pCoeffT2[5][i] = pCoeff2[i][5] = V_InnerProduct(tmpSrc2, pTMASK[2]);	
// 	}
	
	M_Multiply1(pCoeffT0, pCoeff0, pTempA);
	M_Inversion(pTempA);
	M_Multiply2(pTempA, pCoeffT0, Boeff[0]);

	M_Multiply1(pCoeffT1, pCoeff1, pTempA);
	M_Inversion(pTempA);
	M_Multiply2(pTempA, pCoeffT1, Boeff[1]);
	
//	M_Multiply2(pTempA, pCoeffT1, Boeff[2]);
//	M_Multiply1(pCoeffT2, pCoeff2, pTempA);
//	M_Inversion(pTempA);
//	M_Multiply2(pTempA, pCoeffT2, Boeff[2]);

	//delete[]Gx1;
	//delete[]Gy1;
	delete[]G0;
	delete[]G1;
	delete[]G2;
	delete[]G3;
	delete[]G4;
	delete[]G5;
	delete[]G6;
}
double CPlaceTokenMatch::GetCoe(short *Mask0, short *Mask1, int Height, int Width)
{	
	double fm, gm;
	int MatrixSize = Height * Width;
	fm = gm = 0;
	int i, j, index;
	for(j = 0; j < Height; j++)
	{
		for(i = 0; i < Width; i++)
		{
			index = j * Width + i;
			fm += Mask0[index];
			gm += Mask1[index];
		}
	}
	fm /= double(MatrixSize);
	gm /= double(MatrixSize);
	double Sum0, Sum1, Sum2;
	Sum0 = Sum1 = Sum2 = 0;
	for(j = 0; j < Height; j++)
	{
		for(i = 0; i < Width; i++)
		{
			index = j * Width + i;
			Sum0 += (Mask0[index] - fm) * (Mask1[index] - gm);
			Sum1 += (Mask0[index] - fm) * (Mask0[index] - fm);
			Sum2 += (Mask1[index] - gm) * (Mask1[index] - gm);
		}
	}
	double x = Sum0 / sqrt(Sum1 * Sum2);
	return x;
}

// void CPlaceTokenMatch::GetMenSig( int *pData1,  float *pmen1, int4x16 *psig1, int DL)
// {
// 	int i = 0;
// 	int Test[4] = {0};
// 	int4x16 Sum1    = 0;
// 	int4x16 SumS1   = 0;
// 	int4x16 *p1 = (int4x16 *)pData1;
// 	for (; i< DL; ++i)
// 	{
// 		expand_i4x16_to_i32(p1[i], Test, Test + 1, Test + 2, Test + 3);
// 		SumS1 += Test[0] * Test[0] + Test[1] * Test[1] + Test[2] * Test[2] + Test[3] * Test[3];
// 		Sum1  += Test[0] + Test[1] + Test[2] + Test[3];
// 	}	
// 	*pmen1 = Sum1 / (float)(DL << 2);
// 	*psig1 = SumS1 - (Sum1) * Sum1 / (float)(DL << 2);
// }
// 
// float CPlaceTokenMatch::NormCor( int *pData1,  int *pData2, float men1, int4x16 sig1, int DL)
// {
// 	int i       = 0;
// 	int4x16 Sum2    = 0;
// 	int4x16 SumS2   = 0;
// 	int4x16 Sum12   = 0;
// 	int Test[8] = {0};
// 	int4x16 *p1 = (int4x16 *)pData1;
// 	int4x16 *p2 = (int4x16 *)pData2;
// 	for (; i< DL; ++i)
// 	{
// 		expand_i4x16_to_i32(p1[i], Test,     Test + 1, Test + 2, Test + 3);
// 		expand_i4x16_to_i32(p2[i], Test + 4, Test + 5, Test + 6, Test + 7);
// 		Sum12 += Test[0] * Test[4] + Test[1] * Test[5] + Test[2] * Test[6] + Test[3] * Test[7];
// 		SumS2 += Test[4] * Test[4] + Test[5] * Test[5] + Test[6] * Test[6] + Test[7] * Test[7];
// 		Sum2  += Test[4] + Test[5] + Test[6] + Test[7];
// 	}	
// 
// 	float Mean2 = Sum2 / (float)(DL << 2);
// 	int4x16 sig2  = SumS2 - Sum2 * Mean2;
// 	sig2 = sqrt(sig1 * sig2 * 1.0);
// 
// 	if(sig2 > 1 && sig1 > 1)
// 		return (Sum12 - men1 * Sum2) / sig2;
// 	else
// 		return 0;
// }
// void CPlaceTokenMatch::expand_i4x16_to_i32(int4x16 a, int *b0, int *b1, int *b2, int *b3)
// {
// 	*b0 = short((a & 0xFFFF000000000000) >> 48);
// 	*b1 = short((a & 0xFFFF00000000) >> 32);
// 	*b2 = short((a & 0xFFFF0000) >> 16);
// 	*b3 = short( a & 0xFFFF);
// }
void CPlaceTokenMatch::DAT_copy(void* pSrc, void* pDst, int datalen )
{
	memcpy(pDst,pSrc,datalen);
}
void CPlaceTokenMatch::Get_CImage_Rect(short *pMask, BYTE *pImage, short x, short y, short Magnitude)
{
	int toggle = 1, SRow, DRow;//pInImg: cong DSP du ru de shu ju
	BYTE *pBSrc;
	int i, j=0;
	int Cent = (PT_MASKD >> 1);//48
	int MagD = Cent * Magnitude;
	int BegX = x - MagD;
	if(BegX < 0)
		BegX = 0;
	int BegY = y - MagD;
	if(BegY < 0)
		BegY = 0; 
	BYTE *tmpSrc = pImage + PT_IMGW * BegY + BegX;//qi shi dian 
	int tmpW = PT_MASKD * Magnitude;//chuang kou kuan du
	DAT_copy(tmpSrc, (BYTE*)pBTEMPMASK, tmpW);//pBTEMPMASK tao chu de chuang kou???????
	tmpSrc += PT_IMGW * Magnitude;////////////////?????????????????????

	for(i= -Cent; i< Cent; i++)
	{
		if((y + i * Magnitude)> (PT_IMGH - 1))
		{
			memset(pMask + (i + Cent) * PT_MASKD, 0, PT_MASKD * (Cent - 1 - i)*2); 
			break;
		}
		if((y + i * Magnitude)< 0)
			memset(pMask + (i + Cent) * PT_MASKD, 0, PT_MASKD * 2); //////////////??????????
		else
		{
			DAT_copy(tmpSrc, pBTEMPMASK + toggle * tmpW, tmpW);/////?????
			tmpSrc += PT_IMGW * Magnitude;
			DRow = (i + Cent) * PT_MASKD + Cent;
			toggle ^= 1;
			pBSrc = (BYTE*)pBTEMPMASK + toggle * tmpW;
			SRow = 0;
			for(j= -Cent; j< Cent; j++)
			{
				if((x + j * Magnitude)> (PT_IMGW - 1))
				{
					memset(pMask + DRow + j, 0, (Cent - 1 - j)*2); 
					break;
				}
				if((x + j * Magnitude)< 0)
					pMask[DRow + j] = 0;
				else
				{
					pMask[DRow + j] = pBSrc[SRow];
					SRow += Magnitude;
				}
			}
		}
	}
}

void M_Transpose(double **MatI, int nRow, int nCol, double **MatO)
{
	int				i, j;

	for (j = 0; j < nRow; j++) 
	{
		for (i = 0; i < nCol; i++)  
		{
			MatO[i][j] = MatI[j][i];
		}
	}
}
// 矩阵相乘
void M_Multiply(double  **MatIa, double  **MatIb, double **MatO, int nRow, int nDim, int nCol)
{
	int		i, j, l;
	for (i = 0; i < nRow; i++)
	{
		for (j = 0; j < nCol; j++)
		{ 
			for (MatO[i][j] = 0.0, l = 0; l < nDim; l++)
				MatO[i][j] += MatIa[i][l] * MatIb[l][j];
		}
	}
}





double GetCoe(short* Mask0, short* Mask1, int datanum)
{
	int fm, gm;
	fm = gm = 0;
	int j;
	for(j = 0; j< datanum; j++)
	{
		fm += *(Mask0 + j);
		gm += *(Mask1 + j);
	}

	fm /= datanum;
	gm /= datanum;
	int Sum0, Sum1, Sum2;
	Sum0 = Sum1 = Sum2 = 0;
	for(j = 0; j< datanum; j++)
	{	
		Sum0 += (*(Mask0 + j) - fm) * (*(Mask1 + j) - gm);
		Sum1 += (*(Mask0 + j) - fm) * (*(Mask0 + j) - fm);
		Sum2 += (*(Mask1 + j) - gm) * (*(Mask1 + j) - gm);
	}
	double x = Sum0 / sqrt(Sum1 * (double)Sum2);
	return x;
}


int E_SolveGauss1(float	tt[PT_NDIM][PT_NDIM],	// 输入的系数方阵
	float	AffineP[PT_NDIM])	// 未知数个数
{

	int		l = 1, k, i, j, is, js[6];
	float   d, t;
	float   aa[PT_NDIM][PT_NDIM];
	for (k = 0; k < PT_NDIM; k++)
		memcpy(aa[k], tt[k], PT_NDIM * sizeof(float));
	for (k = 0; k < PT_NDIM - 1; k++)
	{
		d = 0.0;
		for (i = k; i < PT_NDIM; i++)
			for (j = k; j < PT_NDIM; j++)
			{
				t = fabs(aa[i][j]);
				if (t > d) 
				{ 
					d = t; 
					js[k] = j; 
					is = i;
				}
			}
			if (d == 0.0) 
			{
				l = 0;
			}
			else
			{
				if (js[k] != k)
				{
					for (i = 0; i < PT_NDIM; i++)
					{
						t			 = aa[i][k]; 
						aa[i][k]	 = aa[i][js[k]];
						aa[i][js[k]] = t;
					}
				}
				if (is != k)
				{
					for (j = k; j< PT_NDIM; j++)
					{
						t		  = aa[k][j];
						aa[k][j]  = aa[is][j];
						aa[is][j] = t;
					} 
					t	  = AffineP[k]; 
					AffineP[k]  = AffineP[is]; 
					AffineP[is] = t;
				}
			}
			if (l == 0)
				return(0);
			for (d = aa[k][k], j = k+1; j < PT_NDIM; j++)
			{ 
				aa[k][j] /= d;
			}
			AffineP[k] /= d;
			for (i = k+ 1; i < PT_NDIM; i++)
			{
				for (j = k + 1; j < PT_NDIM; j++)
				{
					aa[i][j] -= aa[i][k] * aa[k][j];
				}
				AffineP[i] -= aa[i][k] * AffineP[k];
			}
	} 
	d = aa[PT_NDIM - 1][PT_NDIM - 1];
	if (fabs(d) < 1E-10)
		return(0);
	AffineP[PT_NDIM - 1] /= d;
	for (i = PT_NDIM - 2; i >= 0; i--)
	{		
		for (t = 0.0, j = i + 1; j < PT_NDIM; j++)
			t += aa[i][j] * AffineP[j];
		AffineP[i] -= t;
	} 
	js[PT_NDIM-1] = PT_NDIM - 1;
	for (k = PT_NDIM - 1; k >= 0; k--)
	{
		if (js[k] != k)
		{ 
			t		 = AffineP[k]; 
			AffineP[k]	 = AffineP[js[k]];
			AffineP[js[k]] = t;
		}
	}
	return(1);
}



void M_Inversion(double MatIO[PT_NDIM][PT_NDIM])
{ 
	int		i, j, k, u, v;
	int		is[PT_NDIM];
	int		js[PT_NDIM];
	double	d, p;

	for (k = 0; k < PT_NDIM; k++)//zai  MatIO[PT_NDIM][PT_NDIM] zhong zhao dao cong  MatIO[k][k] kai shi de zui da yuan 
	{
		d = MatIO[k][k];
		is[k]	= k;
		js[k]	= k;
		for (i = k; i < PT_NDIM; i++)
		{        
			for (j = k; j < PT_NDIM; j++)
			{
				p = fabs(MatIO[i][j]);
				if (p > d)
				{
					d		= p;
					is[k]	= i;
					js[k]	= j;
				}
			}
		}
		//         if (fabs(d) < 1E-15) 
		// 			break;
		if (is[k] != k)
		{
			for (j = 0; j < PT_NDIM; j++)
			{
				v		= is[k];
				p		= MatIO[k][j];
				MatIO[k][j]	= MatIO[v][j];
				MatIO[v][j]	= p;
			}
		}
		if (js[k] != k)
		{
			for (i = 0; i < PT_NDIM; i++)
			{
				u		= js[k];
				p		= MatIO[i][k];
				MatIO[i][k]	= MatIO[i][u];// jiang zui da yuan huan dao dui jiao xian shang
				MatIO[i][u] = p;
			}
		}

		MatIO[k][k] = 1.0 / MatIO[k][k];

		for (j = 0; j < PT_NDIM; j++)	
			if (j != k) 
				MatIO[k][j] *= MatIO[k][k];

		for (i = 0; i < PT_NDIM; i++)
		{
			if (i != k) 
			{
				for (j = 0; j < PT_NDIM; j++) 
					if (j != k) 
						MatIO[i][j] -= MatIO[i][k] * MatIO[k][j];
			}
		}
		for (i = 0; i < PT_NDIM; i++)	
			if (i != k) 
				MatIO[i][k] *= -MatIO[k][k];
	}

	for (k = PT_NDIM - 1; k >= 0; k--)
	{
		if (js[k] != k)
		{
			for (j = 0; j < PT_NDIM; j++)
			{
				v		= js[k];
				p		= MatIO[k][j];
				MatIO[k][j]	= MatIO[v][j];
				MatIO[v][j]	= p;
			}
		}
		if (is[k] != k)
		{
			for (i = 0; i < PT_NDIM; i++)
			{
				u		= is[k];
				p		= MatIO[i][k];
				MatIO[i][k]	= MatIO[i][u];
				MatIO[i][u]	= p;
			}
		}
	}
}

int	V_InnerProduct(short *vecIa, short *vecIb)
{
	int     fTemp = 0;
	int     i = 0;
	//#pragma MUST_ITERATE(PT_MASKSIZE, PT_MASKSIZE);    
	for (; i < PT_MASKSIZE; i++)	
	{
		fTemp += vecIa[i] * vecIb[i];
	}   	
	return fTemp;
}

void M_Multiply1(double matIa[PT_NDIM][PT_EQUNUM], double matIb[PT_EQUNUM][PT_NDIM], double matO[PT_NDIM][PT_NDIM])
{ 
	int		i, j, l;
	for (i = 0; i < PT_NDIM; i++)
	{
		for (j = 0; j < PT_NDIM; j++)
		{ 
			for (matO[i][j] = 0.0, l = 0; l < PT_EQUNUM; l++)
				matO[i][j] += matIa[i][l] * matIb[l][j];
		}
	}
}
void M_Multiply2(double matIa[PT_NDIM][PT_NDIM], double matIb[PT_NDIM][PT_EQUNUM], double matO[PT_NDIM][PT_EQUNUM])
{ 
	int		i, j, l;
	for (i = 0; i < PT_NDIM; i++)
	{
		for (j = 0; j < PT_EQUNUM; j++)
		{ 
			for (matO[i][j] = 0.0, l = 0; l < PT_NDIM; l++)
				matO[i][j] += matIa[i][l] * matIb[l][j];
		}
	}
}

void M_Multiply3(double matIa[PT_NDIM][PT_EQUNUM], double matIb[PT_EQUNUM], double matO[PT_NDIM])
{ 
	int		i, j;
	for (i = 0; i < PT_NDIM; i++)
	{
		for (j = 0, matO[i]= 0; j < PT_EQUNUM; j++)
			matO[i] += matIa[i][j] * matIb[j];
	}
}








