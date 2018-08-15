#pragma once


// CView_0 ���Դϴ�.

#include "BoxApp.h"

class CView_0 : public CView
{
	DECLARE_DYNCREATE(CView_0)

protected:
	CView_0();           // ���� ����⿡ ���Ǵ� protected �������Դϴ�.
	virtual ~CView_0();

public:
	virtual void OnDraw(CDC* pDC);      // �� �並 �׸��� ���� �����ǵǾ����ϴ�.
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
public:
	void Loop(const GameTimer& gt);
private:
	InstancingAndCullingApp			m_MainApp;

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	//virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
};


