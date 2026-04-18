#include "StdAfx.h"
#include "PackLib/PackManager.h"
#include "EterLib/ResourceManager.h"
#include "EterBase/lzo.h"
#include <winhttp.h>
#include "ItemManager.h"
#include <msgpack.hpp>

static DWORD s_adwItemProtoKey[4] =
{
	173217,
	72619434,
	408587239,
	27973291
};

BOOL CItemManager::SelectItemData(DWORD dwIndex)
{
	TItemMap::iterator f = m_ItemMap.find(dwIndex);

	if (m_ItemMap.end() == f)
	{
		int n = m_vec_ItemRange.size();
		for (int i = 0; i < n; i++)
		{
			CItemData * p = m_vec_ItemRange[i];
			const CItemData::TItemTable * pTable = p->GetTable(); 
			if ((pTable->dwVnum < dwIndex) &&
				dwIndex < (pTable->dwVnum + pTable->dwVnumRange))
			{
				m_pSelectedItemData = p;
				return TRUE;
			}
		}
		Tracef(" CItemManager::SelectItemData - FIND ERROR [%d]\n", dwIndex);
		return FALSE;
	}

	m_pSelectedItemData = f->second;

	return TRUE;
}

CItemData * CItemManager::GetSelectedItemDataPointer()
{
	return m_pSelectedItemData;
}

BOOL CItemManager::GetItemDataPointer(DWORD dwItemID, CItemData ** ppItemData)
{
	if (0 == dwItemID)
		return FALSE;

	TItemMap::iterator f = m_ItemMap.find(dwItemID);

	if (m_ItemMap.end() == f)
	{
		int n = m_vec_ItemRange.size();
		for (int i = 0; i < n; i++)
		{
			CItemData * p = m_vec_ItemRange[i];
			const CItemData::TItemTable * pTable = p->GetTable(); 
			if ((pTable->dwVnum < dwItemID) &&
				dwItemID < (pTable->dwVnum + pTable->dwVnumRange))
			{
				*ppItemData = p;
				return TRUE;
			}
		}
		Tracef(" CItemManager::GetItemDataPointer - FIND ERROR [%d]\n", dwItemID);
		return FALSE;
	}

	*ppItemData = f->second;

	return TRUE;
}

CItemData * CItemManager::MakeItemData(DWORD dwIndex)
{
	TItemMap::iterator f = m_ItemMap.find(dwIndex);

	if (m_ItemMap.end() == f)
	{
		CItemData * pItemData = CItemData::New();

		m_ItemMap.insert(TItemMap::value_type(dwIndex, pItemData));		

		return pItemData;
	}

	return f->second;
}

////////////////////////////////////////////////////////////////////////////////////////
// Load Item Table

bool CItemManager::LoadItemList(const char * c_szFileName)
{
	TPackFile File;

	if (!CPackManager::Instance().GetFile(c_szFileName, File))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(File.size(), File.data());

	CTokenVector TokenVector;
    for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			continue;

		if (!(TokenVector.size() == 3 || TokenVector.size() == 4))
		{
			TraceError(" CItemManager::LoadItemList(%s) - StrangeLine in %d\n", c_szFileName, i);
			continue;
		}

		const std::string & c_rstrID = TokenVector[0];
		//const std::string & c_rstrType = TokenVector[1];
		const std::string & c_rstrIcon = TokenVector[2];

		DWORD dwItemVNum=atoi(c_rstrID.c_str());

		CItemData * pItemData = MakeItemData(dwItemVNum);

		extern BOOL USE_VIETNAM_CONVERT_WEAPON_VNUM;
		if (USE_VIETNAM_CONVERT_WEAPON_VNUM)
		{
			extern DWORD Vietnam_ConvertWeaponVnum(DWORD vnum);
			DWORD dwMildItemVnum = Vietnam_ConvertWeaponVnum(dwItemVNum);
			if (dwMildItemVnum == dwItemVNum)
			{
				if (4 == TokenVector.size())
				{
					const std::string & c_rstrModelFileName = TokenVector[3];
					pItemData->SetDefaultItemData(c_rstrIcon.c_str(), c_rstrModelFileName.c_str());
				}
				else
				{
					pItemData->SetDefaultItemData(c_rstrIcon.c_str());
				}
			}
			else
			{
				DWORD dwMildBaseVnum = dwMildItemVnum / 10 * 10;
				char szMildIconPath[MAX_PATH];				
				sprintf(szMildIconPath, "icon/item/%.5d.tga", dwMildBaseVnum);
				if (4 == TokenVector.size())
				{
					char szMildModelPath[MAX_PATH];
					sprintf(szMildModelPath, "d:/ymir work/item/weapon/%.5d.gr2", dwMildBaseVnum);	
					pItemData->SetDefaultItemData(szMildIconPath, szMildModelPath);
				}
				else
				{
					pItemData->SetDefaultItemData(szMildIconPath);
				}
			}
		}
		else
		{
			if (4 == TokenVector.size())
			{
				const std::string & c_rstrModelFileName = TokenVector[3];
				pItemData->SetDefaultItemData(c_rstrIcon.c_str(), c_rstrModelFileName.c_str());
			}
			else
			{
				pItemData->SetDefaultItemData(c_rstrIcon.c_str());
			}
		}
	}

	return true;
}

