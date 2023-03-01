#pragma once

#include "WndDesign/frame/ScrollFrame.h"
#include "WndDesign/frame/CenterFrame.h"
#include "WndDesign/frame/PaddingFrame.h"
#include "WndDesign/frame/FixedFrame.h"
#include "WndDesign/frame/MinMaxFrame.h"
#include "WndDesign/frame/InnerBorderFrame.h"
#include "WndDesign/layout/SplitLayout.h"
#include "WndDesign/layout/ListLayout.h"
#include "WndDesign/layout/ListLayoutAuto.h"
#include "WndDesign/control/TextBox.h"
#include "WndDesign/message/timeout.h"
#include "WndDesign/message/mouse_tracker.h"

#include <vector>
#include <random>


BEGIN_NAMESPACE(WndDesign)

using index_type = long long;
using value_type = std::wstring;

class NodeView;


class RootView : public ScrollFrame<Bidirectional>, public LayoutType<Relative, Relative> {
private:
	friend class NodeView;
public:
	RootView();

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
				UpdateFrameOffset(frame_offset);
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
			UpdateFrameOffset(frame_offset);
		}
	}

private:
	class ChildFrame : public WndFrameMutable, public LayoutType<Auto, Auto> {
	public:
		WndFrameMutable::WndFrameMutable;
		WndFrameMutable::child;
	};
private:
	ref_ptr<ChildFrame> child_frame;
private:
	NodeView& GetChild() const;

private:
	static constexpr uint step_delay = 1000;
private:
	bool task_running = false;
	Continuation<> continuation = nullptr;
private:
	Task<> Step() {
		//return StartTimeout(step_delay);
		return StartTask([this](Continuation<> continuation) { this->continuation = continuation; });
	}
	void Next() {
		if (task_running) { continuation(); }
	}

private:
	void BuildRoot(std::unique_ptr<NodeView> node);
	void DestroyRoot(std::unique_ptr<NodeView> node);
public:
	Task<> Insert(index_type key, value_type value);
	Task<> Delete(index_type key);

private:
	MouseTracker mouse_tracker;
	Point mouse_down_frame_offset;
private:
	virtual ref_ptr<WndObject> HitTest(Point& point) override { return this; }
	virtual void OnMouseMsg(MouseMsg msg) override {
		switch (mouse_tracker.Track(msg)) {
		case MouseTrackMsg::LeftDown:
			mouse_down_frame_offset = frame_offset;
			SetFocus();
			break;
		case MouseTrackMsg::LeftClick:
			if (task_running) {
				Next();
			} else {
				index_type key = rand() % 20; value_type value(1, L'A' + rand() % 26);
				Insert(key, value);
			}
			break;
		case MouseTrackMsg::LeftDrag:
			UpdateFrameOffset(mouse_down_frame_offset - (msg.point - mouse_tracker.mouse_down_position));
			break;
		}
		switch (msg.type) {
		case MouseMsg::RightDown:
			if (task_running) { return; }
			Delete(rand() % 20);
			break;
		default:
			ScrollFrame::OnMouseMsg(msg);
			break;
		}
	}
	virtual void OnKeyMsg(KeyMsg msg) override {
		switch (msg.type) {
		case KeyMsg::KeyDown:
			switch (msg.key) {
			case Key::Right: Next(); break;
			}
		}
	}
};


class NodeView : public WndFrame, public LayoutType<Auto, Auto> {
private:
	friend class RootView;

public:
	NodeView(RootView& root) : WndFrame{
		new InnerBorderFrame{
			Border(1px, Color::Green),
			new PaddingFrame{
				Padding(2px),
				new SplitLayoutVertical{
					new CenterFrame<Assigned, Auto>{
						new PaddingFrame{
							Padding(5px),
							new FixedFrame<Auto, Auto>{
								30px,
								index_list_view = new ListLayout<Horizontal>(1px)
							}
						}
					},
					child_list_view = new ListLayoutAuto<Horizontal>(1px)
				}
			}
		}
	}, root(root) {
	}
	~NodeView() {}

private:
	RootView& root;
private:
	Task<> Step() { return root.Step(); }

private:
	class IndexView : public FixedFrame<Auto, Assigned> {
	private:
		struct TextStyle : TextBox::Style {
			TextStyle() {
				paragraph.text_align(TextAlign::Center);
				font.size(18px);
			}
		};
	public:
		IndexView(index_type key) : FixedFrame{
			30px,
			border_frame = new InnerBorderFrame{
				Border(1px, Color::Black),
				new ClipFrame<Assigned, Assigned> {
					text_box = new TextBox(TextStyle(), std::to_wstring(key))
				}
			}
		} {
		}

