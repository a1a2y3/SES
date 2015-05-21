#include "StdAfx.h"
//////////////////////////////////////////////////////////////////////
#include "TargetTrack.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTargetTrack::CTargetTrack()
{
	m_IsInsideMargin  = 1;
	m_pLastImage      = NULL;
	m_pLastValidImage = NULL;
	m_pTptMat         = NULL;
	m_pAffMat         = cvCreateMat(3,3,CV_32FC1);
	cvmSet(m_pAffMat,0,0,1);
	cvmSet(m_pAffMat,0,1,0);
	cvmSet(m_pAffMat,0,2,0);
	cvmSet(m_pAffMat,1,0,0);
	cvmSet(m_pAffMat,1,1,1);
	cvmSet(m_pAffMat,1,2,0);
	cvmSet(m_pAffMat,2,0,0);
	cvmSet(m_pAffMat,2,1,0);
	cvmSet(m_pAffMat,2,2,1);	
	IMG = (IplImage**) calloc(MAX_IMG,sizeof(IplImage*));
	PYR = (IplImage**) calloc(MAX_IMG,sizeof(IplImage*));
	bbpts		= (float*) calloc(2*maxnPts,sizeof(float));
	bbptsGuess  = (float*) calloc(2*maxnPts,sizeof(float));
	points[0] = (CvPoint2D32f*)calloc(maxnPts,sizeof(CvPoint2D32f)); // template
	points[1] = (CvPoint2D32f*)calloc(maxnPts,sizeof(CvPoint2D32f)); // target
	points[2] = (CvPoint2D32f*)calloc(maxnPts,sizeof(CvPoint2D32f)); // forward-backward
	ncc    = (float*) calloc(maxnPts,sizeof(float));
	fb     = (float*) calloc(maxnPts,sizeof(float));
	sort_ncc    = (float*) calloc(maxnPts,sizeof(float));
	sort_fb     = (float*) calloc(maxnPts,sizeof(float));
	status = (char*)  calloc(maxnPts,sizeof(char));
}

