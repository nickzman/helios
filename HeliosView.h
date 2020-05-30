//
//  HeliosView.h
//  Helios
//
//  Created by pecos on Tue Oct 02 2001.
//  Copyright (c) 2001 __CompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>
#import <ScreenSaver/ScreenSaver.h>
#include <time.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "Helios.h"
#import "rsTimer.h"

@interface HeliosView : ScreenSaverView
{
    BOOL mainMonitor;
    BOOL mainMonitorOnly;
    BOOL thisScreenIsOn;

    NSOpenGLView *_view;
	BOOL _configuring;
	BOOL drawnOneFrame;
	
	HeliosSaverSettings settings;
	rsTimer timer;
	int sphereType;

    IBOutlet id IBdIons;
    IBOutlet id IBdIonsTxt;

    IBOutlet id IBdSize;
    IBOutlet id IBdSizeTxt;

    IBOutlet id IBdEmitters;
    IBOutlet id IBdEmittersTxt;

    IBOutlet id IBdAttracters;
    IBOutlet id IBdAttractersTxt;

    IBOutlet id IBdSpeed;
    IBOutlet id IBdSpeedTxt;

    IBOutlet id IBdCameraspeed;
    IBOutlet id IBdCameraspeedTxt;

    IBOutlet id IBdSurface;

    // IBOutlet id IBdWireframe;

    IBOutlet id IBdBlur;
    IBOutlet id IBdBlurTxt;

    IBOutlet id IBsphereType;
    
    IBOutlet id configureSheet;
    IBOutlet id IBversionNumberField;
    IBOutlet id IBmainMonitorOnly;

    IBOutlet id IBDefaultValues;
    IBOutlet id IBCancel;
    IBOutlet id IBSave;
}

- (IBAction) closeSheet_save:(id) sender;
- (IBAction) closeSheet_cancel:(id) sender;
- (IBAction) sheet_update:(id) sender;
- (IBAction) restoreDefaults:(id) sender;

- (BOOL) LoadTGA:(const char *)filename intoData:(void *)data;

@end
