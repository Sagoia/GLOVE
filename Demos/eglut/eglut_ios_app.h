
#import <UIKit/UIKit.h>

@interface eglutAppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) UIViewController *rootController;

@end

@interface eglutView : UIView

@end

@interface eglutViewController : UIViewController

@property (strong, readonly) eglutView *glView;

- (void)startRun;

@end
