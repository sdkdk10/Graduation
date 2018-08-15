
// MFCApplication5.cpp : ���� ���α׷��� ���� Ŭ���� ������ �����մϴ�.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "MFCApplication5.h"
#include "MainFrm.h"

#include "MFCApplication5Doc.h"
#include "MFCApplication5View.h"
#include "View_0.h"


#include "GameTimer_Manager.h"
#include "Frame_Manager.h"
#include "GameTimer.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCApplication5App

BEGIN_MESSAGE_MAP(CMFCApplication5App, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CMFCApplication5App::OnAppAbout)
	// ǥ�� ������ ���ʷ� �ϴ� ���� ����Դϴ�.
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()


// CMFCApplication5App ����

CMFCApplication5App::CMFCApplication5App()
{
	m_bHiColorIcons = TRUE;

	// TODO: �Ʒ� ���� ���α׷� ID ���ڿ��� ���� ID ���ڿ��� �ٲٽʽÿ�(����).
	// ���ڿ��� ���� ����: CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("MFCApplication5.AppID.NoVersion"));

	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.
}

// ������ CMFCApplication5App ��ü�Դϴ�.

CMFCApplication5App theApp;


// CMFCApplication5App �ʱ�ȭ

BOOL CMFCApplication5App::InitInstance()
{
	CWinAppEx::InitInstance();


	EnableTaskbarInteraction(FALSE);

	// RichEdit ��Ʈ���� ����Ϸ���  AfxInitRichEdit2()�� �־�� �մϴ�.	
	// AfxInitRichEdit2();

	// ǥ�� �ʱ�ȭ
	// �̵� ����� ������� �ʰ� ���� ���� ������ ũ�⸦ ���̷���
	// �Ʒ����� �ʿ� ���� Ư�� �ʱ�ȭ
	// ��ƾ�� �����ؾ� �մϴ�.
	// �ش� ������ ����� ������Ʈ�� Ű�� �����Ͻʽÿ�.
	// TODO: �� ���ڿ��� ȸ�� �Ǵ� ������ �̸��� ����
	// ������ �������� �����ؾ� �մϴ�.
	SetRegistryKey(_T("���� ���� ���α׷� �����翡�� ������ ���� ���α׷�"));
	LoadStdProfileSettings(4);  // MRU�� �����Ͽ� ǥ�� INI ���� �ɼ��� �ε��մϴ�.


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// ���� ���α׷��� ���� ���ø��� ����մϴ�.  ���� ���ø���
	//  ����, ������ â �� �� ������ ���� ������ �մϴ�.
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMFCApplication5Doc),
		RUNTIME_CLASS(CMainFrame),       // �� SDI ������ â�Դϴ�.
		RUNTIME_CLASS(CMFCApplication5View));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// ǥ�� �� ���, DDE, ���� ���⿡ ���� ������� ���� �м��մϴ�.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// ����ٿ� ������ ����� ����ġ�մϴ�.
	// ���� ���α׷��� /RegServer, /Register, /Unregserver �Ǵ� /Unregister�� ���۵� ��� FALSE�� ��ȯ�մϴ�.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	RECT m_Rect = { 0, 0, 600, 600 };
	AdjustWindowRect(&m_Rect, WS_OVERLAPPEDWINDOW, FALSE);
	int width = m_Rect.right - m_Rect.left;
	int height = m_Rect.bottom - m_Rect.top;
	m_pMainWnd->SetWindowPos(NULL, 300, 100, width, height, 0);

	// â �ϳ��� �ʱ�ȭ�Ǿ����Ƿ� �̸� ǥ���ϰ� ������Ʈ�մϴ�.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	m_pMainWnd->SetWindowTextW(L"Map Tool");

	CGameTimer_Manager::GetInstance()->Ready_Timers(L"Time_Default");
	CGameTimer_Manager::GetInstance()->Ready_Timers(L"Timer_FPS");

	CFrame_Manager::GetInstance()->Ready_Frames(L"Frame 30", 30.f);
	CFrame_Manager::GetInstance()->Ready_Frames(L"Frame 60", 60.f);
	CFrame_Manager::GetInstance()->Ready_Frames(L"Frame 100", 100.f);
	CFrame_Manager::GetInstance()->Ready_Frames(L"Frame 3000", 3000.f);

	const wchar_t* wstrFrame[FPS_END] = { L"Frame 30", L"Frame 60", L"Frame 100" , L"Frame 3000" };

	//mTimer.Reset();
	CGameTimer_Manager::GetInstance()->Get_GameTimer(L"Timer_FPS")->Reset();
	CGameTimer_Manager::GetInstance()->Get_GameTimer(L"Time_Default")->Reset();

	return TRUE;
}

// CMFCApplication5App �޽��� ó����


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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

// ��ȭ ���ڸ� �����ϱ� ���� ���� ���α׷� ����Դϴ�.
void CMFCApplication5App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CMFCApplication5App ����� ���� �ε�/���� �޼���

void CMFCApplication5App::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CMFCApplication5App::LoadCustomState()
{
}

void CMFCApplication5App::SaveCustomState()
{
}

// CMFCApplication5App �޽��� ó����





BOOL CMFCApplication5App::OnIdle(LONG lCount)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	if (this->m_pMainWnd->IsIconic())
		return FALSE;
	else
	{
		CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
		if (pMainFrame != NULL)
		{
			CMyForm* pMainForm = pMainFrame->GetMainForm();
			if (pMainForm != NULL)
			{
				CView_0* pView = pMainForm->Get_View();//->GetView();
				if (pView != NULL)
				{
					GameTimer gt;
					if (pView->IsWindowVisible())
					{
						GameTimer pFPSTimer = *CGameTimer_Manager::GetInstance()->Get_GameTimer(L"Timer_FPS");
						pView->Loop(pFPSTimer);

						//CGameTimer_Manager::GetInstance()->Compute_TimeDelta(L"Time_Default");
						//wchar_t* wstrFrame[FPS_END] = { L"Frame 30", L"Frame 60", L"Frame 100" , L"Frame 3000" };
						//GameTimer pTimer = *(CGameTimer_Manager::GetInstance()->Get_GameTimer(L"Time_Default"));
						//if (CFrame_Manager::GetInstance()->Permit_Call(wstrFrame[1], const_cast<GameTimer&>(pTimer)))
						//{
						//	CGameTimer_Manager::GetInstance()->Compute_TimeDelta(L"Timer_FPS");
						//	GameTimer pFPSTimer = *CGameTimer_Manager::GetInstance()->Get_GameTimer(L"Timer_FPS");
						//	//CalculateFrameStats();
						//	pView->Loop(pFPSTimer);
						//}

						
					}
				}
			}
		}
	}

	return CWinAppEx::OnIdle(lCount);
}
