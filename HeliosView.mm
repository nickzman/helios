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

// #define LOG_DEBUG

typedef struct				// Create A Structure
{
	GLubyte	*imageData;		// Image Data (Up To 32 Bits)
	GLuint	bpp;			// Image Color Depth In Bits Per Pixel.
	GLuint	width;			// Image Width
	GLuint	height;			// Image Height
	GLuint	texID;			// Texture ID Used To Select A Texture
} TextureImage;				// Structure Name
#define TEXSIZE 256

@implementation HeliosView

- (id)initWithFrame:(NSRect)frameRect isPreview:(BOOL) preview
{
    ScreenSaverDefaults *defaults = [ScreenSaverDefaults defaultsForModuleWithName:@"helios"];

#ifdef LOG_DEBUG
    NSLog( @"initWithFrame" );
#endif

	self = [super initWithFrame:frameRect isPreview:preview];
	if (self)
	{
		if (preview)
			mainMonitor = YES;
		else
			mainMonitor = frameRect.origin.x == 0.0 && frameRect.origin.y == 0;
		mainMonitorOnly = [defaults boolForKey:@"mainMonitorOnly"];
		if (mainMonitor || !mainMonitorOnly)
		{
			NSOpenGLPixelFormatAttribute attribs[] =
			{
				NSOpenGLPFAAccelerated,
				NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute)32,
				NSOpenGLPFADoubleBuffer,
				NSOpenGLPFAMinimumPolicy,
				NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16,
				NSOpenGLPFAAllowOfflineRenderers,
				(NSOpenGLPixelFormatAttribute)0
			};
			
			NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
			
			_view = [[NSOpenGLView alloc] initWithFrame:NSZeroRect pixelFormat:format];
			_view.wantsBestResolutionOpenGLSurface = YES;
			[self addSubview:_view];
			
			[_view.openGLContext makeCurrentContext];
			setDefaults(&settings);
			if ([defaults objectForKey:@"dIons"])
			{
				settings.dIons = int([defaults integerForKey:@"dIons"]);
				settings.dSize = int([defaults integerForKey:@"dSize"]);
				settings.dEmitters = int([defaults integerForKey:@"dEmitters"]);
				settings.dAttracters = int([defaults integerForKey:@"dAttracters"]);
				settings.dSpeed = int([defaults integerForKey:@"dSpeed"]);
				settings.dCameraspeed = int([defaults integerForKey:@"dCameraspeed"]);
				settings.dSurface = [defaults boolForKey:@"dSurface"];
				// par.dWireframe = [defaults boolForKey:@"dWireframe"];
				settings.dBlur = int([defaults integerForKey:@"dBlur"]);
				sphereType = int([defaults integerForKey:@"sphereType"]);
			}
			
			NSRect newBounds = [_view convertRectToBacking:_view.bounds];
			
			settings.viewWidth = newBounds.size.width;
			settings.viewHeight = newBounds.size.height;
			settings.doingPreview = preview;
			self.animationTimeInterval = 1.0/60.0;
		}
	}
    return self;
}


- (void)dealloc
{
	if (settings.customSpheremap)
		free(settings.customSpheremap);
}


- (void)setFrameSize:(NSSize)newSize
{
#ifdef LOG_DEBUG
	NSLog(@"setFrameSize: %@", NSStringFromSize(newSize));
#endif
	[super setFrameSize:newSize];
	if (_view)
	{
		[_view setFrameSize:newSize];
		if (_view.wantsBestResolutionOpenGLSurface)	// on Lion & later, if we're using a best resolution surface, then call glViewport() with the appropriate width and height for the backing
		{
			NSRect newBounds = [_view convertRectToBacking:_view.bounds];
			
#ifdef LOG_DEBUG
			NSLog(@"Adjusted setFrameSize: %@", NSStringFromSize(newBounds.size));
#endif
			settings.viewHeight = newBounds.size.height;
			settings.viewWidth = newBounds.size.width;
		}
		else
		{
			settings.viewHeight = _view.bounds.size.height;
			settings.viewWidth = _view.bounds.size.width;
		}
		if (settings.readyToDraw)
		{
			settings.aspectRatio = float(settings.viewWidth)/float(settings.viewHeight);
			glViewport(0, 0, settings.viewWidth, settings.viewHeight);
		}
	}
}


- (void)viewDidMoveToWindow
{
	[super viewDidMoveToWindow];
	if (@available(macOS 12.0, *))	// on Monterey and later, update the time interval for the window's screen's refresh rate
	{
		self.animationTimeInterval = self.window.screen.maximumRefreshInterval;
	}
}


- (void)drawRect:(NSRect)rect
{
	[[NSColor blackColor] set];
    NSRectFill(rect);
    
    if (!_view)
    {
		// In the past, we'd tell the user here here that their computer does not meet the minimum specification for this screen saver. But I wonder how often that happens these days...
	}
}

