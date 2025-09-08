#pragma once
#include "psapi.h"
#include <process.h>
#include <TlHelp32.h>

// CProcessDlg 对话框

class CProcessDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CProcessDlg)

public:
    CProcessDlg(CWnd* pParent = nullptr);   // 标准构造函数
    virtual ~CProcessDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DIALOG1 };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    DECLARE_MESSAGE_MAP()
public:
    CListCtrl m_ProcessList;
    virtual BOOL OnInitDialog();
    void InitProcessList();
    afx_msg void OnBnClickedFlashList();
    afx_msg void OnBnClickedChoseProcess();
};
