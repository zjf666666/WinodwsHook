
// DllInjectDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "DllInject.h"
#include "DllInjectDlg.h"
#include "afxdialogex.h"
#include "CProcessDlg.h"
#include <stdio.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_PROCESS_ID (WM_USER + 1)

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedChoseModule();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDllInjectDlg 对话框
CDllInjectDlg::CDllInjectDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DLLINJECT_DIALOG, pParent)
    , m_Pid(0)
    , m_DllPath(_T(""))
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_wszModuleName = (WCHAR*)malloc(MAX_PATH * sizeof(WCHAR));
}

CDllInjectDlg::~CDllInjectDlg()
{
    free(m_wszModuleName);
    m_wszModuleName = NULL;
}

void CDllInjectDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_ModuleName);
    DDX_Text(pDX, IDC_EDIT2, m_Pid);
    DDX_Control(pDX, IDC_LIST1, m_ModuleList);
    DDX_Text(pDX, IDC_EDIT1, m_DllPath);
}

BEGIN_MESSAGE_MAP(CDllInjectDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &CDllInjectDlg::OnBnClickedChoseModule)
    ON_BN_CLICKED(IDC_BUTTON2, &CDllInjectDlg::OnBnClickedChosePath)
    ON_BN_CLICKED(IDC_BUTTON5, &CDllInjectDlg::OnBnClickedFlashMoudle)
    ON_BN_CLICKED(IDC_BUTTON6, &CDllInjectDlg::OnBnClickedChoseBeInjectModule)
    ON_BN_CLICKED(IDC_BUTTON4, &CDllInjectDlg::OnBnClickedInjectModule)
END_MESSAGE_MAP()


// CDllInjectDlg 消息处理程序

BOOL CDllInjectDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 将“关于...”菜单项添加到系统菜单中。

    // IDM_ABOUTBOX 必须在系统命令范围内。
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
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

    // 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);     // 设置大图标
    SetIcon(m_hIcon, FALSE);    // 设置小图标

    // TODO: 在此添加额外的初始化代码
    m_ModuleList.InsertColumn(0, L"模块名称", 0, 105); // 第三个参数为对齐方式
    m_ModuleList.InsertColumn(1, L"模块基址", 0, 105);
    m_ModuleList.InsertColumn(2, L"模块大小", 0, 105);
    m_ModuleList.InsertColumn(3, L"模块路径", 0, 105);
    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CDllInjectDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDllInjectDlg::OnPaint()
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
        CDialogEx::OnPaint();
    }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDllInjectDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CDllInjectDlg::OnBnClickedChoseModule()
{
    CFileDialog fileDlg(true, L"dll", L"*.dll", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"dll files||");
    if (IDOK != fileDlg.DoModal())
    {
        return;
    }
    m_ModuleName.SetWindowTextW(fileDlg.GetFolderPath() + L"\\" + fileDlg.GetFileName());
}

void CDllInjectDlg::OnBnClickedChosePath()
{
    CProcessDlg dlgProcess;
    if (IDOK != dlgProcess.DoModal())
    {
        return;
    }
    UpdateData(TRUE);
    if (0 != m_Pid)
    {
        InitModuleList();
    }
    UpdateData(FALSE);
}

void CDllInjectDlg::InitModuleList()
{
    m_ModuleList.DeleteAllItems();
    if (0 == m_Pid)
    {
        return;
    }
    HANDLE hModuleHandle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_Pid);
    if (INVALID_HANDLE_VALUE == hModuleHandle)
    {
        int nError = GetLastError();
        printf("get module handle failed, error = %d\n", GetLastError());
        return;
    }
    MODULEENTRY32 objModuleInfo;
    WCHAR wszModuleAddr[MAX_PATH] = { 0 };
    WCHAR wszModuleSize[MAX_PATH] = { 0 };
    ULONG ulIndex = 0;
    objModuleInfo.dwSize = sizeof(MODULEENTRY32);

    bool bRes = Module32First(hModuleHandle, &objModuleInfo);
    if (!bRes)
    {
        printf("get module info failed, error = %d\n", GetLastError());
    }
    if (bRes)
    {
        do
        {
            wsprintf(wszModuleAddr, L"%X", objModuleInfo.modBaseAddr);
            wsprintf(wszModuleSize, L"%d 字节", objModuleInfo.modBaseSize);
            m_ModuleList.InsertItem(ulIndex, objModuleInfo.szModule);
            m_ModuleList.SetItemText(ulIndex, 1, wszModuleAddr);
            m_ModuleList.SetItemText(ulIndex, 2, wszModuleSize);
            m_ModuleList.SetItemText(ulIndex, 3, objModuleInfo.szExePath);
            ++ulIndex;
        } while (Module32Next(hModuleHandle, &objModuleInfo));
    }
    if (NULL != hModuleHandle)
    {
        CloseHandle(hModuleHandle);
    }
}

