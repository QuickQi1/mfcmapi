#pragma once
class CHierarchyTableTreeCtrl;
class CSingleMAPIPropListCtrl;
class CParentWnd;
class CMapiObjects;

#include "HierarchyTableDlg.h"

class CMsgStoreDlg : public CHierarchyTableDlg
{
public:
	CMsgStoreDlg(
		_In_ CParentWnd* pParentWnd,
		_In_ CMapiObjects* lpMapiObjects,
		_In_opt_ LPMAPIFOLDER lpRootFolder,
		ULONG ulDisplayFlags);
	virtual ~CMsgStoreDlg();

private:
	// Overrides from base class
	void OnDeleteSelectedItem() override;
	void HandleAddInMenuSingle(
		_In_ LPADDINMENUPARAMS lpParams,
		_In_ LPMAPIPROP lpMAPIProp,
		_In_ LPMAPICONTAINER lpContainer) override;
	void HandleCopy() override;
	_Check_return_ bool HandlePaste() override;
	void OnInitMenu(_In_ CMenu* pMenu);

	// Menu items
	void OnCreateSubFolder();
	void OnDisplayACLTable();
	void OnDisplayAssociatedContents();
	void OnDisplayCalendarFolder();
	void OnDisplayContactsFolder();
	void OnDisplayDeletedContents();
	void OnDisplayDeletedSubFolders();
	void OnDisplayInbox();
	void OnDisplayMailboxTable();
	void OnDisplayOutgoingQueueTable();
	void OnDisplayReceiveFolderTable();
	void OnDisplayRulesTable();
	void OnDisplaySpecialFolder(ULONG ulFolder);
	void OnDisplayTasksFolder();
	void OnEmptyFolder();
	void OnOpenFormContainer();
	void OnSaveFolderContentsAsMSG();
	void OnSaveFolderContentsAsTextFiles();
	void OnResolveMessageClass();
	void OnSelectForm();
	void OnSetReceiveFolder();
	void OnResendAllMessages();
	void OnResetPermissionsOnItems();
	void OnRestoreDeletedFolder();
	void OnValidateIPMSubtree();
	void OnPasteFolder();
	void OnPasteFolderContents();
	void OnPasteRules();
	void OnPasteMessages();

	DECLARE_MESSAGE_MAP()
};