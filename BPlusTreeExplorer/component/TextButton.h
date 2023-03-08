#pragma once

#include "WndDesign/frame/ClipFrame.h"
#include "WndDesign/control/TextBox.h"
#include "WndDesign/wrapper/Background.h"

#include <functional>


BEGIN_NAMESPACE(WndDesign)


class TextButton : public SolidColorBackground<ClipFrame<Assigned, Assigned>> {
protected:
	struct TextBoxStyle : TextBox::Style {
		TextBoxStyle() {
			paragraph.text_align(TextAlign::Center);
		}
	};
public:
	TextButton(std::wstring text, std::function<void(void)> callback) : Base{
		new TextBox(TextBoxStyle(), std::move(text))
	}, callback(callback) {
		background = background_normal;
	}
private:
	virtual ref_ptr<WndObject> HitTest(Point& point) override { return this; }
protected:
	enum class ButtonState { Normal, Hover, Press } state = ButtonState::Normal;
	std::function<void(void)> callback;
protected:
	static constexpr Color background_normal = color_transparent;
	static constexpr Color background_hover = Color::Gray;
	static constexpr Color background_press = Color::DimGray;
protected:
	void SetBackgroundColor(Color color) { if (this->background != color) { this->background = color; this->Redraw(region_infinite); } }
protected:
	void OnHover() { SetBackgroundColor(background_hover); }
	void OnPress() { SetBackgroundColor(background_press); }
	void OnLeave() { SetBackgroundColor(background_normal); }
protected:
	void OnClick() { callback(); }
protected:
	virtual void OnMouseMsg(MouseMsg msg) override {
		switch (msg.type) {
		case MouseMsg::LeftDown: state = ButtonState::Press; OnPress(); break;
		case MouseMsg::LeftUp: if (state == ButtonState::Press) { state = ButtonState::Hover; OnHover(); OnClick(); } break;
		}
	}
	virtual void OnNotifyMsg(NotifyMsg msg) override {
		switch (msg) {
		case NotifyMsg::MouseEnter: if (state == ButtonState::Normal) { state = ButtonState::Hover; OnHover(); } break;
		case NotifyMsg::MouseLeave: state = ButtonState::Normal; OnLeave(); break;
		}
	}
};


END_NAMESPACE(WndDesign)