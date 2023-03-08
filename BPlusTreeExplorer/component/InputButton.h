#pragma once

#include "WndDesign/frame/ClipFrame.h"
#include "WndDesign/control/TextBox.h"
#include "WndDesign/wrapper/Background.h"

#include <functional>


BEGIN_NAMESPACE(WndDesign)


class InputButton : public SolidColorBackground<ClipFrame<Assigned, Assigned>> {
private:
	struct TextBoxStyle : TextBox::Style {
		TextBoxStyle() {
			paragraph.text_align(TextAlign::Center);
		}
	};

public:
	InputButton(std::wstring text, std::function<void(void)> callback) : Base{
		new TextBox(TextBoxStyle(), std::move(text))
	}, callback(callback) {
		background = background_normal;
	}

private:
	static constexpr Color background_normal = color_transparent;
	static constexpr Color background_hover = Color::Gray;
	static constexpr Color background_press = Color::DimGray;
private:
	void SetBackgroundColor(Color color) { if (this->background != color) { this->background = color; this->Redraw(region_infinite); } }


private:
	enum class State { Normal, Hover, Press, Disabled } state = State::Normal;
	bool disabled = false;
	std::function<void(void)> callback;
private:
	virtual ref_ptr<WndObject> HitTest(Point& point) override { return disabled ? nullptr : this; }
public:
	void Enable() { disabled = false; OnLeave(); }
	void Disable() { disabled = true; OnLeave(); }
private:
	void OnHover() { SetBackgroundColor(background_hover); }
	void OnPress() { SetBackgroundColor(background_press); }
	void OnLeave() { SetBackgroundColor(background_normal); }
protected:
	void OnClick() { callback(); }
protected:
	virtual void OnMouseMsg(MouseMsg msg) override {
		switch (msg.type) {
		case MouseMsg::LeftDown: state = State::Press; OnPress(); break;
		case MouseMsg::LeftUp: if (state == State::Press) { state = State::Hover; OnHover(); OnClick(); } break;
		}
	}
	virtual void OnNotifyMsg(NotifyMsg msg) override {
		switch (msg) {
		case NotifyMsg::MouseEnter: if (state == State::Normal) { state = State::Hover; OnHover(); } break;
		case NotifyMsg::MouseLeave: state = State::Normal; OnLeave(); break;
		}
	}
};


END_NAMESPACE(WndDesign)