CTargetTrack::~CTargetTrack()
{
	if(m_pLastImage != NULL)
	{
		cvReleaseImage(&m_pLastImage);
	}
	if(m_pLastValidImage != NULL)
	{
		cvReleaseImage(&m_pLastValidImage);
	}
	if(m_pAffMat != NULL)
	{
		cvReleaseMat(&m_pAffMat);
	}
	if(m_pTptMat!= NULL)
	{
		cvReleaseMat(&m_pTptMat);
	}
	free(IMG);
	free(PYR);
	free(bbpts);
	free(bbptsGuess);
	free((void*)(points[0]));
	free((void*)(points[1]));
	free((void*)(points[2]));
	free((void*)ncc);
	free((void*)fb);
	free((void*)sort_ncc);
	free((void*)sort_fb);
	free((void*)status);
}
void CTargetTrack::UpDateLastImg(IplImage *pImage1)
{
	if (m_pLastImage!=NULL)
	{
		cvReleaseImage(&m_pLastImage);
	}
	m_pLastImage= cvCloneImage(pImage1);
}
void CTargetTrack::UpDateTptImg(IplImage *pImage1)
{
	if (m_pTptImage!=NULL)
	{
		cvReleaseImage(&m_pTptImage);
	}
	m_pTptImage= cvCloneImage(pImage1);
}
void CTargetTrack::InitiateTrack(IplImage *pImage1, CvPoint2D32f point)
{
	if (m_pLastValidImage!=NULL)
	{
		cvReleaseImage(&m_pLastValidImage);
	}
	m_pLastValidImage= cvCloneImage(pImage1);
	m_LastPoint = point;
	unUpdateCount= 0;
}
int imVar(IplImage *pimg)
{
	int w= pimg->width;
	int h= pimg->height;
	int i,j,idx;
	int e=0,e2=0,g;
	for (i=0;i<h;i++)
	{
		for (j=0;j<w;j++)
		{
			idx= i*pimg->widthStep+j;
			g  = *(pimg->imageData+idx);
			e +=g;
			e2+=(g*g);
		}
	}
	e /=(w*h);
	e2/=(w*h);
	return e2-e*e;
}
int CTargetTrack::TrackCor(IplImage *pCurrentImage, CvPoint2D32f *point2)
{
	IplImage* pTargetImg=cvCreateImage(cvSize(m_nMaskR*2+1,m_nMaskR*2+1),8,1);
	cvGetRectSubPix( m_pLastValidImage, pTargetImg, m_LastPoint );
	IplImage *pSearchImg= cvCreateImage(cvSize(m_nMaskR*2+m_nSearchR*2+1,m_nMaskR*2+m_nSearchR*2+1),8,1);
	cvGetRectSubPix( pCurrentImage, pSearchImg, m_LastPoint );
	// 方差判断，防止图像为常数时相关系数除0
	if (imVar(pTargetImg)<10 || imVar(pSearchImg)<10)
	{
		cvReleaseImage(&pSearchImg);	
		cvReleaseImage(&pTargetImg);
		return 0;
	}
	////////////////////////////////////////////////////////////////////////////
	int iwidth = pSearchImg->width - pTargetImg->width + 1;
	int iheight = pSearchImg->height - pTargetImg->height + 1;
	IplImage *pResultImg = cvCreateImage(cvSize(iwidth,iheight),32,1);
	cvMatchTemplate( pSearchImg, pTargetImg, pResultImg, CV_TM_CCOEFF_NORMED);
//	cvNormalize(pResultImg,pResultImg,1,0,CV_MINMAX);
	double minVal; double maxVal; CvPoint minLoc; CvPoint maxLoc;
	cvMinMaxLoc( pResultImg, &minVal, &maxVal, &minLoc, &maxLoc);
	point2->x= maxLoc.x+m_LastPoint.x-m_nSearchR;
	point2->y= maxLoc.y+m_LastPoint.y-m_nSearchR;	
	cvReleaseImage(&pSearchImg);
	cvReleaseImage(&pResultImg);	
	cvReleaseImage(&pTargetImg);

	return 1;
}
int IsOutRegion(CvPoint2D32f pt, int rleft, int rright, int rbottom, int rtop)
{
	if (pt.x<rleft || pt.x>rright || pt.y<rbottom || pt.y>rtop)
	{
		return 1;
	}
	else
		return 0;
}
void CTargetTrack::InitTpt(IplImage *im)
{
	int w=im->width, h=im->height;
	tpt[0]= cvPoint2D32f(w/4, h/4);
	tpt[1]= cvPoint2D32f(w*3/4, h/4);
	tpt[2]= cvPoint2D32f(w/4, h*3/4);
	tpt[3]= cvPoint2D32f(w*3/4, h*3/4);
	tpt1[0]= tpt[0];
	tpt1[1]= tpt[1];
	tpt1[2]= tpt[2];
	tpt1[3]= tpt[3];
	if(m_pTptMat!= NULL)
	{
		cvReleaseMat(&m_pTptMat);
	}
	m_pTptMat= cvCloneMat(m_pAffMat);
}
// 9个点，跟踪
int CTargetTrack::TrackTest(IplImage *pCurrentImage, CvPoint2D32f *point2, IplImage *pTmpRslt)
{
	int i,mr= 20,w=pCurrentImage->width,h=pCurrentImage->height;
	IplImage* pTargetImg=cvCreateImage(cvSize(m_nMaskR*2+1,m_nMaskR*2+1),8,1);
	IplImage *pSearchImg= cvCreateImage(cvSize(m_nMaskR*2+m_nSearchR*2+1,m_nMaskR*2+m_nSearchR*2+1),8,1);
	int iwidth = pSearchImg->width - pTargetImg->width + 1;
	int iheight = pSearchImg->height - pTargetImg->height + 1;
	IplImage *pResultImg = cvCreateImage(cvSize(iwidth,iheight),32,1);
	double minVal; double maxVal; CvPoint minLoc; CvPoint maxLoc;

	memset(pTmpRslt->imageData, 0, pTmpRslt->imageSize);
	// 目标出视场
	 if (m_IsInsideMargin== 0)
	 {
		 // 选4个点,分别跟踪		 
		 double v[4];
		 
		 for (i=0;i<4;i++)
		 {
			 // 中值流
			 v[i]= localMedianFlow(m_pLastImage, pCurrentImage, tpt1+i, tpt2+i);
		 }
//		 if (v[0]>50 && v[1]>50 && v[2]>50)
		 {		
			// 预测目标位置
			 IplImage *pshowimg= cvCreateImage(cvSize(w,h),8,3);
			 cvCvtColor(pCurrentImage,pshowimg,CV_GRAY2RGB);
			 cvLine(pshowimg,cvPoint(tpt[0].x+0.5,tpt[0].y+0.5),cvPoint(tpt2[0].x+0.5,tpt2[0].y+0.5),cvScalar(0,0,255),3);
			 cvLine(pshowimg,cvPoint(tpt[1].x+0.5,tpt[1].y+0.5),cvPoint(tpt2[1].x+0.5,tpt2[1].y+0.5),cvScalar(0,0,255),3);
			 cvLine(pshowimg,cvPoint(tpt[2].x+0.5,tpt[2].y+0.5),cvPoint(tpt2[2].x+0.5,tpt2[2].y+0.5),cvScalar(0,0,255),3);
			 cvLine(pshowimg,cvPoint(tpt[3].x+0.5,tpt[3].y+0.5),cvPoint(tpt2[3].x+0.5,tpt2[3].y+0.5),cvScalar(0,0,255),3);
			 if (pTmpRslt!=NULL)
				 memcpy(pTmpRslt->imageData,pshowimg->imageData,pshowimg->imageSize);
//				 cvReleaseImage(&pTmpRslt);
//			 pTmpRslt= cvCloneImage(pshowimg);
//			 cvShowImage("pshowimg",pshowimg);
			 cvReleaseImage(&pshowimg);
		 	  
			 double oldMat[18], newMat[18];
			 //4点 射影模型
			 CvMat *BMat= cvCreateMat(3,3,CV_32FC1);	
			 CvMat *srcMat= cvCreateMat(4,2,CV_32FC1);
			 CvMat *dstMat= cvCreateMat(4,2,CV_32FC1);
			 for (i=0;i<4;i++)
			 {
				 cvmSet(srcMat,i,0,tpt[i].x);
				 cvmSet(srcMat,i,1,tpt[i].y);
				 cvmSet(dstMat,i,0,tpt2[i].x);
				 cvmSet(dstMat,i,1,tpt2[i].y);
			 }
			 cvFindHomography(srcMat,dstMat,BMat);	
			 cvmMul(BMat,m_pTptMat,m_pAffMat);
			 point2->x= cvmGet(m_pAffMat,0,0)*m_LastPoint.x+cvmGet(m_pAffMat,0,1)*m_LastPoint.y+cvmGet(m_pAffMat,0,2);
			 point2->y= cvmGet(m_pAffMat,1,0)*m_LastPoint.x+cvmGet(m_pAffMat,1,1)*m_LastPoint.y+cvmGet(m_pAffMat,1,2);
			 double t3 = cvmGet(m_pAffMat,2,0)*m_LastPoint.x+cvmGet(m_pAffMat,2,1)*m_LastPoint.y+cvmGet(m_pAffMat,2,2); 
			 point2->x/= t3;
			 point2->y/= t3;
			 newMat[0]= cvmGet(BMat,0,0);
			 newMat[1]= cvmGet(BMat,0,1);
			 newMat[2]= cvmGet(BMat,0,2);
			 newMat[3]= cvmGet(BMat,1,0);
			 newMat[4]= cvmGet(BMat,1,1);		 
			 newMat[5]= cvmGet(BMat,1,2);
			 newMat[6]= cvmGet(BMat,2,0);
			 newMat[7]= cvmGet(BMat,2,1);		 
			 newMat[8]= cvmGet(BMat,2,2);

			 oldMat[0]= cvmGet(m_pAffMat,0,0);
			 oldMat[1]= cvmGet(m_pAffMat,0,1);
			 oldMat[2]= cvmGet(m_pAffMat,0,2);
			 oldMat[3]= cvmGet(m_pAffMat,1,0);
			 oldMat[4]= cvmGet(m_pAffMat,1,1);		 
			 oldMat[5]= cvmGet(m_pAffMat,1,2);
			 oldMat[6]= cvmGet(m_pAffMat,2,0);
			 oldMat[7]= cvmGet(m_pAffMat,2,1);		 
			 oldMat[8]= cvmGet(m_pAffMat,2,2);	
			 cvReleaseMat(&srcMat);
			 cvReleaseMat(&dstMat);
			 cvReleaseMat(&BMat);

			 // 更新辅助点条件判断
			 int bOut=0;
			 for (i=0;i<4;i++)
			 {
				 bOut+= IsOutRegion(tpt2[i],25, w-26, 25, h-26);
			 }
			 if (bOut>0)
			 {
				 InitTpt(pCurrentImage);		
			 }
			 else
			 {
				 for (i=0;i<4;i++)
				 {
					 tpt1[i]= tpt2[i];
				 }
			 }
			 UpDateLastImg(pCurrentImage);
		 }
		 if (0==m_IsInsideMargin && 0==IsOutRegion(*point2, m_Margin_In, w-1-m_Margin_In, m_Margin_In, h-1-m_Margin_In))
		 {
			 // 图像变形，并相关匹配 （时间间隔可能较长，亮度变化大，光流不可靠）
			 IplImage *pWarpImg=cvCreateImage(cvSize(w,h),8,1);
			 cvWarpPerspective(m_pLastValidImage,pWarpImg,m_pAffMat);
			 // 相关, 考虑改用分块相关
			 CvPoint2D32f point3;
			 cvGetRectSubPix( pWarpImg, pTargetImg, *point2 );
			 cvGetRectSubPix( pCurrentImage, pSearchImg, *point2 );
			 cvMatchTemplate( pSearchImg, pTargetImg, pResultImg, CV_TM_CCOEFF_NORMED);
			 cvMinMaxLoc( pResultImg, &minVal, &maxVal, &minLoc, &maxLoc);
			 point3.x= maxLoc.x+point2->x-m_nSearchR;
			 point3.y= maxLoc.y+point2->y-m_nSearchR;	

//			 cvShowImage("warpimage",pWarpImg);
//			 cvShowImage("m_pLastValidImage",m_pLastValidImage);
			 cvShowImage("pTargetImg",pTargetImg);
			 //cvShowImage("pTarImgOri",pTarImgOri);
//			 cvWaitKey(0);
			 cvReleaseImage(&pWarpImg);
			 //cvReleaseImage(&pTarImgOri);

			 // 重新初始化跟踪
			 if (0==IsOutRegion(point3, m_Margin_Out, w-1-m_Margin_Out, m_Margin_Out, h-1-m_Margin_Out))
			 {
				 *point2= point3;
				 InitiateTrack(pCurrentImage, *point2);
				 m_IsInsideMargin=1;
			 }
			 
		 }
		//if (1==m_IsInsideMargin && 1==IsOutRegion(*point2, m_Margin_Out,w-1-m_Margin_Out,m_Margin_Out,h-1-m_Margin_Out))
		//{
		//	m_IsInsideMargin = 0;
		//}
	 }
	 // 目标在视场
	 else
	 {	
		CvPoint2D32f newPoint;
		int v= localMedianFlow(m_pLastValidImage, pCurrentImage, &m_LastPoint, &newPoint);
//		if (v>50)
		{
			*point2= newPoint;
			InitiateTrack(pCurrentImage, *point2);
		}
		if (1==IsOutRegion(*point2, m_Margin_Out,w-1-m_Margin_Out,m_Margin_Out,h-1-m_Margin_Out))
		{
			m_IsInsideMargin = 0;
			UpDateLastImg(pCurrentImage);
			InitTpt(pCurrentImage);
			m_LastPoint = *point2;

			cvGetRectSubPix( m_pLastValidImage, pTargetImg, *point2 );
			//cvShowImage("m_pLastValidImage",m_pLastValidImage);
			cvShowImage("pTargetImg",pTargetImg);
//				cvWaitKey(0);
		}
	 }
	 cvReleaseImage(&pSearchImg);
	 cvReleaseImage(&pResultImg);	
	 cvReleaseImage(&pTargetImg);
	return 1;
}

