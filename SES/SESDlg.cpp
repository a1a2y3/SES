
// SESDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SES.h"
#include "SESDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

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
	m_flipH= FALSE;
	m_videoInt= 40; // timer ���
	m_videoHalt= FALSE;
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
	ON_BN_CLICKED(IDC_BUTTON_FPS6, &CSESDlg::OnBnClickedButtonFps6)
	ON_BN_CLICKED(IDC_BUTTON_FPS12, &CSESDlg::OnBnClickedButtonFps12)
	ON_BN_CLICKED(IDC_BUTTON_FPS25, &CSESDlg::OnBnClickedButtonFps25)
	ON_BN_CLICKED(IDC_BUTTON_FPS50, &CSESDlg::OnBnClickedButtonFps50)
	ON_BN_CLICKED(IDC_BUTTON_FPS100, &CSESDlg::OnBnClickedButtonFps100)
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
		if (m_mat1.data)
			DrawMatToHDC(m_mat1, IDC_Showimg1);
		if (m_mat_ref.data)
			DrawMatToHDC(m_mat_ref, IDC_ShowImg2);
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
//	m_cam.open(1);
	m_cam>>m_mat1;
	m_mat_process= m_mat1.clone();
	// �����Ƶ�������
	if (m_flipH)
	{
		cv::flip(m_mat_process,m_mat_process,1);
	}
	m_mat1.release();
	m_mat1= m_mat_process.clone();
	DrawMatToHDC(m_mat1, IDC_Showimg1);
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
	if (!(m_cam.open(1)))
	{
		MessageBox("�޷�������ͷ");
	}
	else
	{	
		// ��ʾͼ�����
		m_cam>>m_mat1;
		char chEdit[10];
		_itoa(m_mat1.cols,chEdit,10);
		SetDlgItemText(IDC_EDIT_Realw,chEdit);
		_itoa(m_mat1.rows,chEdit,10);
		SetDlgItemText(IDC_EDIT_Realh,chEdit);
		// ˢ����ʾ��
		Invalidate(TRUE);  
		// ���ö�ʱ��
		SetTimer(1, m_videoInt, NULL);
	}
}


void CSESDlg::OnBnClickedButtonClosecamera()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_cam.isOpened())
	{
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
	DrawMatToHDC( m_mat1, IDC_Showimg1 );            // ������ʾͼƬ���� 
	// ��ʾͼ�����
	char chEdit[10];
	_itoa(m_mat1.cols,chEdit,10);
	SetDlgItemText(IDC_EDIT_Realw,chEdit);
	_itoa(m_mat1.rows,chEdit,10);
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
	imshow("��׼ͼ",m_mat_ref);
	DrawMatToHDC( m_mat_ref, IDC_ShowImg2 );            // ������ʾͼƬ���� 
	// ��ʾͼ�����
	char chEdit[10];
	_itoa(m_mat_ref.cols,chEdit,10);
	SetDlgItemText(IDC_EDIT_Refw,chEdit);
	_itoa(m_mat_ref.rows,chEdit,10);
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
		// ��ʾͼ�����
		m_cam>>m_mat1;
		char chEdit[10];
		_itoa(m_mat1.cols,chEdit,10);
		SetDlgItemText(IDC_EDIT_Realw,chEdit);
		_itoa(m_mat1.rows,chEdit,10);
		SetDlgItemText(IDC_EDIT_Realh,chEdit);
		// ˢ����ʾ��
		Invalidate(TRUE);  
		// ���ö�ʱ��
		SetTimer(1, m_videoInt, NULL);
	}
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
		NULL, NULL);                                        // ѡ��ͼƬ��Լ��
	dlg.m_ofn.lpstrTitle = _T("��Ƶ��ͼ");    // ���ļ��Ի���ı�����
	//	dlg.m_ofn.lpstrFilter=_T("image files (*.jpg) \0*.jpg\0image files (*.bmp)\0*.bmp\0All Files (*.*) \0*.*");
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
		SetTimer(1, m_videoInt, NULL);
		SetDlgItemText(IDC_BUTTON_HALT,"��ͣ");
	}
	m_videoHalt= !m_videoHalt;
}


void CSESDlg::OnBnClickedButtonFps6()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_videoInt= 160;    // ms
	SetTimer(1, m_videoInt, NULL);
}


void CSESDlg::OnBnClickedButtonFps12()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_videoInt= 80;    // ms
	SetTimer(1, m_videoInt, NULL);
}


void CSESDlg::OnBnClickedButtonFps25()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_videoInt= 40;    // ms
	SetTimer(1, m_videoInt, NULL);
}


void CSESDlg::OnBnClickedButtonFps50()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_videoInt= 20;    // ms
	SetTimer(1, m_videoInt, NULL);
}


void CSESDlg::OnBnClickedButtonFps100()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_videoInt= 10;    // ms
	SetTimer(1, m_videoInt, NULL);
}
