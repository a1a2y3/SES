
// SESDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SES.h"
#include "SESDlg.h"
#include "afxdialogex.h"
#include "BlockMatchDirSub.h"
#include "PTTrackLIB.h"
#include "TargetTrack.h"
#include "TLD.h"
#include "Timer.h"
#include "FileProcess.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define _CAM_ID 0
#define _TEST_MODE
#define _MATCH_BMDS
//  #define _TRACK_TLD
  #define _TRACK_PT     // placetoken tracker
//  #define _TRACK_MF     // median flow tracker
using namespace cv;

#ifdef _TRACK_TLD
TLD tracker;
#endif
#ifdef _TRACK_PT
CPlaceTokenMatch tracker;
#endif
#ifdef _TRACK_MF
CTargetTrack tracker;
#endif
CTimeCount tt;

enum videoType {nocam,useCam, useVideo, useSeries};

bool m_flipH= FALSE;
int m_FPS= 0;
bool m_videoHalt= FALSE;
bool m_DoMatch= FALSE;
bool m_DoTrack= FALSE;
CvPoint2D32f m_pnt= cvPoint2D32f(-1.0f,-1.0f);
bool m_Flag1= FALSE;
videoType m_videotype= nocam;
CString m_SeriesName;
int cnt=0;
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
void DrawRect(cv::Mat im, int x, int y, int r, int lineW)
{
	rectangle(im, Point2i(x-r, y-r), Point2i(x+r,y+r), CV_RGB(0, 255, 0), lineW);
}
void DrawCross(cv::Mat im, int x, int y, int r, int lineW)
{
	line(im,Point2i(x-r,y),Point2i(x+r,y), CV_RGB(0, 255, 0), lineW);
	line(im,Point2i(x,y-r),Point2i(x,y+r), CV_RGB(0, 255, 0), lineW);
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSESDlg �Ի���
CSESDlg::CSESDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSESDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSESDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSESDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_OPENCAREMA, &CSESDlg::OnBnClickedButtonOpencarema)
	ON_BN_CLICKED(IDC_BUTTON_CLOSECAMERA, &CSESDlg::OnBnClickedButtonClosecamera)
	ON_BN_CLICKED(IDC_BUTTON_OPENREAL, &CSESDlg::OnBnClickedButtonOpenreal)
	ON_BN_CLICKED(IDC_BUTTON_OPENREF, &CSESDlg::OnBnClickedButtonOpenref)
	ON_BN_CLICKED(IDC_BUTTON_OPENAVI, &CSESDlg::OnBnClickedButtonOpenavi)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CSESDlg::OnBnClickedButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_FLIPH, &CSESDlg::OnBnClickedButtonFliph)
	ON_BN_CLICKED(IDC_BUTTON_SHOOT, &CSESDlg::OnBnClickedButtonShoot)
	ON_BN_CLICKED(IDC_BUTTON_HALT, &CSESDlg::OnBnClickedButtonHalt)
	ON_BN_CLICKED(IDC_BUTTON_TRACK, &CSESDlg::OnBnClickedButtonTrack)
	ON_BN_CLICKED(IDC_BUTTON_OPENSERIESIMG, &CSESDlg::OnBnClickedButtonOpenseriesimg)
	ON_BN_CLICKED(IDC_BUTTON_VIDEO_SLOW, &CSESDlg::OnBnClickedButtonVideoSlow)
	ON_BN_CLICKED(IDC_BUTTON_VIDEO_FAST, &CSESDlg::OnBnClickedButtonVideoFast)
	ON_BN_CLICKED(IDC_BUTTON_SAVEREF, &CSESDlg::OnBnClickedButtonSaveref)
	ON_BN_CLICKED(IDC_BUTTON_TEST_TRACK, &CSESDlg::OnBnClickedButtonTestTrack)
END_MESSAGE_MAP()


// CSESDlg ��Ϣ�������

