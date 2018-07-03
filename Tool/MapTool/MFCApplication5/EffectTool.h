#pragma once

#include "Define.h"
#include "afxwin.h"
// CEffectTool ��ȭ �����Դϴ�.
class CView_0;
class CGameObject;

class CEffectTool : public CDialog
{
	DECLARE_DYNAMIC(CEffectTool)

public:
	CEffectTool(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CEffectTool();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EFFECTTOOL };
#endif

public:
	inline CView_0* GetView() { return m_pView; }

private:
	CCreateContext				m_Context;
	CView_0*					m_pView;

	CGameObject*				m_pCurObject = nullptr;
	bool						m_IsReplace = true;

	vector<CGameObject*>		m_vecEffect;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListBox m_TexListBox;
private:
	XMFLOAT3 m_SPos;
	XMFLOAT3 m_EPos;
	XMFLOAT3 m_SSize;
	XMFLOAT3 m_ESize;
	XMFLOAT3 m_SRot;
	XMFLOAT3 m_ERot;
	XMFLOAT4 m_SColor;
	XMFLOAT4 m_EColor;
public:
	float m_fLifeTime;
	float m_fStartTime;
	afx_msg void OnLbnSelchangeListTex();
	CListBox m_EffectListBox;
	afx_msg void OnBnClickedButtonAddEffect();
	afx_msg void OnBnClickedButtonApplyEffect();
	afx_msg void OnBnClickedButtonPlayEffect();
	afx_msg void OnBnClickedButtonStopEffect();
	CButton m_ButtonIsBillboard;
	UV_FRAME_INFO m_tcurFrameInfo;
	CButton m_ButtonIsFrame;
	afx_msg void OnBnClickedCheckEffectFrame();
	afx_msg void OnBnClickedCheckEndFrame();
	CButton m_ButtonFrameCnt;
	CString m_cstrEffectName;
	afx_msg void OnLbnSelchangeListEffect();
};
