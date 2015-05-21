// LsMatch.h: interface for the CLsMatch class.
#pragma once


#include "opencv\cv.h"
#include "highgui.h"
void Qsort(int start,int length,float a[]);
class CTargetTrack  
{
public:
	// ¸¨ÖúÄ¿±ê¸ú×Ù
	int             localMedianFlow(IplImage *pLastImage, IplImage *pCurrentImage, CvPoint2D32f *pLastPoint, CvPoint2D32f *point2);
	void			InitTpt(IplImage *im);
	int				TrackTest(IplImage *pCurrentImage, CvPoint2D32f *point2, IplImage *pTmpRslt);
	int				TrackCor(IplImage *pCurrentImage, CvPoint2D32f *point2);
	int				TrackOneImage(IplImage *pCurrentImage, CvPoint2D32f *point2);
	int				TrackMulMedianFlow(IplImage *pCurrentImage, CvPoint2D32f *point2);
	int			    FuseAndUpdate(IplImage *pCurrentImage, CvPoint2D32f *pt, int n);
	void			InitiateTrack(IplImage *pImage1, CvPoint2D32f point1);
	void			UpDateLastImg(IplImage *pImage1);
	void			UpDateTptImg(IplImage *pImage1);
	CvPoint2D32f	getResult();
	int             Update(IplImage *pCurrentImage, CvPoint2D32f pt);
	CTargetTrack();
	virtual ~CTargetTrack();

private:
	static const int    m_nMaskR= 25;        //Ä£°å°ë¾¶
	static const int    m_nSearchR= 70;
	static const int m_Margin_In  = 80;
	static const int m_Margin_Out = 60;
	int    m_IsInsideMargin;
	CvPoint2D32f tpt[4], tpt1[4], tpt2[4];
	IplImage               *m_pTptImage;
	IplImage               *m_pLastImage;
	IplImage               *m_pLastValidImage;
	CvPoint2D32f            m_LastPoint, m_LastValidPoint;
	CvMat                  *m_pAffMat;
	CvMat                  *m_pTptMat;
	static const int MAX_COUNT = 500;
	static const int MAX_IMG   = 2;
	static const int win_size = 4;
	int unUpdateCount;
	CvPoint2D32f* points[3];
	IplImage **IMG;
	IplImage **PYR;	
	static const int maxnPts = 441;
	float* bbpts;
	float* bbptsGuess;
	float *ncc;
	float *fb;
	float *sort_ncc;
	float *sort_fb;
	char  *status;
};  