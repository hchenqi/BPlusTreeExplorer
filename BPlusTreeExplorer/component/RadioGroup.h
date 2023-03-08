#pragma once

#include "WndDesign/frame/PaddingFrame.h"
#include "WndDesign/layout/ListLayout.h"
#include "WndDesign/style/value_tag.h"
#include "WndDesign/figure/shape.h"


BEGIN_NAMESPACE(WndDesign)


class RadioGroup : public ListLayout<Vertical> {
public:
	template<class... Ts>
	RadioGroup(Ts... child_args) : ListLayout(0px, (new ItemFrame(std::move(child_args)))...) {}

public:
	struct Item {
		child_ptr child;
		std::function<void(void)> callback;
		bool selected = false;
	};

private:
	class ItemFrame : public PaddingFrame<Assigned, Auto> {
	public:
		ItemFrame(Item item) : PaddingFrame(Padding(20px, 0px, 0px, 0px), std::move(item.child)), callback(item.callback), selected(item.selected) {}

	private:
		RadioGroup& GetParent() { return static_cast<RadioGroup&>(WndObject::GetParent()); }

	private:
		virtual void OnDraw(FigureQueue& figure_queue, Rect draw_region) override {
			PaddingFrame::OnDraw(figure_queue, draw_region);
			figure_queue.add(Point(10px, 10px), new Circle(5px, selected ? Color::Gray : color_transparent, 1.0f, Color::Gray));
		}

	private:
		bool selected;
		std::function<void(void)> callback;
	private:
		virtual ref_ptr<WndObject> HitTest(Point& point) override { return selected ? PaddingFrame::HitTest(point) : this; }
	private:
		void Select() { selected = true; GetParent().OnChildSelect(*this); callback(); Redraw(region_infinite); }
	public:
		void Unselect() { selected = false; Redraw(region_infinite); }
	private:
		virtual void OnMouseMsg(MouseMsg msg) override {
			switch (msg.type) {
			case MouseMsg::LeftDown: Select(); break;
			}
		}
	};

private:
	void OnChildSelect(WndObject& child) {
		for (size_t index = 0; index < Length(); index++) {
			WndObject& wnd = GetChild(index);
			if (&wnd != &child) {
				static_cast<ItemFrame&>(wnd).Unselect();
			}
		}
	}
};


END_NAMESPACE(WndDesign)