BOOL CSESDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CSESDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSESDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// ��ʾͼ��
		if (!m_mat1_display.empty())
			DrawMatToHDC(m_mat1_display, IDC_Showimg1);
		if (!m_mat_ref_display.empty())
			DrawMatToHDC(m_mat_ref_display, IDC_ShowImg2);
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSESDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSESDlg::DrawPicToHDC(IplImage *img, UINT ID)//----�ϰ汾��IplImage��ʽ����ʾ����
{
	CDC *pDC = GetDlgItem(ID)->GetDC();
	HDC hDC=  pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	CvvImage cimg;
	cimg.CopyOf(img);
	cimg.DrawToHDC(hDC,&rect);
	ReleaseDC(pDC);
}
void CSESDlg::DrawMatToHDC(cv::Mat img, UINT ID)//------��Ӧ�°�Mat ����ʾ����
{
	CDC *pDC = GetDlgItem(ID)->GetDC();
	HDC hDC=  pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	CvvImage cimg;
	IplImage ipm = img;
	cimg.CopyOf(&ipm);
	cimg.DrawToHDC_SCALE(hDC,&rect);
	ReleaseDC(pDC);
}
void CSESDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (m_videotype== useCam || m_videotype== useVideo)
	{
		m_cam>>m_mat1;
		if( !m_mat1.data )                                    // �ж��Ƿ�ɹ�����ͼƬ
			return;
	}
	else if (m_videotype== useSeries)
	{
		m_SeriesName = GetNextFileName(m_SeriesName);
		m_mat1 = cv::imread( LPCSTR(m_SeriesName), 1 );    // ��ȡͼƬ�����浽һ���ֲ����� ipl ��
		if( !m_mat1.data )                                    // �ж��Ƿ�ɹ�����ͼƬ
			return;
	}
	else    
	{
		return;       // nocam
	}
	if( m_mat1.empty())                                    // �ж��Ƿ�ɹ�����ͼƬ
		return;
	if (m_flipH)     cv::flip(m_mat1,m_mat1,1);
	m_mat1_display= m_mat1.clone();
	if (m_mat1.channels()==3)
		cvtColor(m_mat1,m_mat_gray,CV_RGB2GRAY);
	else
		m_mat_gray= m_mat1.clone();
	IplImage *im= &IplImage(m_mat_gray);
	if (m_DoTrack)
	{	
#ifdef _TEST_MODE
		cnt++;
		char fileName[25];
		sprintf(fileName,"D:\\data\\result\\%05d.jpg",cnt);
		imwrite(fileName,m_mat1);
#endif
		tt.Start();
		tracker.TrackOneImage(&(IplImage)(m_mat_gray),  &m_pnt);
		DrawCross(m_mat1_display,int(m_pnt.x+0.5),int(m_pnt.y+0.5),25,2);
		tt.End();
		char chEdit[8];
		sprintf(chEdit,"%4d",(int)(m_pnt.x+0.5));
		SetDlgItemText(IDC_EDIT_TARGET_X,chEdit);
		sprintf(chEdit,"%4d",(int)(m_pnt.y+0.5));
		SetDlgItemText(IDC_EDIT_TARGET_Y,chEdit);
		sprintf(chEdit,"%7.1f",tt.GetUseTime());
		SetDlgItemText(IDC_EDIT_USED_TIME,chEdit);
	}
	else 
	{
		if(m_videotype== useCam)
		{
			DrawCross(m_mat1_display,m_mat1_display.cols/2,m_mat1_display.rows/2,25,1);
			DrawRect(m_mat1_display,m_mat1_display.cols/2,m_mat1_display.rows/2,50,2);
		}
	}
	// ��ʾ
	DrawMatToHDC(m_mat1_display, IDC_Showimg1);
	//m_frame=cvQueryFrame(m_capture);
	//DrawPicToHDC(m_frame,IDC_Showimg1);
	//cvFlip(m_frame,m_frame,1);
	CDialogEx::OnTimer(nIDEvent);
}


void CSESDlg::OnBnClickedButtonOpencarema()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_cam.isOpened())
	{
		m_cam.release();
	}
	if (!(m_cam.open(_CAM_ID)))
	{
		MessageBox("�޷�������ͷ");
	}
	else
	{	
		m_videotype= useCam;
		// ��ʾͼ�����
		m_cam>>m_mat1;
		if( m_mat1.empty())                                    // �ж��Ƿ�ɹ�����ͼƬ
			return;
		if (m_flipH)     cv::flip(m_mat1,m_mat1,1);
		m_mat1_display= m_mat1.clone();
		// draw mask
		DrawCross(m_mat1_display,m_mat1_display.cols/2,m_mat1_display.rows/2,25,1);
		DrawRect(m_mat1_display,m_mat1_display.cols/2,m_mat1_display.rows/2,50,2);
		// convert color to gray
		if (m_mat1.channels()==3)
			cvtColor(m_mat1,m_mat_gray,CV_RGB2GRAY);
		else
			m_mat_gray= m_mat1.clone();
		char chEdit[10];
		_itoa_s(m_mat1.cols,chEdit,10);
		SetDlgItemText(IDC_EDIT_Realw,chEdit);
		_itoa_s(m_mat1.rows,chEdit,10);
		SetDlgItemText(IDC_EDIT_Realh,chEdit);
		DrawMatToHDC( m_mat1_display, IDC_Showimg1 ); 
		// ���ö�ʱ��
		m_FPS= 25;
		sprintf(chEdit,"%4d",m_FPS);
		SetDlgItemText(IDC_STATIC_FPS,chEdit);
		SetTimer(1, int(1000/(m_FPS+0.00001)+0.5), NULL);
	}
}


