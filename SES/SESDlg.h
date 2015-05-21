
// SESDlg.h : 头文件
//

#pragma once
#include "cv.h"
#include "highgui.h"
#include "CvvImage.h"

// CSESDlg 对话框
class CSESDlg : public CDialogEx
{
// 构造
public:
	CSESDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SES_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	cv::Mat m_mat1, m_mat1_display, m_mat_gray;
	cv::Mat m_mat_ref, m_mat_ref_display,m_mat_ref_gray;
	cv::VideoCapture m_cam;
	void DrawMatToHDC(cv::Mat img, UINT ID);
	void DrawPicToHDC(IplImage *img, UINT ID);

	
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
	afx_msg void OnBnClickedButtonTrack();
	afx_msg void OnBnClickedButtonOpenseriesimg();
	afx_msg void OnBnClickedButtonVideoSlow();
	afx_msg void OnBnClickedButtonVideoFast();
	afx_msg void OnBnClickedButtonSaveref();
	afx_msg void OnBnClickedButtonTestTrack();
};
