// Displays the rules table for a folder
#include "stdafx.h"
#include "RulesDlg.h"
#include <Controls/ContentsTableListCtrl.h>
#include "MapiObjects.h"
#include "ColumnTags.h"
#include <Controls/SingleMAPIPropListCtrl.h>
#include "ImportProcs.h"

static wstring CLASS = L"CRulesDlg";

#define RULE_INCLUDE_ID 0x00000001
#define RULE_INCLUDE_OTHER 0x00000002

CRulesDlg::CRulesDlg(
	_In_ CParentWnd* pParentWnd,
	_In_ CMapiObjects* lpMapiObjects,
	_In_ LPEXCHANGEMODIFYTABLE lpExchTbl) :
	CContentsTableDlg(
		pParentWnd,
		lpMapiObjects,
		IDS_RULESTABLE,
		mfcmapiDO_NOT_CALL_CREATE_DIALOG,
		nullptr,
		LPSPropTagArray(&sptRULECols),
		RULEColumns,
		IDR_MENU_RULES_POPUP,
		MENU_CONTEXT_RULES_TABLE)
{
	TRACE_CONSTRUCTOR(CLASS);
	m_lpExchTbl = lpExchTbl;
	if (m_lpExchTbl) m_lpExchTbl->AddRef();
	m_bIsAB = false;

	CContentsTableDlg::CreateDialogAndMenu(IDR_MENU_RULES);

	CRulesDlg::OnRefreshView();
}

CRulesDlg::~CRulesDlg()
{
	TRACE_DESTRUCTOR(CLASS);

	if (m_lpExchTbl) m_lpExchTbl->Release();
}

BEGIN_MESSAGE_MAP(CRulesDlg, CContentsTableDlg)
	ON_COMMAND(ID_DELETESELECTEDITEM, OnDeleteSelectedItem)
	ON_COMMAND(ID_MODIFYSELECTEDITEM, OnModifySelectedItem)
END_MESSAGE_MAP()

void CRulesDlg::OnInitMenu(_In_opt_ CMenu* pMenu)
{
	if (pMenu)
	{
		if (m_lpContentsTableListCtrl)
		{
			int iNumSel = m_lpContentsTableListCtrl->GetSelectedCount();
			pMenu->EnableMenuItem(ID_DELETESELECTEDITEM, DIMMSOK(iNumSel));
			pMenu->EnableMenuItem(ID_MODIFYSELECTEDITEM, DIMMSOK(iNumSel));
		}
	}

	CContentsTableDlg::OnInitMenu(pMenu);
}

// Clear the current list and get a new one with whatever code we've got in LoadMAPIPropList
void CRulesDlg::OnRefreshView()
{
	auto hRes = S_OK;

	if (!m_lpExchTbl || !m_lpContentsTableListCtrl) return;

	if (m_lpContentsTableListCtrl->IsLoading()) m_lpContentsTableListCtrl->OnCancelTableLoad();
	DebugPrintEx(DBGGeneric, CLASS, L"OnRefreshView", L"\n");

	if (m_lpExchTbl)
	{
		LPMAPITABLE lpMAPITable = nullptr;
		// Open a MAPI table on the Exchange table property. This table can be
		// read to determine what the Exchange table looks like.
		EC_MAPI(m_lpExchTbl->GetTable(0, &lpMAPITable));

		if (lpMAPITable)
		{
			EC_H(m_lpContentsTableListCtrl->SetContentsTable(
				lpMAPITable,
				dfDeleted,
				NULL));

			lpMAPITable->Release();
		}
	}
}

void CRulesDlg::OnDeleteSelectedItem()
{
	auto hRes = S_OK;
	CWaitCursor Wait; // Change the mouse to an hourglass while we work.

	LPROWLIST lpSelectedItems = nullptr;

	EC_H(GetSelectedItems(RULE_INCLUDE_ID, ROW_REMOVE, &lpSelectedItems));

	if (lpSelectedItems)
	{
		EC_MAPI(m_lpExchTbl->ModifyTable(
			0,
			lpSelectedItems));
		MAPIFreeBuffer(lpSelectedItems);
		if (S_OK == hRes) OnRefreshView();
	}
}

