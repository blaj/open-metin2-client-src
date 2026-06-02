#include "stdafx.h"
#include "EterBase/lzo.h"
#include "PackLib/PackManager.h"
#include "pythonnonplayer.h"
#include "InstanceBase.h"
#include "PythonCharacterManager.h"
#include <winhttp.h>
#include <msgpack.hpp>

bool CPythonNonPlayer::LoadNonPlayerData(const char * c_szFileName)
{
	static DWORD s_adwMobProtoKey[4] =
	{   
		4813894,
		18955,
		552631,
		6822045
	};

	TPackFile file;

	Tracef("CPythonNonPlayer::LoadNonPlayerData: %s, sizeof(TMobTable)=%u\n", c_szFileName, sizeof(TMobTable));

	if (!CPackManager::Instance().GetFile(c_szFileName, file))
		return false;

	DWORD dwFourCC, dwElements, dwDataSize;
	memcpy(&dwFourCC, file.data(), sizeof(DWORD));

	if (dwFourCC != MAKEFOURCC('M', 'M', 'P', 'T'))
	{
		TraceError("CPythonNonPlayer::LoadNonPlayerData: invalid Mob proto type %s", c_szFileName);
		return false;
	}

	memcpy(&dwElements, file.data() + sizeof(DWORD), sizeof(DWORD));
	memcpy(&dwDataSize, file.data() + sizeof(DWORD) * 2, sizeof(DWORD));

	BYTE * pbData = new BYTE[dwDataSize];
	memcpy(pbData, file.data() + sizeof(DWORD) * 3, dwDataSize);
	/////

	CLZObject zObj;

	if (!CLZO::Instance().Decompress(zObj, pbData, s_adwMobProtoKey))
	{
		delete [] pbData;
		return false;
	}

	if ((zObj.GetSize() % sizeof(TMobTable)) != 0)
	{
		TraceError("CPythonNonPlayer::LoadNonPlayerData: invalid size %u check data format.", zObj.GetSize());
		return false;
	}

	TMobTable * pTable = (TMobTable *) zObj.GetBuffer();
    for (DWORD i = 0; i < dwElements; ++i, ++pTable)
	{
		TMobTable * pNonPlayerData = new TMobTable;

		memcpy(pNonPlayerData, pTable, sizeof(TMobTable));

		//TraceError("%d : %s type[%d] color[%d]", pNonPlayerData->dwVnum, pNonPlayerData->szLocaleName, pNonPlayerData->bType, pNonPlayerData->dwMonsterColor);
		m_NonPlayerDataMap.insert(TNonPlayerDataMap::value_type(pNonPlayerData->dwVnum, pNonPlayerData));
	}

	delete [] pbData;
	return true;
}

bool CPythonNonPlayer::GetName(DWORD dwVnum, const char ** c_pszName)
{
	const TMobTable * p = GetTable(dwVnum);

	if (!p)
		return false;

	*c_pszName = p->szLocaleName;

	return true;
}

bool CPythonNonPlayer::GetInstanceType(DWORD dwVnum, BYTE* pbType)
{
	const TMobTable * p = GetTable(dwVnum);

	// dwVnum를 찾을 수 없으면 플레이어 캐릭터로 간주 한다. 문제성 코드 -_- [cronan]
	if (!p)
		return false;

	*pbType=p->bType;
	
	return true;
}

const CPythonNonPlayer::TMobTable * CPythonNonPlayer::GetTable(DWORD dwVnum)
{
	TNonPlayerDataMap::iterator itor = m_NonPlayerDataMap.find(dwVnum);

	if (itor == m_NonPlayerDataMap.end())
		return NULL;

	return itor->second;
}

BYTE CPythonNonPlayer::GetEventType(DWORD dwVnum)
{
	const TMobTable * p = GetTable(dwVnum);

	if (!p)
	{
		//Tracef("CPythonNonPlayer::GetEventType - Failed to find virtual number\n");
		return ON_CLICK_EVENT_NONE;
	}

	return p->bOnClickType;
}

