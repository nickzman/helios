//
//  HeliosView.mm
//  Helios
//
//  Created by pecos on Tue Oct 02 2001.
//  Copyright (c) 2001 __CompanyName__. All rights reserved.
//

/*
* Copyright (C) 2002  Terence M. Welsh
 *
 * Helios is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation.
 *
 * Helios is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


// Helios screensaver

#import "HeliosView.h"
#include <stdlib.h>
#include <time.h>
#import <GLUT/glut.h>
#import <OpenGL/OpenGL.h>

#define PI 3.14159265359f
#define PIx2 6.28318530718f
#define wide 200
#define high 150

#define k_original	0
#define k_aaron		1
#define k_wireframe	2

// default values
#define k_mainMonitorOnly	NO
#define k_dIons			1500
#define k_dSize			10
#define k_dEmitters		3
#define k_dAttracters		3
#define k_dSpeed		10
#define k_dCameraspeed		10
#define k_dSurface		TRUE
#define k_dBlur			10
#define k_sphereType		k_original
#define k_complexity		70

#ifdef __ppc__
@implementation NSString (HeliosBC)

- (BOOL)getCString:(char *)cString maxLength:(unsigned)length encoding:(NSStringEncoding)encoding
{
	return CFStringGetCString((CFStringRef)self, cString, length, CFStringConvertNSStringEncodingToEncoding(encoding));
}

@end
#endif


// #define LOG_DEBUG
static void hsl2rgb(float h, float s, float l, float *r, float *g, float *b);

@implementation HeliosView

- (id)initWithFrame:(NSRect)frameRect isPreview:(BOOL) preview
{
    NSString* version;
    ScreenSaverDefaults *defaults = [ScreenSaverDefaults defaultsForModuleWithName:@"helios"];

#ifdef LOG_DEBUG
    NSLog( @"initWithFrame" );
#endif

    if (![super initWithFrame:frameRect isPreview:preview]) return nil;

    if (self) {
        NSOpenGLPixelFormatAttribute attribs[] =
    {	NSOpenGLPFAAccelerated, (NSOpenGLPixelFormatAttribute)1,
        NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute)32,
        // NSOpenGLPFAAccumSize, (NSOpenGLPixelFormatAttribute)32,
        NSOpenGLPFAMinimumPolicy, (NSOpenGLPixelFormatAttribute)1,
        //NSOpenGLPFAMaximumPolicy, (NSOpenGLPixelFormatAttribute)1,
        // NSOpenGLPFAClosestPolicy,
        // NSOpenGLPFADoubleBuffer,
        (NSOpenGLPixelFormatAttribute)0
    };

        NSOpenGLPixelFormat *format = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attribs] autorelease];

        _view = [[[NSOpenGLView alloc] initWithFrame:NSZeroRect pixelFormat:format] autorelease];

        [self addSubview:_view];
        _viewAllocated = TRUE;
        _initedGL = NO;
    }
	NSString *kVersion = [[[NSBundle bundleForClass:[self class]] infoDictionary] objectForKey:@"CFBundleShortVersionString"];

    // Do your subclass initialization here
    version   = [defaults stringForKey:@"version"];
    mainMonitorOnly = [defaults boolForKey:@"mainMonitorOnly"];

    par.dIons = int([defaults integerForKey:@"dIons"]);
    par.dSize = int([defaults integerForKey:@"dSize"]);
    par.dEmitters = int([defaults integerForKey:@"dEmitters"]);
    par.dAttracters = int([defaults integerForKey:@"dAttracters"]);
    par.dSpeed = int([defaults integerForKey:@"dSpeed"]);
    par.dCameraspeed = int([defaults integerForKey:@"dCameraspeed"]);
    par.dSurface = [defaults boolForKey:@"dSurface"];
    // par.dWireframe = [defaults boolForKey:@"dWireframe"];
    par.dBlur = int([defaults integerForKey:@"dBlur"]);
    sphereType = int([defaults integerForKey:@"sphereType"]);
    complexity = int([defaults integerForKey:@"complexity"]);
    
    if( ![version isEqualToString:kVersion] || (version == NULL) ) {
        // first time ever !! 
        [defaults setObject: kVersion forKey: @"version"];
        [defaults setBool: k_mainMonitorOnly forKey: @"mainMonitorOnly"];
        [defaults setInteger: k_dIons forKey: @"dIons"];
        [defaults setInteger: k_dSize forKey: @"dSize"];
        [defaults setInteger: k_dEmitters forKey: @"dEmitters"];
        [defaults setInteger: k_dAttracters forKey: @"dAttracters"];
        [defaults setInteger: k_dSpeed forKey: @"dSpeed"];
        [defaults setInteger: k_dCameraspeed forKey: @"dCameraspeed"];
        [defaults setBool: k_dSurface forKey: @"dSurface"];
        // [defaults setBool: FALSE forKey: @"dWireframe"];
        [defaults setInteger: k_dBlur forKey: @"dBlur"];
        [defaults setInteger: k_sphereType forKey: @"sphereType"];
        [defaults setFloat: k_complexity forKey: @"complexity"];
        
        [defaults synchronize];

        mainMonitorOnly = k_mainMonitorOnly;
        par.dIons = k_dIons;
        par.dSize = k_dSize;
        par.dEmitters = k_dEmitters;
        par.dAttracters = k_dAttracters;
        par.dSpeed = k_dSpeed;
        par.dCameraspeed = k_dCameraspeed;
        par.dSurface = k_dSurface;
        par.dBlur = k_dBlur;
        sphereType = k_sphereType;
        complexity = k_complexity;
    }
    
    windowWidth = float([self bounds].size.width);
    windowHeight = float([self bounds].size.height);
    
    thisScreenIsOn = TRUE;
    initialized = NO;
    
    theTextures[0] = 0;
    theTextures[1] = 0;
    
    gettimeofday(&timeStart,NULL);
    
    return self;
}

- (void)animateOneFrame
{
    // Do your animation stuff here.
    // If you want to use drawRect: just add setNeedsDisplay:YES to this method

    double thisTime; // = time(0);
    struct timeval theTime;
    
    if( thisScreenIsOn == FALSE ) {
        [self stopAnimation];
        return;
    }

    [[_view openGLContext] makeCurrentContext];

    if (!_initedGL) {
        [self InitGL];
        _initedGL = YES;
    }

    if( !initialized ) {
        [self setup_all];
        initialized = TRUE;
        gettimeofday(&theTime, NULL);
        theTime.tv_sec -= timeStart.tv_sec;
        thisTime = theTime.tv_sec + theTime.tv_usec / 1.0e6;
        glClear(GL_COLOR_BUFFER_BIT);
     }
    
    // update time gettimeofday(timeStart,NULL);
    gettimeofday(&theTime, NULL);
    theTime.tv_sec -= timeStart.tv_sec;
    thisTime = theTime.tv_sec + theTime.tv_usec / 1.0e6;
    if(thisTime >= lastTime) {
        par.elapsedTime = float(thisTime - lastTime);
        [self drawAll];
        glFlush();
    }
    lastTime = thisTime;
    // else use elapsedTime from last frame
}

- (void)startAnimation
{
    // Do your animation initialization here
    NSOpenGLContext *context;
	GLint interval = 1;
    int mainScreen;
    int thisScreen;
    
#ifdef LOG_DEBUG
    NSLog( @"startAnimation" );
#endif
    
    thisScreenIsOn = TRUE;
    if( mainMonitorOnly ) {
        thisScreen = [[[[[self window] screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];
        mainScreen = [[[[NSScreen mainScreen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];
        // NSLog( @"test this %d - main %d", thisScreen, mainScreen );
        if( thisScreen != mainScreen ) {
            thisScreenIsOn = FALSE;
        }
    }

    context = [_view openGLContext];
    [context makeCurrentContext];
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFlush();
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &interval);	// don't allow screen tearing
    [super startAnimation];
}

- (void)stopAnimation
{
    // Do your animation termination here
    
    [super stopAnimation];
}

- (BOOL) hasConfigureSheet
{
    // Return YES if your screensaver has a ConfigureSheet
    return YES;
}

- (NSWindow*)configureSheet
{
    NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
	NSString *kVersion = [[thisBundle infoDictionary] objectForKey:@"CFBundleShortVersionString"];

#ifdef LOG_DEBUG
    NSLog( @"configureSheet" );
#endif
    if( ! configureSheet ) [NSBundle loadNibNamed:@"Helios" owner:self];
    // [NSBundle loadNibNamed:@"Localizable" owner:self];
    
    [IBversionNumberField setStringValue:kVersion];

    [IBUpdatesInfo setStringValue:@""];

    [IBmainMonitorOnly setState:(mainMonitorOnly ? NSOnState : NSOffState)];

        
    [IBdIons setIntValue:par.dIons];
    [IBdIonsTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Ions number (%d)" value:@"" table:@""], par.dIons]];

    [IBdSize setIntValue:par.dSize];
    [IBdSizeTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Size (%d)" value:@"" table:@""], par.dSize]];

    [IBdEmitters setIntValue:par.dEmitters];
    [IBdEmittersTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Emitters (%d)" value:@"" table:@""], par.dEmitters]];

    [IBdAttracters setIntValue:par.dAttracters];
    [IBdAttractersTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Attracters (%d)" value:@"" table:@""], par.dAttracters]];

    [IBdSpeed setIntValue:par.dSpeed];
    [IBdSpeedTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Speed (%d)" value:@"" table:@""], par.dSpeed]];

    [IBdCameraspeed setIntValue:par.dCameraspeed];
    [IBdCameraspeedTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Camera Speed (%d)" value:@"" table:@""], par.dCameraspeed]];

    [IBdSurface setState:(par.dSurface ? NSOnState : NSOffState)];
    [IBdSurface setTitle:[thisBundle localizedStringForKey:@"Surface" value:@"" table:@""]];
    
    // [IBdWireframe setState:(par.dWireframe ? NSOnState : NSOffState)];
    // [IBdWireframe setTitle:[thisBundle localizedStringForKey:@"Wireframe" value:@"" table:@""]];

    [IBdBlur setIntValue:par.dBlur];
    [IBdBlurTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Blur (%d)" value:@"" table:@""], par.dBlur]];
    
    [IBsphereType selectItem:nil];
    [IBsphereType addItemWithTitle:
        [thisBundle localizedStringForKey:@"Wireframe" value:@"" table:@""]];
    [IBsphereType selectItemAtIndex:sphereType];
    [IBsphereType setEnabled:par.dSurface];

    [IBcomplexity setIntValue:complexity];
    [IBcomplexityTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Complexity (%d)" value:@"" table:@""], complexity]];

    [IBmainMonitorOnly setTitle:[thisBundle localizedStringForKey:@"Main monitor only" value:@"" table:@""]];
    [IBDefaultValues setTitle:[thisBundle localizedStringForKey:@"Default values" value:@"" table:@""]];
    [IBCancel setTitle:[thisBundle localizedStringForKey:@"Cancel" value:@"" table:@""]];
    [IBSave setTitle:[thisBundle localizedStringForKey:@"Save" value:@"" table:@""]];
    
    return configureSheet;
}

- (IBAction) sheet_update:(id) sender
{
    int fooInt;
    BOOL fooBool;
    NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
    
    if( sender == IBdIons ) {
        fooInt = [IBdIons intValue];
        [IBdIonsTxt setStringValue:[NSString stringWithFormat:
            [thisBundle localizedStringForKey:@"Ions number (%d)" value:@"" table:@""], fooInt]];
    }
    else if( sender == IBdSize ) {
        fooInt = [IBdSize intValue];
        if(fooInt == 0) fooInt = 1;
        [IBdSizeTxt setStringValue:[NSString stringWithFormat:
            [thisBundle localizedStringForKey:@"Size (%d)" value:@"" table:@""], fooInt]];
    }
    else if( sender == IBdEmitters ) {
        fooInt = [IBdEmitters intValue];
        [IBdEmittersTxt setStringValue:[NSString stringWithFormat:
            [thisBundle localizedStringForKey:@"Emitters (%d)" value:@"" table:@""], fooInt]];
    }
    else if( sender == IBdAttracters ) {
        fooInt = [IBdAttracters intValue];
        [IBdAttractersTxt setStringValue:[NSString stringWithFormat:
            [thisBundle localizedStringForKey:@"Attracters (%d)" value:@"" table:@""], fooInt]];
    }
    else if( sender == IBdSpeed ) {
        fooInt = [IBdSpeed intValue];
        if( fooInt == 0 ) fooInt = 1;
        [IBdSpeedTxt setStringValue:[NSString stringWithFormat:
            [thisBundle localizedStringForKey:@"Speed (%d)" value:@"" table:@""], fooInt]];
    }
    else if( sender == IBdCameraspeed ) {
        fooInt = [IBdCameraspeed intValue];
        if(fooInt == 0) fooInt = 1;
        [IBdCameraspeedTxt setStringValue:[NSString stringWithFormat:
            [thisBundle localizedStringForKey:@"Camera Speed (%d)" value:@"" table:@""], fooInt]];
    }
    else if( sender == IBdBlur ) {
        fooInt = [IBdBlur intValue];
        [IBdBlurTxt setStringValue:[NSString stringWithFormat:
            [thisBundle localizedStringForKey:@"Blur (%d)" value:@"" table:@""], fooInt]];
    }
    else if( sender == IBdSurface ) {
        fooBool = ( [IBdSurface state] == NSOnState ) ? true : false;
        [IBsphereType setEnabled:fooBool];
    }
    else if( sender == IBcomplexity ) {
        fooInt = [IBcomplexity intValue];
        [IBcomplexityTxt setStringValue:[NSString stringWithFormat:
            [thisBundle localizedStringForKey:@"Complexity (%d)" value:@"" table:@""], fooInt]];
    }
}

- (IBAction) closeSheet_save:(id) sender {

    int thisScreen;
    int mainScreen;
    
    ScreenSaverDefaults *defaults = [ScreenSaverDefaults defaultsForModuleWithName:@"helios"];
    
#ifdef LOG_DEBUG
    NSLog( @"closeSheet_save" );
#endif
    
    initialized = NO;
    
    mainMonitorOnly = ( [IBmainMonitorOnly state] == NSOnState ) ? true : false;

    par.dIons = [IBdIons intValue];
    par.dSize = [IBdSize intValue];
    if(par.dSize == 0) par.dSize = 1;
    par.dEmitters = [IBdEmitters intValue];
    par.dAttracters = [IBdAttracters intValue];
    par.dSpeed = [IBdSpeed intValue];
    if(par.dSpeed == 0) par.dSpeed = 1;
    par.dCameraspeed = [IBdCameraspeed intValue];
    if(par.dCameraspeed == 0) par.dCameraspeed = 1;
    par.dSurface = ( [IBdSurface state] == NSOnState ) ? true : false;
    // par.dWireframe = ( [IBdWireframe state] == NSOnState ) ? true : false;
    par.dBlur = [IBdBlur intValue];
    sphereType = int([IBsphereType indexOfSelectedItem]);
    complexity = [IBcomplexity intValue];

    
    [defaults setBool: mainMonitorOnly forKey: @"mainMonitorOnly"];

    [defaults setInteger: par.dIons forKey: @"dIons"];
    [defaults setInteger: par.dSize forKey: @"dSize"];
    [defaults setInteger: par.dEmitters forKey: @"dEmitters"];
    [defaults setInteger: par.dAttracters forKey: @"dAttracters"];
    [defaults setInteger: par.dSpeed forKey: @"dSpeed"];
    [defaults setInteger: par.dCameraspeed forKey: @"dCameraspeed"];
    [defaults setBool: par.dSurface forKey: @"dSurface"];
    [defaults setInteger: par.dBlur forKey: @"dBlur"];
    [defaults setInteger: sphereType forKey: @"sphereType"];
    [defaults setInteger: complexity forKey: @"complexity"];
    
    [defaults synchronize];

#ifdef LOG_DEBUG
    NSLog(@"Canged par" );
#endif

    if( mainMonitorOnly ) {
        thisScreen = [[[[[self window] screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];
        mainScreen = [[[[NSScreen mainScreen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];
        // NSLog( @"test this %d - main %d", thisScreen, mainScreen );
        if( thisScreen != mainScreen ) {
            thisScreenIsOn = FALSE;
        }
    }
    if( (thisScreenIsOn == FALSE) && (mainMonitorOnly == FALSE) ) {
        thisScreenIsOn = TRUE;
        [self startAnimation];
    }
    
    [NSApp endSheet:configureSheet];
}

- (IBAction) closeSheet_cancel:(id) sender {

#ifdef LOG_DEBUG
    NSLog( @"closeSheet_cancel" );
#endif
    
    [NSApp endSheet:configureSheet];
}


- (IBAction) restoreDefaults:(id) sender {

    NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];

#ifdef LOG_DEBUG
    NSLog( @"configureSheet" );
#endif

    [IBmainMonitorOnly setState:(k_mainMonitorOnly ? NSOnState : NSOffState)];

    [IBdIons setIntValue:k_dIons];
    [IBdIonsTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Ions number (%d)" value:@"" table:@""], k_dIons]];

    [IBdSize setIntValue:k_dSize];
    [IBdSizeTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Size (%d)" value:@"" table:@""], k_dSize]];

    [IBdEmitters setIntValue:k_dEmitters];
    [IBdEmittersTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Emitters (%d)" value:@"" table:@""], k_dEmitters]];

    [IBdAttracters setIntValue:k_dAttracters];
    [IBdAttractersTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Attracters (%d)" value:@"" table:@""], k_dAttracters]];

    [IBdSpeed setIntValue:k_dSpeed];
    [IBdSpeedTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Speed (%d)" value:@"" table:@""], k_dSpeed]];

    [IBdCameraspeed setIntValue:k_dCameraspeed];
    [IBdCameraspeedTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Camera Speed (%d)" value:@"" table:@""], k_dCameraspeed]];

    [IBdSurface setState:(k_dSurface ? NSOnState : NSOffState)];

    [IBdBlur setIntValue:k_dBlur];
    [IBdBlurTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Blur (%d)" value:@"" table:@""], k_dBlur]];

    [IBsphereType selectItem:nil];
    [IBsphereType selectItemAtIndex:k_sphereType];
    [IBsphereType setEnabled:k_dSurface];
    
    [IBcomplexity setIntValue:k_complexity];
    [IBcomplexityTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Complexity (%d)" value:@"" table:@""], k_complexity]];
}


- (void) dealloc {

#ifdef LOG_DEBUG
    NSLog( @"dealloc" );
#endif

    if( par.elist ) {
        delete[] par.elist;
        par.elist = 0;
    }
    if( par.alist ) {
        delete[] par.alist;
        par.alist = 0;
    }
    if( par.ilist ) {
        delete[] par.ilist;
        par.ilist = 0;
    }
    if(par.dSurface){
        if( par.spheres ) {
            delete[] par.spheres;
            par.spheres = 0;
        }
        if( par.surface ) {
            delete par.surface;
            par.surface = 0;
        }
        if( par.volume ) {
            delete par.volume;
            par.volume = 0;
        }
    }
    
    [super dealloc];
}


// InitGL ---------------------------------------------------------------------

- (GLvoid) InitGL
{

    // glViewport(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
    glViewport( 0, 0, windowWidth, windowHeight );

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    // glEnable(GL_LINE_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glAlphaFunc(GL_GREATER,0.0f);
    glEnable(GL_ALPHA_TEST);

    // glShadeModel(GL_SMOOTH);
    // glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // glEnable(GL_LINE_SMOOTH);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gluPerspective(60.0, (GLfloat)windowWidth/(GLfloat)windowHeight, 0.1, 10000.0f);
    // glTranslatef(0.0, 0.0, -(wide * 2));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}



- (void) setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    if( _viewAllocated )
        [_view setFrameSize:newSize];
    _initedGL = NO;
}


- (void) setTargets:(int) whichTarget
{
    int i;
    float position;
    float change, angle, cosangle, sinangle;
    float height;
    
    switch(whichTarget){
        case 0:  // random
            for(i=0; i<par.dEmitters; i++)
                par.elist[i].settargetpos(rsVec(rsVec(myRandf(1000.0f) - 500.0f, myRandf(1000.0f) - 500.0f, myRandf(1000.0f) - 500.0f)));
            for(i=0; i<par.dAttracters; i++)
                par.alist[i].settargetpos(rsVec(rsVec(myRandf(1000.0f) - 500.0f, myRandf(1000.0f) - 500.0f, myRandf(1000.0f) - 500.0f)));
                break;
        case 1:{  // line (all emitters on one side, all attracters on the other)
            position = -500.0f, change = 1000.0f / (par.dEmitters + par.dAttracters - 1);
            for(i=0; i<par.dEmitters; i++){
                par.elist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
                position += change;
            }
            for(i=0; i<par.dAttracters; i++){
                par.alist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
                position += change;
            }
            break;
        }
        case 2:{  // line (emitters and attracters staggered)
            if(par.dEmitters > par.dAttracters)
                change = 1000.0f / (par.dEmitters * 2 - 1);
            else
                change = 1000.0f / (par.dAttracters * 2 - 1);
            position = -500.0f;
            for(i=0; i<par.dEmitters; i++){
                par.elist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
                position += change * 2.0f;
            }
            position = -500.0f + change;
            for(i=0; i<par.dAttracters; i++){
                par.alist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
                position += change * 2.0f;
            }
            break;
        }
        case 3:{  // 2 lines (parallel)
            change = 1000.0f / (par.dEmitters * 2 - 1);
            position = -500.0f;
            height = -525.0f + (par.dEmitters * 25);
            for(i=0; i<par.dEmitters; i++){
                par.elist[i].settargetpos(rsVec(rsVec(position, height, -50.0f)));
                position += change * 2.0f;
            }
            change = 1000.0f / (par.dAttracters * 2 - 1);
            position = -500.0f;
            height = 525.0f - (par.dAttracters * 25);
            for(i=0; i<par.dAttracters; i++){
                par.alist[i].settargetpos(rsVec(rsVec(position, height, 50.0f)));
                position += change * 2.0f;
            }
            break;
        }
        case 4:{  // 2 lines (skewed)
            change = 1000.0f / (par.dEmitters * 2 - 1);
            position = -500.0f;
            height = -525.0f + (par.dEmitters * 25);
            for(i=0; i<par.dEmitters; i++){
                par.elist[i].settargetpos(rsVec(rsVec(position, height, 0.0f)));
                position += change * 2.0f;
            }
            change = 1000.0f / (par.dAttracters * 2 - 1);
            position = -500.0f;
            height = 525.0f - (par.dAttracters * 25);
            for(i=0; i<par.dAttracters; i++){
                par.alist[i].settargetpos(rsVec(rsVec(10.0f, height, position)));
                position += change * 2.0f;
            }
            break;
        }
        case 5:  // random distribution across a plane
            for(i=0; i<par.dEmitters; i++)
                par.elist[i].settargetpos(rsVec(rsVec(myRandf(1000.0f) - 500.0f, 0.0f, myRandf(1000.0f) - 500.0f)));
            for(i=0; i<par.dAttracters; i++)
                par.alist[i].settargetpos(rsVec(rsVec(myRandf(1000.0f) - 500.0f, 0.0f, myRandf(1000.0f) - 500.0f)));
                break;
        case 6:{  // random distribution across 2 planes
            height = -525.0f + (par.dEmitters * 25);
            for(i=0; i<par.dEmitters; i++)
                par.elist[i].settargetpos(rsVec(rsVec(myRandf(1000.0f) - 500.0f, height, myRandf(1000.0f) - 500.0f)));
            height = 525.0f - (par.dAttracters * 25);
            for(i=0; i<par.dAttracters; i++)
                par.alist[i].settargetpos(rsVec(rsVec(myRandf(1000.0f) - 500.0f, height, myRandf(1000.0f) - 500.0f)));
            break;
        }
        case 7:{  // 2 rings (1 inside and 1 outside)
            angle = 0.5f;
            change = PIx2 / (par.dEmitters);
            for(i=0; i<par.dEmitters; i++){
                angle += change;
                cosangle = cosf(angle) * 200.0f;
                sinangle = sinf(angle) * 200.0f;
                par.elist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
            }
            angle = 1.5f;
            change = PIx2 / float(par.dAttracters);
            for(i=0; i<par.dAttracters; i++){
                angle += change;
                cosangle = cosf(angle) * 500.0f;
                sinangle = sinf(angle) * 500.0f;
                par.alist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
            }
            break;
        }
        case 8:{  // ring (all emitters on one side, all attracters on the other)
            angle = 0.5f;
            change = PIx2 / float(par.dEmitters + par.dAttracters);
            for(i=0; i<par.dEmitters; i++){
                angle += change;
                cosangle = cosf(angle) * 500.0f;
                sinangle = sinf(angle) * 500.0f;
                par.elist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
            }
            for(i=0; i<par.dAttracters; i++){
                angle += change;
                cosangle = cosf(angle) * 500.0f;
                sinangle = sinf(angle) * 500.0f;
                par.alist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
            }
            break;
        }
        case 9:{  // ring (emitters and attracters staggered)
            if(par.dEmitters > par.dAttracters)
                change = PIx2 / (par.dEmitters * 2);
            else
                change = PIx2 / float(par.dAttracters * 2);
            angle = 0.5f;
            for(i=0; i<par.dEmitters; i++){
                cosangle = cosf(angle) * 500.0f;
                sinangle = sinf(angle) * 500.0f;
                par.elist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
                angle += change * 2.0f;
            }
            angle = 0.5f + change;
            for(i=0; i<par.dAttracters; i++){
                cosangle = cosf(angle) * 500.0f;
                sinangle = sinf(angle) * 500.0f;
                par.alist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
                angle += change * 2.0f;
            }
            break;
        }
        case 10:  // 2 points
            for(i=0; i<par.dEmitters; i++)
                par.elist[i].settargetpos(rsVec(rsVec(500.0f, 100.0f, 50.0f)));
            for(i=0; i<par.dAttracters; i++)
                par.alist[i].settargetpos(rsVec(rsVec(-500.0f, -100.0f, -50.0f)));
                break;
    }
}


- (void) drawAll
{
    int i;

    // Camera movements
    // first do translation (distance from center)
    float cameraInterp;
    
    float changeRemaining, change;

    float angle;

    float diff;
    
    float brightFactor = 1;
    float surfaceColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    preCameraInterp += par.dCameraspeed * par.elapsedTime * 0.01f;
    cameraInterp = 0.5f - (0.5f * cosf(preCameraInterp));
    cameraDistance = (1.0f - cameraInterp) * oldCameraDistance + cameraInterp * targetCameraDistance;
    if(preCameraInterp >= PI) {
        oldCameraDistance = targetCameraDistance;
        targetCameraDistance = -myRandf(1300.0f) - 200.0f;
        preCameraInterp = 0.0f;
    }
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, cameraDistance);

    // then do rotation
    rsVec radialVelDiff = targetRadialVel - radialVel;
    changeRemaining = radialVelDiff.normalize();
    change = float(par.dCameraspeed) * 0.0002f * par.elapsedTime;
    if(changeRemaining > change){
        radialVelDiff *= change;
        radialVel += radialVelDiff;
    }
    else{
        radialVel = targetRadialVel;
        if(myRandi(2)){
            targetRadialVel = rsVec(myRandf(1.0f), myRandf(1.0f), myRandf(1.0f));
            targetRadialVel.normalize();
            targetRadialVel *= float(par.dCameraspeed) * myRandf(0.002f);
        }
        else
            targetRadialVel = rsVec(0.0f, 0.0f, 0.0f);
    }
    rsVec tempRadialVel = radialVel;
    angle = tempRadialVel.normalize();
    rsQuat radialQuat;
    radialQuat.make(angle, tempRadialVel[0], tempRadialVel[1], tempRadialVel[2]);
    rotQuat.preMult(radialQuat);
    rsMatrix rotMat;
    rotMat.fromQuat(rotQuat);

    // make billboard matrix for rotating particles when they are drawn
    rotMat.get(par.billboardMat);

    // Calculate new color
    newHsl = rsVec(myRandf(1.0f), 1.0f, 1.0f);
    colorInterp += par.elapsedTime * colorChange;
    // NSLog(@"%f %f %f", colorInterp, par.elapsedTime, colorChange );
    // NSLog(@"%f %f %f", par.newRgb[0], par.newRgb[1], par.newRgb[2] );
    
    if(colorInterp >= 1.0f){
        if(!myRandi(3) && par.dIons >= 100)  // change color suddenly
            newHsl = rsVec(myRandf(1.0f), 1.0f - (myRandf(1.0f) * myRandf(1.0f)), 1.0f);
        oldHsl = newHsl;
        targetHsl = rsVec(myRandf(1.0f), 1.0f - (myRandf(1.0f) * myRandf(1.0f)), 1.0f);
        colorInterp = 0.0f;
        // amount by which to change colorInterp each second
        colorChange = myRandf(0.005f * float(par.dSpeed)) + (0.002f * float(par.dSpeed));
    }
    else{
        diff = targetHsl[0] - oldHsl[0];
        if(diff < -0.5f || (diff > 0.0f && diff < 0.5f))
            newHsl[0] = oldHsl[0] + colorInterp * diff;
        else
            newHsl[0] = oldHsl[0] - colorInterp * diff;
        diff = targetHsl[1] - oldHsl[1];
        newHsl[1] = oldHsl[1] + colorInterp * diff;
        if(newHsl[0] < 0.0f)
            newHsl[0] += 1.0f;
        if(newHsl[0] > 1.0f)
            newHsl[0] -= 1.0f;
        hsl2rgb(newHsl[0], newHsl[1], 1.0f, &(par.newRgb[0]), &(par.newRgb[1]), &(par.newRgb[2]));
    }

    // Release ions
    if(ionsReleased < par.dIons){
        releaseTime -= par.elapsedTime;
        while(ionsReleased < par.dIons && releaseTime <= 0.0f){
            par.ilist[ionsReleased].start(&par);
            ionsReleased ++;
            // all ions released after 2 minutes
            releaseTime += 120.0f / par.dIons;
        }
        // NSLog(@"released %d ions", ionsReleased);
    }

    // Set interpolation value for emitters and attracters
    wait -= par.elapsedTime;
    if(wait <= 0.0f){
        preinterp += par.elapsedTime * float(par.dSpeed) * interpconst;
        interp = 0.5f - (0.5f * cosf(preinterp));
    }
    if(preinterp >= PI){
        // select new taget points (not the same pattern twice in a row)
        lastTarget = newTarget;
        newTarget = myRandi(10);
        if(newTarget == lastTarget)
            newTarget ++;
        [self setTargets:newTarget];
        preinterp = 0.0f;
        interp = 0.0f;
        wait = 10.0f;  // pause after forming each new pattern
        interpconst = 0.001f;
        if(!myRandi(4))  // interpolate really fast sometimes
            interpconst = 0.1f;
    }


    // Update particles
    for(i=0; i<par.dEmitters; i++){
        par.elist[i].interppos(interp);
        // par.elist[i].update();
    }
    for(i=0; i<par.dAttracters; i++){
        par.alist[i].interppos(interp);
        // par.alist[i].update();
    }
    for(i=0; i<ionsReleased; i++)
        par.ilist[i].update(&par);

    // Calculate surface
    if(par.dSurface){
        for(i=0; i<par.dEmitters; i++)
            par.spheres[i].setPosition(par.elist[i].pos[0], par.elist[i].pos[1], par.elist[i].pos[2]);
        for(i=0; i<par.dAttracters; i++)
            par.spheres[par.dEmitters+i].setPosition(par.alist[i].pos[0], par.alist[i].pos[1], par.alist[i].pos[2]);
std::list<impCrawlPoint> crawlpointlist;
        float center[3];
        for(i=0; i<par.dEmitters+par.dAttracters; i++){
            par.spheres[i].center(center);
            crawlpointlist.push_back(impCrawlPoint(center));
        }
        par.surface->reset();
        valuetrig += par.elapsedTime;
        par.volume->surfacevalue = 0.45f + 0.05f * cosf(valuetrig);
        par.volume->make_surface(crawlpointlist);
    }
    
    // Draw
    // clear the screen
    if(par.dBlur){  // partially
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // glAlphaFunc(GL_ALWAYS, 1);
        // glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.5f - sqrtf(sqrtf(par.dBlur)) * 0.150208f);
        glPushMatrix();
        glLoadIdentity();
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3f(-50.0f, -40.0f, -3.0f);
        glVertex3f(50.0f, -40.0f, -3.0f);
        glVertex3f(-50.0f, 40.0f, -3.0f);
        glVertex3f(50.0f, 40.0f, -3.0f);
        glEnd();
/*        glAlphaFunc(GL_GREATER, 0.1);
        glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3f(-50.0f, -40.0f, -3.0f);
        glVertex3f(50.0f, -40.0f, -3.0f);
        glVertex3f(-50.0f, 40.0f, -3.0f);
        glVertex3f(50.0f, 40.0f, -3.0f);
        glEnd();
        glAlphaFunc(GL_ALWAYS, 1); */
        glPopMatrix();
        
        /*glReadBuffer( GL_FRONT );
        glAccum( GL_LOAD, 0.5f + sqrt(sqrt(par.dBlur)) * 0.15495f );
        glAccum( GL_RETURN, 1.0 );
        glDrawBuffer( GL_FRONT );*/
    }
    else  // completely
        glClear(GL_COLOR_BUFFER_BIT);

    // Draw ions
    glBlendFunc(GL_ONE, GL_ONE);
    glBindTexture(GL_TEXTURE_2D, theTextures[0]);
    for(i=0; i<ionsReleased; i++) {
        // NSLog(@"%d %d %d", par.ilist[i].rgb[0], par.ilist[i].rgb[1], par.ilist[i].rgb[2] );
        par.ilist[i].draw(&par);
    }
    
    // Draw surfaces
    if(par.dSurface){
        glBindTexture(GL_TEXTURE_2D, theTextures[1]);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        // find color for surfaces
        if(par.dIons >= 100){
            if( sphereType == k_wireframe )
                brightFactor = 2.0f / (float(par.dBlur + 30) * float(par.dBlur + 30));
            else
                brightFactor = 4.0f / (float(par.dBlur + 30) * float(par.dBlur + 30));
            for(i=0; i<100; i++){
                surfaceColor[0] += par.ilist[i].rgb[0];
                surfaceColor[1] += par.ilist[i].rgb[1];
                surfaceColor[2] += par.ilist[i].rgb[2];
            }
            surfaceColor[0] *= brightFactor;
            surfaceColor[1] *= brightFactor;
            surfaceColor[2] *= brightFactor;
            glColor4fv(surfaceColor);
        }
        else{
            if( sphereType == k_wireframe )
                brightFactor = 200.0f / (float(par.dBlur + 30) * float(par.dBlur + 30));
            else
                brightFactor = 400.0f / (float(par.dBlur + 30) * float(par.dBlur + 30));
            glColor4f(par.newRgb[0] * brightFactor,
                      par.newRgb[1] * brightFactor,
                      par.newRgb[2] * brightFactor, 1.0f);
        }
        // draw the surface
        glPushMatrix();
        glMultMatrixf(par.billboardMat);
        if( sphereType== k_wireframe ){
            glDisable(GL_TEXTURE_2D);
            par.surface->draw_wireframe();
            glEnable(GL_TEXTURE_2D);
        }
        else
            par.surface->draw();
        glPopMatrix();
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }
}


