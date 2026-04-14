#include "StdAfx.h"
#include "EterBase/Utils.h"
#include "GrpText.h"

CGraphicText::CGraphicText(const char* c_szFileName) : CResource(c_szFileName)
{
}

CGraphicText::~CGraphicText()
{
}

bool CGraphicText::CreateDeviceObjects()
{
	return m_fontTexture.CreateDeviceObjects();
}

void CGraphicText::DestroyDeviceObjects()
{
	m_fontTexture.DestroyDeviceObjects();
}

CGraphicFontTexture* CGraphicText::GetFontTexturePointer()
{
	return &m_fontTexture;
}

CGraphicText::TType CGraphicText::Type()
{
	static TType s_type = StringToType("CGraphicText");
	return s_type;
}

bool CGraphicText::OnLoad(int /*iSize*/, const void* /*c_pvBuf*/)
{
	char strName[32];
	int size;
	bool bItalic = false;
	bool bBold = false;
	bool bUnderLine = false;
	bool bStrikeOut = false;

	// format
	// 굴림.fnt		"굴림" 폰트 기본 사이즈 12 로 로딩
	// 굴림:18.fnt  "굴림" 폰트 사이즈 18 로 로딩
	// 굴림:14i.fnt "굴림" 폰트 사이즈 14 & 이탤릭으로 로딩
	const char * p = strrchr(GetFileName(), ':');

	if (p)
	{
		int nameLen = MIN(31, (int)(p - GetFileName()));
		strncpy(strName, GetFileName(), nameLen);
		strName[nameLen] = '\0';
		++p;

		char num[8];

		int i = 0;
		while (*p && isdigit(*p))
		{
			num[i++] = *(p++);
		}

		num[i] = '\0';
		bItalic = strchr(p, 'i') != NULL;
		bBold = strchr(p, 'b') != NULL;
		bUnderLine = strchr(p, 'u') != NULL;
		bStrikeOut = strchr(p, 's') != NULL;
		size = atoi(num);
	}
	else
	{
		p = strrchr(GetFileName(), '.');

		if (!p)
		{
			assert(!"CGraphicText::OnLoadFromFile there is no extension (ie: .fnt)");
			strName[0] = '\0';
		}
		else
		{
			int nameLen = MIN(31, (int)(p - GetFileName()));
			strncpy(strName, GetFileName(), nameLen);
			strName[nameLen] = '\0';
		}

		size = 12;
	}

	if (!m_fontTexture.Create(strName, size, bItalic, bBold, bUnderLine, bStrikeOut))
		return false;

	return true;
}

void CGraphicText::OnClear()
{
	m_fontTexture.Destroy();
}

bool CGraphicText::OnIsEmpty() const
{
	return m_fontTexture.IsEmpty();
}

bool CGraphicText::OnIsType(TType type)
{
	if (CGraphicText::Type() == type)
		return true;
	
	return CResource::OnIsType(type);
}
