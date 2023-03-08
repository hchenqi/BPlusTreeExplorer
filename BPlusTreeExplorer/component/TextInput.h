#pragma once

#include "WndDesign/frame/ClipFrame.h"
#include "WndDesign/frame/PaddingFrame.h"
#include "WndDesign/control/EditBox.h"


BEGIN_NAMESPACE(WndDesign)


class TextInput : public PaddingFrame<Assigned, Assigned> {
public:
	TextInput() : PaddingFrame<Assigned, Assigned> {
		Padding(1px),
		new ClipFrame<Assigned, Assigned>{
			edit_box = new EditBox(EditBox::Style())
		}
	} {
	}

private:
	ref_ptr<EditBox> edit_box;

public:
	const std::wstring& GetText() const { return edit_box->GetText(); }
	void SetText(std::wstring str) { edit_box->Assign(str); }
};


END_NAMESPACE(WndDesign)