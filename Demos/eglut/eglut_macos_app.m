#import <QuartzCore/CAMetalLayer.h>
#import <CoreVideo/CVDisplayLink.h>
#import "eglut_macos_app.h"

extern int macos_main(int argc, const char **argv);
extern void macos_draw_cb(void);

@interface eglutAppDelegate ()

@end

@implementation eglutAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end

@interface eglutViewController () {
    CVDisplayLinkRef _displayLink;
}

@end

@implementation eglutViewController

-(void) dealloc {
    CVDisplayLinkRelease(_displayLink);
    [super dealloc];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    self.view.wantsLayer = YES;		// Back the view with a layer created by the makeBackingLayer method.

    // Do any additional setup after loading the view, typically from a nib.

    NSArray<NSString *> *arguments = [[NSProcessInfo processInfo] arguments];
    int argc = (int)arguments.count;
    const char **argv = calloc(arguments.count, sizeof(const char *));
    for (NSUInteger i = 0; i < arguments.count; ++i) {
        NSString *arg = arguments[i];
        argv[i] = [arg UTF8String];
    }

    macos_main(argc, argv);

    free(argv);
}

- (void)startRun {

    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, renderCallback, (__bridge void *)self);
    CVDisplayLinkStart(_displayLink);
}

static CVReturn renderCallback(CVDisplayLinkRef displayLink,
                               const CVTimeStamp *inNow,
                               const CVTimeStamp *inOutputTime,
                               CVOptionFlags flagsIn,
                               CVOptionFlags *flagsOut,
                               void *displayLinkContext)
{
    macos_draw_cb();
    return kCVReturnSuccess;
}

@end

@interface eglutView()

@end
#pragma mark -
#pragma mark eglutView

@implementation eglutView

/** Indicates that the view wants to draw using the backing layer instead of using drawRect:.  */
-(BOOL) wantsUpdateLayer { return YES; }

/** Returns a Metal-compatible layer. */
+(Class) layerClass { return [CAMetalLayer class]; }

/** If the wantsLayer property is set to YES, this method will be invoked to return a layer instance. */
-(CALayer*) makeBackingLayer {
    CALayer* layer = [self.class.layerClass layer];
    CGSize viewScale = [self convertSizeToBacking: CGSizeMake(1.0, 1.0)];
    layer.contentsScale = MIN(viewScale.width, viewScale.height);
    return layer;
}

@end

FILE *macos_fopen(const char *filename, const char *mode) {
    NSString *file = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:filename] ofType:nil];
    return fopen([file UTF8String], mode);
}
    
