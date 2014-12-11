#include "StdAfx.h"
#include "BlockMatchDirSub.h"
#include "Timer.h"
//#define _MATCH_DEBUG
using namespace cv;
using namespace std;
CBlockMatchDirSub::CBlockMatchDirSub(void)
{	
	m_hMat=cv::Mat::eye(3,3,CV_32FC1);
	m_patchR      = 10;
	m_patchW      = 2* m_patchR+ 1;
	m_pointInLarge= cvPoint2D32f(-1.0f,-1.0f);
	m_pointInSmall= cvPoint2D32f(-1.0f,-1.0f);
	m_smallRX= -1;
	m_smallRY= -1;
	m_pAngleLUT   = NULL;
	m_pLargeImg   = NULL;
	m_pSmallImg   = NULL;
	m_pOriginLargeImg   = NULL;
	m_pOriginSmallImg   = NULL;
	m_pPatchMask  = NULL;
}

CBlockMatchDirSub::~CBlockMatchDirSub(void)
{
	if (m_pAngleLUT!=NULL)
	{
		int i;
		for( i=0; i<255; i++ )
		{
			delete[] m_pAngleLUT[i];
		}
		delete[] m_pAngleLUT;
	}
	if (m_pPatchMask!= NULL)
	{
		delete[] m_pPatchMask;
	}
	if (m_pLargeImg!= NULL)
	{
		cvReleaseImage(&m_pLargeImg);
	}
	if (m_pSmallImg!= NULL)
	{
		cvReleaseImage(&m_pSmallImg);
	}
	if (m_pOriginLargeImg!= NULL)
	{
		cvReleaseImage(&m_pOriginLargeImg);
	}
	if (m_pOriginSmallImg!= NULL)
	{
		cvReleaseImage(&m_pOriginSmallImg);
	}
	if (m_samplepoints.size()!=0)
	{
		m_samplepoints.clear();
	}	
	if (m_simMaps.size()!=0)
	{
		m_simMaps.clear();
	}	
}

void CBlockMatchDirSub::InputImage(IplImage *pLargeImg, IplImage *pSmallImg)
{
	if (m_pLargeImg!= NULL)
	{
		cvReleaseImage(&m_pLargeImg);
	}
	if (m_pSmallImg!= NULL)
	{
		cvReleaseImage(&m_pSmallImg);
	}
	if (m_pOriginLargeImg!= NULL)
	{
		cvReleaseImage(&m_pOriginLargeImg);
	}
	if (m_pOriginSmallImg!= NULL)
	{
		cvReleaseImage(&m_pOriginSmallImg);
	}
	m_pLargeImg= cvCloneImage(pLargeImg);
	m_pSmallImg= cvCloneImage(pSmallImg);
	m_pOriginLargeImg= cvCloneImage(pLargeImg);
	m_pOriginSmallImg= cvCloneImage(pSmallImg);
	m_smallRX= (pSmallImg->width-1)/2.0f;
	m_smallRY= (pSmallImg->height-1)/2.0f;
}
void CBlockMatchDirSub::MemPrepare()
{
	// set LUT
	m_pPatchMask= new uchar[m_patchW*m_patchW];
	// set patch radius mask
	int i,j,idx;
	for (i=0;i<m_patchW;i++)
	{
		for (j=0;j<m_patchW;j++)
		{
			idx=i*m_patchW+j;
			if ( (i-m_patchR)*(i-m_patchR)+(j-m_patchR)*(j-m_patchR) <= m_patchR*m_patchR )
			{
				m_pPatchMask[idx]=1;
			}
			else
			{
				m_pPatchMask[idx]=0;
			}
		}
	}
	m_pAngleLUT = SetAngleLUT();
}

void CBlockMatchDirSub::GetSamplePoints()
{
	int i,j,m,n;
	int w= m_pSmallImg->width;
	int h= m_pSmallImg->height;
	int sd= m_patchW;	
	for (i=m_patchR;i<h-m_patchR;i+=sd)
	{
		for (j=m_patchR;j<w-m_patchR;j+=sd)
		{
			m=i;
			n=j;
			if (i>h-m_patchR-1)
			{
				m= h-m_patchR-1;
			}
			if (j>m_pSmallImg->width-m_patchR-1)
			{
				n= m_pSmallImg->width-m_patchR-1;
			}
			m_samplepoints.push_back(cvPoint(n,m));
		}
	}
}