- (void)animateOneFrame
{
    if (!_configuring && _view)
    {
        if (mainMonitor || !mainMonitorOnly)
        {
			settings.frameTime = timer.tick();
			if (settings.readyToDraw)
			{
				[[_view openGLContext] makeCurrentContext];
				if (!drawnOneFrame && settings.dBlur)	// on the first frame, if dBlur is on, the color buffer will not be cleared, and we'll get a flickering red color if we don't clear it first
					glClear(GL_COLOR_BUFFER_BIT);
				drawnOneFrame = YES;
				draw(&settings);
				[[_view openGLContext] flushBuffer];
			}
        }
    }
}

- (void)startAnimation
{
#ifdef LOG_DEBUG
    NSLog( @"startAnimation" );
#endif
	[super startAnimation];
	if (!_configuring && _view)
	{
		if (mainMonitor || !mainMonitorOnly)
		{
			GLint interval = 1;
			
			if (sphereType == 1 && !settings.customSpheremap)
			{
				settings.customSpheremap = malloc(TEXSIZE*TEXSIZE*3);
				if (![self LoadTGA:[[[NSBundle bundleForClass:self.class] pathForResource:@"aaron_map" ofType:@"tga"] fileSystemRepresentation] intoData:settings.customSpheremap])
					free(settings.customSpheremap);
			}
			else
			{
				if (settings.customSpheremap)
				{
					free(settings.customSpheremap);
					settings.customSpheremap = NULL;
				}
			}
			[self lockFocus];
			[[_view openGLContext] makeCurrentContext];
			
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();
			CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &interval);	// don't allow screen tearing
			[[_view openGLContext] flushBuffer];
			
			doSaver(&settings);
			gettimeofday(&timer.prev_tv, NULL);	// reset the timer
			[self unlockFocus];
		}
	}
}

