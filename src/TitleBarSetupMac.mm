#include <QWidget>
#import <Cocoa/Cocoa.h>

extern "C" void setupTitleBar(WId winId)
{
    if (winId == 0) return;

    NSView* view = (NSView*)winId;
    NSWindow* window = [view window];
    window.titlebarAppearsTransparent = YES;

    NSWindowStyleMask windowMask =
            NSWindowStyleMaskFullSizeContentView
                | NSWindowStyleMaskBorderless
                | NSWindowStyleMaskTitled
                | NSWindowStyleMaskClosable
                | NSWindowStyleMaskMiniaturizable
                | NSWindowStyleMaskResizable;

    [window setStyleMask: windowMask];
}
