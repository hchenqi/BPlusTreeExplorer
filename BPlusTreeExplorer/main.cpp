#include "WndDesign/window/Global.h"
#include "WndDesign/widget/TitleBarFrame.h"
#include "WndDesign/frame/CenterFrame.h"

#include "NodeView.h"


using namespace WndDesign;


struct MainFrameStyle : TitleBarFrame::Style {
	MainFrameStyle() {
		title.assign(L"B+ Tree Explorer");
	}
};


int main() {
	global.AddWnd(
		new TitleBarFrame{
			MainFrameStyle(),
			new CenterFrame<Assigned, Assigned>{
				new RootView()
			}
		}
	);
	global.MessageLoop();
}
