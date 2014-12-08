
// SESDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SES.h"
#include "SESDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CSESDlg 对话框




CSESDlg::CSESDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSESDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_flipH= FALSE;
	m_videoInt= 40; // timer 间隔
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


// CSESDlg 消息处理程序

BOOL CSESDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSESDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// 显示图像
		if (m_mat1.data)
			DrawMatToHDC(m_mat1, IDC_Showimg1);
		if (m_mat_ref.data)
			DrawMatToHDC(m_mat_ref, IDC_ShowImg2);
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSESDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSESDlg::DrawPicToHDC(IplImage *img, UINT ID)//----老版本的IplImage格式的显示函数
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
void CSESDlg::DrawMatToHDC(cv::Mat img, UINT ID)//------对应新版Mat 的显示函数
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
	// TODO: 在此添加消息处理程序代码和/或调用默认值
//	m_cam.open(1);
	m_cam>>m_mat1;
	m_mat_process= m_mat1.clone();
	// 添加视频处理代码
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
	// TODO: 在此添加控件通知处理程序代码
	if (m_cam.isOpened())
	{
		m_cam.release();
	}
	if (!(m_cam.open(1)))
	{
		MessageBox("无法打开摄像头");
	}
	else
	{	
		// 显示图像参数
		m_cam>>m_mat1;
		char chEdit[10];
		_itoa(m_mat1.cols,chEdit,10);
		SetDlgItemText(IDC_EDIT_Realw,chEdit);
		_itoa(m_mat1.rows,chEdit,10);
		SetDlgItemText(IDC_EDIT_Realh,chEdit);
		// 刷新显示区
		Invalidate(TRUE);  
		// 设置定时器
		SetTimer(1, m_videoInt, NULL);
	}
}


void CSESDlg::OnBnClickedButtonClosecamera()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_cam.isOpened())
	{
		m_cam.release();
	}
	KillTimer(1);
}


void CSESDlg::OnBnClickedButtonOpenreal()
{
	// TODO: 在此添加控件通知处理程序代码
	// 打开图像前，先关闭摄像头
	if (m_cam.isOpened())
	{
		m_cam.release();
	}
	KillTimer(1);
	CFileDialog dlg(TRUE, _T("*.jpg"), NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
		NULL, NULL);                                        // 选项图片的约定
	dlg.m_ofn.lpstrTitle = _T("打开实时图");    // 打开文件对话框的标题名
	dlg.m_ofn.lpstrFilter=_T("image files (*.jpg) \0*.jpg\0image files (*.bmp)\0*.bmp\0All Files (*.*) \0*.*");
	if( dlg.DoModal() != IDOK )                    // 判断是否获得图片
		return;

	CString mPath = dlg.GetPathName();            // 获取图片路径
	m_mat1 = cv::imread( LPCSTR(mPath), 1 );    // 读取图片、缓存到一个局部变量 ipl 中
	if( !m_mat1.data )                                    // 判断是否成功载入图片
		return;	
	DrawMatToHDC( m_mat1, IDC_Showimg1 );            // 调用显示图片函数 
	// 显示图像参数
	char chEdit[10];
	_itoa(m_mat1.cols,chEdit,10);
	SetDlgItemText(IDC_EDIT_Realw,chEdit);
	_itoa(m_mat1.rows,chEdit,10);
	SetDlgItemText(IDC_EDIT_Realh,chEdit);
}


void CSESDlg::OnBnClickedButtonOpenref()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE, NULL, NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
		NULL, NULL);                                        // 选项图片的约定
	dlg.m_ofn.lpstrTitle = _T("打开基准图");    // 打开文件对话框的标题名
//	dlg.m_ofn.lpstrFilter=_T("image files (*.jpg) \0*.jpg\0image files (*.bmp)\0*.bmp\0All Files (*.*) \0*.*");
	if( dlg.DoModal() != IDOK )                    // 判断是否获得图片
		return;

	CString mPath = dlg.GetPathName();            // 获取图片路径
	m_mat_ref= cv::imread( LPCSTR(mPath), 1 );    // 读取图片、缓存到一个局部变量 ipl 中
	if( !m_mat_ref.data )                                    // 判断是否成功载入图片
	{
		MessageBox("打开图像出错！");
		return;	
	}
	imshow("基准图",m_mat_ref);
	DrawMatToHDC( m_mat_ref, IDC_ShowImg2 );            // 调用显示图片函数 
	// 显示图像参数
	char chEdit[10];
	_itoa(m_mat_ref.cols,chEdit,10);
	SetDlgItemText(IDC_EDIT_Refw,chEdit);
	_itoa(m_mat_ref.rows,chEdit,10);
	SetDlgItemText(IDC_EDIT_Refh,chEdit);
}



