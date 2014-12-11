#pragma once
#include <cv.h>
#include <highgui.h>

class CBlockMatchDirSub
{
public:
	CBlockMatchDirSub(void);
	~CBlockMatchDirSub(void);
private:
	int m_patchR;
	int m_patchW;
	float m_smallRX;
	float m_smallRY;
	CvPoint2D32f m_pointInLarge,m_pointInSmall;

	short **m_pAngleLUT;	
	uchar *m_pPatchMask;	
	IplImage *m_pLargeImg;
	IplImage *m_pSmallImg;
	IplImage *m_pOriginLargeImg;
	IplImage *m_pOriginSmallImg;

	
	cv::Mat m_pointMat;
	std::vector<CvPoint> m_samplepoints;
	std::vector<cv::Mat> m_simMaps;
	int  GetPatchSim_XY_window(uchar *pRef, int refwstep, uchar *pReal, int realwstep);
	void MemPrepare();
	void GetSamplePoints();
public:
	cv::Mat m_hMat;
	float m_Rot,m_Scl;
	float m_belief;
	void GetTransformedPoint(CvPoint2D32f OldPoint, CvPoint2D32f* pNewPoint);
	void InputImage(IplImage *pLargeImg, IplImage *pSmallImg);
	void GetSimMap_XY();
	void SpaceSearchMatch_4d();
	void ManualPointMatch(CvPoint2D32f* plargepoint, CvPoint2D32f* psmallpoint);
	void ShowMatch(std::string filepath="0");
	void GetMatchResult(int targetMode,CvPoint2D32f *pointLarge,CvPoint2D32f *pointSmall);			
	void Match2Layers(IplImage *imLarge, IplImage *imSmall, CvPoint2D32f *pointInLarge);
	void Match2Layers(cv::Mat matLarge, cv::Mat matSmall, CvPoint2D32f *pointInLarge);
};

IplImage* Merge2Images(IplImage *pLargeImage,IplImage *pSmallImage);
short** SetAngleLUT();
void Image_Sobel_Direction_Fast(uchar* pImage, int width, int height, int avgsize );
IplImage* rotateScaleImage(IplImage* src, float fRot,
	float fScale, CvPoint2D32f *tarPt=NULL);
int GetSim_fast(uchar *pRef, int refwstep, uchar *pReal, int realw, int realh, int realwstep, short** pLUT);
void SimpleMatch2D( IplImage *pLargeImg, IplImage *pSmallImg, CvPoint2D32f *pMatchLoc, int schstep=2);
void drawCross(IplImage *im, CvPoint2D32f tp);