void CSESDlg::OnBnClickedButtonClosecamera()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_cam.isOpened())
	{
		m_videotype= nocam;
		m_cam.release();
	}
	KillTimer(1);
}


void CSESDlg::OnBnClickedButtonOpenreal()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	// ��ͼ��ǰ���ȹر�����ͷ
	if (m_cam.isOpened())
	{
		m_cam.release();
	}
	KillTimer(1);
	CFileDialog dlg(TRUE, _T("*.jpg"), NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
		NULL, NULL);                                        // ѡ��ͼƬ��Լ��
	dlg.m_ofn.lpstrTitle = _T("��ʵʱͼ");    // ���ļ��Ի���ı�����
	dlg.m_ofn.lpstrFilter=_T("image files (*.jpg) \0*.jpg\0image files (*.bmp)\0*.bmp\0All Files (*.*) \0*.*");
	if( dlg.DoModal() != IDOK )                    // �ж��Ƿ���ͼƬ
		return;

	CString mPath = dlg.GetPathName();            // ��ȡͼƬ·��
	m_mat1 = cv::imread( LPCSTR(mPath), 1 );    // ��ȡͼƬ�����浽һ���ֲ����� ipl ��
	if( !m_mat1.data )                                    // �ж��Ƿ�ɹ�����ͼƬ
		return;	
	m_mat1_display= m_mat1.clone();
	DrawMatToHDC( m_mat1_display, IDC_Showimg1 );            // ������ʾͼƬ���� 
	// ��ʾͼ�����
	char chEdit[10];
	_itoa_s(m_mat1.cols,chEdit,10);
	SetDlgItemText(IDC_EDIT_Realw,chEdit);
	_itoa_s(m_mat1.rows,chEdit,10);
	SetDlgItemText(IDC_EDIT_Realh,chEdit);
	
}


void CSESDlg::OnBnClickedButtonOpenref()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CFileDialog dlg(TRUE, NULL, NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
		NULL, NULL);                                        // ѡ��ͼƬ��Լ��
	dlg.m_ofn.lpstrTitle = _T("�򿪻�׼ͼ");    // ���ļ��Ի���ı�����
//	dlg.m_ofn.lpstrFilter=_T("image files (*.jpg) \0*.jpg\0image files (*.bmp)\0*.bmp\0All Files (*.*) \0*.*");
	if( dlg.DoModal() != IDOK )                    // �ж��Ƿ���ͼƬ
		return;

	CString mPath = dlg.GetPathName();            // ��ȡͼƬ·��
	m_mat_ref= cv::imread( LPCSTR(mPath), 1 );    // ��ȡͼƬ�����浽һ���ֲ����� ipl ��
	if( !m_mat_ref.data )                                    // �ж��Ƿ�ɹ�����ͼƬ
	{
		MessageBox("��ͼ�����");
		return;	
	}
	m_mat_ref_display= m_mat_ref.clone();
	if (m_mat_ref.channels()==3)
	{
		cvtColor(m_mat_ref,m_mat_ref_gray,CV_RGB2GRAY);
	}
	else
	{
		m_mat_ref_gray= m_mat_ref.clone();
	}
	
	CvPoint2D32f drawpoint= cvPoint2D32f((m_mat_ref_gray.cols-1)/2.0f,(m_mat_ref_gray.rows-1)/2.0f);
	drawCross(&IplImage(m_mat_ref_display),drawpoint);
	imshow("��׼ͼ",m_mat_ref);
	DrawMatToHDC( m_mat_ref_display, IDC_ShowImg2 );            // ������ʾͼƬ���� 
	// ��ʾͼ�����
	char chEdit[10];
	_itoa_s(m_mat_ref.cols,chEdit,10);
	SetDlgItemText(IDC_EDIT_Refw,chEdit);
	_itoa_s(m_mat_ref.rows,chEdit,10);
	SetDlgItemText(IDC_EDIT_Refh,chEdit);
}



