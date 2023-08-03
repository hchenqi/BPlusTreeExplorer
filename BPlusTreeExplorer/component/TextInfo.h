#pragma once

#include "WndDesign/control/TextBox.h"
#include "WndDesign/message/state.h"


BEGIN_NAMESPACE(WndDesign)


class TextInfo : public TextBox {
public:
	TextInfo(const State<std::wstring>& state, Style style) :
		TextBox(style, std::move(text)), watcher(state, [&](const std::wstring& text) { Assign(text); }) {
	}
private:
	State<std::wstring>::Watcher watcher;
};


END_NAMESPACE(WndDesign)