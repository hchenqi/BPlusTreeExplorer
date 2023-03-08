#pragma once

#include "WndDesign/frame/ClipFrame.h"
#include "WndDesign/layout/ListLayout.h"
#include "WndDesign/layout/TableLayout.h"
#include "WndDesign/widget/RadioGroup.h"

#include "component/TextLabel.h"
#include "component/TextButton.h"

#include "NodeView.h"


BEGIN_NAMESPACE(WndDesign)


class ControlPanel : public ClipFrame<Assigned, Assigned> {
public:
	ControlPanel(RootView& root_view) : ClipFrame<Assigned, Assigned>{
		new ListLayout<Vertical>{
			0px,
			new ClipFrame<Assigned, Auto>(new TextBox(TextBox::Style(), L"stepping mode")),
			new RadioGroup{
				[&](const RootView::StepMode& mode) { root_view.SetStepMode(mode); },
				RadioItem{
					RootView::StepMode::None,
					new ClipFrame<Assigned, Auto>(new TextBox(TextBox::Style(), L"none")),
				},
				RadioItem{
					RootView::StepMode::Timeout,
					new ClipFrame<Assigned, Auto>(new TextBox(TextBox::Style(), L"timeout")),
				},
				RadioItem{
					RootView::StepMode::Manuel,
					child_ptr<Assigned, Auto>() = new TableLayout{
						std::vector<TableLayout::ColumnStyle>(3, { 33pct, 5px }),
						TableLayout::Row{
							{25px},
							new TextBox(TextBox::Style(), L"manual"),
							new TextButton(L"Next", [&]() { root_view.Next(); }),
							new TextButton(L"Skip", [&]() { root_view.Skip(); }),
						}
					},
					true
				},
			}
		}
	} {
	}
};


END_NAMESPACE(WndDesign)