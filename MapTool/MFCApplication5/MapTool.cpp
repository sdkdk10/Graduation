// MapTool.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MFCApplication5.h"
#include "MapTool.h"
#include "afxdialogex.h"

#include "View_0.h"
#include "ObjectManager.h"
#include "Component_Manager.h"
#include "Texture_Manager.h"
#include "Component.h"
#include "GameObject.h"
#include "MapObject.h"
#include "Transform.h"

// CMapTool 대화 상자입니다.

IMPLEMENT_DYNAMIC(CMapTool, CDialog)

CMapTool::CMapTool(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_MAPTOOL, pParent)
	, m_fPosX(0)
	, m_fPosY(0)
	, m_fPosZ(0)
	, m_fScaleX(0)
	, m_fScaleY(0)
	, m_fScaleZ(0)
	, m_fRotX(0)
	, m_fRotY(0)
	, m_fRotZ(0)
{

}

CMapTool::~CMapTool()
{
}

void CMapTool::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MODEL, m_ModelListBox);
	DDX_Control(pDX, IDC_LIST_OBJECTS, m_ObjectListBox);
	DDX_Text(pDX, IDC_EDIT_POSX, m_fPosX);
	DDX_Text(pDX, IDC_EDIT_POSY, m_fPosY);
	DDX_Text(pDX, IDC_EDIT_POSZ, m_fPosZ);
	DDX_Text(pDX, IDC_EDIT_SCALEX, m_fScaleX);
	DDX_Text(pDX, IDC_EDIT_SCALEY, m_fScaleY);
	DDX_Text(pDX, IDC_EDIT_SCALEZ, m_fScaleZ);
	DDX_Text(pDX, IDC_EDIT_ROTX, m_fRotX);
	DDX_Text(pDX, IDC_EDIT_ROTY, m_fRotY);
	DDX_Text(pDX, IDC_EDIT_ROTZ, m_fRotZ);
	DDX_Control(pDX, IDC_LIST_TEXTURE, m_TextureListBox);
}


BEGIN_MESSAGE_MAP(CMapTool, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_MODEL, &CMapTool::OnLbnSelchangeListModel)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CMapTool::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CMapTool::OnBnClickedButtonApply)
	ON_LBN_SELCHANGE(IDC_LIST_OBJECTS, &CMapTool::OnLbnSelchangeListObjects)
	ON_LBN_SELCHANGE(IDC_LIST_TEXTURE, &CMapTool::OnLbnSelchangeListTexture)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CMapTool::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CMapTool::OnBnClickedButtonLoad)
END_MESSAGE_MAP()


// CMapTool 메시지 처리기입니다.


BOOL CMapTool::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	RECT Rect = { 0, 0, 1000, 800 };
	AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, FALSE);
	int width = Rect.right - Rect.left;
	int height = Rect.bottom - Rect.top;
	SetWindowPos(NULL, 100, 50, width, height, 0);

	// 맵뷰 크기 지정
	m_Context.m_pNewViewClass = RUNTIME_CLASS(CView_0);
	m_pView = (CView_0*)((CFrameWnd*)this)->CreateView(&m_Context);
	m_pView->OnInitialUpdate();
	m_pView->ShowWindow(SW_NORMAL);
	//m_pView->SetTo

	unordered_map<wstring, CComponent*> mapComponent = CComponent_Manager::GetInstance()->Get_ComponentMap(CComponent_Manager::COM_MESH);

	auto iter = mapComponent.begin();
	auto iter_end = mapComponent.end();
	for (iter; iter != iter_end; ++iter)
	{
		m_ModelListBox.AddString(iter->first.c_str());
	}

	m_IsReplace = true;

	unordered_map<wstring, Texture*> mapTexture = CTexture_Manager::GetInstance()->Get_TextureMap(HEAP_DEFAULT);
	auto tex_iter = mapTexture.begin();
	auto tex_iter_end = mapTexture.end();

	for (tex_iter; tex_iter != tex_iter_end; ++tex_iter)
	{
		m_TextureListBox.AddString(tex_iter->first.c_str());
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CMapTool::OnLbnSelchangeListModel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	int iSel = m_ModelListBox.GetCurSel();
	CString strModel;
	m_ModelListBox.GetText(iSel, strModel);
	CComponent* pMesh = CComponent_Manager::GetInstance()->Clone_Component(T2W(strModel.GetBuffer(0)), CComponent_Manager::COM_MESH);
	if (nullptr == pMesh)
		return;

	// > 오브젝트를 맵에 배치했다면 새로운 오브젝트를 생성한다.
	if (m_IsReplace)
	{
		
		m_pCurObject = CMapObject::Create(CObjectManager::GetInstance()->GetDevice()
			, CObjectManager::GetInstance()->GetSrvDescriptorHeap()[HEAP_DEFAULT]
			, CObjectManager::GetInstance()->GetCbvSrvDescriptorSize()
			, T2W(strModel.GetBuffer(0)));

		CObjectManager::GetInstance()->Add_Object(m_pCurObject);
		CObjectManager::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, m_pCurObject);
		m_IsReplace = false;
	}

	// > 오브젝트를 맵에 배치하기 전 모델을 바꾼다.
	else
	{
		dynamic_cast<CMapObject*>(m_pCurObject)->SetMesh(T2W(strModel.GetBuffer(0)));
	}
}

