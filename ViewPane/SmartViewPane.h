#pragma once
#include "DropDownPane.h"
#include "TextPane.h"

class SmartViewPane : public DropDownPane
{
public:
	static SmartViewPane* Create(UINT uidLabel);

	void SetStringW(const wstring& szMsg);
	void DisableDropDown();
	void SetParser(__ParsingTypeEnum iParser);
	void Parse(SBinary myBin);

private:
	SmartViewPane();
	ULONG GetFlags() override;
	void Initialize(int iControl, _In_ CWnd* pParent, _In_ HDC hdc) override;
	void SetWindowPos(int x, int y, int width, int height) override;
	int GetFixedHeight() override;
	int GetLines() override;

	void SetMargins(
		int iMargin,
		int iSideMargin,
		int iLabelHeight, // Height of the label
		int iSmallHeightMargin,
		int iLargeHeightMargin,
		int iButtonHeight, // Height of buttons below the control
		int iEditHeight) override; // height of an edit control

	TextPane m_TextPane;
	bool m_bHasData;
	bool m_bDoDropDown;
};