#pragma once

#include "WndDesign/frame/ClipFrame.h"
#include "WndDesign/frame/CenterFrame.h"
#include "WndDesign/frame/PaddingFrame.h"
#include "WndDesign/frame/FixedFrame.h"
#include "WndDesign/frame/MinFrame.h"
#include "WndDesign/frame/InnerBorderFrame.h"
#include "WndDesign/layout/SplitLayout.h"
#include "WndDesign/layout/ListLayout.h"
#include "WndDesign/layout/ListLayoutAuto.h"
#include "WndDesign/control/TextBox.h"
#include "WndDesign/message/state.h"
#include "WndDesign/message/timeout.h"

#include "component/ScrollView.h"

#include <vector>


BEGIN_NAMESPACE(WndDesign)

using index_type = long long;
using value_type = std::wstring;

class NodeView;


class RootView : public ScrollView {
private:
	friend class NodeView;
public:
	RootView();
	~RootView() { Skip(); }

	// root node
private:
	ref_ptr<WndFrameMutable> root_frame;
private:
	NodeView& GetRootNode() const;

	// operation
private:
	Task<> BuildRoot(std::unique_ptr<NodeView> node);
	void DestroyRoot(std::unique_ptr<NodeView> node);
public:
	Task<> Insert(std::pair<index_type, value_type> pair);
	Task<> Find(index_type key);
	Task<> Update(std::pair<index_type, value_type> pair);
	Task<> Delete(index_type key);

	// stepping
public:
	State<bool> run_state = false;
	State<std::wstring> info_state = std::wstring();
public:
	enum class StepMode { None, Timeout, Manuel };
private:
	StepMode step_mode = StepMode::Manuel;
	uint step_delay = 1000;
	Continuation<> continuation = nullptr;
public:
	void SetStepMode(StepMode mode) { Skip(); step_mode = mode; }
private:
	Task<> Step(std::wstring info) {
		info_state.Set(info);
		switch (step_mode) {
		case StepMode::None: break;
		case StepMode::Timeout: co_await SetTimeout(step_delay); break;
		case StepMode::Manuel: co_await SetTask([this](Continuation<> continuation) { this->continuation = continuation; }); break;
		}
		co_return;
	}
public:
	void Next() { if (step_mode == StepMode::Manuel && run_state.Get() == true) { continuation(); } }
	void Skip() { if (step_mode == StepMode::Manuel && run_state.Get() == true) { continuation(); Skip(); } }
};


class NodeView : public InnerBorderFrame<Auto, Auto> {
private:
	friend class RootView;

public:
	NodeView(RootView& root) : InnerBorderFrame{
		border_normal,
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
	}, root(root) {
	}
	~NodeView() {}

private:
	RootView& root;
private:
	Task<> Step(std::wstring info) { return root.Step(info); }

public:
	static constexpr Border border_normal = Border(1px, Color::Black);
	static constexpr Border border_finding = Border(1px, Color::Yellow);
	static constexpr Border border_created = Border(1px, Color::Green);
	static constexpr Border border_updated = border_created;
	static constexpr Border border_deleting = Border(1px, Color::Red);
	static constexpr Border border_splitting = border_deleting;
	static constexpr Border border_merging = border_deleting;
	static constexpr Border border_found = border_deleting;

public:
	void SetBorder(Border border) { InnerBorderFrame::SetBorder(border); root.ScrollIntoView(*this, Extend(Rect(point_zero, size), 30px)); }
	void ResetBorder() { InnerBorderFrame::SetBorder(border_normal); }

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
		IndexView(RootView& root, index_type key) : FixedFrame{
			30px,
			border_frame = new InnerBorderFrame<Assigned, Assigned>{
				border_normal,
				new ClipFrame<Assigned, Assigned> {
					text_box = new TextBox(TextStyle(), std::to_wstring(key))
				}
			}
		}, root(root) {
		}

	private:
		RootView& root;
		ref_ptr<TextBox> text_box;
	public:
		void SetIndex(index_type key) { text_box->Assign(std::to_wstring(key)); }