int CBlockMatchDirSub::GetPatchSim_XY_window(BYTE *pRef, int refwstep, BYTE *pReal, int realwstep)
{
	int i,j,dref,dreal,sum=0;
	for (i=0;i<m_patchW;i++)
	{
		for (j=0;j<m_patchW;j++)
		{
			dref=*(pRef+i*refwstep+j);
			dreal=*(pReal+i*realwstep+j);
			sum = sum+ m_pAngleLUT[dreal][dref];//*m_pPatchMask[i];
		}
	}
	return sum;
}
void CBlockMatchDirSub::GetSimMap_XY()
{
	MemPrepare();
	GetSamplePoints();
	int i,j,m;
	//边缘图
	uchar* pRefImg= (uchar*)m_pLargeImg->imageData;
	int refw= m_pLargeImg->widthStep;
	int refh= m_pLargeImg->height; 
	int refsize   = refw* refh;
	uchar* pRealImg= (uchar*)m_pSmallImg->imageData;
	int realw= m_pSmallImg->widthStep;
	int realh= m_pSmallImg->height;
	int realsize  = realw* realh;

	cvSmooth(m_pLargeImg,m_pLargeImg,CV_GAUSSIAN,5);
	cvSmooth(m_pSmallImg,m_pSmallImg,CV_GAUSSIAN,5);
	Image_Sobel_Direction_Fast( pRefImg, refw, refh, 1 );
	Image_Sobel_Direction_Fast( pRealImg, realw, realh, 1 );

	//制作采样点

	int nPoints= m_samplepoints.size();

	CvPoint corner;		
	m_pointMat= cv::Mat::zeros(3,nPoints,CV_32FC1);
	float *pFloat;

	int srx=1+m_pLargeImg->width-m_pSmallImg->width;
	int sry=1+m_pLargeImg->height-m_pSmallImg->height;

	float sum;
	for (m=0;m<nPoints;m++)  // 对于每个采样点，匹配一次
	{
		m_simMaps.push_back(cv::Mat::zeros(sry,srx,CV_32FC1));
		pFloat= (m_simMaps.at(m)).ptr<float>(0);
		corner= m_samplepoints.at(m);
		// 采样点矩阵，后面将对采样点变换，求变换后的点坐标
		m_pointMat.at<float>(0,m)= (float)(corner.x-m_smallRX);
		m_pointMat.at<float>(1,m)= (float)(corner.y-m_smallRY);
		m_pointMat.at<float>(2,m)= 1.0f;
		for (i=0;i<sry;i++)
		{
			for (j=0;j<srx;j++)
			{
				sum = (float) (GetPatchSim_XY_window(pRefImg+(i+corner.y-m_patchR)* refw+j+corner.x-m_patchR, refw,
					pRealImg+(corner.y-m_patchR)* realw+corner.x-m_patchR, realw));
				pFloat[i*(srx)+j]= (float)sum;
			}
		}
//#ifdef _MATCH_DEBUG
//		{
//			IplImage *pSimMap=cvCreateImage(cvSize(srx,sry),8,1);	
//			uchar* pUChar= (uchar*)pSimMap->imageData;
//			char imagename[30],imagepath[50];
//			double amax,amin;
//			amax=0, amin= 99999999;
//			for (i=0;i<srx*sry;i++)
//			{
//				if (amin>pFloat[i])
//				{
//					amin= pFloat[i];
//				}
//				if (amax<pFloat[i])
//				{
//					amax= pFloat[i];
//				}
//			}
//			for(i= 0; i< pSimMap->height; i++)
//			{
//				for(j= 0; j< pSimMap->width; j++)
//				{
//					pUChar[i*pSimMap->widthStep+j]=(255.f*(pFloat[i*srx+j]-amin)/(amax-amin));
//				}
//			}
//			sprintf(imagename,"Tst_simPatch%d.bmp",m);
//			cvShowImage(imagename,pSimMap);		
//			//sprintf(imagepath,"d:\\data\\Tst_simPatch%d.bmp",m);
//			//cvSaveImage(imagepath,pSimMap);	
//			cvReleaseImage( &pSimMap );
//		}
//#endif
	}
}

