
// SecurityGuardUIDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "SecurityGuardUI.h"
#include "SecurityGuardUIDlg.h"
#include "afxdialogex.h"

#include <vector>

#include "../SecurityCore/Logger.h"
#include "../include/dto/FileSystemHookRequestDTO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CSecurityGuardUIDlg 对话框



CSecurityGuardUIDlg::CSecurityGuardUIDlg(CWnd* pParent /*=nullptrptr*/)
    : CDialogEx(IDD_SECURITYGUARDUI_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSecurityGuardUIDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_editPid);
    DDX_Control(pDX, IDC_EDIT2, m_editProcessName);
}

BEGIN_MESSAGE_MAP(CSecurityGuardUIDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &CSecurityGuardUIDlg::OnBnClickedProcessFileMoniter)
END_MESSAGE_MAP()


// CSecurityGuardUIDlg 消息处理程序

BOOL CSecurityGuardUIDlg::OnInitDialog()
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
    m_hPipe = INVALID_HANDLE_VALUE;
    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSecurityGuardUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSecurityGuardUIDlg::OnPaint()
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
HCURSOR CSecurityGuardUIDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CSecurityGuardUIDlg::OnBnClickedProcessFileMoniter()
{
    CString cstrText;
    m_editPid.GetWindowText(cstrText);
    int nPid;
    if (1 != _stscanf_s(cstrText, _T("%d"), &nPid))
    {
        Logger::GetInstance().Error(L"Pid is not digit");
        return;
    }

    // 2-3. 组装JSON数据
    FileSystemHookRequest dto("0", 0, "0", 5000, nPid, "");
    std::string json = dto.ToJson();
    //std::string json = R"({
    //    "processId": 1234,
    //    "hookType": 300,
    //    "targetFunctions": ["CreateFileW", "ReadFile", "WriteFile"],
    //    "monitorPaths": ["C:\\Windows\\System32"]
    //})";

    // 4. 发送数据
    struct {
        uint32_t magic = 0x47555357;
        uint32_t version = 1;
        uint32_t type = 300;
        uint32_t correlationId = 1001;
        uint32_t flags = 0;
        uint32_t payloadLength = 0; // 负载长度（字节数），后续读取 payloadLength 字节的负载
        uint64_t timestamp = 0;     // 时间戳（可选，用于调试和日志）
        uint32_t size; // 消息体长度
    } header;

    header.size = static_cast<uint32_t>(json.length());  // 后续赋值
    DWORD written;

    if (INVALID_HANDLE_VALUE != m_hPipe)
    {
        goto _send;
    }

    // 1. 连接管道
    m_hPipe = CreateFile(
        L"\\\\.\\pipe\\WindowsSecurityGuard",
        GENERIC_WRITE,
        0, nullptr, OPEN_EXISTING, 0, nullptr
    );

    if (m_hPipe == INVALID_HANDLE_VALUE) {
        int nError = GetLastError();
        CString str;
        str.Format(_T("%d"), nError); // 转换为十进制字符串
        MessageBox(L"连接失败", L"错误", MB_ICONERROR);
        MessageBox(str, L"错误", MB_ICONERROR);
        return;
    }

    //CloseHandle(hPipe);
_send:
    WriteFile(m_hPipe, &header, sizeof(header), &written, nullptr);
    WriteFile(m_hPipe, json.c_str(), json.length(), &written, nullptr);
    MessageBox(L"Hook请求已发送", L"完成", MB_ICONINFORMATION);
}

void CSecurityGuardUIDlg::Init()
{
    std::wstring pipeName = L"\\\\.\\pipe\\WindowsSecurityGuard";
    // 连接命名管道
    m_hPipe = CreateFileW(
        pipeName.c_str(),                    // 管道名称
        GENERIC_READ | GENERIC_WRITE,        // 读写权限
        0,                                   // 不共享
        nullptr,                             // 默认安全属性
        OPEN_EXISTING,                       // 打开现有管道
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // 重叠I/O
        nullptr                              // 无模板文件
    );

    if (INVALID_HANDLE_VALUE == m_hPipe)
    {
        Logger::GetInstance().Error(L"CreateFileW faile! error = %d", GetLastError());
        return;
    }

    // 设置为消息流
    DWORD mode = PIPE_READMODE_MESSAGE;
    if (FALSE == SetNamedPipeHandleState(m_hPipe, &mode, nullptr, nullptr))
    {
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
        Logger::GetInstance().Error(L"SetNamedPipeHandleState faile! error = %d", GetLastError());
        return;
    }
}