	private:
		ref_ptr<TextBox> text_box;
	public:
		void SetIndex(index_type key) { text_box->SetText(std::to_wstring(key)); }

	public:
		static constexpr Border border_normal = Border(1px, Color::Black);
		static constexpr Border border_find = Border(1px, Color::Orange);
		static constexpr Border border_inserted = Border(1px, Color::Green);
		static constexpr Border border_delete = Border(1px, Color::Red);
		static constexpr Border border_updated = Border(1px, Color::Blue);
	public:
		ref_ptr<InnerBorderFrame<Assigned, Assigned>> border_frame;
	public:
		void SetBorderState(Border border) { border_frame->SetBorder(border); }
		void ResetBorderState() { border_frame->SetBorder(border_normal); }
	};

	class ValueView : public MinFrame {
	private:
		struct TextStyle : TextBox::Style {};
	public:
		ValueView(value_type value) : MinFrame{
			Size(30px, 30px),
			new PaddingFrame{
				Padding(8px, 0px),
				new TextBox(TextStyle(), value)
			}
		} {
		}
	};

private:
	static constexpr size_t child_number_max = 3;
	static constexpr size_t child_number_min = (child_number_max + 1) / 2;
	static_assert(child_number_min < child_number_max);
	static_assert(child_number_min >= 2);

private:
	ref_ptr<NodeView> parent_view = nullptr;
	ref_ptr<NodeView> prev_view = nullptr;
	ref_ptr<NodeView> next_view = nullptr;
	bool leaf = false;
private:
	std::vector<index_type> index_list;
	ref_ptr<ListLayout<Horizontal>> index_list_view;
	ref_ptr<ListLayoutAuto<Horizontal>> child_list_view;
private:
	bool IsRoot() const { return parent_view == nullptr; }
	bool IsLeaf() const { return leaf; }
	RootView& GetRoot() const { return root; }
private:
	IndexView& GetIndex(size_t index) const { return static_cast<IndexView&>(index_list_view->GetChild(index)); }
	NodeView& GetChild(size_t index) const { return static_cast<NodeView&>(child_list_view->GetChild(index)); }
private:
	ListLayoutAuto<Horizontal>::child_ptr ExtractChild(size_t index) {
		return child_list_view->ExtractChild(index);
	}
	std::vector<ListLayoutAuto<Horizontal>::child_ptr> ExtractChild(size_t index, size_t count) {
		return child_list_view->ExtractChild(index, count);
	}
	void InsertChild(size_t index, ListLayoutAuto<Horizontal>::child_ptr child) {
		if (!IsLeaf()) { static_cast<NodeView&>(*child).parent_view = this; }
		child_list_view->InsertChild(index, std::move(child));
	}
	void InsertChild(size_t index, std::vector<ListLayoutAuto<Horizontal>::child_ptr> child_list) {
		if (!IsLeaf()) { for (auto& it : child_list) { static_cast<NodeView&>(*it).parent_view = this; } }
		child_list_view->InsertChild(index, std::move(child_list));
	}

