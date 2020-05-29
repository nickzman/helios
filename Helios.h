//
//  Helios.h
//  Helios
//
//  Created by Nick Zitzmann on 5/26/20.
//

#ifndef Helios_h
#define Helios_h

#include <sys/types.h>
#include "rsVec.h"

class attracter;
class emitter;
class impCubeVolume;
class impSurface;
class impSphere;
class ion;
class rsText;

#define LIGHTSIZE 64

typedef struct HeliosSaverSettings
{
#ifdef WIN32
	LPCTSTR registryPath = ("Software\\Really Slick\\Helios");
	HGLRC hglrc;
	HDC hdc;
#endif
	int32_t viewHeight;
	int32_t viewWidth;
	bool doingPreview;
	
	int readyToDraw = 0;
	unsigned char lightTexture[LIGHTSIZE][LIGHTSIZE];
	float frameTime = 0.0f;
	emitter *elist;
	attracter *alist;
	ion *ilist;
	rsVec newRgb;
	float billboardMat[16];
	float aspectRatio;
	// text output
	rsText* textwriter;
	// Parameters edited in the dialog box
	int dIons;
	int dSize;
	int dEmitters;
	int dAttracters;
	int dSpeed;
	int dCameraspeed;
	int dSurface;
	int dBlur;

	impCubeVolume* volume;
	impSurface* surface;
	impSphere* spheres;
	
	rsVec oldHsl;
	rsVec newHsl;
	rsVec targetHsl;
	float colorInterp;
	float colorChange;
	
	float wait;
	float preinterp;
	float interp;
	float interpconst;
	
	void *customSpheremap;
	float totalTime = 0.0f;
	bool kStatistics;
} _HeliosSaverSettings;

__private_extern__ void setDefaults(HeliosSaverSettings *inSettings);
__private_extern__ void draw(HeliosSaverSettings *inSettings);
__private_extern__ void doSaver(HeliosSaverSettings *inSettings);
__private_extern__ void cleanUp(HeliosSaverSettings *inSettings);

#endif /* Helios_h */
