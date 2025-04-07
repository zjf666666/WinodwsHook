
// DllInjectDlg.h: 头文件
//

#pragma once


// CDllInjectDlg 对话框
class CDllInjectDlg : public CDialogEx
{
// 构造
public:
    CDllInjectDlg(CWnd* pParent = nullptr); // 标准构造函数
    virtual ~CDllInjectDlg(); // 析构函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DLLINJECT_DIALOG };
#endif

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


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
    afx_msg void OnBnClickedChoseModule();
    afx_msg void OnBnClickedChosePath();
    CEdit m_ModuleName;
    CString m_cstrModulePath;
    DWORD m_Pid;
    WCHAR* m_wszModuleName;

    void InitModuleList();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    CListCtrl m_ModuleList;
    afx_msg void OnBnClickedFlashMoudle();
    afx_msg void OnBnClickedChoseBeInjectModule();

    // 提权函数
    void Elevate();
    // 进程id，模块路径
    void Inject(DWORD dwPid, WCHAR* wszPath);
    afx_msg void OnBnClickedInjectModule();
    CString m_DllPath;
};
