#pragma once

#include "Packet.h"

class CPythonShoulderSash : public CSingleton<CPythonShoulderSash>
{
	public:
		DWORD	dwPrice;
		typedef std::vector<TShoulderSashMaterial> TShoulderSashMaterials;
	
	public:
		CPythonShoulderSash();
		virtual ~CPythonShoulderSash();
		
		void	Clear();
		void	AddMaterial(DWORD dwRefPrice, BYTE bPos, TItemPos tPos);
		void	AddResult(DWORD dwItemVnum, DWORD dwMinAbs, DWORD dwMaxAbs);
		void	RemoveMaterial(DWORD dwRefPrice, BYTE bPos);
		DWORD	GetPrice() {return dwPrice;}
		bool	GetAttachedItem(BYTE bPos, BYTE & bHere, WORD & wCell);
		void	GetResultItem(DWORD & dwItemVnum, DWORD & dwMinAbs, DWORD & dwMaxAbs);
	
	protected:
		TShoulderSashResult	m_vShoulderSashResult;
		TShoulderSashMaterials	m_vShoulderSashMaterials;
};
