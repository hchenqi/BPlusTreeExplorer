#pragma once

#include "WndDesign/frame/ScrollFrame.h"
#include "WndDesign/message/mouse_tracker.h"


BEGIN_NAMESPACE(WndDesign)


class ScrollView : public ScrollFrame<Bidirectional>, public LayoutType<Relative, Relative> {
public:
	using ScrollFrame::ScrollFrame;

	// layout
private:
	Size size_ref;
private:
	virtual Size OnSizeRefUpdate(Size size_ref) override {
		if (this->size_ref != size_ref) {
			this->size_ref = size_ref;
			Size size = Size(std::min(size_ref.width, child_size.width), std::min(size_ref.height, child_size.height));
			if (this->size != size) {
				this->size = size;
				frame_offset = ClampFrameOffset(frame_offset);
			}
		}
		return size;
	}
	virtual void OnChildSizeUpdate(WndObject& child, Size child_size) override {
		if (this->child_size != child_size) {
			this->child_size = child_size;
			Size size = Size(std::min(size_ref.width, child_size.width), std::min(size_ref.height, child_size.height));
			if (this->size != size) {
				this->size = size;
				SizeUpdated(size);
			}
			frame_offset = ClampFrameOffset(frame_offset);
		}
	}
public:
	void ScrollIntoView(WndObject& descendent, Rect region) {
		Rect frame(point_zero, size); region = Rect(child->ConvertDescendentPoint(descendent, region.point), region.size);
		ScrollFrame::ScrollIntoView(region.Intersect(frame + (region.Center() - frame.Center())));
	}

	// message
private:
	MouseTracker mouse_tracker;
	Point mouse_down_frame_offset;
private:
	virtual ref_ptr<WndObject> HitTest(MouseMsg& msg) override { return this; }
private:
	virtual void OnMouseMsg(MouseMsg msg) override {
		switch (msg.type) {
		case MouseMsg::WheelVertical: Scroll(Vector(0.0f, (float)-msg.wheel_delta)); break;
		case MouseMsg::WheelHorizontal: Scroll(Vector((float)msg.wheel_delta, 0.0f)); break;
		case MouseMsg::LeftDown: SetCapture(); break;
		case MouseMsg::LeftUp: ReleaseCapture(); break;
		}
		switch (mouse_tracker.Track(msg)) {
		case MouseTrackMsg::LeftDown: mouse_down_frame_offset = frame_offset; SetFocus(); break;
		case MouseTrackMsg::LeftDrag: ScrollFrame::ScrollIntoView(Rect(mouse_down_frame_offset - (msg.point - mouse_tracker.mouse_down_position), size)); break;
		}
	}
};


END_NAMESPACE(WndDesign)