void CBlockMatchDirSub::SpaceSearchMatch_4d()
{
	int m,x,y,x0=0,y0=0,cx,cy;
	int srx=1+m_pLargeImg->width-m_pSmallImg->width;
	int sry=1+m_pLargeImg->height-m_pSmallImg->height;
	//   a0,a1
	//   a2,a3
	float s=1.0,s0=1.0,r=0,r0=0,sim,maxsim=0;
	int nPoints= m_samplepoints.size();
	cv::Mat pointTransformedMat(3,nPoints,CV_32FC1);
	cv::Mat tmphMat=cv::Mat::eye(3,3,CV_32FC1);
	m_hMat=cv::Mat::eye(3,3,CV_32FC1);
	double meanTotal=0;
	int meanCnt=0;
	int cnt=0;
#ifdef _MATCH_DEBUG
	double *pResultData= new double[srx*sry*117];
#endif
	for (r=-0.28f;r<=0.28f;r+=0.07f) // 9
	{
		for (s=0.7f;s<=1.3f;s+=0.05f)  // 13
		{			
			tmphMat.at<float>(0,0)= s*cos(r);
			tmphMat.at<float>(1,1)= s*cos(r);
			tmphMat.at<float>(0,1)= -s*sin(r);
			tmphMat.at<float>(1,0)= s*sin(r);
			tmphMat.at<float>(0,2)= 0;
			tmphMat.at<float>(1,2)= 0;
			pointTransformedMat= tmphMat* m_pointMat;
			for (y=0;y<sry;y+=2)
			{
				for (x=0;x<srx;x+=2)
				{					
					//tmphMat.at<float>(0,2)= x;
					//tmphMat.at<float>(1,2)= y;
					//pointTransformedMat= tmphMat* m_pointMat;
					sim= 0;
					for (m=0;m<nPoints;m++)
					{
						cx= (int)(pointTransformedMat.at<float>(0,m)-m_pointMat.at<float>(0,m)+x+0.5);
						cy= (int)(pointTransformedMat.at<float>(1,m)-m_pointMat.at<float>(1,m)+y+0.5);
						if (cx<0||cx>=srx||cy<0||cy>=sry)
						{
							continue;
						}
						//simMap 间隔2 采样
						sim= sim+ (m_simMaps.at(m)).at<float>(cy,cx);
					}
					if (sim> maxsim)
					{
						maxsim= sim;
						x0=x;
						y0=y;
						s0=s;
						r0=r;
					}
					meanTotal+=sim;
					meanCnt ++;
#ifdef _MATCH_DEBUG
					pResultData[cnt*(srx*sry)+y*srx+x]= sim;
#endif					
				}
			}
			cnt++;
		}
	}
#ifdef _MATCH_DEBUG
	{
		IplImage *pSimMap=cvCreateImage(cvSize(srx,sry),8,1);	
		uchar* pUChar= (uchar*)pSimMap->imageData;
		char imagename[30],imagepath[50];
		double amax,amin;
		int i,j;
		for (m=0;m<cnt;m++)
		{
			amax=0, amin= 99999999;
			for (i=0;i<srx*sry;i++)
			{
				if (amin>pResultData[i] && pResultData[i]>0)
				{
					amin= pResultData[i];
				}
				if (amax<pResultData[i])
				{
					amax= pResultData[i];
				}
			}
			for (i=0;i<srx*sry;i++)
			{
				if (pResultData[i]<0.001)
				{
					pResultData[i]=amin;
				}
			}
			for(i= 0; i< pSimMap->height; i++)
			{
				for(j= 0; j< pSimMap->width; j++)
				{
					pUChar[i*pSimMap->widthStep+j]=(255.f*(pResultData[m*(srx*sry)+i*srx+j]-amin)/(amax-amin));
				}
			}
			sprintf(imagename,"Tst_simCom%d.bmp",m);
			cvShowImage(imagename,pSimMap);		
			//sprintf(imagepath,"d:\\data\\Tst_simCom%d.bmp",m);
			//cvSaveImage(imagepath,pSimMap);	
		}
		cvReleaseImage( &pSimMap );
	}
	delete []pResultData;
#endif
	int x1=x0,y1=y0;
	float r1=r0,s1=s0;
	for (r=r0-0.05f;r<=r0+0.05f;r+=0.01f) // 9
	{
		for (s=s0-0.05f;s<=s0+0.05f;s+=0.01f)  // 13
		{			
			tmphMat.at<float>(0,0)= s*cos(r);
			tmphMat.at<float>(1,1)= s*cos(r);
			tmphMat.at<float>(0,1)= -s*sin(r);
			tmphMat.at<float>(1,0)= s*sin(r);
			tmphMat.at<float>(0,2)= 0;
			tmphMat.at<float>(1,2)= 0;
			pointTransformedMat= tmphMat * m_pointMat;
			for (y=y0-3;y<=y0+3;y++)
			{
				for (x=x0-3;x<=x0+3;x++)
				{					
					sim= 0;
					for (m=0;m<nPoints;m++)
					{
						cx= (int)(pointTransformedMat.at<float>(0,m)-m_pointMat.at<float>(0,m)+x+0.5);
						cy= (int)(pointTransformedMat.at<float>(1,m)-m_pointMat.at<float>(1,m)+y+0.5);
						if (cx<0||cx>=srx||cy<0||cy>=sry)
						{
							continue;
						}
						sim+= (m_simMaps.at(m)).at<float>(cy,cx);
					}
					if (sim>= maxsim)
					{
						maxsim= sim;
						x1=x;
						y1=y;
						s1=s;
						r1=r;
					}
				}
			}
		}
	}
	meanTotal= meanTotal/double(meanCnt);
	m_belief= float(1.0- (meanTotal*0.5)/(maxsim-meanTotal*0.5));
	//FILE *fp = fopen("d:\\data\\BMDS.txt", "a" );
	//fprintf(fp,"匹配 x1:%3d, y1:%3d, r1:%4.3f, s1:%4.3f, max:%6.1f\n",x1,y1,r1,s1,maxsim);
	//fclose(fp);
	m_hMat.at<float>(0,0)= s1*cos(r1);
	m_hMat.at<float>(1,1)= s1*cos(r1);
	m_hMat.at<float>(0,1)= -s1*sin(r1);
	m_hMat.at<float>(1,0)= s1*sin(r1);
	m_hMat.at<float>(0,2)= x1+m_smallRX;
	m_hMat.at<float>(1,2)= y1+m_smallRY;
	m_Rot= r1;
	m_Scl= s1;
}


