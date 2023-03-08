#pragma once

#include "WndDesign/frame/ClipFrame.h"
#include "WndDesign/frame/InnerBorderFrame.h"
#include "WndDesign/frame/PaddingFrame.h"
#include "WndDesign/control/EditBox.h"


BEGIN_NAMESPACE(WndDesign)


class NumericInput : public InnerBorderFrame<Assigned, Assigned> {
public:
	NumericInput() : InnerBorderFrame<Assigned, Assigned>{
		border_invalid,
		new PaddingFrame{
			Padding(1px),
			new ClipFrame<Assigned, Assigned>{
				edit_view = new EditView(*this)
			}
		}
	} {
	}

private:
	static constexpr Border border_valid = Border(0px, color_transparent);
	static constexpr Border border_invalid = Border(1px, Color::Red);

private:
	class EditView : public EditBox {
	public:
		EditView(NumericInput& input) : EditBox(EditBox::Style()), input(input) {}
	private:
		NumericInput& input;
	private:
		virtual void OnTextUpdate() override { EditBox::OnTextUpdate(); input.OnTextUpdate(text); }
	};

private:
	ref_ptr<EditView> edit_view;
	bool valid = false;
	long long value = 0;
public:
	bool Valid() const { return valid; }
	long long Value() const { return value; }
private:
	void OnTextUpdate(const std::wstring& text) {
		try {
			value = std::stoll(text);
			valid = true;
			SetBorder(border_valid);
		} catch (...) {
			valid = false;
			SetBorder(border_invalid);
		}
	}
public:
	void SetValue(long long value) { edit_view->Assign(std::to_wstring(value)); }
};


END_NAMESPACE(WndDesign)