- (void) setup_all
{
    int i, j;
    float x, y, temp;
    float sphereScaleFactor;
    char thePath[16384+1];
    
    srandom(unsigned(time(NULL)));

    par.newRgb[0] = 1.0f;
    par.newRgb[1] = 0.5f;
    par.newRgb[2] = 0.0f;
    
    /****** DrawAll internals */
    ionsReleased = 0;
    releaseTime = 0.0f;
    targetCameraDistance = -1000.0f;
    preCameraInterp = PI;
    radialVel = rsVec(0.0f, 0.0f, 0.0f);
    targetRadialVel = radialVel;
    rotQuat = rsQuat(0.0f, 0.0f, 0.0f, 1.0f);
    newHsl = rsVec(myRandf(1.0f), 1.0f, 1.0f);
    colorInterp = 1.0f;
    wait = 0.0f;
    preinterp = PI;
    interpconst = 0.001f;
    newTarget = 0;
    valuetrig = 0.0f;
    
    // Init light texture
    for(i=0; i<LIGHTSIZE; i++){
        for(j=0; j<LIGHTSIZE; j++) {
            x = float(i - LIGHTSIZE / 2) / float(LIGHTSIZE / 2);
            y = float(j - LIGHTSIZE / 2) / float(LIGHTSIZE / 2);
            temp = 1.0f - float(sqrt((x * x) + (y * y)));
            if(temp > 1.0f)
                temp = 1.0f;
            if(temp < 0.0f)
                temp = 0.0f;
            par.lightTexture[i][j] = ((unsigned char)(255.0f * temp * temp));
        }
    }

    glDeleteTextures( 1, &(theTextures[0]) );
    glGenTextures(1, &(theTextures[0]));		// Generate OpenGL texture IDs
    glBindTexture(GL_TEXTURE_2D, theTextures[0]);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 1, LIGHTSIZE, LIGHTSIZE, GL_LUMINANCE,
                      GL_UNSIGNED_BYTE, par.lightTexture);

    
    if( sphereType == k_wireframe ) {
        glDeleteTextures( 1, &(theTextures[1]) );
    }
    else {
        // Init sphere texture
        if( sphereType == k_original )
            [[[NSBundle bundleForClass:[HeliosView class]] pathForResource:@"spheremap" ofType:@"tga"] getCString:thePath maxLength:16384 encoding:NSUTF8StringEncoding];
        else // if( sphereType == k_aaron )
            [[[NSBundle bundleForClass:[HeliosView class]] pathForResource:@"aaron_map" ofType:@"tga"] getCString:thePath maxLength:16384 encoding:NSUTF8StringEncoding];

        if( ![self LoadTGA:thePath] ) {
            par.dSurface = FALSE;
        }

        glDeleteTextures( 1, &(theTextures[1]) );
        glGenTextures(1, &(theTextures[1]));		// Generate OpenGL texture IDs
        glBindTexture(GL_TEXTURE_2D, theTextures[1]);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TEXSIZE, TEXSIZE, GL_RGB,
                          GL_UNSIGNED_BYTE, spheremap);

        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    }
    glEnable(GL_TEXTURE_2D);

    
    // Initialize light display list
    glDeleteLists( 1, 1 ); // no effect if called before ever creating a List
    glNewList(1, GL_COMPILE);
    glBindTexture(GL_TEXTURE_2D, theTextures[0]);
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0.5f, -0.5f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0.5f, 0.5f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0.5f, 0.5f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-0.5f, 0.5f, 0.0f);
    glEnd();
    glEndList();

    // Initialize particles
    if( par.elist ) {
        delete[] par.elist;
        par.elist = 0;
    }
    par.elist = new emitter[par.dEmitters];

    if( par.alist ) {
        delete[] par.alist;
        par.alist = 0;
    }
    par.alist = new attracter[par.dAttracters];

    if( par.ilist ) {
        delete[] par.ilist;
        par.ilist = 0;
    }
    par.ilist = new ion[par.dIons]();
    for(size_t ilist_idx = 0; ilist_idx < par.dIons; ++ilist_idx)
    {
        par.ilist[ilist_idx].init(&par);
    }

    // Initialize surface
    if(par.dSurface){
        if( par.spheres ) {
            delete[] par.spheres;
            par.spheres = 0;
        }
        par.spheres = new impSphere[par.dEmitters + par.dAttracters];
        sphereScaleFactor = 1.0f / float(sqrt(double(2 * par.dEmitters + par.dAttracters)));
        for(i=0; i<par.dEmitters; i++)
            par.spheres[i].setScale(400.0f * sphereScaleFactor);
        for(i=0; i<par.dAttracters; i++)
            par.spheres[i + par.dEmitters].setScale(200.0f * sphereScaleFactor);

        if( par.volume ) {
            // (par.volume)->~impCubeVolume();
            delete par.volume;
            par.volume = 0;
        }
        par.volume = new impCubeVolume(50, 50, 50, 70.0f-0.35f*complexity);
        // par.volume->init(50, 50, 50, 35.0f);
        par.volume->spheres = par.spheres;
        par.volume->num = par.dEmitters+par.dAttracters;
        
        if( par.surface ) {
            delete par.surface;
            par.surface = 0;
        }
        par.surface = new impSurface;
        // par.surface->init(6000);
        par.volume->surface = par.surface;

    }
}