void CSESDlg::OnBnClickedButtonOpenavi()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CFileDialog dlg(TRUE, NULL, NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
		NULL, NULL);                                        // ѡ��ͼƬ��Լ��
	dlg.m_ofn.lpstrTitle = _T("����Ƶ�ļ�");    // ���ļ��Ի���ı�����
//	dlg.m_ofn.lpstrFilter=_T("*.AVI\0*.AVI\0All Files (*.*) \0*.*");

	if( dlg.DoModal() != IDOK )                    // �ж��Ƿ���ͼƬ
		return;

	CString mPath = dlg.GetPathName();            // ��ȡͼƬ·��
	if (m_cam.isOpened())
	{
		m_cam.release();
	}
	if(!m_cam.open(LPCSTR(mPath)))
	{
		MessageBox("����Ƶ����");
	}
	else
	{
		m_videotype= useVideo;
		// ��ʾͼ�����
		m_cam>>m_mat1;
		if( m_mat1.empty())                                    // �ж��Ƿ�ɹ�����ͼƬ
			return;
		if (m_flipH)     cv::flip(m_mat1,m_mat1,1);
		m_mat1_display= m_mat1.clone();
		if (m_mat1.channels()==3)
			cvtColor(m_mat1,m_mat_gray,CV_RGB2GRAY);
		else
			m_mat_gray= m_mat1.clone();
		char chEdit[10];
		_itoa_s(m_mat1.cols,chEdit,10);
		SetDlgItemText(IDC_EDIT_Realw,chEdit);
		_itoa_s(m_mat1.rows,chEdit,10);
		SetDlgItemText(IDC_EDIT_Realh,chEdit);
		// ˢ����ʾ��
		DrawMatToHDC( m_mat1_display, IDC_Showimg1 ); 
		// ���ö�ʱ��
		m_FPS= 0;
		sprintf(chEdit,"%4d",m_FPS);
		SetDlgItemText(IDC_STATIC_FPS,chEdit);
		SetTimer(1, int(1000/(m_FPS+0.00001)+0.5), NULL);
	}
}

void CSESDlg::OnBnClickedButtonOpenseriesimg()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	// ��ͼ��ǰ���ȹر�����ͷ
	if (m_cam.isOpened())
	{
		m_videotype= nocam;
		m_cam.release();
	}
	KillTimer(1);
	CFileDialog dlg(TRUE, _T("*.jpg"), NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
		NULL, NULL);                                        // ѡ��ͼƬ��Լ��
	dlg.m_ofn.lpstrTitle = _T("��ʵʱͼ");    // ���ļ��Ի���ı�����
	dlg.m_ofn.lpstrFilter=_T("image files (*.jpg) \0*.jpg\0image files (*.bmp)\0*.bmp\0All Files (*.*) \0*.*");
	if( dlg.DoModal() != IDOK )                    // �ж��Ƿ���ͼƬ
		return;

	m_SeriesName = dlg.GetPathName();            // ��ȡͼƬ·��
	m_mat1 = cv::imread( LPCSTR(m_SeriesName), 1 );    // ��ȡͼƬ�����浽һ���ֲ����� ipl ��
	if( m_mat1.empty())                                    // �ж��Ƿ�ɹ�����ͼƬ
		return;
	if (m_flipH)     cv::flip(m_mat1,m_mat1,1);
	m_mat1_display= m_mat1.clone();
	if (m_mat1.channels()==3)
		cvtColor(m_mat1,m_mat_gray,CV_RGB2GRAY);
	else
		m_mat_gray= m_mat1.clone();
	DrawMatToHDC( m_mat1_display, IDC_Showimg1 );            // ������ʾͼƬ���� 
	m_videotype= useSeries;
	// ��ʾͼ�����
	char chEdit[10];
	_itoa_s(m_mat1.cols,chEdit,10);
	SetDlgItemText(IDC_EDIT_Realw,chEdit);
	_itoa_s(m_mat1.rows,chEdit,10);
	SetDlgItemText(IDC_EDIT_Realh,chEdit);
	m_FPS= 0;
	sprintf(chEdit,"%4d",m_FPS);
	SetDlgItemText(IDC_STATIC_FPS,chEdit);
	SetTimer(1, int(1000/(m_FPS+0.00001)+0.5), NULL);
}

