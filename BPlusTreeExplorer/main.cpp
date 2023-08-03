#include "WndDesign/window/Global.h"
#include "WndDesign/widget/TitleBarFrame.h"
#include "WndDesign/frame/CenterFrame.h"

#include "component/TextInfo.h"

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
		[]() {
			alloc_ptr<RootView> root_view = new RootView();
			return new SplitLayoutVertical{
				new CenterFrame<Assigned, Assigned>{
					root_view
				},
				new SplitLayoutVertical{
					new CenterFrame<Assigned, Auto>(new TextInfo(root_view->info_state, TextInfo::Style())),
					new SolidColorBackground<FixedFrame<Assigned, Auto>, Color::LightSteelBlue>{
						100px,
						new DivideLayout<Horizontal>{
							new InputPanel(*root_view),
							new ControlPanel(*root_view)
						}
					}
				}
			};
		}()
	} {
	}
};

void main() {
	global.AddWnd(new MainFrame());
	global.MessageLoop();
}
