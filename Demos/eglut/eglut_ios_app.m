
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import "eglut_ios_app.h"

extern int ios_main(int argc, const char **argv);
extern void ios_draw_cb(void);

@interface eglutAppDelegate ()

@end

@implementation eglutAppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.rootController = [[eglutViewController alloc] init];
    
    [self.window addSubview:self.rootController.view];
    [self.window setRootViewController:self.rootController];
    
    [self.window makeKeyAndVisible];
    
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


@end

@interface eglutViewController () {
    CADisplayLink* _displayLink;
}

@end

@implementation eglutViewController


- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    _glView = [[eglutView alloc] initWithFrame:self.view.frame];
    [self.view addSubview:_glView];
    
    NSArray<NSString *> *arguments = [[NSProcessInfo processInfo] arguments];
    int argc = (int)arguments.count;
    const char **argv = calloc(arguments.count, sizeof(const char *));
    for (NSUInteger i = 0; i < arguments.count; ++i) {
        NSString *arg = arguments[i];
        argv[i] = [arg UTF8String];
    }
    
    ios_main(argc, argv);
    
    free(argv);
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)startRun {
    uint32_t fps = 30;
    _displayLink = [CADisplayLink displayLinkWithTarget: self selector: @selector(renderLoop)];
    [_displayLink setFrameInterval: 60 / fps];
    [_displayLink addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
}

-(void)renderLoop {
    ios_draw_cb();
}

@end

@interface eglutView()

@end

@implementation eglutView

+ (id)layerClass {
    return [CAMetalLayer class];
}

@end

FILE *ios_fopen(const char *filename, const char *mode) {
    NSString *file = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:filename] ofType:nil];
    return fopen([file UTF8String], mode);
}
