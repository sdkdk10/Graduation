// MyForm.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "MFCApplication5.h"
#include "MyForm.h"
//#include "View_0.h"

// CMyForm
list<FILE_PATH>		m_ObjectList;
list<FILE_PATH>		m_TextureList;
IMPLEMENT_DYNCREATE(CMyForm, CFormView)

CMyForm::CMyForm()
	: CFormView(IDD_MYFORM)
{

}

CMyForm::~CMyForm()
{
}

void CMyForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_LoadObjectListBox);
	DDX_Control(pDX, IDC_LIST_LOAD_TEX, m_LoadTextureListBox);
	DDX_Control(pDX, IDC_RADIO_MAPTOOL, m_ToolRadio[0]);
	DDX_Control(pDX, IDC_RADIO_EFFECTTOOL, m_ToolRadio[1]);
}

BEGIN_MESSAGE_MAP(CMyForm, CFormView)
	ON_BN_CLICKED(IDC_MAPTOOL, &CMyForm::OnBnClickedMaptool)
	ON_LBN_SELCHANGE(IDC_LIST1, &CMyForm::OnLbnSelchangeList1)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BUTTON_EFFECT_TOOL, &CMyForm::OnBnClickedButtonEffectTool)
	ON_LBN_SELCHANGE(IDC_LIST_LOAD_TEX, &CMyForm::OnLbnSelchangeListLoadTex)
END_MESSAGE_MAP()


// CMyForm �����Դϴ�.

#ifdef _DEBUG
void CMyForm::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CMyForm::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

#endif
#endif //_DEBUG


// CMyForm �޽��� ó�����Դϴ�.


void CMyForm::OnBnClickedMaptool()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	if (m_MapTool.GetSafeHwnd() == NULL)
		m_MapTool.Create(IDD_MAPTOOL);
	m_eCurTool = TOOL_MAP;
	m_MapTool.ShowWindow(SW_SHOW);
}


void CMyForm::OnLbnSelchangeList1()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}


void CMyForm::OnDropFiles(HDROP hDropInfo)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.

	int iFileCount = DragQueryFile(hDropInfo, -1, NULL, NULL);

	wchar_t szFullPath[256] = L"";

	for (int i = 0; i < iFileCount; ++i)
	{
		DragQueryFile(hDropInfo, i, (LPWSTR)szFullPath, 256);
		int j = 0;
		FindPath(szFullPath, L"ASE");
		FindPath(szFullPath, L"dds");
	}
	UpdateData();
	auto iter = m_ObjectList.begin();
	auto iter_end = m_ObjectList.end();
	for (; iter != iter_end; ++iter)
	{
		wstring wstr = (*iter).wstrFileName + L"||" + (*iter).wstrPath;
		m_LoadObjectListBox.AddString(wstr.c_str());
	}
	iter = m_TextureList.begin();
	iter_end = m_TextureList.end();
	for (; iter != iter_end; ++iter)
	{
		wstring wstr = (*iter).wstrFileName + L" || " + (*iter).wstrPath;
		m_LoadTextureListBox.AddString(wstr.c_str());
  	}
	UpdateData(false);
	CFormView::OnDropFiles(hDropInfo);
}

CView_0 * CMyForm::Get_View()
{
	if (m_eCurTool == TOOL_MAP)
		return m_MapTool.GetView();
	else if (m_eCurTool == TOOL_EFFECT)
		return m_EffectTool.GetView();
	return nullptr;
}

void CMyForm::FindPath(const std::wstring& wstrPath, const std::wstring& wstrExt)
{
	CFileFind Find;
	Find.FindFile(wstrPath.c_str());
	int iContinue = 1;

	while (iContinue)
	{
		iContinue = Find.FindNextFile();

		if (Find.IsDots())
			continue;
		//.�� ..�� �ɷ����� ���� �б⹮.

		//ã�� ������ �������� �������� �������ִ� �Լ�.
		if (Find.IsDirectory())	//����
		{
			//����Լ�
			FindPath(
			std::wstring(Find.GetFilePath() + L"\\*.*")
			, wstrExt);
		}
		else		//����.
		{
			//���������� �ý��������� ���.
			if (Find.IsSystem())
				continue;

			CString strExtension;
			CString strCmp;
			string strDot = ".";
			strExtension = PathFindExtension(Find.GetFilePath());
			strCmp = (strDot + converter_tool.to_bytes(wstrExt)).c_str();
			if (strExtension != strCmp
				&& strExtension != strCmp)
				continue;

			//���� �̸��� ���� �Լ�.
			// > Mesh
			if (wstrExt == L"ASE")
			{
				std::wstring wstrFileName = Find.GetFileName().operator LPCWSTR();
				wstring a = Find.GetFileTitle();
				wstring b = Find.GetFilePath();

				//wstring FullPath = L"../../../Models/StaticMesh/" + wstrFileName;
				wstring FullPath = L"";
				if(m_ToolRadio[0].GetCheck())
					FullPath = L"../../../Slash/Chapter 16 Instancing and Frustum Culling/InstancingAndCulling/Assets/Models/StaticMesh/MapObject/" + wstrFileName;
				else if(m_ToolRadio[1].GetCheck())
					FullPath = L"../../../Slash/Chapter 16 Instancing and Frustum Culling/InstancingAndCulling/Assets/Models/StaticMesh/Effect/" + wstrFileName;
				
				FILE_PATH newFile;
				wstring::size_type stTmp = wstrFileName.find(L'.', 0);
				wstrFileName.erase(stTmp, 4);
				newFile.wstrFileName = wstrFileName;
				newFile.wstrPath = FullPath;
				FullPath.clear();
				if (m_ToolRadio[0].GetCheck())
					FullPath = L"Asset/Models/StaticMesh/MapObject/" + wstrFileName;
				else if (m_ToolRadio[1].GetCheck())
					FullPath = L"Asset/Models/StaticMesh/Effect/" + wstrFileName;
				newFile.wstrPath_Client = FullPath;

				m_ObjectList.push_back(newFile);
			}

			// > Texture
			else if (wstrExt == L"dds")
			{
				std::wstring wstrFileName = Find.GetFileName().operator LPCWSTR();
				wstring FullPath = L"../../../Textures/Effect/" + wstrFileName;
				FILE_PATH newFile;
				wstring::size_type stTmp = wstrFileName.find(L'.', 0);
				wstrFileName.erase(stTmp, 4);
				newFile.wstrFileName = wstrFileName;
				newFile.wstrPath = FullPath;
				FullPath.clear();
				FullPath = L"Asset/Textures/" + wstrFileName;
				newFile.wstrPath_Client = FullPath;

				m_TextureList.push_back(newFile);
			}
		}
	}
}


void CMyForm::OnBnClickedButtonEffectTool()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (m_EffectTool.GetSafeHwnd() == NULL)
		m_EffectTool.Create(IDD_EFFECTTOOL);
	m_eCurTool = TOOL_EFFECT;
	m_EffectTool.ShowWindow(SW_SHOW);
}

void CMyForm::OnLbnSelchangeListLoadTex()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}
