#include "WndDesign/window/Global.h"
#include "WndDesign/widget/TitleBarFrame.h"
#include "WndDesign/frame/CenterFrame.h"

#include "NodeView.h"
#include "InputPanel.h"
#include "ControlPanel.h"


using namespace WndDesign;


class MainFrame : public TitleBarFrame {
private:
	struct Style : TitleBarFrame::Style {
		Style() {
			title.assign(L"B+ Tree Explorer");
		}
	};

public:
	MainFrame() : TitleBarFrame{
		Style(),
		new SplitLayoutVertical{
			new CenterFrame<Assigned, Assigned>{
				root_view = new RootView()
			},
			new SplitLayoutVertical{
				new ClipFrame<Assigned, Auto>(text_info = new TextBox(TextBox::Style(), L"")),
				new SolidColorBackground<FixedFrame<Assigned, Auto>, Color::LightSteelBlue>{
					102px,
					new DivideLayout<Horizontal>{
						input_panel = new InputPanel(),
						control_panel = new ControlPanel()
					}
				}
			}
		}
	} {
		input_panel->root_view = root_view;
		control_panel->root_view = root_view;
		root_view->OnStateUpdate = [&](bool running, std::wstring info) {
			running ? input_panel->Disable() : input_panel->Enable();
			text_info->Assign(info);
		};
	}

private:
	ref_ptr<RootView> root_view;
	ref_ptr<InputPanel> input_panel;
	ref_ptr<ControlPanel> control_panel;
	ref_ptr<TextBox> text_info;
};

int main() {
	global.AddWnd(new MainFrame());
	global.MessageLoop();
}
