
// SESDlg.h : ͷ�ļ�
//

#pragma once
#include "cv.h"
#include "highgui.h"
#include "CvvImage.h"

// CSESDlg �Ի���
class CSESDlg : public CDialogEx
{
// ����
public:
	CSESDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SES_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	cv::Mat m_mat1, m_mat_process, m_mat_display;
	cv::Mat m_mat_ref;
	cv::VideoCapture m_cam;
	void DrawMatToHDC(cv::Mat img, UINT ID);
	void DrawPicToHDC(IplImage *img, UINT ID);

	bool m_flipH;
	int  m_videoInt;
	bool m_videoHalt;
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonOpencarema();
	afx_msg void OnBnClickedButtonClosecamera();
	afx_msg void OnBnClickedButtonOpenreal();
	afx_msg void OnBnClickedButtonOpenref();
	afx_msg void OnBnClickedButtonOpenavi();
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnBnClickedButtonFliph();
	afx_msg void OnBnClickedButtonShoot();
	afx_msg void OnBnClickedButtonHalt();
	afx_msg void OnBnClickedButtonFps6();
	afx_msg void OnBnClickedButtonFps12();
	afx_msg void OnBnClickedButtonFps25();
	afx_msg void OnBnClickedButtonFps50();
	afx_msg void OnBnClickedButtonFps100();
};
