// View_0.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "MFCApplication5.h"
#include "View_0.h"


// CView_0

IMPLEMENT_DYNCREATE(CView_0, CView)

CView_0::CView_0()
	: m_MainApp(g_hInst)
{

}

CView_0::~CView_0()
{
}

BEGIN_MESSAGE_MAP(CView_0, CView)
END_MESSAGE_MAP()


// CView_0 �׸����Դϴ�.

void CView_0::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: ���⿡ �׸��� �ڵ带 �߰��մϴ�.
}


// CView_0 �����Դϴ�.

#ifdef _DEBUG
void CView_0::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CView_0::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


void CView_0::Loop(const GameTimer & gt)
{
	m_MainApp.Run(gt);
	/*m_MainApp.Update(gt);
	m_MainApp.Draw(gt);*/
}

// CView_0 �޽��� ó�����Դϴ�.


void CView_0::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_MainApp.Initialize();

	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
}


//BOOL CView_0::PreCreateWindow(CREATESTRUCT& cs)
//{
//	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
////	cs.x = 400;
//	cs.cx = 800;
//	cs.cy = 600;
//	return CView::PreCreateWindow(cs);
//}