void CSESDlg::OnBnClickedButtonExit()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnOK();
}


void CSESDlg::OnBnClickedButtonFliph()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_flipH= !m_flipH;
	if (!m_mat1.empty()) cv::flip(m_mat1,m_mat1,1);
	m_mat1_display= m_mat1.clone();
	if (!m_mat_gray.empty()) cv::flip(m_mat_gray,m_mat_gray,1);
	DrawMatToHDC(m_mat1_display, IDC_Showimg1);
}


void CSESDlg::OnBnClickedButtonShoot()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if( !m_mat1.data )                                    // �ж��Ƿ�ɹ�����ͼƬ
	{
		return;	
	}
	CFileDialog dlg(FALSE, _T("*.BMP"), NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
		_T("*.BMP"), NULL);                                        // ѡ��ͼƬ��Լ��
	dlg.m_ofn.lpstrTitle = _T("��Ƶ��ͼ");    // ���ļ��Ի���ı�����
	dlg.m_ofn.lpstrFilter=_T("image files (*.bmp) \0*.bmp\0image files (*.jpg)\0*.jpg\0All Files (*.*) \0*.*");
	if( dlg.DoModal() != IDOK )                    // �ж��Ƿ���ͼƬ
		return;

	CString mPath = dlg.GetPathName();            // ��ȡͼƬ·��
	cv::imwrite(LPCSTR(mPath),m_mat1);
}


void CSESDlg::OnBnClickedButtonHalt()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (!m_videoHalt)
	{
		SetTimer(1, 99999999, NULL);
		SetDlgItemText(IDC_BUTTON_HALT,"����");
	}
	else
	{
		SetTimer(1, int(1000/(m_FPS+0.00001)+0.5), NULL);
		SetDlgItemText(IDC_BUTTON_HALT,"��ͣ");
	}
	m_videoHalt= !m_videoHalt;
}
void CSESDlg::OnBnClickedButtonVideoSlow()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_FPS= m_FPS-5;
	if (m_FPS<=0)
	{
		m_FPS= 0;
	}
	char chEdit[5];
	sprintf(chEdit,"%4d",m_FPS);
	SetDlgItemText(IDC_STATIC_FPS,chEdit);
	SetTimer(1, int(1000/(m_FPS+0.00001)+0.5), NULL);
}


void CSESDlg::OnBnClickedButtonVideoFast()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_FPS= m_FPS+5;
	if (m_FPS>=200)
	{
		m_FPS= 200;
	}
	char chEdit[5];
	sprintf(chEdit,"%4d",m_FPS);
	SetDlgItemText(IDC_STATIC_FPS,chEdit);
	SetTimer(1, int(1000/(m_FPS+0.00001)+0.5), NULL);
}


void CSESDlg::OnBnClickedButtonTrack()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_mat_ref_gray.empty())
	{
		MessageBox("���ȴ򿪻�׼ͼ����Ƶ");
		return;
	}
	if (m_mat_gray.empty())
	{
		MessageBox("���ȴ�ʵʱͼ����Ƶ");
		return;
	}
	if (m_DoTrack)
	{
		m_DoTrack= FALSE;
		SetDlgItemText(IDC_BUTTON_TRACK,"����ƥ��/Ŀ�����");
		return;
	}
	// ƥ��
	tt.Start();
	CBlockMatchDirSub bmds;
	bmds.Match2Layers(&IplImage(m_mat_gray),&IplImage(m_mat_ref_gray),&m_pnt);
	tt.End();	
	// ����
#ifdef _TRACK_PT
	if (!tracker.m_GaborOK)
	{
		MessageBox("�Ҳ��� TURNHARDATA.HAR");
	}
	tracker.PT_IMGH= m_mat_gray.rows;
	tracker.PT_IMGW= m_mat_gray.cols;
#endif
	//m_pnt.y= m_mat_gray.rows/2;
	//m_pnt.x= m_mat_gray.cols/2;
	tracker.InitiateTrack(&(IplImage)(m_mat_gray), m_pnt);
	m_DoTrack= TRUE;
	SetDlgItemText(IDC_BUTTON_TRACK,"ֹͣ����");
	drawCross(&IplImage(m_mat1_display),m_pnt);
	DrawMatToHDC(m_mat1_display, IDC_Showimg1);
	char chEdit[8];
	sprintf(chEdit,"%4d",(int)(m_pnt.x+0.5));
	SetDlgItemText(IDC_EDIT_TARGET_X,chEdit);
	sprintf(chEdit,"%4d",(int)(m_pnt.y+0.5));
	SetDlgItemText(IDC_EDIT_TARGET_Y,chEdit);

	sprintf(chEdit,"%7.1f",tt.GetUseTime());
	SetDlgItemText(IDC_EDIT_USED_TIME,chEdit);
}

