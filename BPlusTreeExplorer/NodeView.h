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

#include <vector>
#include <optional>

#include <random>


BEGIN_NAMESPACE(WndDesign)

using index_type = long long;
using value_type = std::wstring;

class NodeView;


class ScrollView : public ScrollFrame<Bidirectional> {
public:
	ScrollView();
public:
	ScrollFrame::OnMouseMsg;
};


class RootView : public WndFrameMutable, public LayoutType<Auto, Auto> {
private:
	friend class NodeView;
public:
	RootView();
private:
	NodeView& GetChild() const;
private:
	void BuildRoot(std::unique_ptr<NodeView> node);
	void DestroyRoot(std::unique_ptr<NodeView> node);
public:
	void Insert(index_type key, value_type value);
	void Delete(index_type key);
public:
	ref_ptr<ScrollView> parent = nullptr;
private:
	virtual ref_ptr<WndObject> HitTest(Point& point) override { return this; }
	virtual void OnMouseMsg(MouseMsg msg) override {
		switch (msg.type) {
		case MouseMsg::LeftDown: {
			//static const index_type value[] = { 27, 45, 35, 73, 47, 74, 99, 13, 51, 6, 36, 34, 58, 39, 61, 53 };
			//static size_t index = 0;
			//if (index >= 16) { return; }
			//Insert(value[index++], L"");
			index_type key = rand() % 20; value_type value(1, L'A' + rand() % 26);
			Insert(key, value);
			break;
		}
		case MouseMsg::RightDown: {
			index_type key = rand() % 20;
			Delete(key);
			break;
		}
		default:
			parent->OnMouseMsg(msg);
			break;
		}
	}
};


inline ScrollView::ScrollView() : ScrollFrame(new PaddingFrame(Padding(50px), [&]() {auto root = new RootView(); root->parent = this; return root; }())) { srand(time(nullptr)); }


class NodeView : public WndFrame, public LayoutType<Auto, Auto> {
private:
	friend class RootView;

public:
	NodeView() : WndFrame{
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
	} {
	}

