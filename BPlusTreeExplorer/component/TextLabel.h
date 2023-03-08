#pragma once

#include "WndDesign/frame/ClipFrame.h"
#include "WndDesign/control/TextBox.h"


BEGIN_NAMESPACE(WndDesign)


class TextLabel : public ClipFrame<Assigned, Assigned> {
private:
	struct TextBoxStyle : TextBox::Style {
		TextBoxStyle() {
			paragraph.text_align(TextAlign::Center);
		}
	};

public:
	TextLabel(std::wstring text) : ClipFrame<Assigned, Assigned>{
		new TextBox(TextBoxStyle(), std::move(text))
	} {
	}
};


END_NAMESPACE(WndDesign)