void CSESDlg::OnBnClickedButtonOpenavi()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE, NULL, NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
		NULL, NULL);                                        // 选项图片的约定
	dlg.m_ofn.lpstrTitle = _T("打开视频文件");    // 打开文件对话框的标题名
//	dlg.m_ofn.lpstrFilter=_T("*.AVI\0*.AVI\0All Files (*.*) \0*.*");

	if( dlg.DoModal() != IDOK )                    // 判断是否获得图片
		return;

	CString mPath = dlg.GetPathName();            // 获取图片路径
	if (m_cam.isOpened())
	{
		m_cam.release();
	}
	if(!m_cam.open(LPCSTR(mPath)))
	{
		MessageBox("打开视频出错！");
	}
	else
	{
		// 显示图像参数
		m_cam>>m_mat1;
		char chEdit[10];
		_itoa(m_mat1.cols,chEdit,10);
		SetDlgItemText(IDC_EDIT_Realw,chEdit);
		_itoa(m_mat1.rows,chEdit,10);
		SetDlgItemText(IDC_EDIT_Realh,chEdit);
		// 刷新显示区
		Invalidate(TRUE);  
		// 设置定时器
		SetTimer(1, m_videoInt, NULL);
	}
}


void CSESDlg::OnBnClickedButtonExit()
{
	// TODO: 在此添加控件通知处理程序代码
	OnOK();
}


void CSESDlg::OnBnClickedButtonFliph()
{
	// TODO: 在此添加控件通知处理程序代码

	m_flipH= !m_flipH;
}


void CSESDlg::OnBnClickedButtonShoot()
{
	// TODO: 在此添加控件通知处理程序代码
	if( !m_mat1.data )                                    // 判断是否成功载入图片
	{
		return;	
	}
	CFileDialog dlg(FALSE, _T("*.BMP"), NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
		NULL, NULL);                                        // 选项图片的约定
	dlg.m_ofn.lpstrTitle = _T("视频截图");    // 打开文件对话框的标题名
	//	dlg.m_ofn.lpstrFilter=_T("image files (*.jpg) \0*.jpg\0image files (*.bmp)\0*.bmp\0All Files (*.*) \0*.*");
	if( dlg.DoModal() != IDOK )                    // 判断是否获得图片
		return;

	CString mPath = dlg.GetPathName();            // 获取图片路径
	cv::imwrite(LPCSTR(mPath),m_mat1);
}


void CSESDlg::OnBnClickedButtonHalt()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_videoHalt)
	{
		SetTimer(1, 99999999, NULL);
		SetDlgItemText(IDC_BUTTON_HALT,"继续");
	}
	else
	{
		SetTimer(1, m_videoInt, NULL);
		SetDlgItemText(IDC_BUTTON_HALT,"暂停");
	}
	m_videoHalt= !m_videoHalt;
}


void CSESDlg::OnBnClickedButtonFps6()
{
	// TODO: 在此添加控件通知处理程序代码
	m_videoInt= 160;    // ms
	SetTimer(1, m_videoInt, NULL);
}


void CSESDlg::OnBnClickedButtonFps12()
{
	// TODO: 在此添加控件通知处理程序代码
	m_videoInt= 80;    // ms
	SetTimer(1, m_videoInt, NULL);
}


void CSESDlg::OnBnClickedButtonFps25()
{
	// TODO: 在此添加控件通知处理程序代码
	m_videoInt= 40;    // ms
	SetTimer(1, m_videoInt, NULL);
}


void CSESDlg::OnBnClickedButtonFps50()
{
	// TODO: 在此添加控件通知处理程序代码
	m_videoInt= 20;    // ms
	SetTimer(1, m_videoInt, NULL);
}


void CSESDlg::OnBnClickedButtonFps100()
{
	// TODO: 在此添加控件通知处理程序代码
	m_videoInt= 10;    // ms
	SetTimer(1, m_videoInt, NULL);
}