/********************> LoadTGA() <*****/
// Loads A TGA File Into Memory
- (BOOL) LoadTGA:(char *)filename {
    GLubyte		TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
    GLubyte		TGAcompare[12];			// Used To Compare TGA Header
    GLubyte		header[6];			// First 6 Useful Bytes From The Header
    GLuint		bytesPerPixel;			// Holds Number Of Bytes Per Pixel Used In The TGA File
    GLuint		imageSize;			// Used To Store The Image Size When Setting Aside Ram

    FILE *file = fopen(filename, "r");			// Open The TGA File
    TextureImage theTexture;

    if( file == NULL ) {
        NSLog (@"unable to load texture image" );
        NSLog (@" resource \"%s\" not found", filename );
        return false;
    }

    if(	fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
        memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0		||	// Does The Header Match What We Want?
        fread(header,1,sizeof(header),file)!=sizeof(header))			// If So Read Next 6 Header Bytes
    {
        NSLog (@"unable to load texture image" );
        NSLog (@" not a valid uncompressed TGA file" );
        fclose(file);				// If Anything Failed, Close The File
        return false;				// Return False
    }

    theTexture.width  = header[1] * 256 + header[0];	// Determine The TGA Width (highbyte*256+lowbyte)
    theTexture.height = header[3] * 256 + header[2];	// Determine The TGA Height (highbyte*256+lowbyte)

    if(	theTexture.width	<=0	||		// Is The Width Less Than Or Equal To Zero
        theTexture.height	<=0	||		// Is The Height Less Than Or Equal To Zero
        header[4]!=24 )					// Is The TGA 24 Bit (RGB) ?
    {
        NSLog (@"unable to load texture image" );
        NSLog (@" not a valid uncompressed 24 or 32 bpp TGA file" );
        fclose(file);					// If Anything Failed, Close The File
        return false;					// Return False
    }

    if(	theTexture.width	!= TEXSIZE	||	// Is The Width Not Equal to TEXSIZE
        theTexture.height	!= TEXSIZE )		// Is The Height Not Equal to TEXSIZE
    {
        NSLog (@"unable to load texture image" );
        NSLog (@" Not a valid Texture image (must be 128x128)" );
        fclose(file);					// If Anything Failed, Close The File
        return false;					// Return False
    }

    theTexture.bpp	= header[4];			// Grab The TGA's Bits Per Pixel (8)
    bytesPerPixel	= theTexture.bpp/8;		// Divide By 8 To Get The Bytes Per Pixel
    imageSize		= theTexture.width*theTexture.height*bytesPerPixel;	// Calculate The Memory Required For The TGA Data

    if( // theTexture.imageData==NULL ||			// Does The Storage Memory Exist?
       fread(spheremap, 1, imageSize, file)!=TEXSIZE*TEXSIZE*3)	// Does The Image Size Match The Memory Reserved?
    {
        NSLog (@"unable to load texture image" );
        NSLog (@" size does not mach" );

        fclose(file);						// Close The File
        return false;						// Return False
    }
    
    fclose (file);					// Close The File

    return true;					// Texture Building Went Ok, Return True
}

