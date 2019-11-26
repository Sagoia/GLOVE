#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

@interface eglutAppDelegate : NSResponder <NSApplicationDelegate>

@property (strong, nonatomic) NSWindow *window;
@property (strong, nonatomic) NSViewController *rootController;

@end

#pragma mark -
#pragma mark eglutView

@interface eglutView : NSView

@end

@interface eglutViewController : NSViewController

@property (strong, readonly) eglutView *glView;

- (void)startRun;

@end