void CBlockMatchDirSub::Match2Layers(IplImage *imLarge, IplImage *imSmall, CvPoint2D32f *pointInLarge)
{
	int mr=105;
	float scx= imSmall->width/105.0f, scy= imSmall->height/(float)(mr);
	IplImage *im1= cvCreateImage(cvSize(int(imLarge->width/scx+0.5),
		int(imLarge->height/scy+0.5)),8,1);
	IplImage *im2= cvCreateImage(cvSize(mr,mr),8,1);
	cvResize(imLarge,im1);
	cvResize(imSmall,im2);
	InputImage(im1,im2);	
 	GetSimMap_XY();
	SpaceSearchMatch_4d();
	CvPoint2D32f pointinsmall,pointinlarge;
	pointinsmall= cvPoint2D32f((im2->width-1)/2.0f,(im2->height-1)/2.0f);
	GetMatchResult(3,&pointinlarge,&pointinsmall);
	ShowMatch();
	pointinlarge.x*=scx;
	pointinlarge.y*=scy;
	// 图像变换，细匹配
	if (m_pLargeImg!= NULL)
	{
		cvReleaseImage(&m_pLargeImg);
	}
	if (m_pSmallImg!= NULL)
	{
		cvReleaseImage(&m_pSmallImg);
	}
	m_pLargeImg= cvCloneImage(imLarge);
	m_pSmallImg= cvCloneImage(imSmall);
	pointinsmall= cvPoint2D32f((m_pSmallImg->width-1)/2.0f,(m_pSmallImg->height-1)/2.0f);
	pointinsmall.x= (float)(int)(pointinsmall.x+0.5);
	pointinsmall.y= (float)(int)(pointinsmall.y+0.5);
	pointinlarge.x= (float)(int)(pointinlarge.x+0.5);
	pointinlarge.y= (float)(int)(pointinlarge.y+0.5);
	rotateScaleImage(m_pSmallImg, m_Rot, m_Scl, &pointinsmall);
	int nxr= int(m_pSmallImg->width*0.4);
	int nyr= int(m_pSmallImg->height*0.4);
	int nw=2*nxr+1, nh=2*nyr+1;
	int srx=int(scx*8+0.5), sry=int(scy*8+0.5);
	IplImage *im3=cvCreateImage(cvSize(nw+srx*2, nh+sry*2),8,1);
	IplImage *im4=cvCreateImage(cvSize(nw,nh),8,1);
	int i,px=int(pointinsmall.x+0.5),py=int(pointinsmall.y+0.5);
	for (i=-nyr;i<=nyr;i++)
	{
		memcpy(im4->imageData+(i+nyr)*im4->widthStep,m_pSmallImg->imageData+
			(py+i)*m_pSmallImg->widthStep+px-nxr, nw);
	}
	for (i=-nyr-sry;i<=nyr+sry;i++)
	{
		memcpy(im3->imageData+(i+nyr+sry)*im3->widthStep,m_pLargeImg->imageData+
			(int)(pointinlarge.y+i)*m_pLargeImg->widthStep+(int)(pointinlarge.x-nxr-srx),
			nw+2*srx);
	}
	cvShowImage("im3",im3);
	cvShowImage("im4",im4);
	CvPoint2D32f pt2;		
	SimpleMatch2D(im3,im4,&pt2);
	pointInLarge->x= pointinlarge.x+pt2.x-srx;
	pointInLarge->y= pointinlarge.y+pt2.y-sry;	
	cvReleaseImage(&im1);
	cvReleaseImage(&im2);
	cvReleaseImage(&im3);
	cvReleaseImage(&im4);
}
void CBlockMatchDirSub::Match2Layers(cv::Mat matLarge, cv::Mat matSmall, CvPoint2D32f *pointInLarge)
{
	int mr=105;
	IplImage *imLarge,*imSmall;
	imLarge= &IplImage(matLarge);
	imSmall= &IplImage(matSmall);
	float scx= imSmall->width/105.0f, scy= imSmall->height/(float)(mr);
	IplImage *im1= cvCreateImage(cvSize(int(imLarge->width/scx+0.5),
		int(imLarge->height/scy+0.5)),8,1);
	IplImage *im2= cvCreateImage(cvSize(mr,mr),8,1);
	cvResize(imLarge,im1);
	cvResize(imSmall,im2);
	InputImage(im1,im2);	
	GetSimMap_XY();
	SpaceSearchMatch_4d();
	CvPoint2D32f pointinsmall,pointinlarge;
	pointinsmall= cvPoint2D32f((im2->width-1)/2.0f,(im2->height-1)/2.0f);
	GetMatchResult(3,&pointinlarge,&pointinsmall);
	ShowMatch();
	pointinlarge.x*=scx;
	pointinlarge.y*=scy;
	// 图像变换，细匹配
	if (m_pLargeImg!= NULL)
	{
		cvReleaseImage(&m_pLargeImg);
	}
	if (m_pSmallImg!= NULL)
	{
		cvReleaseImage(&m_pSmallImg);
	}
	m_pLargeImg= cvCloneImage(imLarge);
	m_pSmallImg= cvCloneImage(imSmall);
	pointinsmall= cvPoint2D32f((m_pSmallImg->width-1)/2.0f,(m_pSmallImg->height-1)/2.0f);
	pointinsmall.x= (float)(int)(pointinsmall.x+0.5);
	pointinsmall.y= (float)(int)(pointinsmall.y+0.5);
	pointinlarge.x= (float)(int)(pointinlarge.x+0.5);
	pointinlarge.y= (float)(int)(pointinlarge.y+0.5);
	rotateScaleImage(m_pSmallImg, m_Rot, m_Scl, &pointinsmall);
	int nxr= int(m_pSmallImg->width*0.4);
	int nyr= int(m_pSmallImg->height*0.4);
	int nw=2*nxr+1, nh=2*nyr+1;
	int srx=int(scx*8+0.5), sry=int(scy*8+0.5);
	IplImage *im3=cvCreateImage(cvSize(nw+srx*2, nh+sry*2),8,1);
	IplImage *im4=cvCreateImage(cvSize(nw,nh),8,1);
	int i,px=int(pointinsmall.x+0.5),py=int(pointinsmall.y+0.5);
	for (i=-nyr;i<=nyr;i++)
	{
		memcpy(im4->imageData+(i+nyr)*im4->widthStep,m_pSmallImg->imageData+
			(py+i)*m_pSmallImg->widthStep+px-nxr, nw);
	}
	for (i=-nyr-sry;i<=nyr+sry;i++)
	{
		memcpy(im3->imageData+(i+nyr+sry)*im3->widthStep,m_pLargeImg->imageData+
			(int)(pointinlarge.y+i)*m_pLargeImg->widthStep+(int)(pointinlarge.x-nxr-srx),
			nw+2*srx);
	}
	cvShowImage("im3",im3);
	cvShowImage("im4",im4);
	CvPoint2D32f pt2;		
	SimpleMatch2D(im3,im4,&pt2);
	pointInLarge->x= pointinlarge.x+pt2.x-srx;
	pointInLarge->y= pointinlarge.y+pt2.y-sry;	
	cvReleaseImage(&im1);
	cvReleaseImage(&im2);
	cvReleaseImage(&im3);
	cvReleaseImage(&im4);
}
void CBlockMatchDirSub::ManualPointMatch(CvPoint2D32f* plargepoint, CvPoint2D32f* psmallpoint)
{
	CvMat *AMat;	
	AMat= cvCreateMat(2,3,CV_32FC1);
	CvPoint2D32f *psmallpointShift= new CvPoint2D32f[3];
	int i;
	for (i=0;i<3;i++)
	{
		psmallpointShift[i].x= psmallpoint[i].x-m_smallRX;
		psmallpointShift[i].y= psmallpoint[i].y-m_smallRY;
	}

	cvGetAffineTransform(psmallpointShift,plargepoint,AMat);
	m_hMat.at<float>(0,0)= (float)cvmGet(AMat,0,0);
	m_hMat.at<float>(1,1)= (float)cvmGet(AMat,1,1);
	m_hMat.at<float>(0,1)= (float)cvmGet(AMat,0,1);
	m_hMat.at<float>(1,0)= (float)cvmGet(AMat,1,0);
	m_hMat.at<float>(0,2)= (float)cvmGet(AMat,0,2);
	m_hMat.at<float>(1,2)= (float)cvmGet(AMat,1,2);

	cvReleaseMat(&AMat);
}
void CBlockMatchDirSub::GetTransformedPoint(CvPoint2D32f OldPoint, CvPoint2D32f* pNewPoint)
{
	cv::Mat newPoint(3,1,CV_32FC1);
	cv::Mat oldPoint(3,1,CV_32FC1);
	oldPoint.at<float>(0,0)= OldPoint.x-m_smallRX;
	oldPoint.at<float>(1,0)= OldPoint.y-m_smallRY;
	oldPoint.at<float>(2,0)= 1.0f;
	newPoint= m_hMat* oldPoint;
	pNewPoint->x= newPoint.at<float>(0,0);
	pNewPoint->y= newPoint.at<float>(1,0);
}
void CBlockMatchDirSub::GetMatchResult(int targetMode,CvPoint2D32f *pointLarge,CvPoint2D32f *pointSmall)
{
	cv::Mat outputTargetPoint(3,1,CV_32FC1);
	cv::Mat inputTargetPoint(3,1,CV_32FC1);
	if (targetMode==1||targetMode==2)      // 大图变换到小图
	{
		cv::Mat hMatinv(3,3,CV_32FC1);
		invert(m_hMat,hMatinv);
		inputTargetPoint.at<float>(0,0)= pointLarge->x;
		inputTargetPoint.at<float>(1,0)= pointLarge->y;
		inputTargetPoint.at<float>(2,0)= 1.0f;
		outputTargetPoint= hMatinv* inputTargetPoint;
		pointSmall->x= outputTargetPoint.at<float>(0,0)+m_smallRX;
		pointSmall->y= outputTargetPoint.at<float>(1,0)+m_smallRY;
	}
	else if (targetMode==3||targetMode==4)  // 小图变换到大图
	{
		inputTargetPoint.at<float>(0,0)= pointSmall->x-m_smallRX;
		inputTargetPoint.at<float>(1,0)= pointSmall->y-m_smallRY;
		inputTargetPoint.at<float>(2,0)= 1.0f;
		outputTargetPoint= m_hMat* inputTargetPoint;
		pointLarge->x= outputTargetPoint.at<float>(0,0);
		pointLarge->y= outputTargetPoint.at<float>(1,0);
	}
	else
	{
		return;
	}
	m_pointInSmall= *pointSmall;
	m_pointInLarge= *pointLarge;
}