void euclideanDistance (CvPoint2D32f *point1, CvPoint2D32f *point2, float *match, int nPts) 
{
	int i;
	for (i = 0; i < nPts; i++) {

		match[i] = sqrt((point1[i].x - point2[i].x)*(point1[i].x - point2[i].x) + 
			(point1[i].y - point2[i].y)*(point1[i].y - point2[i].y) );

	}
}
void normCrossCorrelation(IplImage *imgI, IplImage *imgJ, CvPoint2D32f *points0, CvPoint2D32f *points1, 
						  int nPts, char *status, float *match,int winsize, int method) 
{
	int i;
	IplImage *rec0 = cvCreateImage( cvSize(winsize, winsize), 8, 1 );
	IplImage *rec1 = cvCreateImage( cvSize(winsize, winsize), 8, 1 );
	IplImage *res  = cvCreateImage( cvSize( 1, 1 ), IPL_DEPTH_32F, 1 );

	for (i = 0; i < nPts; i++) 
	{
		if (status[i] == 1) 
		{
			cvGetRectSubPix( imgI, rec0, points0[i] );
			cvGetRectSubPix( imgJ, rec1, points1[i] );
			cvMatchTemplate( rec0,rec1, res, method );
			match[i] = ((float *)(res->imageData))[0]; 
		} 
		else 
		{
			match[i] = 0.0;
		}
	}
	cvReleaseImage( &rec0 );
	cvReleaseImage( &rec1 );
	cvReleaseImage( &res );
}
void bb_points(CvPoint2D32f centerPoint, int d, int n, float *pts)
{
	int i,j,idx;
	float x=centerPoint.x, y=centerPoint.y;
	for (i=-n;i<=n;i++)
	{
		for (j=-n;j<=n;j++)
		{
			idx= (i+n)*(2*n+1)+j+n;
			pts[2*idx]  = x+j*d;
			pts[2*idx+1]= y+i*d; 
		}
	}

}
void quickSort(float* arr,int startPos, int endPos) 
{ 
	int i,j; 
	float key; 
	key=arr[startPos]; 
	i=startPos; 
	j=endPos; 
	while(i<j) 
	{ 
		while(arr[j]>=key && i<j)--j; 
		arr[i]=arr[j]; 
		while(arr[i]<=key && i<j)++i; 
		arr[j]=arr[i]; 
	} 
	arr[i]=key; 
	if(i-1>startPos) quickSort(arr,startPos,i-1); 
	if(endPos>i+1) quickSort(arr,i+1,endPos); 
} 
int CTargetTrack::localMedianFlow(IplImage *pLastImage, IplImage *pCurrentImage, CvPoint2D32f *pLastPoint, CvPoint2D32f *point2)
{
	int i, nPts= 441;
	int I       = 0;
	int J       = 1;
	int NCC_Winsize = 10;
	float medncc,medfb,dXsum,dYsum,cnt;
	IMG[I]= pLastImage;
	IMG[J]= pCurrentImage;
	PYR[I]= cvCreateImage( cvGetSize(pCurrentImage), 8, 1 );
	PYR[J]= cvCreateImage( cvGetSize(pCurrentImage), 8, 1 );
	bb_points(*pLastPoint, 4, 10, bbpts);
	bb_points(*pLastPoint, 4, 10, bbptsGuess);
	for (i = 0; i < nPts; i++) 
	{
		points[0][i].x = bbpts[2*i];        points[0][i].y = bbpts[2*i+1];
		points[1][i].x = bbptsGuess[2*i];   points[1][i].y = bbptsGuess[2*i+1];
		points[2][i].x = bbpts[2*i];        points[2][i].y = bbpts[2*i+1];
	}
	cvCalcOpticalFlowPyrLK( IMG[I], IMG[J], PYR[I], PYR[J], points[0], points[1], nPts, cvSize(win_size,win_size), 5, status, 0, cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), CV_LKFLOW_INITIAL_GUESSES);
	cvCalcOpticalFlowPyrLK( IMG[J], IMG[I], PYR[J], PYR[I], points[1], points[2], nPts, cvSize(win_size,win_size), 5, 0     , 0, cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), CV_LKFLOW_INITIAL_GUESSES | CV_LKFLOW_PYR_A_READY | CV_LKFLOW_PYR_B_READY );

	normCrossCorrelation(IMG[I],IMG[J],points[0],points[1],nPts, status, ncc, NCC_Winsize,CV_TM_CCOEFF_NORMED);
	euclideanDistance( points[0],points[2],fb,nPts);

	memcpy(sort_ncc,ncc,nPts*sizeof(float));
	memcpy(sort_fb, fb, nPts*sizeof(float));
	quickSort(sort_ncc,0,nPts-1) ;
	quickSort(sort_fb,0,nPts-1) ;

	medncc= sort_ncc[nPts/2];
	medfb = sort_fb [nPts/2];
	dXsum=0;
	dYsum=0;
	cnt= 0;
	for (i=0;i<nPts;i++)
	{
		if (ncc[i]>medncc && fb[i]<medfb && ncc[i]>0.5 && fb[i]<4)
		{
			dXsum+= (points[1][i].x-points[0][i].x);
			dYsum+= (points[1][i].y-points[0][i].y);
			cnt++;
		}
	}
	if (cnt<=nPts/8)
	{
		cvReleaseImage(&PYR[I]);
		cvReleaseImage(&PYR[J]);
		return 0;
	}
	else
	{
		dXsum/= cnt;
		dYsum/= cnt;
		point2->x= pLastPoint->x+dXsum;
		point2->y= pLastPoint->y+dYsum;
		cvReleaseImage(&PYR[I]);
		cvReleaseImage(&PYR[J]);
		return cnt;
	}
}