	// bottom up
private:
	Task<> UpdateParentIndex() { if (!IsRoot()) { co_await parent_view->UpdateIndexAt(*this); } }
private:
	Task<> InsertAt(size_t index, index_type key, value_type value) {
		index_list.insert(index_list.begin() + index, key);
		index_list_view->InsertChild(index, new IndexView(key));
		child_list_view->InsertChild(index, new ValueView(value));
		GetIndex(index).SetBorderState(IndexView::border_inserted);
		co_await Step();
		GetIndex(index).ResetBorderState();
		if (index == 0) { co_await UpdateParentIndex(); }
		if (index_list.size() > child_number_max) { co_await Split(); }
	}
	Task<> InsertAfter(NodeView& child, std::unique_ptr<NodeView> node) {
		size_t index = child_list_view->GetChildIndex(child) + 1;
		index_type key = node->index_list.front();
		index_list.insert(index_list.begin() + index, key);
		index_list_view->InsertChild(index, new IndexView(key));
		child_list_view->InsertChild(index, std::move(node));
		GetIndex(index).SetBorderState(IndexView::border_inserted);
		co_await Step();
		GetIndex(index).ResetBorderState();
		if (index_list.size() > child_number_max) { co_await Split(); }
	}
	Task<> UpdateIndexAt(NodeView& child) {
		size_t index = child_list_view->GetChildIndex(child);
		index_type key = child.index_list.front();
		index_list[index] = key;
		GetIndex(index).SetIndex(key);
		GetIndex(index).SetBorderState(IndexView::border_updated);
		co_await Step();
		GetIndex(index).ResetBorderState();
		if (index == 0) { co_await UpdateParentIndex(); }
	}
	Task<> Split() {
		size_t index = index_list.size() / 2;
		std::unique_ptr<NodeView> sibling(new NodeView(root));
		sibling->parent_view = parent_view;
		sibling->prev_view = this;
		sibling->next_view = next_view;
		sibling->leaf = leaf;
		sibling->index_list.insert(sibling->index_list.end(), index_list.begin() + index, index_list.end()); index_list.erase(index_list.begin() + index, index_list.end());
		sibling->index_list_view->InsertChild(-1, index_list_view->ExtractChild(index, -1));
		sibling->InsertChild(-1, ExtractChild(index, -1));
		if (next_view) { next_view->prev_view = sibling.get(); }
		next_view = sibling.get();
		if (IsRoot()) {
			GetRoot().BuildRoot(std::move(sibling));
		} else {
			co_await parent_view->InsertAfter(*this, std::move(sibling));
		}
	}
	void Adopt(std::unique_ptr<NodeView> first, std::unique_ptr<NodeView> second) {
		index_type key_first = first->index_list.front(), key_second = second->index_list.front();
		index_list.emplace_back(key_first); index_list.emplace_back(key_second);

		std::vector<ListLayout<Horizontal>::child_ptr> index_list;
		index_list.emplace_back(new IndexView(key_first)); index_list.emplace_back(new IndexView(key_second));
		index_list_view->InsertChild(0, std::move(index_list));

		std::vector<ListLayoutAuto<Horizontal>::child_ptr> child_list;
		child_list.emplace_back(std::move(first)); child_list.emplace_back(std::move(second));
		InsertChild(0, std::move(child_list));
	}
private:
	Task<> DeleteAt(size_t index) {
		GetIndex(index).SetBorderState(IndexView::border_delete);
		co_await Step();
		GetIndex(index).ResetBorderState();
		index_list.erase(index_list.begin() + index);
		index_list_view->EraseChild(index);
		child_list_view->EraseChild(index);
		if (index == 0 && !(index_list.size() < child_number_min && !next_view && prev_view)) { co_await UpdateParentIndex(); }
		if (index_list.size() < child_number_min) { co_await Merge(); }
	}
	Task<> DeleteChild(NodeView& child) {
		co_await DeleteAt(child_list_view->GetChildIndex(child));
	}
	Task<> Merge() {
		if (next_view) {
			if (next_view->index_list.size() + index_list.size() > child_number_max) {
				index_list.insert(index_list.end(), next_view->index_list.begin(), next_view->index_list.begin() + 1); next_view->index_list.erase(next_view->index_list.begin(), next_view->index_list.begin() + 1);
				index_list_view->InsertChild(-1, next_view->index_list_view->ExtractChild(0));
				InsertChild(-1, next_view->ExtractChild(0));
				co_await next_view->parent_view->UpdateIndexAt(*next_view);
			} else {
				index_list.insert(index_list.end(), next_view->index_list.begin(), next_view->index_list.end()); next_view->index_list.clear();
				index_list_view->InsertChild(-1, next_view->index_list_view->ExtractChild(0, -1));
				InsertChild(-1, next_view->ExtractChild(0, -1));
				ref_ptr<NodeView> temp = next_view;
				next_view = next_view->next_view;
				if (next_view) { next_view->prev_view = this; }
				co_await temp->parent_view->DeleteChild(*temp);
			}
		} else if (prev_view) {
			if (prev_view->index_list.size() + index_list.size() > child_number_max) {
				index_list.insert(index_list.begin(), prev_view->index_list.end() - 1, prev_view->index_list.end()); prev_view->index_list.erase(prev_view->index_list.end() - 1, prev_view->index_list.end());
				index_list_view->InsertChild(0, prev_view->index_list_view->ExtractChild(prev_view->index_list_view->Length() - 1));
				InsertChild(0, prev_view->ExtractChild(prev_view->child_list_view->Length() - 1));
				co_await parent_view->UpdateIndexAt(*this);
			} else {
				prev_view->index_list.insert(prev_view->index_list.end(), index_list.begin(), index_list.end()); index_list.clear();
				prev_view->index_list_view->InsertChild(-1, index_list_view->ExtractChild(0, -1));
				prev_view->InsertChild(-1, ExtractChild(0, -1));
				prev_view->next_view = next_view;
				if (next_view) { next_view->prev_view = prev_view; }
				co_await parent_view->DeleteChild(*this);
			}
		} else {
			if (IsLeaf()) { co_return; }
			if (index_list.size() == 1) {
				GetRoot().DestroyRoot(reinterpret_cast<std::unique_ptr<NodeView>&&>(std::move(ExtractChild(0))));
			}
		}
	}