void CRulesDlg::OnModifySelectedItem()
{
	auto hRes = S_OK;
	CWaitCursor Wait; // Change the mouse to an hourglass while we work.

	LPROWLIST lpSelectedItems = nullptr;

	EC_H(GetSelectedItems(RULE_INCLUDE_ID | RULE_INCLUDE_OTHER, ROW_MODIFY, &lpSelectedItems));

	if (lpSelectedItems)
	{
		EC_MAPI(m_lpExchTbl->ModifyTable(
			0,
			lpSelectedItems));
		MAPIFreeBuffer(lpSelectedItems);
		if (S_OK == hRes) OnRefreshView();
	}
}

_Check_return_ HRESULT CRulesDlg::GetSelectedItems(ULONG ulFlags, ULONG ulRowFlags, _In_ LPROWLIST* lppRowList) const
{
	if (!lppRowList || !m_lpContentsTableListCtrl) return MAPI_E_INVALID_PARAMETER;
	*lppRowList = nullptr;
	auto hRes = S_OK;
	int iNumItems = m_lpContentsTableListCtrl->GetSelectedCount();

	if (!iNumItems) return S_OK;
	if (iNumItems > MAXNewROWLIST) return MAPI_E_INVALID_PARAMETER;

	LPROWLIST lpTempList = nullptr;

	EC_H(MAPIAllocateBuffer(CbNewROWLIST(iNumItems), reinterpret_cast<LPVOID*>(&lpTempList)));

	if (lpTempList)
	{
		lpTempList->cEntries = iNumItems;
		auto iSelectedItem = -1;

		for (auto iArrayPos = 0; iArrayPos < iNumItems; iArrayPos++)
		{
			lpTempList->aEntries[iArrayPos].ulRowFlags = ulRowFlags;
			lpTempList->aEntries[iArrayPos].cValues = 0;
			lpTempList->aEntries[iArrayPos].rgPropVals = nullptr;
			iSelectedItem = m_lpContentsTableListCtrl->GetNextItem(
				iSelectedItem,
				LVNI_SELECTED);
			if (-1 != iSelectedItem)
			{
				// TODO: Rewrite with GetSelectedItems
				auto lpData = m_lpContentsTableListCtrl->GetSortListData(iSelectedItem);
				if (lpData)
				{
					if (ulFlags & RULE_INCLUDE_ID && ulFlags & RULE_INCLUDE_OTHER)
					{
						EC_H(MAPIAllocateMore(
							lpData->cSourceProps * sizeof(SPropValue),
							lpTempList,
							reinterpret_cast<LPVOID*>(&lpTempList->aEntries[iArrayPos].rgPropVals)));
						if (SUCCEEDED(hRes) && lpTempList->aEntries[iArrayPos].rgPropVals)
						{
							ULONG ulDst = 0;
							for (ULONG ulSrc = 0; ulSrc < lpData->cSourceProps; ulSrc++)
							{
								if (lpData->lpSourceProps[ulSrc].ulPropTag == PR_RULE_PROVIDER_DATA)
								{
									if (!lpData->lpSourceProps[ulSrc].Value.bin.cb ||
										!lpData->lpSourceProps[ulSrc].Value.bin.lpb)
									{
										// PR_RULE_PROVIDER_DATA was NULL - we don't want this
										continue;
									}
								}

								EC_H(MyPropCopyMore(
									&lpTempList->aEntries[iArrayPos].rgPropVals[ulDst],
									&lpData->lpSourceProps[ulSrc],
									MAPIAllocateMore,
									lpTempList));
								ulDst++;
							}
							lpTempList->aEntries[iArrayPos].cValues = ulDst;
						}
					}
					else if (ulFlags & RULE_INCLUDE_ID)
					{
						lpTempList->aEntries[iArrayPos].cValues = 1;
						lpTempList->aEntries[iArrayPos].rgPropVals = PpropFindProp(
							lpData->lpSourceProps,
							lpData->cSourceProps,
							PR_RULE_ID);
					}
				}
			}
		}
	}

	*lppRowList = lpTempList;
	return hRes;
}

void CRulesDlg::HandleAddInMenuSingle(
	_In_ LPADDINMENUPARAMS lpParams,
	_In_ LPMAPIPROP /*lpMAPIProp*/,
	_In_ LPMAPICONTAINER /*lpContainer*/)
{
	if (lpParams)
	{
		lpParams->lpExchTbl = m_lpExchTbl;
	}

	InvokeAddInMenu(lpParams);
}