BOOL CDllInjectDlg::PreTranslateMessage(MSG* pMsg)
{
    switch (pMsg->message)
    {
    case WM_PROCESS_ID:
    {
        // 刷新数据
        UpdateData(TRUE);  // true将控件的值赋值给成员变量
        m_Pid = (DWORD)pMsg->wParam;
        InitModuleList();
        UpdateData(FALSE); // false将成员变量的值赋值给控件
        break;
    }
    default:
        break;
    }

    return CDialogEx::PreTranslateMessage(pMsg);
}

void CDllInjectDlg::OnBnClickedFlashMoudle()
{
    UpdateData(TRUE);
    if (0 != m_Pid)
    {
        InitModuleList();
    }
    UpdateData(FALSE);
}


void CDllInjectDlg::OnBnClickedChoseBeInjectModule()
{
    UpdateData(TRUE);
    DWORD dwPos = (DWORD)m_ModuleList.GetFirstSelectedItemPosition();
    --dwPos;
    CString cstrModuleName = m_ModuleList.GetItemText(dwPos, 1);
    m_wszModuleName = cstrModuleName.AllocSysString();
    AfxMessageBox(L"选择成功");
    UpdateData(FALSE);
}

void CDllInjectDlg::Elevate()
{
    HANDLE hProcess = NULL;
    HANDLE hToken = NULL;
    LUID luidValue = { 0 };
    TOKEN_PRIVILEGES tokenPrivileges = { 0 };
    BOOL bRet = FALSE;
    DWORD dwRet = 0;

    hProcess = GetCurrentProcess(); // 获取当前进程句柄
    if (NULL == hProcess)
    {
        printf("GetCurrentProcess falied, error = %d", GetLastError());
        return;
    }
    bRet = OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken); // 获取进程令牌
    if (!bRet)
    {
        printf("OpenProcessToken falied, error = %d", GetLastError());
        CloseHandle(hProcess);
        if (NULL != hToken)
        {
            CloseHandle(hToken);
        }
        return;
    }

    bRet = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luidValue); //获取本地系统的特权LUID值
    if (!bRet)
    {
        printf("LookupPrivilegeValue falied, error = %d", GetLastError());
        CloseHandle(hProcess);
        CloseHandle(hToken);
        return;
    }
    
    // 设置权限信息
    tokenPrivileges.PrivilegeCount = 1;
    tokenPrivileges.Privileges[0].Luid = luidValue;
    tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // 提升进程令牌访问权限
    bRet = AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, 0, NULL, NULL);
    if (!bRet)
    {
        printf("AdjustTokenPrivileges falied, error = %d", GetLastError());
        CloseHandle(hProcess);
        CloseHandle(hToken);
        return;
    }
}

typedef DWORD(WINAPI* _NtCreateThreadEx32)(PHANDLE ThreadHandle, ACCESS_MASK DesiredAccess, LPVOID ObjectAttributes, HANDLE ProcessHandle, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, BOOL CreateSuspended, DWORD dwStackSize, DWORD dw1, DWORD dw2, LPVOID Unknown);
void CDllInjectDlg::Inject(DWORD dwPid, WCHAR* wszPath)
{
    SIZE_T dwSize = 0;
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
    if (NULL == hProcess)
    {
        printf("get process handle failed, error = %d\n", GetLastError());
        return;
    }
    LPVOID pAddr = VirtualAllocEx(hProcess, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    
    if (NULL == pAddr)
    {
        CloseHandle(hProcess);
        printf("alloc failed, error = %d\n", GetLastError());
        return;
    }
    
    bool bRes = WriteProcessMemory(hProcess, pAddr, wszPath, (wcslen(wszPath) + 1 ) * 2, &dwSize);
    
    if (!bRes)
    {
        CloseHandle(hProcess);
        printf("WriteProcessMemory failed, error = %d\n", GetLastError());
        return;
    }

    FARPROC  pFunProcAddr = GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryW");
    if (pFunProcAddr == NULL)
    {
        CloseHandle(hProcess);
        printf("Get LoadLibraryW Address failed, error = %d\n", GetLastError());
        return;
    }

    HANDLE hRemote = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)pFunProcAddr, pAddr, NULL, NULL);

    if (NULL == hRemote)
    {
        CloseHandle(hProcess);
        printf("CreateRemoteThread failed, error = %d\n", GetLastError());
        return;
    }

    WaitForSingleObject(hRemote, 5000);
    VirtualFreeEx(hProcess, pAddr, (wcslen(wszPath) + 1) * 2, MEM_RELEASE);
    if (NULL != hProcess)
    {
        CloseHandle(hProcess);
    }
    if (NULL != hRemote)
    {
        CloseHandle(hRemote);
    }

    AfxMessageBox(L"完成注入");
}


void CDllInjectDlg::OnBnClickedInjectModule()
{
    UpdateData(TRUE);
    WCHAR* wszModulePath = m_DllPath.AllocSysString();
    Elevate();

    Inject(m_Pid, wszModulePath);
    UpdateData(FALSE);
}