	// top down
private:
	Task<> Insert(index_type key, value_type value) {
		size_t index = std::upper_bound(index_list.begin(), index_list.end(), key) - index_list.begin();
		if (IsLeaf()) {
			co_await InsertAt(index, key, value);
		} else {
			if (index > 0) { index--; }
			GetIndex(index).SetBorderState(IndexView::border_find);
			co_await Step();
			GetIndex(index).ResetBorderState();
			co_await GetChild(index).Insert(key, value);
		}
	}
	Task<> Delete(index_type key) {
		size_t index = std::upper_bound(index_list.begin(), index_list.end(), key) - index_list.begin();
		if (index == 0) { co_return; } else { index--; }
		if (IsLeaf()) {
			if (index_list[index] != key) { co_return; }
			co_await DeleteAt(index);
		} else {
			GetIndex(index).SetBorderState(IndexView::border_find);
			co_await Step();
			GetIndex(index).ResetBorderState();
			co_await GetChild(index).Delete(key);
		}
	}
};


inline RootView::RootView() : ScrollFrame{
	new PaddingFrame{
		Padding(50px),
		child_frame = new ChildFrame{
			new NodeView(*this)
		}
	}
} {
	GetChild().leaf = true;
	srand(time(nullptr));
}

inline NodeView& RootView::GetChild() const { return static_cast<NodeView&>(*child_frame->child); }

inline void RootView::BuildRoot(std::unique_ptr<NodeView> node) {
	std::unique_ptr<NodeView> root_old(static_cast<NodeView*>(child_frame->Reset(new NodeView(*this)).release()));
	GetChild().Adopt(std::move(root_old), std::move(node));
}

inline void RootView::DestroyRoot(std::unique_ptr<NodeView> node) { node->parent_view = nullptr; child_frame->Reset(std::move(node)); }

inline Task<> RootView::Insert(index_type key, value_type value) {
	task_running = true;
	co_await GetChild().Insert(key, value);
	task_running = false;
}

inline Task<> RootView::Delete(index_type key) {
	task_running = true;
	co_await GetChild().Delete(key);
	task_running = false;
}


END_NAMESPACE(WndDesign)