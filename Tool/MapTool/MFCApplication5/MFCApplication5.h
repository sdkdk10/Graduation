
// MFCApplication5.h : MFCApplication5 ���� ���α׷��� ���� �� ��� ����
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"       // �� ��ȣ�Դϴ�.


// CMFCApplication5App:
// �� Ŭ������ ������ ���ؼ��� MFCApplication5.cpp�� �����Ͻʽÿ�.
//

class CMFCApplication5App : public CWinAppEx
{
public:
	enum Frame_State { FPS_30, FPS_60, FPS_100, FPS_NON, FPS_END };

public:
	CMFCApplication5App();


// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);
};

extern CMFCApplication5App theApp;
