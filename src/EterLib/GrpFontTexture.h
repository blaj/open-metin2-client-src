#pragma once

#include "GrpTexture.h"
#include "GrpImageTexture.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>
#include <map>

class CGraphicFontTexture : public CGraphicTexture
{
	public:
		typedef wchar_t TCharacterKey;

		typedef struct SCharacterInfomation
		{
			short index;
			short width;
			short height;
			float left;
			float top;
			float right;
			float bottom;
			float advance;
			float bearingX;
		} TCharacterInfomation;

		typedef std::vector<TCharacterInfomation*> TPCharacterInfomationVector;

	public:
		CGraphicFontTexture();
		virtual ~CGraphicFontTexture();

		void Destroy();
		bool Create(const char* c_szFontName, int fontSize, bool bItalic, bool bBold, bool bUnderLine, bool bStrikeOut);

		bool CreateDeviceObjects();
		void DestroyDeviceObjects();

		bool CheckTextureIndex(DWORD dwTexture);
		void SelectTexture(DWORD dwTexture);

		bool UpdateTexture();

		TCharacterInfomation* GetCharacterInfomation(wchar_t keyValue);
		TCharacterInfomation* UpdateCharacterInfomation(TCharacterKey keyValue);

		float GetKerning(wchar_t prev, wchar_t cur);

		bool IsEmpty() const;

		bool HasUnderLine()				const { return m_bUnderLine; }
		bool HasStrikeOut()				const { return m_bStrikeOut; }
		int  GetUnderlineOffset()		const { return m_underlineOffset; }
		int  GetUnderlineThickness()	const { return m_underlineThickness; }
		int  GetStrikeOutOffset()		const { return m_strikeOutOffset; }
		int  GetStrikeOutThickness()	const { return m_strikeOutThickness; }
		int  GetAscender()				const { return m_ascender; }

	protected:
		void Initialize();

		bool AppendTexture();

	protected:
		typedef std::vector<CGraphicImageTexture*> TGraphicImageTexturePointerVector;
		typedef std::map<TCharacterKey, TCharacterInfomation> TCharacterInfomationMap;

	protected:
		FT_Face m_ftFace;

		// CPU-side atlas buffer (replaces CGraphicDib)
		DWORD* m_pAtlasBuffer;
		int m_atlasWidth;
		int m_atlasHeight;

		TGraphicImageTexturePointerVector m_pFontTextureVector;

		TCharacterInfomationMap m_charInfoMap;

		int m_x;
		int m_y;
		int m_step;
		bool m_isDirty;

		LONG m_fontSize;
		bool m_bItalic;
		bool	m_bBold;
		bool	m_bUnderLine;
		bool	m_bStrikeOut;

		// FreeType metrics cached per-font
		int m_ascender;
		int m_lineHeight;
		bool m_hasKerning;
		int  m_boldStrength;
		int  m_underlineOffset;
		int  m_underlineThickness;
		int  m_strikeOutOffset;
		int  m_strikeOutThickness;
};