BYTE CPythonNonPlayer::GetEventTypeByVID(DWORD dwVID)
{
	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);

	if (NULL == pInstance)
	{
		//Tracef("CPythonNonPlayer::GetEventTypeByVID - There is no Virtual Number\n");
		return ON_CLICK_EVENT_NONE;
	}

	WORD dwVnum = pInstance->GetVirtualNumber();
	return GetEventType(dwVnum);
}

DWORD CPythonNonPlayer::GetMonsterColor(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwMonsterColor;
}

const char*	CPythonNonPlayer::GetMonsterName(DWORD dwVnum)
{	
	const CPythonNonPlayer::TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		static const char* sc_szEmpty="";
		return sc_szEmpty;
	}

	return c_pTable->szLocaleName;
}

DWORD CPythonNonPlayer::GetMonsterMaxHP(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		DWORD dwMaxHP = 0;
		return dwMaxHP;
	}

	return c_pTable->dwMaxHP;
}

DWORD CPythonNonPlayer::GetMonsterRaceFlag(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		DWORD dwRaceFlag = 0;
		return dwRaceFlag;
	}

	return c_pTable->dwRaceFlag;
}

DWORD CPythonNonPlayer::GetMonsterLevel(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		DWORD level = 0;
		return level;
	}

	return c_pTable->bLevel;
}

DWORD CPythonNonPlayer::GetMonsterDamage1(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		DWORD range = 0;
		return range;
	}

	return c_pTable->dwDamageRange[0];
}

DWORD CPythonNonPlayer::GetMonsterDamage2(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		DWORD range = 0;
		return range;
	}

	return c_pTable->dwDamageRange[1];
}

DWORD CPythonNonPlayer::GetMonsterExp(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		DWORD dwExp = 0;
		return dwExp;
	}

	return c_pTable->dwExp;
}

float CPythonNonPlayer::GetMonsterDamageMultiply(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		DWORD fDamMultiply = 0;
		return fDamMultiply;
	}

	return c_pTable->fDamMultiply;
}

DWORD CPythonNonPlayer::GetMonsterST(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		DWORD bStr = 0;
		return bStr;
	}

	return c_pTable->bStr;
}

DWORD CPythonNonPlayer::GetMonsterDX(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		DWORD bDex = 0;
		return bDex;
	}

	return c_pTable->bDex;
}

bool CPythonNonPlayer::IsMonsterStone(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		DWORD bType = 0;
		return false;
	}

	return c_pTable->bType == 2;
}
BYTE CPythonNonPlayer::GetMonsterRegenCycle(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bRegenCycle;
}
BYTE CPythonNonPlayer::GetMonsterRegenPercent(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bRegenPercent;
}
DWORD CPythonNonPlayer::GetMonsterGoldMin(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwGoldMin;
}

DWORD CPythonNonPlayer::GetMonsterGoldMax(DWORD dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwGoldMax;
}
DWORD CPythonNonPlayer::GetMonsterResist(DWORD dwVnum, BYTE bResistNum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	if (bResistNum >= MOB_RESISTS_MAX_NUM)
		return 0;

	return c_pTable->cResists[bResistNum];
}

void CPythonNonPlayer::GetMatchableMobList(int iLevel, int iInterval, TMobTableList * pMobTableList)
{
/*
	pMobTableList->clear();

	TNonPlayerDataMap::iterator itor = m_NonPlayerDataMap.begin();
	for (; itor != m_NonPlayerDataMap.end(); ++itor)
	{
		TMobTable * pMobTable = itor->second;

		int iLowerLevelLimit = iLevel-iInterval;
		int iUpperLevelLimit = iLevel+iInterval;

		if ((pMobTable->abLevelRange[0] >= iLowerLevelLimit && pMobTable->abLevelRange[0] <= iUpperLevelLimit) ||
			(pMobTable->abLevelRange[1] >= iLowerLevelLimit && pMobTable->abLevelRange[1] <= iUpperLevelLimit))
		{
			pMobTableList->push_back(pMobTable);
		}
	}
*/
}