	public:
		ref_ptr<InnerBorderFrame<Assigned, Assigned>> border_frame;
	public:
		void SetBorder(Border border) { border_frame->SetBorder(border); root.ScrollIntoView(*this, Extend(Rect(point_zero, size), 30px)); }
		void ResetBorder() { border_frame->SetBorder(border_normal); }
	};

	class ValueView : public MinFrame<Auto, Auto> {
	private:
		struct TextStyle : TextBox::Style {};
	public:
		ValueView(value_type value) : MinFrame{
			Size(30px, 30px),
			new PaddingFrame{
				Padding(8px, 0px),
				text_box = new TextBox(TextStyle(), value)
			}
		} {
		}
	private:
		ref_ptr<TextBox> text_box;
	public:
		void SetValue(value_type value) { text_box->Assign(value); }
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
		index_list_view->InsertChild(index, new IndexView(root, key));
		child_list_view->InsertChild(index, new ValueView(value));

		GetIndex(index).SetBorder(border_created);
		co_await Step(L"new leaf node inserted");
		GetIndex(index).ResetBorder();

		if (index == 0) { co_await UpdateParentIndex(); }
		if (index_list.size() > child_number_max) { co_await Split(); }
	}
	Task<> InsertAfter(NodeView& child, std::unique_ptr<NodeView> node) {
		size_t index = child_list_view->GetChildIndex(child) + 1;
		index_type key = node->index_list.front();
		index_list.insert(index_list.begin() + index, key);
		index_list_view->InsertChild(index, new IndexView(root, key));
		child_list_view->InsertChild(index, std::move(node));

		GetIndex(index).SetBorder(border_created);
		co_await Step(L"new internal node inserted");
		GetIndex(index).ResetBorder();

		if (index_list.size() > child_number_max) { co_await Split(); }
	}
	Task<> UpdateIndexAt(NodeView& child) {
		size_t index = child_list_view->GetChildIndex(child);
		index_type key = child.index_list.front();
		index_list[index] = key;
		GetIndex(index).SetIndex(key);

		GetIndex(index).SetBorder(border_updated);
		co_await Step(L"index updated");
		GetIndex(index).ResetBorder();

		if (index == 0) { co_await UpdateParentIndex(); }
	}
	Task<> Split() {
		SetBorder(border_splitting);
		co_await Step(L"node splitting");
		ResetBorder();

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
			co_await GetRoot().BuildRoot(std::move(sibling));
		} else {
			co_await parent_view->InsertAfter(*this, std::move(sibling));
		}
	}
	Task<> Adopt(std::unique_ptr<NodeView> first, std::unique_ptr<NodeView> second) {
		index_type key_first = first->index_list.front(), key_second = second->index_list.front();
		index_list.emplace_back(key_first); index_list.emplace_back(key_second);

		std::vector<ListLayout<Horizontal>::child_ptr> index_list;
		index_list.emplace_back(new IndexView(root, key_first)); index_list.emplace_back(new IndexView(root, key_second));
		index_list_view->InsertChild(0, std::move(index_list));

		std::vector<ListLayoutAuto<Horizontal>::child_ptr> child_list;
		child_list.emplace_back(std::move(first)); child_list.emplace_back(std::move(second));
		InsertChild(0, std::move(child_list));

		SetBorder(border_created);
		co_await Step(L"new root node created");
		ResetBorder();
	}