const std::string& __SnapString(const std::string& c_rstSrc, std::string& rstTemp)
{
	UINT uSrcLen=c_rstSrc.length();
	if (uSrcLen<2)
		return c_rstSrc;

	if (c_rstSrc[0]!='"')
		return c_rstSrc;

	UINT uLeftCut=1;
	
	UINT uRightCut=uSrcLen;
	if (c_rstSrc[uSrcLen-1]=='"')
		uRightCut=uSrcLen-1;	

	rstTemp=c_rstSrc.substr(uLeftCut, uRightCut-uLeftCut);
	return rstTemp;
}

bool CItemManager::LoadItemDesc(const char* c_szFileName)
{
	TPackFile kFile;
	if (!CPackManager::Instance().GetFile(c_szFileName, kFile))
	{
		Tracenf("CItemManager::LoadItemDesc(c_szFileName=%s) - Load Error", c_szFileName);
		return false;
	}

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.size(), kFile.data());

	std::string stTemp;

	CTokenVector kTokenVector;
	for (DWORD i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		if (!kTextFileLoader.SplitLineByTab(i, &kTokenVector))
			continue;

		while (kTokenVector.size()<ITEMDESC_COL_NUM)
			kTokenVector.push_back("");

		//assert(kTokenVector.size()==ITEMDESC_COL_NUM);
		
		DWORD dwVnum=atoi(kTokenVector[ITEMDESC_COL_VNUM].c_str());
		const std::string& c_rstDesc=kTokenVector[ITEMDESC_COL_DESC];
		const std::string& c_rstSumm=kTokenVector[ITEMDESC_COL_SUMM];
		TItemMap::iterator f = m_ItemMap.find(dwVnum);
		if (m_ItemMap.end() == f)
			continue;

		CItemData* pkItemDataFind = f->second;

		pkItemDataFind->SetDescription(__SnapString(c_rstDesc, stTemp));
		pkItemDataFind->SetSummary(__SnapString(c_rstSumm, stTemp));
	}
	return true;
}

DWORD GetHashCode( const char* pString )
{
	   unsigned long i,len;
	   unsigned long ch;
	   unsigned long result;
	   
	   len     = strlen( pString );
	   result = 5381;
	   for( i=0; i<len; i++ )
	   {
	   	   ch = (unsigned long)pString[i];
	   	   result = ((result<< 5) + result) + ch; // hash * 33 + ch
	   }	  

	   return result;
}