void CBlockMatchDirSub::ShowMatch(string filepath)
{
	//  灰度图替换边缘图，用于匹配，显示
	if (m_pLargeImg!= NULL)
	{
		cvReleaseImage(&m_pLargeImg);
	}
	if (m_pSmallImg!= NULL)
	{
		cvReleaseImage(&m_pSmallImg);
	}
	m_pLargeImg= cvCloneImage(m_pOriginLargeImg);
	m_pSmallImg= cvCloneImage(m_pOriginSmallImg);
	cv::Mat pointTransformedMat(3,4,CV_32FC1);
	cv::Mat smallcenterpoint(3,4,CV_32FC1);
	//  1:lt, 2:rt, 3:rb, 4:lb
	//lt
	smallcenterpoint.at<float>(0,0)= (float)(0-m_smallRX);
	smallcenterpoint.at<float>(1,0)= (float)(0-m_smallRY);
	smallcenterpoint.at<float>(2,0)= 1.0f;
	//rt
	smallcenterpoint.at<float>(0,1)= (float)(m_smallRX);
	smallcenterpoint.at<float>(1,1)= (float)(0-m_smallRY);
	smallcenterpoint.at<float>(2,1)= 1.0f;
	//rb
	smallcenterpoint.at<float>(0,2)= (float)(m_smallRX);
	smallcenterpoint.at<float>(1,2)= (float)(m_smallRY);
	smallcenterpoint.at<float>(2,2)= 1.0f;
	//lb
	smallcenterpoint.at<float>(0,3)= (float)(0-m_smallRX);
	smallcenterpoint.at<float>(1,3)= (float)(m_smallRY);
	smallcenterpoint.at<float>(2,3)= 1.0f;

	pointTransformedMat= m_hMat* smallcenterpoint;

	int linewidth=2;
	int lineR= 20;
	// 画图
	cvCircle (m_pLargeImg,
		cvPoint((int)(pointTransformedMat.at<float>(0,0)+0.5), (int)(pointTransformedMat.at<float>(1,0)+0.5)),
		3, cvScalar(255,0,0), 2);
	//大图画小图对应矩形区域
	cvLine(m_pLargeImg, 
		cvPoint((int)(pointTransformedMat.at<float>(0,0)+0.5), (int)(pointTransformedMat.at<float>(1,0)+0.5)), 
		cvPoint((int)(pointTransformedMat.at<float>(0,1)+0.5), (int)(pointTransformedMat.at<float>(1,1)+0.5)), 
		cvScalar(255,0,0), linewidth);
	cvLine(m_pLargeImg, 
		cvPoint((int)(pointTransformedMat.at<float>(0,2)+0.5), (int)(pointTransformedMat.at<float>(1,2)+0.5)), 
		cvPoint((int)(pointTransformedMat.at<float>(0,1)+0.5), (int)(pointTransformedMat.at<float>(1,1)+0.5)), 
		cvScalar(255,0,0), linewidth);
	cvLine(m_pLargeImg, 
		cvPoint((int)(pointTransformedMat.at<float>(0,0)+0.5), (int)(pointTransformedMat.at<float>(1,0)+0.5)), 
		cvPoint((int)(pointTransformedMat.at<float>(0,3)+0.5), (int)(pointTransformedMat.at<float>(1,3)+0.5)), 
		cvScalar(255,0,0), linewidth);
	cvLine(m_pLargeImg, 
		cvPoint((int)(pointTransformedMat.at<float>(0,2)+0.5), (int)(pointTransformedMat.at<float>(1,2)+0.5)), 
		cvPoint((int)(pointTransformedMat.at<float>(0,3)+0.5), (int)(pointTransformedMat.at<float>(1,3)+0.5)), 
		cvScalar(255,0,0), linewidth);
	//大图画十字丝
	CvPoint tp;
	tp= cvPoint((int)(m_pointInLarge.x+0.5), (int)(m_pointInLarge.y+0.5));
	cvLine(m_pLargeImg, cvPoint(tp.x-lineR, tp.y), cvPoint(tp.x+lineR, tp.y), cvScalar(255,0,0), linewidth);
	cvLine(m_pLargeImg, cvPoint(tp.x, tp.y-lineR), cvPoint(tp.x, tp.y+lineR), cvScalar(255,0,0), linewidth);		
	//小图画十字丝
	tp= cvPoint((int)(m_pointInSmall.x+0.5), (int)(m_pointInSmall.y+0.5));
	cvLine(m_pSmallImg, cvPoint(tp.x-lineR, tp.y), cvPoint(tp.x+lineR, tp.y), cvScalar(255,0,0), linewidth);
	cvLine(m_pSmallImg, cvPoint(tp.x, tp.y-lineR), cvPoint(tp.x, tp.y+lineR), cvScalar(255,0,0), linewidth);		
	
	float a0=m_hMat.at<float>(0,0);
	float a1=m_hMat.at<float>(0,1);
	float s0=sqrt(a0*a0+a1*a1);
	float r0=acos(a0/s0)*180/(float)(CV_PI);
	IplImage* pMergeImage= Merge2Images(m_pLargeImg,m_pSmallImg);
	cvShowImage("中间结果",pMergeImage);
	string str;
	if (filepath!="0")
	{
		cvSaveImage(str.c_str(),pMergeImage);
	}	
	cvReleaseImage(&pMergeImage);
}
void SimpleMatch2D( IplImage *pLargeImg, IplImage *pSmallImg, CvPoint2D32f *pMatchLoc, int schstep)
{
	uchar* pRefImg= (uchar*)pLargeImg->imageData;
	uchar* pRealImg= (uchar*)pSmallImg->imageData;
	int realw = pSmallImg->width;
	int realwstep= pSmallImg->widthStep;
	int realh= pSmallImg->height;

	int refw  = pLargeImg->width;
	int refwstep= pLargeImg->widthStep;
	int refh = pLargeImg->height;
	int srWidth, srHeight;
	srWidth = refw-realw+1;
	srHeight= refh-realh+1;
#ifdef _MATCH_DEBUG
	double *pResultData= new double[srWidth*srHeight];
#endif
	short** pAngleLUT= SetAngleLUT();

	cvSmooth(pLargeImg,pLargeImg,CV_GAUSSIAN,5);
	Image_Sobel_Direction_Fast( pRefImg, refwstep, refh, 1 );

	cvSmooth(pSmallImg,pSmallImg,CV_GAUSSIAN,5);
	Image_Sobel_Direction_Fast( pRealImg, realwstep, realh, 1 );

	int i,j;
	int sum, maxsum, minsum;
	minsum = 99999999;
	maxsum = 0;

	int io,jo;	
	for(i= 0; i< srHeight; i+=schstep)
	{
		for(j= 0; j< srWidth; j+=schstep)
		{
			sum = 0;
			sum = GetSim_fast( pRefImg+i*refwstep+j, refwstep,pRealImg,realw,realh,realwstep,pAngleLUT);
			if( sum < minsum )
			{
				minsum = sum;
			}
			if( sum > maxsum )
			{
				maxsum = sum;
				io = i;
				jo = j;
			}
#ifdef _MATCH_DEBUG
			pResultData[i*srWidth+j]= sum;
#endif
		}
	}
	
#ifdef _MATCH_DEBUG
	cout<<"XYMETHOD: ("<<pMatchLoc->x<<","<<pMatchLoc->y<<")"<<endl;
	IplImage *pSimMap=cvCreateImage(cvSize(((srWidth-1)+1+3)/4*4,(srHeight-1)+1),8,1);	
	uchar* pUChar= (uchar*)pSimMap->imageData;
	for(i= 0; i< pSimMap->height; i++)
	{
		for(j= 0; j< pSimMap->width; j++)
		{
			pUChar[i*pSimMap->widthStep+j]= (255.f * (pResultData[i*srWidth+j]-minsum)/(maxsum-minsum));
		}
	}	
	cvShowImage("sim map",pSimMap);	
	cvReleaseImage( &pSimMap );
#endif
	int i1=io,j1=jo;
	for(i= -2*schstep; i<= 2*schstep; i++)
	{
		for(j= -2*schstep; j<= 2*schstep; j++)
		{
			sum = 0;
			sum = GetSim_fast( pRefImg+(io+i)*refwstep+jo+j, refwstep,
				pRealImg,realw,realh,realwstep,pAngleLUT);
			if( sum < minsum )
			{
				minsum = sum;
			}
			if( sum > maxsum )
			{
				maxsum = sum;
				i1 = io+i;
				j1 = jo+j;
			}
		}
	}
	pMatchLoc->x=(float)jo;
	pMatchLoc->y=(float)io;
	// 清理内存
	for( i=0; i<255; i++ )
	{
		delete[] pAngleLUT[i];
	}
	delete[] pAngleLUT;
#ifdef _MATCH_DEBUG
	delete[] pResultData;
#endif
}
IplImage* Merge2Images(IplImage *pLargeImage,IplImage *pSmallImage)
{
	int w,h,dis=15;
	w= pLargeImage->width+pSmallImage->width+dis;
	h= MAX(pLargeImage->height,pSmallImage->height);
	IplImage *pMergeImage;
	pMergeImage= cvCreateImage(cvSize(w,h),pLargeImage->depth,pLargeImage->nChannels);
	memset(pMergeImage->imageData,0,pMergeImage->widthStep*pMergeImage->height);
	//	cvFillImage(pMergeImage,0);
	cvSetImageROI(pMergeImage,cvRect(0,0,pLargeImage->width,pLargeImage->height));
	cvCopy(pLargeImage,pMergeImage);
	cvSetImageROI(pMergeImage,cvRect(pLargeImage->width+dis,0,pSmallImage->width,pSmallImage->height));
	cvCopy(pSmallImage,pMergeImage);
	cvResetImageROI(pMergeImage);
	return pMergeImage;
}

