
// MFCApplication5View.h : CMFCApplication5View Ŭ������ �������̽�
//

#pragma once

class CMFCApplication5Doc;

class CMFCApplication5View : public CView
{
protected: // serialization������ ��������ϴ�.
	CMFCApplication5View();
	DECLARE_DYNCREATE(CMFCApplication5View)

// Ư���Դϴ�.
public:
	CMFCApplication5Doc* GetDocument() const;

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual void OnDraw(CDC* pDC);  // �� �並 �׸��� ���� �����ǵǾ����ϴ�.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// �����Դϴ�.
public:
	virtual ~CMFCApplication5View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // MFCApplication5View.cpp�� ����� ����
inline CMFCApplication5Doc* CMFCApplication5View::GetDocument() const
   { return reinterpret_cast<CMFCApplication5Doc*>(m_pDocument); }
#endif

