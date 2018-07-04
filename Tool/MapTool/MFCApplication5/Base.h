#pragma once

class CBase
{
protected:
	inline explicit CBase();
	inline virtual ~CBase();

protected:
	unsigned long			m_dwRefCnt = 0;			// ��ü�� �ּҰ� ��� �����Ϳ��� �����ǰ� �ִ��� �����ϴ� �������
	bool					m_IsEnable = true;		// ��ü�� ��ȿ���� Ȯ���ϴ� �������

public:
	inline unsigned long AddRef();
	inline bool	IsEnable() { return m_IsEnable; }
	inline virtual void Set_Enable(bool isEnable) { m_IsEnable = isEnable; }

public:
	inline unsigned long Release();

protected:
	virtual void Free() = 0;
};

#include "Base.inl"