int GetSim_fast(uchar *pRef, int refwstep, uchar *pReal, int realw, int realh, int realwstep, short** pLUT)
{
	int i,j,dref,dreal,sum=0;
	for (i=0;i<realh;i++)
	{
		for (j=0;j<realw;j++)
		{
			dref=*(pRef+i*refwstep+j);
			dreal=*(pReal+i*realwstep+j);
			sum = sum+ pLUT[dreal][dref];
		}
	}
	return sum;
}

// 相似变换
// 旋转 fRot    
// 放缩 fScale   
// 利用射影变换函数
IplImage* rotateScaleImage(IplImage* src, float fRot, float fScale, CvPoint2D32f *tarPt)
{
	int i,w=src->width,h=src->height;
	double angleRad= fRot*CV_PI/180;
	int newwidth= (int)( fabs(fScale* h* sin(angleRad))+fabs(fScale* w* cos(angleRad))+ 1 );
	int newheight=(int)( fabs(fScale* h* cos(angleRad))+fabs(fScale* w* sin(angleRad))+ 1 );
	IplImage *warpImage= cvCreateImage(cvSize(newwidth,newheight),src->depth,src->nChannels);

	CvPoint2D32f srcPts[4],dstPts[4];
	srcPts[0]= cvPoint2D32f(0,0);
	srcPts[1]= cvPoint2D32f(0,h);
	srcPts[2]= cvPoint2D32f(w,h);
	srcPts[3]= cvPoint2D32f(w,0);

	dstPts[0]= cvPoint2D32f(0,0);
	dstPts[1]= cvPoint2D32f(fScale*h*sin(angleRad),fScale*h*cos(angleRad));
	dstPts[2]= cvPoint2D32f(fScale*h*sin(angleRad)+fScale*w*cos(angleRad),
		fScale*h*cos(angleRad)-fScale*w*sin(angleRad));
	dstPts[3]= cvPoint2D32f(fScale*w*cos(angleRad),-fScale*w*sin(angleRad));

	float xoffset,yoffset;
	xoffset= MIN( MIN(dstPts[0].x,dstPts[1].x),MIN(dstPts[2].x,dstPts[3].x));
	yoffset= MIN( MIN(dstPts[0].y,dstPts[1].y),MIN(dstPts[2].y,dstPts[3].y));
	for(i=0;i<4;i++)
	{		
		dstPts[i].x -= xoffset;		
		dstPts[i].y -= yoffset;
	}

	CvMat *Amat=cvCreateMat(3,3,CV_32FC1);

	cvGetPerspectiveTransform(srcPts,dstPts,Amat);
	cvWarpPerspective(src,warpImage,Amat);

	if (tarPt!=NULL)
	{
		CvMat *Bmat=cvCreateMat(3,1,CV_32FC1);
		cvmSet(Bmat,0,0,tarPt->x);
		cvmSet(Bmat,1,0,tarPt->y);
		cvmSet(Bmat,2,0,1.0f);
		cvMatMul(Amat,Bmat,Bmat);
		tarPt->x= (float)(cvmGet(Bmat,0,0)/cvmGet(Bmat,2,0));
		tarPt->y= (float)(cvmGet(Bmat,1,0)/cvmGet(Bmat,2,0));
		cvReleaseMat(&Bmat);
	}

	cvReleaseMat(&Amat);

	return warpImage;
}
short** SetAngleLUT()
{
	// set LUT
	short** pAngleLUT = new short*[255];
	int i, j, temp;
	for( i=0; i<255; i++ )
	{
		pAngleLUT[i] = new short[255];
	}
	for( i=0; i<255; i++ )
	{
		for( j=0; j<255; j++ )
		{
			pAngleLUT[j][i] = 0;
			if( i > 1 && j > 1 )
			{
				temp = abs( i- j );
				if( temp > 90 )
					temp = 180- temp;
				if( temp < 45 )
				{
					//					pAngleLUT[j][i] = (45-temp)* (45-temp);
					pAngleLUT[j][i] = (45-temp)>0? (45-temp): (temp-45);
				}
			}
		}
	}
	return pAngleLUT;
}
// 梯度方向相关
//  梯度方向图
void Image_Sobel_Direction_Fast(uchar* pImage, int width, int height, int avgsize )
{
	int	i, j, x, y, index, Imagesize= width* height;
	double angle;
	int* pDx = new int[Imagesize];
	int* pDy = new int[Imagesize];
	uchar* pTemp= new uchar[Imagesize];
	for( x=1; x<width-1; x++ )
	{
		for( y=1; y<height-1; y++ )
		{
			index = y* width+ x;
			pDx[index] = pImage[index+1]- pImage[index-1];
			pDy[index] = pImage[index+width]- pImage[index-width];
		}
	}	
	memset( pTemp, 1, Imagesize );// 1 表示数据为空
	int arm = avgsize;
	int dx, dy, sumd, nG, vx, vy;
	for( x = arm+1; x < width - arm-1; x ++)
	{
		for( y = arm+1;  y < height - arm-1; y ++)
		{
			vx = vy = sumd = 0;
			for( i = -arm; i <= arm; i++ )
			{
				for( j = -arm; j <= arm; j++ )
				{
					index = (y+j)* width+ x+i;
					dx = pDx[index-width]+ 2*pDx[index]+ pDx[index+width];
					dy = pDy[index-1]+ 2*pDy[index]+ pDy[index+1];
					vx += 2* dx* dy;
					dx = dx* dx, dy = dy* dy;
					vy += dx- dy;
					sumd += dx+ dy;
				}
			}
			nG  = sumd/ (2*arm+1)/ (2*arm+1);
			angle = ( CV_PI- atan2( double(vy), vx ) )/*/ 2.0+PI/2*/;
			pTemp[y*width+x] = nG < 169 ? 0 : (unsigned char)( int(angle/ CV_PI* 90)+ 74); //74~253
		}
	}
	memcpy( pImage, pTemp, Imagesize );
	delete []pDx;
	delete []pDy;
	delete []pTemp;
}
void drawCross(IplImage *im, CvPoint2D32f tp)
{
	int linewidth=2;
	int lineR= 20;
	//大图画十字丝
	cvLine(im, cvPoint(tp.x-lineR, tp.y), cvPoint(tp.x+lineR, tp.y), cvScalar(255,0,0), linewidth);
	cvLine(im, cvPoint(tp.x, tp.y-lineR), cvPoint(tp.x, tp.y+lineR), cvScalar(255,0,0), linewidth);		
}