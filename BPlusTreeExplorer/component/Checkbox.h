#pragma once

#include "WndDesign/figure/shape.h"
#include "WndDesign/control/Placeholder.h"

#include <functional>


BEGIN_NAMESPACE(WndDesign)


class Checkbox : public Placeholder<Auto, Auto> {
public:
	Checkbox(std::function<void(bool)> callback) : Placeholder(Size(20.0f, 20.0f)), callback(callback) {}

	// state
protected:
	bool checked = true;
	std::function<void(bool)> callback;
public:
	bool Checked() const { return checked; }

	// paint
protected:
	virtual void OnDraw(FigureQueue& figure_queue, Rect draw_region) override {
		figure_queue.add(point_zero, new Rectangle(size, 2.0f, Color::Gray));
		if (checked) {
			figure_queue.add(Point(4.0f, 10.0f), new Line(Vector(4.0f, 6.0f), 2.0f, Color::Gray));
			figure_queue.add(Point(8.0f, 16.0f), new Line(Vector(8.0f, -12.0f), 2.0f, Color::Gray));
		}
	}

	// message
protected:
	bool mouse_press = false;
protected:
	virtual void OnMouseMsg(MouseMsg msg) override {
		switch (msg.type) {
		case MouseMsg::LeftDown: mouse_press = true; break;
		case MouseMsg::LeftUp: if (mouse_press) { checked = !checked; callback(checked); Redraw(region_infinite); } break;
		}
	}
	virtual void OnNotifyMsg(NotifyMsg msg) override {
		switch (msg) {
		case NotifyMsg::MouseLeave: mouse_press = false; break;
		}
	}
};


END_NAMESPACE(WndDesign)