bool CItemManager::LoadItemTable(const char* c_szFileName)
{	
	TPackFile file;
	LPCVOID pvData;

	if (!CPackManager::Instance().GetFile(c_szFileName, file))
		return false;

	DWORD dwFourCC, dwElements, dwDataSize;
	DWORD dwVersion=0;
	DWORD dwStride=0;

	uint8_t* p = file.data();
	memcpy(&dwFourCC, p, sizeof(DWORD));
	p += sizeof(DWORD);

	if (dwFourCC == MAKEFOURCC('M', 'I', 'P', 'X'))
	{
		memcpy(&dwVersion, p, sizeof(DWORD));
		p += sizeof(DWORD);

		memcpy(&dwStride, p, sizeof(DWORD));
		p += sizeof(DWORD);
	
		if (dwVersion != 1)
		{
			TraceError("CPythonItem::LoadItemTable: invalid item_proto[%s] VERSION[%d]", c_szFileName, dwVersion);
			return false;
		}
		if (dwStride != sizeof(CItemData::TItemTable))
		{
			TraceError("CPythonItem::LoadItemTable: invalid item_proto[%s] STRIDE[%d] != sizeof(SItemTable)", 
				c_szFileName, dwStride, sizeof(CItemData::TItemTable));
			return false;
		}
	}
	else if (dwFourCC != MAKEFOURCC('M', 'I', 'P', 'T'))
	{
		TraceError("CPythonItem::LoadItemTable: invalid item proto type %s", c_szFileName);
		return false;
	}

	memcpy(&dwElements, p, sizeof(DWORD));
	p += sizeof(DWORD);

	memcpy(&dwDataSize, p, sizeof(DWORD));
	p += sizeof(DWORD);

	BYTE * pbData = new BYTE[dwDataSize];
	memcpy(pbData, p, dwDataSize);

	/////

	CLZObject zObj;

	if (!CLZO::Instance().Decompress(zObj, pbData, s_adwItemProtoKey))
	{
		delete [] pbData;
		return false;
	}

	/////

	char szName[64+1];
	
	CItemData::TItemTable * table = (CItemData::TItemTable *) zObj.GetBuffer();
	std::map<DWORD,DWORD> itemNameMap;

	for (DWORD i = 0; i < dwElements; ++i, ++table)
	{
		CItemData * pItemData;
		DWORD dwVnum = table->dwVnum;

		TItemMap::iterator f = m_ItemMap.find(dwVnum);
		if (m_ItemMap.end() == f)
		{
			_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", dwVnum);

			if (CResourceManager::Instance().IsFileExist(szName) == false)
			{
				std::map<DWORD, DWORD>::iterator itVnum = itemNameMap.find(GetHashCode(table->szName));
				
				if (itVnum != itemNameMap.end())
					_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", itVnum->second);
				else
					_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", dwVnum-dwVnum % 10);

				if (CResourceManager::Instance().IsFileExist(szName) == false)
				{
					#ifdef _DEBUG
					TraceError("%16s(#%-5d) cannot find icon file. setting to default.", table->szName, dwVnum);
					#endif
					const DWORD EmptyBowl = 27995;
					_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", EmptyBowl);
				}
			}
				
			pItemData = CItemData::New();

			pItemData->SetDefaultItemData(szName);
			m_ItemMap.insert(TItemMap::value_type(dwVnum, pItemData));
		}
		else
		{
			pItemData = f->second;
		}	
		if (itemNameMap.find(GetHashCode(table->szName)) == itemNameMap.end())
			itemNameMap.insert(std::map<DWORD,DWORD>::value_type(GetHashCode(table->szName),table->dwVnum));
		pItemData->SetItemTableData(table);
		if (0 != table->dwVnumRange)
		{
			m_vec_ItemRange.push_back(pItemData);
		}
	}

	delete [] pbData;
	return true;
}

bool CItemManager::LoadItemScale(const char* c_szFileName)
{
	TPackFile kFile;
	if (!CPackManager::Instance().GetFile(c_szFileName, kFile))
	{
		Tracenf("CItemManager::LoadItemScale(c_szFileName=%s) - Load Error", c_szFileName);
		return false;
	}

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.size(), kFile.data());

	CTokenVector kTokenVector;
	for (DWORD i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		if (!kTextFileLoader.SplitLineByTab(i, &kTokenVector))
		{
			continue;
		}

		if (kTokenVector.size() < ITEMSCALE_NUM)
		{
			TraceError("LoadItemScale: invalid line %d (%s).", i, c_szFileName);
			continue;
		}

		const std::string& strJob = kTokenVector[ITEMSCALE_JOB];
		const std::string& strSex = kTokenVector[ITEMSCALE_SEX];
		const std::string& strScaleX = kTokenVector[ITEMSCALE_SCALE_X];
		const std::string& strScaleY = kTokenVector[ITEMSCALE_SCALE_Y];
		const std::string& strScaleZ = kTokenVector[ITEMSCALE_SCALE_Z];
		const std::string& strPositionX = kTokenVector[ITEMSCALE_POSITION_X];
		const std::string& strPositionY = kTokenVector[ITEMSCALE_POSITION_Y];
		const std::string& strPositionZ = kTokenVector[ITEMSCALE_POSITION_Z];

		for (int j = 0; j < 5; ++j)
		{
			CItemData* pItemData = MakeItemData(atoi(kTokenVector[ITEMSCALE_VNUM].c_str()) + j);
			pItemData->SetItemScale(strJob, strSex, strScaleX, strScaleY, strScaleZ, strPositionX, strPositionY, strPositionZ);
		}
	}

	return true;
}