void CSESDlg::OnBnClickedButtonSaveref()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_mat1.empty())
	{
		return;
	}
	m_mat_ref.create(m_mat1.rows/2,m_mat1.rows/2,m_mat1.type());
	m_mat1(Rect(m_mat1.cols/4, m_mat1.rows/4, m_mat1.rows/2, m_mat1.rows/2)).copyTo(m_mat_ref);
	m_mat_ref_display= m_mat_ref.clone();
	// ��ʾ
	if (m_mat_ref.channels()==3)
	{
		cvtColor(m_mat_ref,m_mat_ref_gray,CV_RGB2GRAY);
	}
	else
	{
		m_mat_ref_gray= m_mat_ref.clone();
	}
	CvPoint2D32f drawpoint= cvPoint2D32f((m_mat_ref_gray.cols-1)/2.0f,(m_mat_ref_gray.rows-1)/2.0f);
	drawCross(&IplImage(m_mat_ref_display),drawpoint);
	imshow("��׼ͼ",m_mat_ref);
	DrawMatToHDC( m_mat_ref_display, IDC_ShowImg2 );            // ������ʾͼƬ���� 
	// ��ʾͼ�����
	char chEdit[10];
	_itoa_s(m_mat_ref.cols,chEdit,10);
	SetDlgItemText(IDC_EDIT_Refw,chEdit);
	_itoa_s(m_mat_ref.rows,chEdit,10);
	SetDlgItemText(IDC_EDIT_Refh,chEdit);
	// �����׼ͼ
	CFileDialog dlg(FALSE, _T("*.BMP"), NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
		_T("*.BMP"), NULL);                                        // ѡ��ͼƬ��Լ��
	dlg.m_ofn.lpstrTitle = _T("�����׼ͼ");    // ���ļ��Ի���ı�����
	dlg.m_ofn.lpstrFilter=_T("image files (*.bmp) \0*.bmp\0image files (*.jpg)\0*.jpg\0All Files (*.*) \0*.*");
	if( dlg.DoModal() != IDOK )                    // �ж��Ƿ���ͼƬ
		return;
	CString mPath = dlg.GetPathName();            // ��ȡͼƬ·��
	cv::imwrite(LPCSTR(mPath),m_mat_ref);
}


void CSESDlg::OnBnClickedButtonTestTrack()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_mat_gray.empty())
	{
		MessageBox("���ȴ�ʵʱͼ����Ƶ");
		return;
	}
	if (m_DoTrack)
	{
		m_DoTrack= FALSE;
		SetDlgItemText(IDC_BUTTON_TEST_TRACK,"�����㷨����");
		return;
	}
	// ����
#ifdef _TRACK_PT
	if (!tracker.m_GaborOK)
	{
		MessageBox("�Ҳ��� TURNHARDATA.HAR");
	}

	tracker.PT_IMGH= m_mat_gray.rows;
	tracker.PT_IMGW= m_mat_gray.cols;
#endif	
	m_pnt.y= m_mat_gray.rows/2;
	m_pnt.x= m_mat_gray.cols/2;
//	TLD *ptracker= new TLD;
	tracker.InitiateTrack(&(IplImage)(m_mat_gray), m_pnt);
	cnt= 0;
	m_DoTrack= TRUE;
	SetDlgItemText(IDC_BUTTON_TEST_TRACK,"ֹͣ����");
	drawCross(&IplImage(m_mat1_display),m_pnt);
	imshow("�����㷨����",m_mat1_display);
	DrawMatToHDC(m_mat1_display, IDC_Showimg1);

	char chEdit[8];
	sprintf(chEdit,"%4d",(int)(m_pnt.x+0.5));
	SetDlgItemText(IDC_EDIT_TARGET_X,chEdit);
	sprintf(chEdit,"%4d",(int)(m_pnt.y+0.5));
	SetDlgItemText(IDC_EDIT_TARGET_Y,chEdit);

	sprintf(chEdit,"%7.1f",tt.GetUseTime());
	SetDlgItemText(IDC_EDIT_USED_TIME,chEdit);
}
