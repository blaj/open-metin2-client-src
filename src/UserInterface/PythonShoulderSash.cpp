#include "StdAfx.h"
#include "PythonShoulderSash.h"
#include "PythonNetworkStream.h"

CPythonShoulderSash::CPythonShoulderSash()
{
	Clear();
}

CPythonShoulderSash::~CPythonShoulderSash()
{
	Clear();
}

void CPythonShoulderSash::Clear()
{
	dwPrice = 0;
	
	ZeroMemory(&m_vShoulderSashResult, sizeof(m_vShoulderSashResult));
	
	m_vShoulderSashMaterials.clear();
	m_vShoulderSashMaterials.resize(SHOULDER_SASH_WINDOW_MAX_MATERIALS);
	for (BYTE bPos = 0; bPos < m_vShoulderSashMaterials.size(); ++bPos)
	{
		TShoulderSashMaterial tMaterial;
		tMaterial.bHere = 0;
		tMaterial.wCell = 0;
		m_vShoulderSashMaterials[bPos] = tMaterial;
	}
}

void CPythonShoulderSash::AddMaterial(DWORD dwRefPrice, BYTE bPos, TItemPos tPos)
{
	if (bPos >= SHOULDER_SASH_WINDOW_MAX_MATERIALS)
	{
		return;
	}
	
	if (bPos == 0)
	{
		dwPrice = dwRefPrice;
	}
	
	TShoulderSashMaterial tMaterial;
	tMaterial.bHere = 1;
	tMaterial.wCell = tPos.cell;
	m_vShoulderSashMaterials[bPos] = tMaterial;
}

void CPythonShoulderSash::AddResult(DWORD dwItemVnum, DWORD dwMinAbs, DWORD dwMaxAbs)
{
	TShoulderSashResult tResult;
	tResult.dwItemVnum = dwItemVnum;
	tResult.dwMinAbs = dwMinAbs;
	tResult.dwMaxAbs = dwMaxAbs;
	m_vShoulderSashResult = tResult;
}

void CPythonShoulderSash::RemoveMaterial(DWORD dwRefPrice, BYTE bPos)
{
	if (bPos >= SHOULDER_SASH_WINDOW_MAX_MATERIALS)
	{
		return;
	}
	
	if (bPos == 1)
	{
		dwPrice = dwRefPrice;
	}
	
	TShoulderSashMaterial tMaterial;
	tMaterial.bHere = 0;
	tMaterial.wCell = 0;
	m_vShoulderSashMaterials[bPos] = tMaterial;
}

bool CPythonShoulderSash::GetAttachedItem(BYTE bPos, BYTE & bHere, WORD & wCell)
{
	if (bPos >= SHOULDER_SASH_WINDOW_MAX_MATERIALS)
	{
		return false;
	}
	
	bHere = m_vShoulderSashMaterials[bPos].bHere;
	wCell = m_vShoulderSashMaterials[bPos].wCell;
	return true;
}

void CPythonShoulderSash::GetResultItem(DWORD & dwItemVnum, DWORD & dwMinAbs, DWORD & dwMaxAbs)
{
	dwItemVnum = m_vShoulderSashResult.dwItemVnum;
	dwMinAbs = m_vShoulderSashResult.dwMinAbs;
	dwMaxAbs = m_vShoulderSashResult.dwMaxAbs;
}

PyObject * SendShoulderSashCloseRequest(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendShoulderSashClosePacket();
	return Py_BuildNone();
}

PyObject * SendShoulderSashAdd(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bPos;
	TItemPos tPos;
	if (!PyTuple_GetInteger(poArgs, 0, &tPos.window_type))
	{
		return Py_BuildException();
	}
	else if (!PyTuple_GetInteger(poArgs, 1, &tPos.cell))
	{
		return Py_BuildException();
	}
	else if (!PyTuple_GetInteger(poArgs, 2, &bPos))
	{
		return Py_BuildException();
	}
	
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendShoulderSashAddPacket(tPos, bPos);
	return Py_BuildNone();
}

PyObject * SendShoulderSashRemove(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bPos;
	if (!PyTuple_GetInteger(poArgs, 0, &bPos))
	{
		return Py_BuildException();
	}
	
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendShoulderSashRemovePacket(bPos);
	return Py_BuildNone();
}

PyObject * GetShoulderSashPrice(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonShoulderSash::Instance().GetPrice());
}

PyObject * GetShoulderSashAttachedItem(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bPos;
	if (!PyTuple_GetInteger(poArgs, 0, &bPos))
	{
		return Py_BuildException();
	}
	
	BYTE bHere;
	WORD wCell;
	bool bGet = CPythonShoulderSash::Instance().GetAttachedItem(bPos, bHere, wCell);
	if (!bGet)
	{
		bHere = 0;
		wCell = 0;
	}
	
	return Py_BuildValue("ii", bHere, wCell);
}

PyObject * GetShoulderSashResultItem(PyObject * poSelf, PyObject * poArgs)
{
	DWORD dwItemVnum, dwMinAbs, dwMaxAbs;
	CPythonShoulderSash::Instance().GetResultItem(dwItemVnum, dwMinAbs, dwMaxAbs);
	return Py_BuildValue("iii", dwItemVnum, dwMinAbs, dwMaxAbs);
}

PyObject * SendShoulderSashRefineRequest(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bHere;
	WORD wCell;
	bool bGet = CPythonShoulderSash::Instance().GetAttachedItem(1, bHere, wCell);
	if (bGet)
	{
		if (bHere)
		{
			CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
			rkNetStream.SendShoulderSashRefinePacket();
		}
	}
	
	return Py_BuildNone();
}

void initShoulderSash()
{
	static PyMethodDef functions[] = {
										{"SendCloseRequest", SendShoulderSashCloseRequest, METH_VARARGS},
										{"Add", SendShoulderSashAdd, METH_VARARGS},
										{"Remove", SendShoulderSashRemove, METH_VARARGS},
										{"GetPrice", GetShoulderSashPrice, METH_VARARGS},
										{"GetAttachedItem", GetShoulderSashAttachedItem, METH_VARARGS},
										{"GetResultItem", GetShoulderSashResultItem, METH_VARARGS},
										{"SendRefineRequest", SendShoulderSashRefineRequest, METH_VARARGS},
										{NULL, NULL, NULL},
	};
	
	PyObject* pModule = Py_InitModule("shoulderSash", functions);
	PyModule_AddIntConstant(pModule, "ABSORPTION_SOCKET", SHOULDER_SASH_ABSORPTION_SOCKET);
	PyModule_AddIntConstant(pModule, "ABSORBED_SOCKET", SHOULDER_SASH_ABSORBED_SOCKET);
	PyModule_AddIntConstant(pModule, "CLEAN_ATTR_VALUE0", SHOULDER_SASH_CLEAN_ATTR_VALUE0);
	PyModule_AddIntConstant(pModule, "WINDOW_MAX_MATERIALS", SHOULDER_SASH_WINDOW_MAX_MATERIALS);
	PyModule_AddIntConstant(pModule, "CLEAN_ATTR_VALUE_FIELD", 0);
	PyModule_AddIntConstant(pModule, "LIMIT_RANGE", 1000);
}