void CPythonNonPlayer::Clear()
{
}

void CPythonNonPlayer::Destroy()
{
	for (TNonPlayerDataMap::iterator itor=m_NonPlayerDataMap.begin(); itor!=m_NonPlayerDataMap.end(); ++itor)
	{
		delete itor->second;
	}
	m_NonPlayerDataMap.clear();
	m_MonsterDropMap.clear();
}

CPythonNonPlayer::CPythonNonPlayer()
{
	Clear();
}

CPythonNonPlayer::~CPythonNonPlayer(void)
{
	Destroy();
}

bool CPythonNonPlayer::LoadDropDataFromApi(const char* c_szApiUrl, const char* c_szPath)
{
	CreateDirectoryA("cache", NULL);

	const char* szCacheFile = "cache/monster_drops.bin";
	const char* szChecksumFile = "cache/monster_drops.checksum";

	auto ToWide = [](const char* str) -> std::wstring {
		if (!str)
			return L"";
		int len = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
		std::wstring result(len, 0);
		MultiByteToWideChar(CP_ACP, 0, str, -1, result.data(), len);
		return result;
		};

	std::vector<uint8_t> checksumBytes;
	if (!HttpGet(ToWide(c_szApiUrl).c_str(), L"/api/definitions/monster-drops/checksum", checksumBytes))
	{
		TraceError("CPythonNonPlayer::LoadDropDataFromApi: failed to fetch checksum");
		return LoadDropCache(szCacheFile);
	}

	std::string serverChecksum(checksumBytes.begin(), checksumBytes.end());
	serverChecksum.erase(std::remove(serverChecksum.begin(), serverChecksum.end(), '"'), serverChecksum.end());
	if (!serverChecksum.empty())
		serverChecksum.erase(serverChecksum.find_last_not_of("\r\n ") + 1);

	std::string localChecksum;
	if (FILE* f = fopen(szChecksumFile, "r"))
	{
		char buf[128] = {};
		fgets(buf, sizeof(buf), f);
		fclose(f);
		localChecksum = buf;
		if (!localChecksum.empty())
			localChecksum.erase(localChecksum.find_last_not_of("\r\n ") + 1);
	}

	if (!localChecksum.empty() && localChecksum == serverChecksum)
	{
		Tracef("CPythonNonPlayer::LoadDropDataFromApi: cache hit, loading from %s\n", szCacheFile);
		return LoadDropCache(szCacheFile);
	}

	Tracef("CPythonNonPlayer::LoadDropDataFromApi: cache miss, fetching from API\n");
	std::vector<uint8_t> response;
	if (!HttpGet(ToWide(c_szApiUrl).c_str(), ToWide(c_szPath).c_str(), response))
	{
		TraceError("CPythonNonPlayer::LoadDropDataFromApi: HTTP request failed");
		return LoadDropCache(szCacheFile);
	}

	if (!ParseDropData(response))
		return false;

	SaveDropCache(szCacheFile, response);
	if (FILE* f = fopen(szChecksumFile, "w"))
	{
		fputs(serverChecksum.c_str(), f);
		fclose(f);
	}

	return true;
}

