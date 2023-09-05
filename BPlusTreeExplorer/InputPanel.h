#pragma once

#include "WndDesign/frame/BorderFrame.h"
#include "WndDesign/layout/SplitLayout.h"
#include "WndDesign/layout/TableLayout.h"
#include "WndDesign/layout/DivideLayout.h"

#include "component/TextLabel.h"
#include "component/Checkbox.h"
#include "component/NumericInput.h"
#include "component/TextInput.h"
#include "component/TextButton.h"

#include "NodeView.h"

#include <random>


BEGIN_NAMESPACE(WndDesign)


class InputPanel : public WndFrame, LayoutType<Assigned, Assigned> {
public:
	InputPanel(RootView& root_view) : WndFrame{
		new SplitLayoutVertical{
			new BorderFrame{
				Border(0px, Color::SlateGray),
				input_group = new InputGroup()
			},
			new DivideLayout<Horizontal>{
				new InputButton(root_view.run_state, L"Insert", [&]() { root_view.Insert(input_group->GetValue()); }),
				new InputButton(root_view.run_state, L"Find", [&]() { root_view.Find(input_group->GetValue().first); }),
				new InputButton(root_view.run_state, L"Update", [&]() { root_view.Update(input_group->GetValue()); }),
				new InputButton(root_view.run_state, L"Delete", [&]() { root_view.Delete(input_group->GetValue().first); }),
			}
		}
	} {
	}

private:
	class InputGroup : public TableLayout {
	public:
		using width_type = Assigned;
		using height_type = Auto;

	public:
		InputGroup() : TableLayout{
			std::vector<TableLayout::ColumnStyle>(2, { 50pct }),
			TableLayout::Row({ 25px, 1px, Color::SlateGray }, new TextLabel(L"random"), new CenterFrame<Auto, Assigned>(checkbox = new Checkbox([this](bool checked) { checked ? GenerateValue() : ResetValue(); }))),
			TableLayout::Row({ 25px, 1px, Color::SlateGray }, new TextLabel(L"key"), numeric_input = new NumericInput()),
			TableLayout::Row({ 25px, 1px, Color::SlateGray }, new TextLabel(L"value"), text_input = new TextInput()),
		} {
			srand(time(nullptr));
			GenerateValue();
		}

	private:
		ref_ptr<Checkbox> checkbox;
		ref_ptr<NumericInput> numeric_input;
		ref_ptr<TextInput> text_input;
	private:
		void GenerateValue() {
			numeric_input->SetValue(rand() % 100);
			text_input->SetText(std::wstring(1, L'A' + rand() % 26));
		}
		void ResetValue() {
			numeric_input->SetValue(0);
			text_input->SetText(L"");
		}
	public:
		std::pair<long long, std::wstring> GetValue() {
			auto value = std::make_pair(numeric_input->Value(), text_input->GetText());
			checkbox->Checked() ? GenerateValue() : ResetValue();
			return value;
		}
	};

	class InputButton : public TextButton {
	public:
		InputButton(const State<bool>& state, std::wstring text, std::function<void(void)> callback) :
			TextButton(text, callback), watcher(state, [&](bool) { OnLeave(); }) {
		}
	private:
		State<bool>::Watcher watcher;
	private:
		virtual ref_ptr<WndObject> HitTest(MouseMsg& msg) override { return *watcher.Get() ? nullptr : this; }
	};

private:
	ref_ptr<InputGroup> input_group;
};


END_NAMESPACE(WndDesign)