	~NodeView() {}

private:
	static constexpr size_t child_number_max = 3;
	static constexpr size_t child_number_min = (child_number_max + 1) / 2;
	static_assert(child_number_min < child_number_max);
	static_assert(child_number_min >= 2);

private:
	static constexpr uint animation_delay = 1000;

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
	RootView& GetRoot() const { return static_cast<RootView&>(GetParent()); }
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
	void UpdateParentIndex() { if (!IsRoot()) { parent_view->UpdateIndexAt(*this); } }
private:
	void InsertAt(size_t index, index_type key, value_type value) {
		index_list.insert(index_list.begin() + index, key);
		index_list_view->InsertChild(index, new IndexView(key));
		child_list_view->InsertChild(index, new ValueView(value));
		GetIndex(index).SetBorderState(IndexView::border_inserted);
		SetTimeout([=, this]() {
			GetIndex(index).ResetBorderState();
			if (index == 0) { UpdateParentIndex(); }
			if (index_list.size() > child_number_max) { Split(); }
		}, animation_delay);
	}
	void InsertAfter(NodeView& child, std::unique_ptr<NodeView> node) {
		size_t index = child_list_view->GetChildIndex(child) + 1;
		index_type key = node->index_list.front();
		index_list.insert(index_list.begin() + index, key);
		index_list_view->InsertChild(index, new IndexView(key));
		child_list_view->InsertChild(index, std::move(node));
		GetIndex(index).SetBorderState(IndexView::border_inserted);
		SetTimeout([=, this]() {
			GetIndex(index).ResetBorderState();
			if (index_list.size() > child_number_max) { Split(); }
		}, animation_delay);
	}
	void UpdateIndexAt(NodeView& child) {
		size_t index = child_list_view->GetChildIndex(child);
		index_type key = child.index_list.front();
		index_list[index] = key;
		GetIndex(index).SetBorderState(IndexView::border_updated);
		SetTimeout([=, this]() {
			GetIndex(index).ResetBorderState();
			GetIndex(index).SetIndex(key);
			if (index == 0) { UpdateParentIndex(); }
		}, animation_delay);
	}
	void Split() {
		size_t index = index_list.size() / 2;
		std::unique_ptr<NodeView> sibling(new NodeView());
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
			parent_view->InsertAfter(*this, std::move(sibling));
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
	void DeleteAt(size_t index) {
		GetIndex(index).SetBorderState(IndexView::border_delete);
		SetTimeout([=, this]() {
			GetIndex(index).ResetBorderState();
			index_list.erase(index_list.begin() + index);
			index_list_view->EraseChild(index);
			child_list_view->EraseChild(index);
			if (index == 0 && !(index_list.size() < child_number_min && !next_view && prev_view)) { UpdateParentIndex(); }
			if (index_list.size() < child_number_min) { Merge(); }
		}, animation_delay);
	}
	void DeleteChild(NodeView& child) {
		DeleteAt(child_list_view->GetChildIndex(child));
	}
	void Merge() {
		if (next_view) {
			if (next_view->index_list.size() + index_list.size() > child_number_max) {
				index_list.insert(index_list.end(), next_view->index_list.begin(), next_view->index_list.begin() + 1); next_view->index_list.erase(next_view->index_list.begin(), next_view->index_list.begin() + 1);
				index_list_view->InsertChild(-1, next_view->index_list_view->ExtractChild(0));
				InsertChild(-1, next_view->ExtractChild(0));
				next_view->parent_view->UpdateIndexAt(*next_view);
			} else {
				index_list.insert(index_list.end(), next_view->index_list.begin(), next_view->index_list.end()); next_view->index_list.clear();
				index_list_view->InsertChild(-1, next_view->index_list_view->ExtractChild(0, -1));
				InsertChild(-1, next_view->ExtractChild(0, -1));
				ref_ptr<NodeView> temp = next_view;
				next_view = next_view->next_view;
				if (next_view) { next_view->prev_view = this; }
				temp->parent_view->DeleteChild(*temp);
			}
		} else if (prev_view) {
			if (prev_view->index_list.size() + index_list.size() > child_number_max) {
				index_list.insert(index_list.begin(), prev_view->index_list.end() - 1, prev_view->index_list.end()); prev_view->index_list.erase(prev_view->index_list.end() - 1, prev_view->index_list.end());
				index_list_view->InsertChild(0, prev_view->index_list_view->ExtractChild(prev_view->index_list_view->Length() - 1));
				InsertChild(0, prev_view->ExtractChild(prev_view->child_list_view->Length() - 1));
				parent_view->UpdateIndexAt(*this);
			} else {
				prev_view->index_list.insert(prev_view->index_list.end(), index_list.begin(), index_list.end()); index_list.clear();
				prev_view->index_list_view->InsertChild(-1, index_list_view->ExtractChild(0, -1));
				prev_view->InsertChild(-1, ExtractChild(0, -1));
				prev_view->next_view = next_view;
				if (next_view) { next_view->prev_view = prev_view; }
				parent_view->DeleteChild(*this);
			}
		} else {
			if (IsLeaf()) { return; }
			if (index_list.size() == 1) {
				GetRoot().DestroyRoot(reinterpret_cast<std::unique_ptr<NodeView>&&>(std::move(ExtractChild(0))));
			}
		}
	}

	// top down
private:
	void Insert(index_type key, value_type value) {
		size_t index = std::upper_bound(index_list.begin(), index_list.end(), key) - index_list.begin();
		if (IsLeaf()) {
			InsertAt(index, key, value);
		} else {
			if (index > 0) { index--; }
			GetIndex(index).SetBorderState(IndexView::border_find);
			SetTimeout([=, this]() {
				GetIndex(index).ResetBorderState();
				GetChild(index).Insert(key, value);
			}, animation_delay);
		}
	}
	void Delete(index_type key) {
		size_t index = std::upper_bound(index_list.begin(), index_list.end(), key) - index_list.begin();
		if (index == 0) { return; } else { index--; }
		if (IsLeaf()) {
			if (index_list[index] != key) { return; }
			DeleteAt(index);
		} else {
			GetIndex(index).SetBorderState(IndexView::border_find);
			SetTimeout([=, this]() {
				GetIndex(index).ResetBorderState();
				GetChild(index).Delete(key);
			}, animation_delay);
		}
	}
};


inline RootView::RootView() : WndFrameMutable(new NodeView()) { GetChild().leaf = true; }

inline NodeView& RootView::GetChild() const { return static_cast<NodeView&>(*child); }

inline void RootView::BuildRoot(std::unique_ptr<NodeView> node) {
	std::unique_ptr<NodeView> root_old(static_cast<NodeView*>(Reset(new NodeView()).release()));
	GetChild().Adopt(std::move(root_old), std::move(node));
}

inline void RootView::DestroyRoot(std::unique_ptr<NodeView> node) { node->parent_view = nullptr; Reset(std::move(node)); }

inline void RootView::Insert(index_type key, value_type value) { GetChild().Insert(key, value); }

inline void RootView::Delete(index_type key) { GetChild().Delete(key); }


END_NAMESPACE(WndDesign)