bool CPythonNonPlayer::HttpGet(const wchar_t* host, const wchar_t* path, std::vector<uint8_t>& outResponse)
{
	HINTERNET hSession = WinHttpOpen(
		L"Metin2Client/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession)
		return false;

	HINTERNET hConnect = WinHttpConnect(hSession, host, 5001, 0);
	if (!hConnect)
	{
		WinHttpCloseHandle(hSession);
		return false;
	}

	HINTERNET hRequest = WinHttpOpenRequest(
		hConnect, L"GET", path,
		NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
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
	do
	{
		WinHttpQueryDataAvailable(hRequest, &dwSize);
		if (dwSize == 0)
			break;

		std::vector<uint8_t> buffer(dwSize);
		WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded);
		outResponse.insert(outResponse.end(), buffer.begin(), buffer.begin() + dwDownloaded);
	} while (dwSize > 0);

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
	return true;
}

bool CPythonNonPlayer::SaveDropCache(const char* path, const std::vector<uint8_t>& bytes)
{
	FILE* f = fopen(path, "wb");
	if (!f)
		return false;
	fwrite(bytes.data(), 1, bytes.size(), f);
	fclose(f);
	Tracef("CPythonNonPlayer::LoadDropDataFromApi: saved %zu bytes to cache\n", bytes.size());
	return true;
}

bool CPythonNonPlayer::LoadDropCache(const char* path)
{
	FILE* f = fopen(path, "rb");
	if (!f)
		return false;

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	std::vector<uint8_t> bytes(size > 0 ? size : 0);
	if (size > 0)
		fread(bytes.data(), 1, size, f);
	fclose(f);

	return ParseDropData(bytes);
}

bool CPythonNonPlayer::ParseDropData(const std::vector<uint8_t>& data)
{
	try
	{
		msgpack::object_handle oh = msgpack::unpack(
			reinterpret_cast<const char*>(data.data()), data.size());

		msgpack::object root = oh.get();
		if (root.type != msgpack::type::MAP)
			return false;

		m_MonsterDropMap.clear();

		for (uint32_t i = 0; i < root.via.map.size; ++i)
		{
			auto& kv = root.via.map.ptr[i];
			DWORD dwMonsterVnum = kv.key.as<uint32_t>();

			if (kv.val.type != msgpack::type::ARRAY)
				continue;

			std::vector<TMonsterDrop> drops;
			drops.reserve(kv.val.via.array.size);

			for (uint32_t j = 0; j < kv.val.via.array.size; ++j)
			{
				auto& entry = kv.val.via.array.ptr[j];
				if (entry.type != msgpack::type::ARRAY || entry.via.array.size < 2)
					continue;

				TMonsterDrop drop;
				drop.dwItemVnum = entry.via.array.ptr[0].as<uint32_t>();
				drop.wCount = entry.via.array.ptr[1].as<uint16_t>();
				drops.push_back(drop);
			}

			m_MonsterDropMap[dwMonsterVnum] = std::move(drops);
		}

		Tracef("CPythonNonPlayer::ParseDropData: parsed drops for %zu mobs\n", m_MonsterDropMap.size());
		return true;
	}
	catch (const std::exception& e)
	{
		TraceError("CPythonNonPlayer::ParseDropData: %s", e.what());
		return false;
	}
}

const std::vector<CPythonNonPlayer::TMonsterDrop>* CPythonNonPlayer::GetDrops(DWORD dwVnum) const
{
	TMonsterDropMap::const_iterator it = m_MonsterDropMap.find(dwVnum);
	return it != m_MonsterDropMap.end() ? &it->second : NULL;
}

bool CPythonNonPlayer::LoadNonPlayerDataFromApi(const char* c_szApiUrl, const char* c_szPath)
{
	CreateDirectoryA("cache", NULL);

	const char* szCacheFile = "cache/mob_proto.bin";
	const char* szChecksumFile = "cache/mob_proto.checksum";

	auto ToWide = [](const char* str) -> std::wstring {
		if (!str)
			return L"";
		int len = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
		std::wstring result(len, 0);
		MultiByteToWideChar(CP_ACP, 0, str, -1, result.data(), len);
		return result;
		};

	std::vector<uint8_t> checksumBytes;
	if (!HttpGet(ToWide(c_szApiUrl).c_str(), L"/api/definitions/monsters/checksum", checksumBytes))
	{
		TraceError("CPythonNonPlayer::LoadNonPlayerDataFromApi: failed to fetch checksum");
		return LoadMobProtoCache(szCacheFile);
	}

	std::string serverChecksum(checksumBytes.begin(), checksumBytes.end());
	serverChecksum.erase(std::remove(serverChecksum.begin(), serverChecksum.end(), '"'), serverChecksum.end());
	if (!serverChecksum.empty())
		serverChecksum.erase(serverChecksum.find_last_not_of("\r\n ") + 1);

	std::string localChecksum;
	if (FILE* f = fopen(szChecksumFile, "r"))
	{
		char buf[128] = {};
		fgets(buf, sizeof(buf), f);
		fclose(f);
		localChecksum = buf;
		if (!localChecksum.empty())
			localChecksum.erase(localChecksum.find_last_not_of("\r\n ") + 1);
	}

	if (!localChecksum.empty() && localChecksum == serverChecksum)
	{
		Tracef("CPythonNonPlayer::LoadNonPlayerDataFromApi: cache hit, loading from %s\n", szCacheFile);
		return LoadMobProtoCache(szCacheFile);
	}

	Tracef("CPythonNonPlayer::LoadNonPlayerDataFromApi: cache miss, fetching from API\n");
	std::vector<uint8_t> response;
	if (!HttpGet(ToWide(c_szApiUrl).c_str(), ToWide(c_szPath).c_str(), response))
	{
		TraceError("CPythonNonPlayer::LoadNonPlayerDataFromApi: HTTP request failed");
		return LoadMobProtoCache(szCacheFile);
	}

	if (!ParseMobProto(response))
		return false;

	if (FILE* f = fopen(szCacheFile, "wb"))
	{
		fwrite(response.data(), 1, response.size(), f);
		fclose(f);
	}
	if (FILE* f = fopen(szChecksumFile, "w"))
	{
		fputs(serverChecksum.c_str(), f);
		fclose(f);
	}

	return true;
}

bool CPythonNonPlayer::LoadMobProtoCache(const char* path)
{
	FILE* f = fopen(path, "rb");
	if (!f)
		return false;

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	std::vector<uint8_t> bytes(size > 0 ? size : 0);
	if (size > 0)
		fread(bytes.data(), 1, size, f);
	fclose(f);

	return ParseMobProto(bytes);
}

bool CPythonNonPlayer::ParseMobProto(const std::vector<uint8_t>& data)
{
	try
	{
		msgpack::object_handle oh = msgpack::unpack(
			reinterpret_cast<const char*>(data.data()), data.size());

		msgpack::object root = oh.get();
		if (root.type != msgpack::type::ARRAY)
			return false;

		for (TNonPlayerDataMap::iterator it = m_NonPlayerDataMap.begin(); it != m_NonPlayerDataMap.end(); ++it)
			delete it->second;
		m_NonPlayerDataMap.clear();

		for (uint32_t i = 0; i < root.via.array.size; ++i)
		{
			msgpack::object& mob = root.via.array.ptr[i];
			if (mob.type != msgpack::type::ARRAY || mob.via.array.size < 47)
				continue;

			auto& f = mob.via.array.ptr;

			TMobTable* t = new TMobTable;
			memset(t, 0, sizeof(TMobTable));

			t->dwVnum = f[0].as<uint32_t>();

			auto name = f[1].as<std::string>();
			strncpy_s(t->szName, sizeof(t->szName), name.c_str(), _TRUNCATE);
			strncpy_s(t->szLocaleName, sizeof(t->szLocaleName), name.c_str(), _TRUNCATE);

			t->bType = f[2].as<uint8_t>();
			t->bRank = f[3].as<uint8_t>();
			t->bBattleType = f[4].as<uint8_t>();
			t->bLevel = f[5].as<uint8_t>();
			t->bSize = f[6].as<uint8_t>();
			t->dwGoldMin = f[7].as<uint32_t>();
			t->dwGoldMax = f[8].as<uint32_t>();
			t->dwExp = f[9].as<uint32_t>();
			t->dwMaxHP = f[10].as<uint32_t>();
			t->bRegenCycle = f[11].as<uint8_t>();
			t->bRegenPercent = f[12].as<uint8_t>();
			t->wDef = f[13].as<uint16_t>();
			t->dwAIFlag = f[14].as<uint32_t>();
			t->dwRaceFlag = f[15].as<uint32_t>();
			t->dwImmuneFlag = f[16].as<uint32_t>();
			t->bStr = f[17].as<uint8_t>();
			t->bDex = f[18].as<uint8_t>();
			t->bCon = f[19].as<uint8_t>();
			t->bInt = f[20].as<uint8_t>();
			t->dwDamageRange[0] = f[21].as<uint32_t>();
			t->dwDamageRange[1] = f[22].as<uint32_t>();
			t->sAttackSpeed = f[23].as<int16_t>();
			t->sMovingSpeed = f[24].as<int16_t>();
			t->bAggresiveHPPct = f[25].as<uint8_t>();
			t->wAggressiveSight = f[26].as<uint16_t>();
			t->wAttackRange = f[27].as<uint16_t>();

			if (f[28].type == msgpack::type::ARRAY)
			{
				auto& e = f[28].via.array;
				for (uint32_t j = 0; j < e.size && j < MOB_ENCHANTS_MAX_NUM; ++j)
					t->cEnchants[j] = (char)e.ptr[j].as<int>();
			}
			if (f[29].type == msgpack::type::ARRAY)
			{
				auto& r = f[29].via.array;
				for (uint32_t j = 0; j < r.size && j < MOB_RESISTS_MAX_NUM; ++j)
					t->cResists[j] = (char)r.ptr[j].as<int>();
			}

			t->dwResurrectionVnum = f[30].as<uint32_t>();
			t->dwDropItemVnum = f[31].as<uint32_t>();
			t->bMountCapacity = f[32].as<uint8_t>();
			t->bOnClickType = f[33].as<uint8_t>();
			t->bEmpire = f[34].as<uint8_t>();

			auto folder = f[35].as<std::string>();
			strncpy_s(t->szFolder, sizeof(t->szFolder), folder.c_str(), _TRUNCATE);

			t->fDamMultiply = f[36].as<float>();
			t->dwSummonVnum = f[37].as<uint32_t>();
			t->dwDrainSP = f[38].as<uint32_t>();
			t->dwMonsterColor = f[39].as<uint32_t>();
			t->dwPolymorphItemVnum = f[40].as<uint32_t>();

			if (f[41].type == msgpack::type::ARRAY)
			{
				auto& sk = f[41].via.array;
				for (uint32_t j = 0; j < sk.size && j < MOB_SKILL_MAX_NUM; ++j)
				{
					if (sk.ptr[j].type == msgpack::type::ARRAY && sk.ptr[j].via.array.size >= 2)
					{
						t->Skills[j].dwVnum = sk.ptr[j].via.array.ptr[0].as<uint32_t>();
						t->Skills[j].bLevel = sk.ptr[j].via.array.ptr[1].as<uint8_t>();
					}
				}
			}

			t->bBerserkPoint = f[42].as<uint8_t>();
			t->bStoneSkinPoint = f[43].as<uint8_t>();
			t->bGodSpeedPoint = f[44].as<uint8_t>();
			t->bDeathBlowPoint = f[45].as<uint8_t>();
			t->bRevivePoint = f[46].as<uint8_t>();

			m_NonPlayerDataMap.insert(TNonPlayerDataMap::value_type(t->dwVnum, t));
		}

		Tracef("CPythonNonPlayer::ParseMobProto: parsed %zu mobs\n", m_NonPlayerDataMap.size());
		return true;
	}
	catch (const std::exception& e)
	{
		TraceError("CPythonNonPlayer::ParseMobProto: %s", e.what());
		return false;
	}
}
