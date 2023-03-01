#include "WndDesign/window/Global.h"
#include "WndDesign/widget/TitleBarFrame.h"

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
			new RootView()
		}
	);
	global.MessageLoop();
}