bool CItemManager::LoadItemTableFromApi(const char* c_szApiUrl, const char* c_szPath)
{
	auto ToWide = [](const char* str) -> std::wstring {
		if (!str) return L"";
		int len = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
		std::wstring result(len, 0);
		MultiByteToWideChar(CP_ACP, 0, str, -1, result.data(), len);
		return result;
		};

	std::vector<uint8_t> response;
	if (!HttpGet(ToWide(c_szApiUrl).c_str(), ToWide(c_szPath).c_str(), response))
	{
		TraceError("LoadItemTableFromAPI: HTTP request failed");
		return false;
	}

	try
	{
		msgpack::object_handle oh = msgpack::unpack(
			reinterpret_cast<const char*>(response.data()), response.size());

		msgpack::object root = oh.get();

		if (root.type != msgpack::type::ARRAY)
		{
			TraceError("LoadItemTableFromAPI: expected root array");
			return false;
		}

		char szName[64 + 1];
		std::map<DWORD, DWORD> itemNameMap;  // hash nazwy ? vnum (do szukania ikon)

		for (uint32_t i = 0; i < root.via.array.size; ++i)
		{
			msgpack::object& item = root.via.array.ptr[i];

			if (item.type != msgpack::type::ARRAY || item.via.array.size < 23)
			{
				TraceError("LoadItemTableFromAPI: invalid item at index %u", i);
				continue;
			}

			auto& f = item.via.array.ptr;

			CItemData::TItemTable table{};

			table.dwVnum = f[0].as<uint32_t>();
			table.dwVnumRange = f[1].as<uint32_t>();

			auto name = f[2].as<std::string>();
			strncpy_s(table.szName, sizeof(table.szName), name.c_str(), _TRUNCATE);
			strncpy_s(table.szLocaleName, sizeof(table.szLocaleName), name.c_str(), _TRUNCATE);

			table.bType = f[4].as<uint8_t>();
			table.bSubType = f[5].as<uint8_t>();
			table.bWeight = f[6].as<uint8_t>();
			table.bSize = f[7].as<uint8_t>();

			table.dwAntiFlags = f[8].as<uint32_t>();
			table.dwFlags = f[9].as<uint32_t>();
			table.dwWearFlags = f[10].as<uint32_t>();
			table.dwImmuneFlag = f[11].as<uint32_t>();

			table.dwIBuyItemPrice = f[12].as<uint32_t>();
			table.dwISellItemPrice = f[13].as<uint32_t>();

			if (f[14].type == msgpack::type::ARRAY)
			{
				auto& limits = f[14].via.array;
				for (uint32_t j = 0; j < std::min(limits.size, (uint32_t)2); ++j)
				{
					if (limits.ptr[j].type == msgpack::type::ARRAY && limits.ptr[j].via.array.size >= 2)
					{
						table.aLimits[j].bType = limits.ptr[j].via.array.ptr[0].as<uint8_t>();
						table.aLimits[j].lValue = limits.ptr[j].via.array.ptr[1].as<int32_t>();
					}
				}
			}

			if (f[15].type == msgpack::type::ARRAY)
			{
				auto& applies = f[15].via.array;
				for (uint32_t j = 0; j < std::min(applies.size, (uint32_t)3); ++j)
				{
					if (applies.ptr[j].type == msgpack::type::ARRAY && applies.ptr[j].via.array.size >= 2)
					{
						table.aApplies[j].bType = applies.ptr[j].via.array.ptr[0].as<uint8_t>();
						table.aApplies[j].lValue = applies.ptr[j].via.array.ptr[1].as<int32_t>();
					}
				}
			}

			if (f[16].type == msgpack::type::ARRAY)
			{
				auto& values = f[16].via.array;
				for (uint32_t j = 0; j < std::min(values.size, (uint32_t)6); ++j)
					table.alValues[j] = values.ptr[j].as<int32_t>();
			}

			if (f[17].type == msgpack::type::ARRAY)
			{
				auto& sockets = f[17].via.array;
				for (uint32_t j = 0; j < std::min(sockets.size, (uint32_t)3); ++j)
					table.alSockets[j] = sockets.ptr[j].as<int32_t>();
			}

			table.dwRefinedVnum = f[18].as<uint32_t>();
			table.wRefineSet = f[19].as<uint16_t>();
			table.bAlterToMagicItemPct = f[20].as<uint8_t>();
			table.bSpecular = f[21].as<uint8_t>();
			table.bGainSocketPct = f[22].as<uint8_t>();

			// === Logika ikon + m_ItemMap (tak samo jak LoadItemTable) ===
			CItemData* pItemData;
			DWORD dwVnum = table.dwVnum;

			TItemMap::iterator it = m_ItemMap.find(dwVnum);
			if (m_ItemMap.end() == it)
			{
				_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", dwVnum);

				if (!CResourceManager::Instance().IsFileExist(szName))
				{
					auto itHash = itemNameMap.find(GetHashCode(table.szName));
					if (itHash != itemNameMap.end())
						_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", itHash->second);
					else
						_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", dwVnum - dwVnum % 10);

					if (!CResourceManager::Instance().IsFileExist(szName))
					{
						TraceError("%16s(#%-5d) cannot find icon file. setting to default.", table.szName, dwVnum);
						const DWORD EmptyBowl = 27995;
						_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", EmptyBowl);
					}
				}

				pItemData = CItemData::New();
				pItemData->SetDefaultItemData(szName);
				m_ItemMap.insert(TItemMap::value_type(dwVnum, pItemData));
			}
			else
			{
				pItemData = it->second;
			}

			if (itemNameMap.find(GetHashCode(table.szName)) == itemNameMap.end())
				itemNameMap.insert(std::map<DWORD, DWORD>::value_type(GetHashCode(table.szName), dwVnum));

			pItemData->SetItemTableData(&table);

			if (table.dwVnumRange != 0)
				m_vec_ItemRange.push_back(pItemData);
		}
	}
	catch (const msgpack::unpack_error& e)
	{
		TraceError("LoadItemTableFromAPI: unpack error: %s", e.what());
		return false;
	}
	catch (const std::exception& e)
	{
		TraceError("LoadItemTableFromAPI: error: %s", e.what());
		return false;
	}

	return true;
}