- (void)stopAnimation
{
    [super stopAnimation];
	
	if (!_configuring && _view)
    {
		if (settings.readyToDraw)
		{
			[[_view openGLContext] makeCurrentContext];
			cleanUp(&settings);
            settings.frameTime=0;
			drawnOneFrame = NO;
		}
	}
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
	if (!configureSheet) [[NSBundle bundleForClass:self.class] loadNibNamed:@"Helios" owner:self topLevelObjects:NULL];
    // [NSBundle loadNibNamed:@"Localizable" owner:self];
    
    [IBversionNumberField setStringValue:kVersion];

    [IBmainMonitorOnly setState:(mainMonitorOnly ? NSOnState : NSOffState)];

        
    [IBdIons setIntValue:settings.dIons];
    [IBdIonsTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Ions number (%d)" value:@"" table:@""], settings.dIons]];

    [IBdSize setIntValue:settings.dSize];
    [IBdSizeTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Size (%d)" value:@"" table:@""], settings.dSize]];

    [IBdEmitters setIntValue:settings.dEmitters];
    [IBdEmittersTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Emitters (%d)" value:@"" table:@""], settings.dEmitters]];

    [IBdAttracters setIntValue:settings.dAttracters];
    [IBdAttractersTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Attracters (%d)" value:@"" table:@""], settings.dAttracters]];

    [IBdSpeed setIntValue:settings.dSpeed];
    [IBdSpeedTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Speed (%d)" value:@"" table:@""], settings.dSpeed]];

    [IBdCameraspeed setIntValue:settings.dCameraspeed];
    [IBdCameraspeedTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Camera Speed (%d)" value:@"" table:@""], settings.dCameraspeed]];

    [IBdSurface setState:(settings.dSurface ? NSOnState : NSOffState)];
    [IBdSurface setTitle:[thisBundle localizedStringForKey:@"Surface" value:@"" table:@""]];
    
    // [IBdWireframe setState:(par.dWireframe ? NSOnState : NSOffState)];
    // [IBdWireframe setTitle:[thisBundle localizedStringForKey:@"Wireframe" value:@"" table:@""]];

    [IBdBlur setIntValue:settings.dBlur];
    [IBdBlurTxt setStringValue:[NSString stringWithFormat:
        [thisBundle localizedStringForKey:@"Blur (%d)" value:@"" table:@""], settings.dBlur]];
    
    [IBsphereType selectItemAtIndex:sphereType];
    [IBsphereType setEnabled:settings.dSurface];

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
}

- (IBAction) closeSheet_save:(id) sender {
	BOOL wasAnimating = self.animating;
	ScreenSaverDefaults *defaults = [ScreenSaverDefaults defaultsForModuleWithName:@"helios"];
    
#ifdef LOG_DEBUG
    NSLog( @"closeSheet_save" );
#endif
    
	_configuring = NO;
	[self stopAnimation];
	
    mainMonitorOnly = ( [IBmainMonitorOnly state] == NSOnState ) ? true : false;

    settings.dIons = [IBdIons intValue];
    settings.dSize = [IBdSize intValue];
    if(settings.dSize == 0) settings.dSize = 1;
    settings.dEmitters = [IBdEmitters intValue];
    settings.dAttracters = [IBdAttracters intValue];
    settings.dSpeed = [IBdSpeed intValue];
    if(settings.dSpeed == 0) settings.dSpeed = 1;
    settings.dCameraspeed = [IBdCameraspeed intValue];
    if(settings.dCameraspeed == 0) settings.dCameraspeed = 1;
    settings.dSurface = ( [IBdSurface state] == NSOnState ) ? true : false;
    // par.dWireframe = ( [IBdWireframe state] == NSOnState ) ? true : false;
    settings.dBlur = [IBdBlur intValue];
    sphereType = int([IBsphereType indexOfSelectedItem]);

    
    [defaults setBool: mainMonitorOnly forKey: @"mainMonitorOnly"];

    [defaults setInteger: settings.dIons forKey: @"dIons"];
    [defaults setInteger: settings.dSize forKey: @"dSize"];
    [defaults setInteger: settings.dEmitters forKey: @"dEmitters"];
    [defaults setInteger: settings.dAttracters forKey: @"dAttracters"];
    [defaults setInteger: settings.dSpeed forKey: @"dSpeed"];
    [defaults setInteger: settings.dCameraspeed forKey: @"dCameraspeed"];
    [defaults setBool: settings.dSurface forKey: @"dSurface"];
    [defaults setInteger: settings.dBlur forKey: @"dBlur"];
    [defaults setInteger: sphereType forKey: @"sphereType"];
    
    [defaults synchronize];

#ifdef LOG_DEBUG
    NSLog(@"Canged par" );
#endif

	if (wasAnimating)
		[self startAnimation];
	[NSApp endSheet:configureSheet];
	[configureSheet close];
}

- (IBAction) closeSheet_cancel:(id) sender {

#ifdef LOG_DEBUG
    NSLog( @"closeSheet_cancel" );
#endif
	_configuring = NO;
    [NSApp endSheet:configureSheet];
	[configureSheet close];
}


- (IBAction) restoreDefaults:(id) sender {

    NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
	HeliosSaverSettings dummySettings;

#ifdef LOG_DEBUG
    NSLog( @"configureSheet" );
#endif

	setDefaults(&dummySettings);
    [IBmainMonitorOnly setState:NSOffState];

	[IBdIons setIntValue:dummySettings.dIons];
    [IBdIonsTxt setStringValue:[NSString stringWithFormat:
								[thisBundle localizedStringForKey:@"Ions number (%d)" value:@"" table:@""], dummySettings.dIons]];

	[IBdSize setIntValue:dummySettings.dSize];
    [IBdSizeTxt setStringValue:[NSString stringWithFormat:
								[thisBundle localizedStringForKey:@"Size (%d)" value:@"" table:@""], dummySettings.dSize]];

	[IBdEmitters setIntValue:dummySettings.dEmitters];
    [IBdEmittersTxt setStringValue:[NSString stringWithFormat:
									[thisBundle localizedStringForKey:@"Emitters (%d)" value:@"" table:@""], dummySettings.dEmitters]];

	[IBdAttracters setIntValue:dummySettings.dAttracters];
    [IBdAttractersTxt setStringValue:[NSString stringWithFormat:
									  [thisBundle localizedStringForKey:@"Attracters (%d)" value:@"" table:@""], dummySettings.dAttracters]];

	[IBdSpeed setIntValue:dummySettings.dSpeed];
    [IBdSpeedTxt setStringValue:[NSString stringWithFormat:
								 [thisBundle localizedStringForKey:@"Speed (%d)" value:@"" table:@""], dummySettings.dSpeed]];

	[IBdCameraspeed setIntValue:dummySettings.dCameraspeed];
    [IBdCameraspeedTxt setStringValue:[NSString stringWithFormat:
									   [thisBundle localizedStringForKey:@"Camera Speed (%d)" value:@"" table:@""], dummySettings.dCameraspeed]];

	[IBdSurface setState:(dummySettings.dSurface ? NSOnState : NSOffState)];

	[IBdBlur setIntValue:dummySettings.dBlur];
    [IBdBlurTxt setStringValue:[NSString stringWithFormat:
								[thisBundle localizedStringForKey:@"Blur (%d)" value:@"" table:@""], dummySettings.dBlur]];

    [IBsphereType selectItem:nil];
    [IBsphereType selectItemAtIndex:0];
	[IBsphereType setEnabled:[IBdSurface state] == NSOnState];
}



/********************> LoadTGA() <*****/
// Loads A TGA File Into Memory
- (BOOL) LoadTGA:(const char *)filename intoData:(void *)data {
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
       fread(data, 1, imageSize, file)!=TEXSIZE*TEXSIZE*3)	// Does The Image Size Match The Memory Reserved?
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
