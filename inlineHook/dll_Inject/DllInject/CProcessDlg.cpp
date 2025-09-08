// CProcessDlg.cpp: 实现文件
//

#include "pch.h"
#include "DllInject.h"
#include "CProcessDlg.h"
#include "afxdialogex.h"

// WM_USER是windows系统为非系统消息保留的id，不可以冲突，需要维护好各个窗口见通信的id
// 如这里用了+1 后续的窗口就得用+1外的数据
#define WM_PROCESS_ID (WM_USER + 1) 

// CProcessDlg 对话框
IMPLEMENT_DYNAMIC(CProcessDlg, CDialogEx)

CProcessDlg::CProcessDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DIALOG1, pParent)
{

}

CProcessDlg::~CProcessDlg()
{
}

void CProcessDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_ProcessList);
}


BEGIN_MESSAGE_MAP(CProcessDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON1, &CProcessDlg::OnBnClickedFlashList)
    ON_BN_CLICKED(IDC_BUTTON3, &CProcessDlg::OnBnClickedChoseProcess)
END_MESSAGE_MAP()


// CProcessDlg 消息处理程序


BOOL CProcessDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 设置样式，表格线 + 整行选中
    m_ProcessList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
    m_ProcessList.InsertColumn(0, L"进程名", 0, 105);
    m_ProcessList.InsertColumn(1, L"pid", 0, 105);
    InitProcessList();
    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}

void CProcessDlg::InitProcessList()
{
    PROCESSENTRY32 objProcessInfo;
    HANDLE hProcessHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    WCHAR wszProcessName[MAX_PATH] = { 0 };
    WCHAR wszProcessID[MAX_PATH] = { 0 };
    ULONG ulInex = 0;

    m_ProcessList.DeleteAllItems();
    objProcessInfo.dwSize = sizeof(PROCESSENTRY32);
    bool bRes = Process32First(hProcessHandle, &objProcessInfo);
    if (bRes)
    {
        do
        {
            wsprintf(wszProcessID, L"%d", objProcessInfo.th32ProcessID);
            m_ProcessList.InsertItem(ulInex, objProcessInfo.szExeFile);
            m_ProcessList.SetItemText(ulInex, 1, wszProcessID);
            ++ulInex;
        } while (Process32Next(hProcessHandle, &objProcessInfo));
    }
    if (NULL != hProcessHandle)
    {
        CloseHandle(hProcessHandle);
    }
}

void CProcessDlg::OnBnClickedFlashList()
{
    InitProcessList();
}

void CProcessDlg::OnBnClickedChoseProcess()
{
    DWORD dwPos = (DWORD)m_ProcessList.GetFirstSelectedItemPosition(); // 获取选中位置
    --dwPos;

    CString cstrProcessID = m_ProcessList.GetItemText(dwPos, 1);
    int nPid = _ttoi(cstrProcessID);
    CWnd* pcWnd = FindWindow(NULL, L"DllInject");// 当前实现类内的窗口句柄为CWnd类型，其他类的窗口句柄为HWND类型 需要区分
    HWND hWnd = pcWnd->GetSafeHwnd();
    ::PostMessage(hWnd, WM_PROCESS_ID, nPid, NULL);
    OnOK(); // 退出窗口
}