private:
	Task<> DeleteAt(size_t index) {
		GetIndex(index).SetBorder(border_deleting);
		co_await Step(L"deleting node");
		GetIndex(index).ResetBorder();

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
			SetBorder(border_merging); next_view->SetBorder(border_merging);
			co_await Step(L"nodes merging");
			ResetBorder(); next_view->ResetBorder();

			if (next_view->index_list.size() + index_list.size() > child_number_max) {
				index_list.insert(index_list.end(), next_view->index_list.begin(), next_view->index_list.begin() + 1); next_view->index_list.erase(next_view->index_list.begin(), next_view->index_list.begin() + 1);
				index_list_view->InsertChild(-1, next_view->index_list_view->ExtractChild(0));
				InsertChild(-1, next_view->ExtractChild(0));

				SetBorder(border_updated); next_view->SetBorder(border_updated);
				co_await Step(L"nodes updated");
				ResetBorder(); next_view->ResetBorder();

				co_await next_view->parent_view->UpdateIndexAt(*next_view);
			} else {
				index_list.insert(index_list.end(), next_view->index_list.begin(), next_view->index_list.end()); next_view->index_list.clear();
				index_list_view->InsertChild(-1, next_view->index_list_view->ExtractChild(0, -1));
				InsertChild(-1, next_view->ExtractChild(0, -1));

				SetBorder(border_updated); next_view->SetBorder(border_updated);
				co_await Step(L"nodes updated");
				ResetBorder(); next_view->ResetBorder();

				ref_ptr<NodeView> temp = next_view;
				next_view = next_view->next_view;
				if (next_view) { next_view->prev_view = this; }
				co_await temp->parent_view->DeleteChild(*temp);
			}
		} else if (prev_view) {
			SetBorder(border_merging); prev_view->SetBorder(border_merging);
			co_await Step(L"nodes merging");
			ResetBorder(); prev_view->ResetBorder();

			if (prev_view->index_list.size() + index_list.size() > child_number_max) {
				index_list.insert(index_list.begin(), prev_view->index_list.end() - 1, prev_view->index_list.end()); prev_view->index_list.erase(prev_view->index_list.end() - 1, prev_view->index_list.end());
				index_list_view->InsertChild(0, prev_view->index_list_view->ExtractChild(prev_view->index_list_view->Length() - 1));
				InsertChild(0, prev_view->ExtractChild(prev_view->child_list_view->Length() - 1));

				SetBorder(border_updated); prev_view->SetBorder(border_updated);
				co_await Step(L"nodes updated");
				ResetBorder(); prev_view->ResetBorder();

				co_await parent_view->UpdateIndexAt(*this);
			} else {
				prev_view->index_list.insert(prev_view->index_list.end(), index_list.begin(), index_list.end()); index_list.clear();
				prev_view->index_list_view->InsertChild(-1, index_list_view->ExtractChild(0, -1));
				prev_view->InsertChild(-1, ExtractChild(0, -1));

				SetBorder(border_updated); prev_view->SetBorder(border_updated);
				co_await Step(L"nodes updated");
				ResetBorder(); prev_view->ResetBorder();

				prev_view->next_view = next_view;
				if (next_view) { next_view->prev_view = prev_view; }
				co_await parent_view->DeleteChild(*this);
			}
		} else {
			if (IsLeaf()) { co_return; }
			if (index_list.size() == 1) {
				SetBorder(border_deleting);
				co_await Step(L"deleting root node");
				ResetBorder();

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
			GetIndex(index).SetBorder(border_finding);
			co_await Step(L"finding child node");
			GetIndex(index).ResetBorder();

			co_await GetChild(index).Insert(key, value);
		}
	}
	Task<> Find(index_type key) {
		size_t index = std::upper_bound(index_list.begin(), index_list.end(), key) - index_list.begin();
		if (index == 0) { co_return; } else { index--; }
		if (IsLeaf()) {
			if (index_list[index] != key) { co_return; }

			GetIndex(index).SetBorder(border_found);
			co_await Step(L"key found");
			GetIndex(index).ResetBorder();

			co_return;
		} else {
			GetIndex(index).SetBorder(border_finding);
			co_await Step(L"finding child node");
			GetIndex(index).ResetBorder();

			co_await GetChild(index).Find(key);
		}
	}
	Task<> Update(index_type key, value_type value) {
		size_t index = std::upper_bound(index_list.begin(), index_list.end(), key) - index_list.begin();
		if (index == 0) { co_return; } else { index--; }
		if (IsLeaf()) {
			if (index_list[index] != key) { co_return; }
			static_cast<ValueView&>(child_list_view->GetChild(index)).SetValue(value);

			GetIndex(index).SetBorder(border_updated);
			co_await Step(L"value updated");
			GetIndex(index).ResetBorder();

			co_return;
		} else {
			GetIndex(index).SetBorder(border_finding);
			co_await Step(L"finding child node");
			GetIndex(index).ResetBorder();

			co_await GetChild(index).Update(key, value);
		}
	}
	Task<> Delete(index_type key) {
		size_t index = std::upper_bound(index_list.begin(), index_list.end(), key) - index_list.begin();
		if (index == 0) { co_return; } else { index--; }
		if (IsLeaf()) {
			if (index_list[index] != key) { co_return; }
			co_await DeleteAt(index);
		} else {
			GetIndex(index).SetBorder(border_finding);
			co_await Step(L"finding child node");
			GetIndex(index).ResetBorder();

			co_await GetChild(index).Delete(key);
		}
	}
};


inline RootView::RootView() : ScrollView{
	new PaddingFrame{
		Padding(50px),
		WndDesign::child_ptr<Auto, Auto>() = root_frame = new WndFrameMutable{
			new NodeView(*this)
		}
	}
} {
	GetRootNode().leaf = true;
}

inline NodeView& RootView::GetRootNode() const { return static_cast<NodeView&>(root_frame->GetChild()); }

inline Task<> RootView::BuildRoot(std::unique_ptr<NodeView> node) {
	std::unique_ptr<NodeView> root_old(static_cast<NodeView*>(root_frame->Reset(new NodeView(*this)).release()));
	co_await GetRootNode().Adopt(std::move(root_old), std::move(node));
}

inline void RootView::DestroyRoot(std::unique_ptr<NodeView> node) {
	node->parent_view = nullptr; root_frame->Reset(std::move(node));
}

inline Task<> RootView::Insert(std::pair<index_type, value_type> pair) {
	run_state.Set(true);
	co_await GetRootNode().Insert(pair.first, pair.second);
	run_state.Set(false); info_state.Set(L"");
}

inline Task<> RootView::Find(index_type key) {
	run_state.Set(true);
	co_await GetRootNode().Find(key);
	run_state.Set(false); info_state.Set(L"");
}

inline Task<> RootView::Update(std::pair<index_type, value_type> pair) {
	run_state.Set(true);
	co_await GetRootNode().Update(pair.first, pair.second);
	run_state.Set(false); info_state.Set(L"");
}

inline Task<> RootView::Delete(index_type key) {
	run_state.Set(true);
	co_await GetRootNode().Delete(key);
	run_state.Set(false); info_state.Set(L"");
}


END_NAMESPACE(WndDesign)