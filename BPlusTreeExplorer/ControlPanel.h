#pragma once

#include "WndDesign/layout/ListLayout.h"
#include "WndDesign/control/TextBox.h"
#include "WndDesign/frame/ClipFrame.h"
#include "WndDesign/layout/TableLayout.h"

#include "component/RadioGroup.h"

#include "NodeView.h"


BEGIN_NAMESPACE(WndDesign)


class ControlPanel : public WndFrame, LayoutType<Assigned, Assigned> {
public:
	ControlPanel() : WndFrame{
		new ClipFrame<Assigned, Assigned>{
			new ListLayout<Vertical>{
				0px,
				new ClipFrame<Assigned, Auto>(new TextBox(TextBox::Style(), L"stepping mode")),
				new RadioGroup{
					RadioGroup::Item{
						new ClipFrame<Assigned, Auto>(new TextBox(TextBox::Style(), L"none")),
						[this]() { root_view->SetStepMode(RootView::StepMode::None); }
					},
					RadioGroup::Item{
						new ClipFrame<Assigned, Auto>(new TextBox(TextBox::Style(), L"timeout")),
						[this]() { root_view->SetStepMode(RootView::StepMode::Timeout); }
					},
					RadioGroup::Item{
						child_ptr<Assigned, Auto>() = new TableLayout{
							std::vector<TableLayout::ColumnStyle>(2, {33pct}),
							TableLayout::Row{
								{25px},
								new TextBox(TextBox::Style(), L"manual"),
								new InputButton(L"Next", [this]() { root_view->Next(); }),
							}
						},
						[this]() { root_view->SetStepMode(RootView::StepMode::Manuel); },
						true
					},
				}
			}
		}
	} {
	}

public:
	ref_ptr<RootView> root_view = nullptr;
};


END_NAMESPACE(WndDesign)