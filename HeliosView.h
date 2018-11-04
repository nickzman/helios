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
#include <sys/time.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

#include "rsMath/rsMath.h"
#include "Implicit/impCubeVolume.h"
#include "Implicit/impCrawlPoint.h"
#include "Implicit/impSphere.h"
#include <list>
// #include "spheremap.h"

#define LIGHTSIZE 64

class particle;
class emitter;
class attracter;
class ion;

typedef struct				// Create A Structure
{
    GLubyte	*imageData;		// Image Data (Up To 32 Bits)
    GLuint	bpp;			// Image Color Depth In Bits Per Pixel.
    GLuint	width;			// Image Width
    GLuint	height;			// Image Height
    GLuint	texID;			// Texture ID Used To Select A Texture
} TextureImage;				// Structure Name


typedef struct {
    // Global variables
    unsigned char lightTexture[LIGHTSIZE][LIGHTSIZE];
    float elapsedTime; // = 0.0f;
    emitter *elist;
    attracter *alist;
    ion *ilist;
    float newRgb[3];
    float billboardMat[16];
    // Parameters edited in the dialog box
    int dIons;
    int dSize;
    int dEmitters;
    int dAttracters;
    int dSpeed;
    int dCameraspeed;
    BOOL dSurface;
    // BOOL dWireframe;
    int dBlur;
    int dPriority;

    impCubeVolume* volume;
    impSurface* surface;
    impSphere* spheres;
} myParameters;



class particle{
public:
    rsVec pos;
    // rsVec rgb;
    float rgb[3];
    float size;
};


class emitter:public particle {
public:
    rsVec oldpos;
    rsVec targetpos;

    inline emitter()  {
        pos = rsVec(float(random()) / INT_MAX * 1000.0f - 500.0f,
                    float(random()) / INT_MAX * 1000.0f - 500.0f,
                    float(random()) / INT_MAX * 1000.0f - 500.0f);
    };
    // inline ~emitter() {};
    inline void settargetpos(rsVec target) {oldpos = pos; targetpos = target;};
    inline void interppos(float n) {pos = oldpos * (1.0f - n) + targetpos * n;};
    // inline void update() {};
};

class attracter: public particle {
public:
    rsVec oldpos;
    rsVec targetpos;

    inline attracter() {
        pos = rsVec(float(random()) / INT_MAX * 1000.0f - 500.0f,
                    float(random()) / INT_MAX * 1000.0f - 500.0f,
                    float(random()) / INT_MAX * 1000.0f - 500.0f);
    };
    // inline ~attracter() {};
    inline void settargetpos(rsVec target){oldpos = pos; targetpos = target;};
    inline void interppos(float n){pos = oldpos * (1.0f - n) + targetpos * n;};
    // inline void update(){};
};


class ion: public particle{
public:
    float speed = 0.0f;

    inline ion();
    inline void init(myParameters* p);
    inline void start(myParameters* p);
    inline void update(myParameters* p);
    inline void draw(myParameters* p);
};


@interface HeliosView : ScreenSaverView {
    
    BOOL mainMonitorOnly;
    BOOL thisScreenIsOn;

    NSOpenGLView *_view;
    BOOL _viewAllocated;
    BOOL _initedGL;
    BOOL initialized;
    struct timeval timeStart;
    double lastTime;

    int complexity;
    
    /****** DrawAll internals */
    int ionsReleased;
    float releaseTime;
    float oldCameraDistance;
    float cameraDistance;
    float targetCameraDistance;
    float preCameraInterp;
    rsVec radialVel;
    rsVec targetRadialVel;
    rsQuat rotQuat;
    rsVec oldHsl;
    rsVec newHsl;
    rsVec targetHsl;
    float colorInterp;
    float colorChange;
    float wait;
    float preinterp;
    float interp;
    float interpconst;
    int newTarget;
    int lastTarget;
    float valuetrig;

    /* width and height of the window */
    float windowWidth;
    float windowHeight;

    int sphereType;

    /* parameters that are user configurable */
#define TEXSIZE 256

    unsigned char spheremap[TEXSIZE][TEXSIZE][3];
    GLuint theTextures[2];
    myParameters par;

    IBOutlet id IBcomplexity;
    IBOutlet id IBcomplexityTxt;

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
    IBOutlet id IBUpdatesInfo;
    IBOutlet id IBmainMonitorOnly;

    IBOutlet id IBDefaultValues;
    IBOutlet id IBCancel;
    IBOutlet id IBSave;
}

- (IBAction) closeSheet_save:(id) sender;
- (IBAction) closeSheet_cancel:(id) sender;
- (IBAction) sheet_update:(id) sender;
- (IBAction) restoreDefaults:(id) sender;

- (GLvoid) InitGL;

- (void) setTargets:(int) whichTarget;
// - (void) surfaceFunctionUpdate:(float*) position;
- (void) drawAll;
- (void) setup_all;
- (BOOL) LoadTGA:(char *)filename;

@end