int CTargetTrack::TrackOneImage(IplImage *pCurrentImage, CvPoint2D32f *point2)
{
	int i, nPts= 441;
	int I       = 0;
	int J       = 1;
	int NCC_Winsize = 10;
	float medncc,medfb,dXsum,dYsum,cnt;
	IMG[I]= m_pLastValidImage;
	IMG[J]= pCurrentImage;
	PYR[I]= cvCreateImage( cvGetSize(pCurrentImage), 8, 1 );
	PYR[J]= cvCreateImage( cvGetSize(pCurrentImage), 8, 1 );
	bb_points(m_LastPoint, 5,10, bbpts);
	bb_points(m_LastPoint, 5,10, bbptsGuess);
	for (i = 0; i < nPts; i++) 
	{
		points[0][i].x = bbpts[2*i];        points[0][i].y = bbpts[2*i+1];
		points[1][i].x = bbptsGuess[2*i];   points[1][i].y = bbptsGuess[2*i+1];
		points[2][i].x = bbpts[2*i];        points[2][i].y = bbpts[2*i+1];
	}
	cvCalcOpticalFlowPyrLK( IMG[I], IMG[J], PYR[I], PYR[J], points[0], points[1], nPts, cvSize(win_size,win_size), 5, status, 0, cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), CV_LKFLOW_INITIAL_GUESSES);
	cvCalcOpticalFlowPyrLK( IMG[J], IMG[I], PYR[J], PYR[I], points[1], points[2], nPts, cvSize(win_size,win_size), 5, 0     , 0, cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), CV_LKFLOW_INITIAL_GUESSES | CV_LKFLOW_PYR_A_READY | CV_LKFLOW_PYR_B_READY );
	//  CV_TM_SQDIFF
	//  CV_TM_CCOEFF_NORMED
	normCrossCorrelation(IMG[I],IMG[J],points[0],points[1],nPts, status, ncc, NCC_Winsize,CV_TM_CCOEFF_NORMED);
	euclideanDistance( points[0],points[2],fb,nPts);

	memcpy(sort_ncc,ncc,nPts*sizeof(float));
	memcpy(sort_fb, fb, nPts*sizeof(float));
	quickSort(sort_ncc,0,nPts-1) ;
	quickSort(sort_fb,0,nPts-1) ;

	medncc= sort_ncc[nPts/2];
	medfb = sort_fb [nPts/2];
	dXsum=0;
	dYsum=0;
	cnt= 0;
	for (i=0;i<nPts;i++)
	{
		if (ncc[i]>medncc && fb[i]<medfb)
		{
			dXsum+= (points[1][i].x-points[0][i].x);
			dYsum+= (points[1][i].y-points[0][i].y);
			cnt++;
		}
	}
	if (cnt<=nPts/16)
	{
		*point2= m_LastPoint;
		cvReleaseImage(&PYR[I]);
		cvReleaseImage(&PYR[J]);
		InitiateTrack(pCurrentImage, *point2);
		return 0;
	}
	else
	{
		dXsum/= cnt;
		dYsum/= cnt;
		point2->x= m_LastPoint.x+dXsum;
		point2->y= m_LastPoint.y+dYsum;
		cvReleaseImage(&PYR[I]);
		cvReleaseImage(&PYR[J]);
		InitiateTrack(pCurrentImage, *point2);
		return 1;
	}
}
int CTargetTrack::TrackMulMedianFlow(IplImage *pCurrentImage, CvPoint2D32f *point2)
{
	int i, nPts= 441;
	int I       = 0;
	int J       = 1;
	int NCC_Winsize = 10;
	float medncc,medfb,dXsum,dYsum,cnt;
	IMG[I]= m_pLastValidImage;
	IMG[J]= pCurrentImage;
	PYR[I]= cvCreateImage( cvGetSize(pCurrentImage), 8, 1 );
	PYR[J]= cvCreateImage( cvGetSize(pCurrentImage), 8, 1 );
	bb_points(m_LastPoint, 4, 10, bbpts);
	bb_points(m_LastPoint, 4, 10, bbptsGuess);
	for (i = 0; i < nPts; i++) 
	{
		points[0][i].x = bbpts[2*i];        points[0][i].y = bbpts[2*i+1];
		points[1][i].x = bbptsGuess[2*i];   points[1][i].y = bbptsGuess[2*i+1];
		points[2][i].x = bbpts[2*i];        points[2][i].y = bbpts[2*i+1];
	}
	cvCalcOpticalFlowPyrLK( IMG[I], IMG[J], PYR[I], PYR[J], points[0], points[1], nPts, cvSize(win_size,win_size), 5, status, 0, cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), CV_LKFLOW_INITIAL_GUESSES);
	cvCalcOpticalFlowPyrLK( IMG[J], IMG[I], PYR[J], PYR[I], points[1], points[2], nPts, cvSize(win_size,win_size), 5, 0     , 0, cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), CV_LKFLOW_INITIAL_GUESSES | CV_LKFLOW_PYR_A_READY | CV_LKFLOW_PYR_B_READY );

	normCrossCorrelation(IMG[I],IMG[J],points[0],points[1],nPts, status, ncc, NCC_Winsize,CV_TM_CCOEFF_NORMED);
	euclideanDistance( points[0],points[2],fb,nPts);

	memcpy(sort_ncc,ncc,nPts*sizeof(float));
	memcpy(sort_fb, fb, nPts*sizeof(float));
	quickSort(sort_ncc,0,nPts-1) ;
	quickSort(sort_fb,0,nPts-1) ;

	medncc= sort_ncc[nPts/2];
	medfb = sort_fb [nPts/2];
	dXsum=0;
	dYsum=0;
	cnt= 0;
	for (i=0;i<nPts;i++)
	{
		if (ncc[i]>medncc && fb[i]<medfb && ncc[i]>0.5 && fb[i]<4)
		{
			dXsum+= (points[1][i].x-points[0][i].x);
			dYsum+= (points[1][i].y-points[0][i].y);
			cnt++;
		}
	}
	if (cnt<=nPts/8)
	{
		*point2= m_LastPoint;
		cvReleaseImage(&PYR[I]);
		cvReleaseImage(&PYR[J]);
		return 0;
	}
	else
	{
		dXsum/= cnt;
		dYsum/= cnt;
		point2->x= m_LastPoint.x+dXsum;
		point2->y= m_LastPoint.y+dYsum;
		cvReleaseImage(&PYR[I]);
		cvReleaseImage(&PYR[J]);
		return 1;
	}
}
int CTargetTrack::Update(IplImage *pCurrentImage, CvPoint2D32f pt)
{
	int i,id=0;
	int I       = 0;
	int J       = 1;
	IMG[I]= m_pLastValidImage;
	IMG[J]= pCurrentImage;
	// 计算相关系数
	IplImage *rec0 = cvCreateImage( cvSize(21, 21), 8, 1 );
	IplImage *rec1 = cvCreateImage( cvSize(21, 21), 8, 1 );
	IplImage *res  = cvCreateImage( cvSize( 1, 1 ), IPL_DEPTH_32F, 1 );
	float c2;
	cvGetRectSubPix( m_pLastValidImage, rec0, m_LastPoint );
	if (imVar(rec0)<5)
	{
		cvReleaseImage( &rec0 );
		cvReleaseImage( &rec1 );
		cvReleaseImage( &res );
		return 0;
	}
	cvGetRectSubPix( pCurrentImage, rec1, pt);
	if (imVar(rec1)<5)
	{
		cvReleaseImage( &rec0 );
		cvReleaseImage( &rec1 );
		cvReleaseImage( &res );
		return 0;
	}		
	cvMatchTemplate( rec0,rec1, res, CV_TM_CCOEFF_NORMED );
	c2= *(float *)(res->imageData);	
	// 相关系数大于0.6则更新模板	
	cvReleaseImage( &rec0 );
	cvReleaseImage( &rec1 );
	cvReleaseImage( &res );	
	if (c2<0.6) 
	{
		unUpdateCount++;
		return 0;
	}
	InitiateTrack(pCurrentImage, pt);
	return id+1;

}
int CTargetTrack::FuseAndUpdate(IplImage *pCurrentImage, CvPoint2D32f *pt, int n)
{
	int i,id=0;
	CvPoint2D32f tPt;
	int I       = 0;
	int J       = 1;
	IMG[I]= m_pLastValidImage;
	IMG[J]= pCurrentImage;
	// 计算相关系数
	IplImage *rec0 = cvCreateImage( cvSize(21, 21), 8, 1 );
	IplImage *rec1 = cvCreateImage( cvSize(21, 21), 8, 1 );
	IplImage *res  = cvCreateImage( cvSize( 1, 1 ), IPL_DEPTH_32F, 1 );
	float c1=0.0f,c2;
	cvGetRectSubPix( m_pLastValidImage, rec0, m_LastPoint );
	if (imVar(rec0)<5)
	{
		cvReleaseImage( &rec0 );
		cvReleaseImage( &rec1 );
		cvReleaseImage( &res );
		return 0;
	}
	for (i=0;i<n;i++)
	{
		cvGetRectSubPix( pCurrentImage, rec1, pt[i] );
		if (imVar(rec1)<5)
		{
			cvReleaseImage( &rec0 );
			cvReleaseImage( &rec1 );
			cvReleaseImage( &res );
			return 0;
		}		
		cvMatchTemplate( rec0,rec1, res, CV_TM_CCOEFF_NORMED );
		c2= *(float *)(res->imageData);	
		if (c1< c2)
		{
			c1= c2;
			tPt= pt[i];
			id=i;
		}
	}
	// 相关系数大于0.6则更新模板	
	cvReleaseImage( &rec0 );
	cvReleaseImage( &rec1 );
	cvReleaseImage( &res );	
	if (c1<0.6) 
	{
		unUpdateCount++;
		return 0;
	}
	InitiateTrack(pCurrentImage, tPt);
	return id+1;

}
CvPoint2D32f CTargetTrack::getResult()
{
	return m_LastPoint;
}