@end


ion::ion() {}
void ion::init(myParameters* p)
{
    float temp = 0.0f;

    pos = rsVec(0.0f, 0.0f, 0.0f);
    rgb[0] = rgb[1] = rgb[2] = 1.0f;
    temp = myRandf(2.0f) + 0.4f;
    size = float(p->dSize) * temp;
    speed = float(p->dSpeed) * 12.0f / temp;
}


void ion::start(myParameters* p){
    int i = myRandi(p->dEmitters);
    pos = p->elist[i].pos;
    float offset = p->elapsedTime * speed;
    switch(myRandi(14)){
        case 0:
            pos[0] += offset;
            break;
        case 1:
            pos[0] -= offset;
            break;
        case 2:
            pos[1] += offset;
            break;
        case 3:
            pos[1] -= offset;
            break;
        case 4:
            pos[2] += offset;
            break;
        case 5:
            pos[2] -= offset;
            break;
        case 6:
            pos[0] += offset;
            pos[1] += offset;
            pos[2] += offset;
            break;
        case 7:
            pos[0] -= offset;
            pos[1] += offset;
            pos[2] += offset;
            break;
        case 8:
            pos[0] += offset;
            pos[1] -= offset;
            pos[2] += offset;
            break;
        case 9:
            pos[0] -= offset;
            pos[1] -= offset;
            pos[2] += offset;
            break;
        case 10:
            pos[0] += offset;
            pos[1] += offset;
            pos[2] -= offset;
            break;
        case 11:
            pos[0] -= offset;
            pos[1] += offset;
            pos[2] -= offset;
            break;
        case 12:
            pos[0] += offset;
            pos[1] -= offset;
            pos[2] -= offset;
            break;
        case 13:
            pos[0] -= offset;
            pos[1] -= offset;
            pos[2] -= offset;
    }

    rgb[0] = p->newRgb[0];
    rgb[1] = p->newRgb[1];
    rgb[2] = p->newRgb[2];
}