bool CItemManager::HttpGet(const wchar_t* host, const wchar_t* path, std::vector<uint8_t>& outResponse)
{
	HINTERNET hSession = WinHttpOpen(
		L"Metin2Client/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	if (!hSession)
	{
		return false;
	}

	HINTERNET hConnect = WinHttpConnect(hSession, host, 5001, 0);
	if (!hConnect)
	{
		WinHttpCloseHandle(hSession);
		return false;
	}

	HINTERNET hRequest = WinHttpOpenRequest(
		hConnect, L"GET", path,
		NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES, 0); // 0 = HTTP, WINHTTP_FLAG_SECURE = HTTPS

	if (!hRequest)
	{
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return false;
	}

	WinHttpAddRequestHeaders(hRequest, L"Accept: application/x-msgpack", -1L, WINHTTP_ADDREQ_FLAG_ADD);

	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
	{
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return false;
	}

	WinHttpReceiveResponse(hRequest, NULL);

	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	std::string response;

	do
	{
		WinHttpQueryDataAvailable(hRequest, &dwSize);
		if (dwSize == 0)
		{
			break;
		}

		std::vector<uint8_t> buffer(dwSize);
		WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded);
		outResponse.insert(outResponse.end(), buffer.begin(), buffer.begin() + dwDownloaded);
	} while (dwSize > 0);

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	return true;
}

void CItemManager::Destroy()
{
	TItemMap::iterator i;
	for (i=m_ItemMap.begin(); i!=m_ItemMap.end(); ++i)
		CItemData::Delete(i->second);

	m_ItemMap.clear();
}

CItemManager::CItemManager() : m_pSelectedItemData(NULL)
{
}
CItemManager::~CItemManager()
{
	Destroy();
}