void CMapTool::OnBnClickedButtonAdd()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();

	m_vecObject.push_back(m_pCurObject);
	const string str = m_pCurObject->GetMeshName();
	int iSel = m_ModelListBox.GetCurSel();
	CString strModel;
	m_ModelListBox.GetText(iSel, strModel);
	m_ObjectListBox.AddString(strModel);
	m_pCurObject = nullptr;
	m_IsReplace = true;

	UpdateData(FALSE);
}


void CMapTool::OnBnClickedButtonApply()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();

	int iSel = m_ObjectListBox.GetCurSel();
	m_vecObject[iSel]->GetTransform()->Translation(m_fPosX, m_fPosY, m_fPosZ);
	m_vecObject[iSel]->GetTransform()->Scaling(m_fScaleX, m_fScaleY, m_fScaleZ);
	m_vecObject[iSel]->GetTransform()->Rotation(m_fRotX, m_fRotY, m_fRotZ);

	UpdateData(FALSE);
}


void CMapTool::OnLbnSelchangeListObjects()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();

	int iSel = m_ObjectListBox.GetCurSel();

	m_fPosX = m_vecObject[iSel]->GetTransform()->GetPosition().x;
	m_fPosY = m_vecObject[iSel]->GetTransform()->GetPosition().y;
	m_fPosZ = m_vecObject[iSel]->GetTransform()->GetPosition().z;

	m_fScaleX = m_vecObject[iSel]->GetTransform()->GetScale().x;
	m_fScaleY = m_vecObject[iSel]->GetTransform()->GetScale().y;
	m_fScaleZ = m_vecObject[iSel]->GetTransform()->GetScale().z;

	m_fRotX = m_vecObject[iSel]->GetTransform()->GetRotation().x;
	m_fRotY = m_vecObject[iSel]->GetTransform()->GetRotation().y;
	m_fRotZ = m_vecObject[iSel]->GetTransform()->GetRotation().z;

	UpdateData(FALSE);
}


void CMapTool::OnLbnSelchangeListTexture()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();

	int iSel = m_TextureListBox.GetCurSel();
	CString strTex;
	m_TextureListBox.GetText(iSel, strTex);
	Texture* tex = CTexture_Manager::GetInstance()->Find_Texture(T2W(strTex.GetBuffer(0)), HEAP_DEFAULT);

	int iObjSel = m_ObjectListBox.GetCurSel();
	dynamic_cast<CMapObject*>(m_vecObject[iObjSel])->SetTexture(tex);

	UpdateData(FALSE);
}


void CMapTool::OnBnClickedButtonSave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ofstream out("MapObject.txt");

	if (out.is_open() == false)
		return;

	size_t iSize = m_vecObject.size();
	

	for (size_t i = 0; i < iSize; ++i)
	{
		out << m_vecObject[i]->GetMeshName() << '\t';
		out << dynamic_cast<CMapObject*>(m_vecObject[i])->Get_TexName() << '\t';
		out << m_vecObject[i]->GetTransform()->GetPosition().x << '\t' << m_vecObject[i]->GetTransform()->GetPosition().y << '\t' << m_vecObject[i]->GetTransform()->GetPosition().z << '\t';
		out << m_vecObject[i]->GetTransform()->GetScale().x << '\t' << m_vecObject[i]->GetTransform()->GetScale().y << '\t' << m_vecObject[i]->GetTransform()->GetScale().z << '\t';
		out << m_vecObject[i]->GetTransform()->GetRotation().x << '\t' << m_vecObject[i]->GetTransform()->GetRotation().y << '\t' << m_vecObject[i]->GetTransform()->GetRotation().z << endl;
	}
}


void CMapTool::OnBnClickedButtonLoad()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();
	size_t iSize = m_vecObject.size();
	for (size_t i = 0; i < iSize; ++i)
		Safe_Release(m_vecObject[i]);
	m_vecObject.clear();
	CObjectManager::GetInstance()->Clear_Object();

	ifstream in("MapObject.txt");
	if (in.is_open() == false)
		return;

	string str;

	while (!in.eof())
	{
		in >> str;		// MeshName

		size_t needed = ::mbstowcs(NULL, &str[0], str.length());
		std::wstring output;
		output.resize(needed);

		// real call
		::mbstowcs(&output[0], &str[0], str.length());

		CGameObject* pObject = CMapObject::Create(CObjectManager::GetInstance()->GetDevice(), CObjectManager::GetInstance()->GetSrvDescriptorHeap()[HEAP_DEFAULT], CObjectManager::GetInstance()->GetCbvSrvDescriptorSize(), const_cast<wchar_t*>(output.c_str()));

		in >> str;			// TexName
		dynamic_cast<CMapObject*>(pObject)->SetTexture(str);
		float Value[9] = { 0 };
		for (int i = 0; i < 9; ++i)
			in >> Value[i];
		pObject->GetTransform()->Translation(Value[0], Value[1], Value[2]);
		pObject->GetTransform()->Scaling(Value[3], Value[4], Value[5]);
		pObject->GetTransform()->Rotation(Value[6], Value[7], Value[8]);
		
		CObjectManager::GetInstance()->Add_Object(pObject);
		CObjectManager::GetInstance()->GetRenderer()->Add_RenderGroup(CRenderer::RENDER_NONALPHA_FORWARD, pObject);


		m_vecObject.push_back(pObject);
		string str = pObject->GetMeshName();
		CString strModel(str.c_str());
		m_ObjectListBox.AddString(strModel);

	}

	UpdateData(FALSE);
}