void ion::update(myParameters* p){
    int i;
    int startOver = 0;
    float startOverDistance = 0;
    rsVec force, tempvec;
    float length;
    float temp = 0;

    force = rsVec(0.0f, 0.0f, 0.0f);
    for(i=0; i<p->dEmitters; i++){
        tempvec = pos - p->elist[i].pos;
        length = tempvec.normalize();
        if(length > 11000.0f)
            startOver = 1;
        if(length <= 1.0f)
            temp = 1.0f;
        else
            temp = 1.0f / length;
        tempvec *= temp;
        force += tempvec;
    }
    startOverDistance = speed * p->elapsedTime;
    for(i=0; i<p->dAttracters; i++){
        tempvec = p->alist[i].pos - pos;
        length = tempvec.normalize();
        if(length < startOverDistance)
            startOver = 1;
        if(length <= 1.0f)
            temp = 1.0f;
        else
            temp = 1.0f / length;
        tempvec *= temp;
        force += tempvec;
    }

    // Start this ion at an emitter if it gets too close to an attracter
    // or too far from an emitter
    if(startOver)
        start(p);
    else{
        force.normalize();
        pos += (force * p->elapsedTime * speed);
    }
}

void ion::draw(myParameters* p)
{
    glColor4f(rgb[0], rgb[1], rgb[2], 1.0f);
    glPushMatrix();
    glTranslatef(pos[0] * p->billboardMat[0] + pos[1] * p->billboardMat[4] + pos[2] * p->billboardMat[8],
                 pos[0] * p->billboardMat[1] + pos[1] * p->billboardMat[5] + pos[2] * p->billboardMat[9],
                 pos[0] * p->billboardMat[2] + pos[1] * p->billboardMat[6] + pos[2] * p->billboardMat[10]);
    glScalef(size, size, size);
    glCallList(1);
    glPopMatrix();
}

static void hsl2rgb(float h, float s, float l, float *r, float *g, float *b)
{
    NSColor *theColor;
    
    theColor = [NSColor colorWithCalibratedHue: h
                                    saturation: s
                                    brightness: l
                                         alpha: 1.0f ];
    *r = float([theColor redComponent]);
    *g = float([theColor greenComponent]);
    *b = float([theColor blueComponent]);

    return;
}

