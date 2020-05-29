/*
 * Copyright (C) 2001-2010  Terence M. Welsh
 *
 * This file is part of Helios.
 *
 * Helios is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
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


#ifdef WIN32
#include <windows.h>
#include <rsWin32Saver/rsWin32Saver.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <regstr.h>
#include <commctrl.h>
#include <resource.h>
#endif
#include <stdio.h>
#include "rsText.h"
#include <math.h>
#include <time.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include "rsMath.h"
#include "Rgbhsl.h"
#include "impCubeVolume.h"
#include "impCrawlPoint.h"
#include "impSphere.h"
#include <list>
#include "spheremap.h"
#include "Helios.h"


//#define LIGHTSIZE 64
#define PI 3.14159265359f
#define PIx2 6.28318530718f


class particle;
class emitter;
class attracter;
class ion;



// Global variables
/*LPCTSTR registryPath = ("Software\\Really Slick\\Helios");
HGLRC hglrc;
HDC hdc;
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
impSphere* spheres;*/



class particle{
public:
	rsVec pos;
	rsVec rgb;
	float size;
};


class emitter:public particle{
public:
	rsVec oldpos;
	rsVec targetpos;

	emitter();
	~emitter(){};
	void settargetpos(rsVec target){oldpos = pos; targetpos = target;};
	void interppos(float n){pos = oldpos * (1.0f - n) + targetpos * n;};
	void update(){};
};

emitter::emitter(){
	pos = rsVec(rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f);
}


class attracter:public particle{
public:
	rsVec oldpos;
	rsVec targetpos;

	attracter();
	~attracter(){};
	void settargetpos(rsVec target){oldpos = pos; targetpos = target;};
	void interppos(float n){pos = oldpos * (1.0f - n) + targetpos * n;};
	void update(){};
};

attracter::attracter(){
	pos = rsVec(rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f);
}


class ion:public particle{
public:
	float speed;

	ion() {}
	ion(HeliosSaverSettings *inSettings);
	~ion(){};
	void start(HeliosSaverSettings *inSettings);
	void update(HeliosSaverSettings *inSettings);
	void draw(HeliosSaverSettings *inSettings);
};

ion::ion(HeliosSaverSettings *inSettings){
	float temp;

	pos = rsVec(0.0f, 0.0f, 0.0f);
	rgb = rsVec(0.0f, 0.0f, 0.0f);
	temp = rsRandf(2.0f) + 0.4f;
	size = float(inSettings->dSize) * temp;
	speed = float(inSettings->dSpeed) * 12.0f / temp;
}

void ion::start(HeliosSaverSettings *inSettings){
	int i = rsRandi(inSettings->dEmitters);
	pos = inSettings->elist[i].pos;
	float offset = inSettings->frameTime * speed;
	switch(rsRandi(14)){
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

	rgb = inSettings->newRgb;
}


void ion::update(HeliosSaverSettings *inSettings){
	int i;
	int startOver = 0;
	static float startOverDistance;
	static rsVec force, tempvec;
	static float length, temp;

	force = rsVec(0.0f, 0.0f, 0.0f);
	for(i=0; i<inSettings->dEmitters; i++){
		tempvec = pos - inSettings->elist[i].pos;
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
	startOverDistance = speed * inSettings->frameTime;
	for(i=0; i<inSettings->dAttracters; i++){
		tempvec = inSettings->alist[i].pos - pos;
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
		start(inSettings);
	else{
		force.normalize();
		pos += (force * inSettings->frameTime * speed);
	}
}

void ion::draw(HeliosSaverSettings *inSettings){
	glColor3f(rgb[0], rgb[1], rgb[2]);
	glPushMatrix();
		glTranslatef(pos[0] * inSettings->billboardMat[0] + pos[1] * inSettings->billboardMat[4] + pos[2] * inSettings->billboardMat[8],
			pos[0] * inSettings->billboardMat[1] + pos[1] * inSettings->billboardMat[5] + pos[2] * inSettings->billboardMat[9],
			pos[0] * inSettings->billboardMat[2] + pos[1] * inSettings->billboardMat[6] + pos[2] * inSettings->billboardMat[10]);
		glScalef(size, size, size);
		glCallList(1);
	glPopMatrix();
}


void setTargets(int whichTarget, HeliosSaverSettings *inSettings){
	int i;

	switch(whichTarget){
	case 0:  // random
		for(i=0; i<inSettings->dEmitters; i++)
			inSettings->elist[i].settargetpos(rsVec(rsVec(rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f)));
		for(i=0; i<inSettings->dAttracters; i++)
			inSettings->alist[i].settargetpos(rsVec(rsVec(rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f)));
		break;
	case 1:{  // line (all emitters on one side, all attracters on the other)
		float position = -500.0f, change = 1000.0f / float(inSettings->dEmitters + inSettings->dAttracters - 1);
		for(i=0; i<inSettings->dEmitters; i++){
			inSettings->elist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
			position += change;
		}
		for(i=0; i<inSettings->dAttracters; i++){
			inSettings->alist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
			position += change;
		}
		break;
		}
	case 2:{  // line (emitters and attracters staggered)
		float change;
		if(inSettings->dEmitters > inSettings->dAttracters)
			change = 1000.0f / float(inSettings->dEmitters * 2 - 1);
		else
			change = 1000.0f / float(inSettings->dAttracters * 2 - 1);
		float position = -500.0f;
		for(i=0; i<inSettings->dEmitters; i++){
			inSettings->elist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
			position += change * 2.0f;
		}
		position = -500.0f + change;
		for(i=0; i<inSettings->dAttracters; i++){
			inSettings->alist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
			position += change * 2.0f;
		}
		break;
		}
	case 3:{  // 2 lines (parallel)
		float change = 1000.0f / float(inSettings->dEmitters * 2 - 1);
		float position = -500.0f;
		float height = -525.0f + float(inSettings->dEmitters * 25);
		for(i=0; i<inSettings->dEmitters; i++){
			inSettings->elist[i].settargetpos(rsVec(rsVec(position, height, -50.0f)));
			position += change * 2.0f;
		}
		change = 1000.0f / float(inSettings->dAttracters * 2 - 1);
		position = -500.0f;
		height = 525.0f - float(inSettings->dAttracters * 25);
		for(i=0; i<inSettings->dAttracters; i++){
			inSettings->alist[i].settargetpos(rsVec(rsVec(position, height, 50.0f)));
			position += change * 2.0f;
		}
		break;
		}
	case 4:{  // 2 lines (skewed)
		float change = 1000.0f / float(inSettings->dEmitters * 2 - 1);
		float position = -500.0f;
		float height = -525.0f + float(inSettings->dEmitters * 25);
		for(i=0; i<inSettings->dEmitters; i++){
			inSettings->elist[i].settargetpos(rsVec(rsVec(position, height, 0.0f)));
			position += change * 2.0f;
		}
		change = 1000.0f / float(inSettings->dAttracters * 2 - 1);
		position = -500.0f;
		height = 525.0f - float(inSettings->dAttracters * 25);
		for(i=0; i<inSettings->dAttracters; i++){
			inSettings->alist[i].settargetpos(rsVec(rsVec(10.0f, height, position)));
			position += change * 2.0f;
		}
		break;
		}
	case 5:  // random distribution across a plane
		for(i=0; i<inSettings->dEmitters; i++)
			inSettings->elist[i].settargetpos(rsVec(rsVec(rsRandf(1000.0f) - 500.0f, 0.0f, rsRandf(1000.0f) - 500.0f)));
		for(i=0; i<inSettings->dAttracters; i++)
			inSettings->alist[i].settargetpos(rsVec(rsVec(rsRandf(1000.0f) - 500.0f, 0.0f, rsRandf(1000.0f) - 500.0f)));
		break;
	case 6:{  // random distribution across 2 planes
		float height = -525.0f + float(inSettings->dEmitters * 25);
		for(i=0; i<inSettings->dEmitters; i++)
			inSettings->elist[i].settargetpos(rsVec(rsVec(rsRandf(1000.0f) - 500.0f, height, rsRandf(1000.0f) - 500.0f)));
		height = 525.0f - float(inSettings->dAttracters * 25);
		for(i=0; i<inSettings->dAttracters; i++)
			inSettings->alist[i].settargetpos(rsVec(rsVec(rsRandf(1000.0f) - 500.0f, height, rsRandf(1000.0f) - 500.0f)));
		break;
		}
	case 7:{  // 2 rings (1 inside and 1 outside)
		float angle = 0.5f, cosangle, sinangle;
		float change = PIx2 / float(inSettings->dEmitters);
		for(i=0; i<inSettings->dEmitters; i++){
			angle += change;
			cosangle = cosf(angle) * 200.0f;
			sinangle = sinf(angle) * 200.0f;
			inSettings->elist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
		}
		angle = 1.5f;
		change = PIx2 / float(inSettings->dAttracters);
		for(i=0; i<inSettings->dAttracters; i++){
			angle += change;
			cosangle = cosf(angle) * 500.0f;
			sinangle = sinf(angle) * 500.0f;
			inSettings->alist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
		}
		break;
		}
	case 8:{  // ring (all emitters on one side, all attracters on the other)
		float angle = 0.5f, cosangle, sinangle;
		float change = PIx2 / float(inSettings->dEmitters + inSettings->dAttracters);
		for(i=0; i<inSettings->dEmitters; i++){
			angle += change;
			cosangle = cosf(angle) * 500.0f;
			sinangle = sinf(angle) * 500.0f;
			inSettings->elist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
		}
		for(i=0; i<inSettings->dAttracters; i++){
			angle += change;
			cosangle = cosf(angle) * 500.0f;
			sinangle = sinf(angle) * 500.0f;
			inSettings->alist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
		}
		break;
		}
	case 9:{  // ring (emitters and attracters staggered)
		float change;
		if(inSettings->dEmitters > inSettings->dAttracters)
			change = PIx2 / float(inSettings->dEmitters * 2);
		else
			change = PIx2 / float(inSettings->dAttracters * 2);
		float angle = 0.5f, cosangle, sinangle;
		for(i=0; i<inSettings->dEmitters; i++){
			cosangle = cosf(angle) * 500.0f;
			sinangle = sinf(angle) * 500.0f;
			inSettings->elist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
			angle += change * 2.0f;
		}
		angle = 0.5f + change;
		for(i=0; i<inSettings->dAttracters; i++){
			cosangle = cosf(angle) * 500.0f;
			sinangle = sinf(angle) * 500.0f;
			inSettings->alist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
			angle += change * 2.0f;
		}
		break;
		}
	case 10:  // 2 points
		for(i=0; i<inSettings->dEmitters; i++)
			inSettings->elist[i].settargetpos(rsVec(rsVec(500.0f, 100.0f, 50.0f)));
		for(i=0; i<inSettings->dAttracters; i++)
			inSettings->alist[i].settargetpos(rsVec(rsVec(-500.0f, -100.0f, -50.0f)));
		break;
	}
}


float surfaceFunction(float* position, void *contextInfo){
	HeliosSaverSettings *inSettings = (HeliosSaverSettings *)contextInfo;
	static int i;
	static float value;
	const int points = inSettings->dEmitters + inSettings->dAttracters;	// change by NZ; this was originally static but mustn't ever be static

	value = 0.0f;
	for(i=0; i<points; i++)
		value += inSettings->spheres[i].value(position);

	return(value);
}


void draw(HeliosSaverSettings *inSettings){
	int i;
	static int ionsReleased = 0;
	static float releaseTime = 0.0f;

	// Camera movements
	// first do translation (distance from center)
	static float oldCameraDistance;
	static float cameraDistance;
	static float targetCameraDistance = -1000.0f;
	static float preCameraInterp = PI;
	float cameraInterp;
	preCameraInterp += float(inSettings->dCameraspeed) * inSettings->frameTime * 0.01f;
	cameraInterp = 0.5f - (0.5f * cosf(preCameraInterp));
	cameraDistance = (1.0f - cameraInterp) * oldCameraDistance
		+ cameraInterp * targetCameraDistance;
	if(preCameraInterp >= PI){
		oldCameraDistance = targetCameraDistance;
		targetCameraDistance = -rsRandf(1300.0f) - 200.0f;
		preCameraInterp = 0.0f;
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, cameraDistance);

	// then do rotation
	static rsVec radialVel = rsVec(0.0f, 0.0f, 0.0f);
	static rsVec targetRadialVel = radialVel;
	static rsQuat rotQuat = rsQuat(0.0f, 0.0f, 0.0f, 1.0f);
	rsVec radialVelDiff = targetRadialVel - radialVel;
	float changeRemaining = radialVelDiff.normalize();
	float change = float(inSettings->dCameraspeed) * 0.0002f * inSettings->frameTime;
	if(changeRemaining > change){
		radialVelDiff *= change;
		radialVel += radialVelDiff;
	}
	else{
		radialVel = targetRadialVel;
		if(rsRandi(2)){
			targetRadialVel = rsVec(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
			targetRadialVel.normalize();
			targetRadialVel *= float(inSettings->dCameraspeed) * rsRandf(0.002f);
		}
		else
			targetRadialVel = rsVec(0.0f, 0.0f, 0.0f);
	}
	rsVec tempRadialVel = radialVel;
	float angle = tempRadialVel.normalize();
	rsQuat radialQuat;
	radialQuat.make(angle, tempRadialVel[0], tempRadialVel[1], tempRadialVel[2]);
	rotQuat.preMult(radialQuat);
	rsMatrix rotMat;
	rotMat.fromQuat(rotQuat);

	// make billboard matrix for rotating particles when they are drawn
	rotMat.get(inSettings->billboardMat);

	// Calculate new color
	/*static rsVec oldHsl, newHsl = rsVec(rsRandf(1.0f), 1.0f, 1.0f), targetHsl;
	static float colorInterp = 1.0f, colorChange;*/
	inSettings->colorInterp += inSettings->frameTime * inSettings->colorChange;
	if(inSettings->colorInterp >= 1.0f){
		if(!rsRandi(3) && inSettings->dIons >= 100)  // change color suddenly
			inSettings->newHsl = rsVec(rsRandf(1.0f), 1.0f - (rsRandf(1.0f) * rsRandf(1.0f)), 1.0f);
		inSettings->oldHsl = inSettings->newHsl;
		inSettings->targetHsl = rsVec(rsRandf(1.0f), 1.0f - (rsRandf(1.0f) * rsRandf(1.0f)), 1.0f);
		inSettings->colorInterp = 0.0f;
		// amount by which to change colorInterp each second
		inSettings->colorChange = rsRandf(0.005f * float(inSettings->dSpeed)) + (0.002f * float(inSettings->dSpeed));
	}
	else{
		float diff = inSettings->targetHsl[0] - inSettings->oldHsl[0];
		if(diff < -0.5f || (diff > 0.0f && diff < 0.5f))
			inSettings->newHsl[0] = inSettings->oldHsl[0] + inSettings->colorInterp * diff;
		else
			inSettings->newHsl[0] = inSettings->oldHsl[0] - inSettings->colorInterp * diff;
		diff = inSettings->targetHsl[1] - inSettings->oldHsl[1];
			inSettings->newHsl[1] = inSettings->oldHsl[1] + inSettings->colorInterp * diff;
		if(inSettings->newHsl[0] < 0.0f)
			inSettings->newHsl[0] += 1.0f;
		if(inSettings->newHsl[0] > 1.0f)
			inSettings->newHsl[0] -= 1.0f;
		hsl2rgb(inSettings->newHsl[0], inSettings->newHsl[1], 1.0f, inSettings->newRgb[0], inSettings->newRgb[1], inSettings->newRgb[2]);
	}

	// Release ions
	if(ionsReleased < inSettings->dIons){
		releaseTime -= inSettings->frameTime;
		while(ionsReleased < inSettings->dIons && releaseTime <= 0.0f){
			inSettings->ilist[ionsReleased].start(inSettings);
			ionsReleased ++;
			// all ions released after 2 minutes
			releaseTime += 120.0f / float(inSettings->dIons);
		}
	}

	// Set interpolation value for emitters and attracters
	/*static float wait = 0.0f;
	static float preinterp = PI, interp;
	static float interpconst = 0.001f;*/
	inSettings->wait -= inSettings->frameTime;
	if(inSettings->wait <= 0.0f){
		inSettings->preinterp += inSettings->frameTime * float(inSettings->dSpeed) * inSettings->interpconst;
		inSettings->interp = 0.5f - (0.5f * cosf(inSettings->preinterp));
	}
	if(inSettings->preinterp >= PI){
		// select new taget points (not the same pattern twice in a row)
		static int newTarget = 0, lastTarget;
		lastTarget = newTarget;
		newTarget = rsRandi(10);
		if(newTarget == lastTarget)
			newTarget ++;
		setTargets(newTarget, inSettings);
		inSettings->preinterp = 0.0f;
		inSettings->interp = 0.0f;
		inSettings->wait = 10.0f;  // pause after forming each new pattern
		inSettings->interpconst = 0.001f;
		if(!rsRandi(4))  // interpolate really fast sometimes
			inSettings->interpconst = 0.1f;
	}


	// Update particles
	for(i=0; i<inSettings->dEmitters; i++){
		inSettings->elist[i].interppos(inSettings->interp);
		inSettings->elist[i].update();
	}
	for(i=0; i<inSettings->dAttracters; i++){
		inSettings->alist[i].interppos(inSettings->interp);
		inSettings->alist[i].update();
	}
	for(i=0; i<ionsReleased; i++)
		inSettings->ilist[i].update(inSettings);

	// Calculate surface
	if(inSettings->dSurface){
		for(i=0; i<inSettings->dEmitters; i++)
			inSettings->spheres[i].setPosition(inSettings->elist[i].pos[0], inSettings->elist[i].pos[1], inSettings->elist[i].pos[2]);
		for(i=0; i<inSettings->dAttracters; i++)
			inSettings->spheres[inSettings->dEmitters+i].setPosition(inSettings->alist[i].pos[0], inSettings->alist[i].pos[1], inSettings->alist[i].pos[2]);
		impCrawlPointVector cpv;
		for(i=0; i<inSettings->dEmitters+inSettings->dAttracters; i++)
			inSettings->spheres[i].addCrawlPoint(cpv);
		inSettings->surface->reset();
		static float valuetrig = 0.0f;
		valuetrig += inSettings->frameTime;
		inSettings->volume->setSurfaceValue(0.45f + 0.05f * cosf(valuetrig));
		inSettings->volume->makeSurface(cpv);
	}

	// Draw
	// clear the screen
	if(inSettings->dBlur){  // partially
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			glOrtho(0.0, 1.0, 0.0, 1.0, 1.0, -1.0);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
				glLoadIdentity();
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4f(0.0f, 0.0f, 0.0f, 0.5f - (float(sqrtf(sqrtf(float(inSettings->dBlur)))) * 0.15495f));
				glBegin(GL_TRIANGLE_STRIP);
					glVertex3f(0.0f, 0.0f, 0.0f);
					glVertex3f(1.0f, 0.0f, 0.0f);
					glVertex3f(0.0f, 1.0f, 0.0f);
					glVertex3f(1.0f, 1.0f, 0.0f);
				glEnd();
			glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}
	else  // completely
		glClear(GL_COLOR_BUFFER_BIT);

	// Draw ions
	glMatrixMode(GL_MODELVIEW);
	glBlendFunc(GL_ONE, GL_ONE);
	glBindTexture(GL_TEXTURE_2D, 1);
	for(i=0; i<ionsReleased; i++)
		inSettings->ilist[i].draw(inSettings);

	// Draw surfaces
	float brightFactor;
	float surfaceColor[3] = {0.0f, 0.0f, 0.0f};
	if(inSettings->dSurface){
		glBindTexture(GL_TEXTURE_2D, 2);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		// find color for surfaces
		if(inSettings->dIons >= 100){
			brightFactor = 4.0f / (float(inSettings->dBlur + 30) * float(inSettings->dBlur + 30));
			for(i=0; i<100; i++){
				surfaceColor[0] += inSettings->ilist[i].rgb[0] * brightFactor;
				surfaceColor[1] += inSettings->ilist[i].rgb[1] * brightFactor;
				surfaceColor[2] += inSettings->ilist[i].rgb[2] * brightFactor;
			}
			glColor3fv(surfaceColor);
		}
		else{
			brightFactor = 400.0f / (float(inSettings->dBlur + 30) * float(inSettings->dBlur + 30));
			glColor3f(inSettings->newRgb[0] * brightFactor, inSettings->newRgb[1] * brightFactor, inSettings->newRgb[2] * brightFactor);
		}
		// draw the surface
		glPushMatrix();
			glMultMatrixf(inSettings->billboardMat);
			inSettings->surface->draw();
		glPopMatrix();
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}

	// If graphics card does a true buffer swap instead of a copy swap
	// then everything must get drawn on both buffers
	/*if(dBlur && pfd_swap_exchange){
		wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0f, 0.0f, 0.0f, 0.5f - (float(sqrtf(sqrtf(dBlur))) * 0.15495f));
		glPushMatrix();
		glLoadIdentity();
		glBegin(GL_TRIANGLE_STRIP);
			glVertex3f(-5.0f, -4.0f, -3.0f);
			glVertex3f(5.0f, -4.0f, -3.0f);
			glVertex3f(-5.0f, 4.0f, -3.0f);
			glVertex3f(5.0f, 4.0f, -3.0f);
		glEnd();
		glPopMatrix();

		// Draw ions
		glBlendFunc(GL_ONE, GL_ONE);
		glBindTexture(GL_TEXTURE_2D, 1);
		for(i=0; i<ionsReleased; i++)
			ilist[i].draw();

		// Draw surfaces
		if(dSurface){
			glBindTexture(GL_TEXTURE_2D, 2);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			if(dIons >= 100)
				glColor3fv(surfaceColor);
			else
				glColor3f(newRgb[0] * brightFactor, newRgb[1] * brightFactor, newRgb[2] * brightFactor);
			glPushMatrix();
				glMultMatrixf(billboardMat);
				surface->draw();
			glPopMatrix();
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
		}
	}*/

	// print text
	//static float totalTime = 0.0f;
	inSettings->totalTime += inSettings->frameTime;
	static std::string str;
	static int frames = 0;
	++frames;
	if(frames == 20){
		str = "FPS = " + to_string(20.0f / inSettings->totalTime);
		inSettings->totalTime = 0.0f;
		frames = 0;
	}
	if(inSettings->kStatistics){
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0f, 50.0f * inSettings->aspectRatio, 0.0f, 50.0f, -1.0f, 1.0f);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(1.0f, 48.0f, 0.0f);

		glColor3f(1.0f, 0.6f, 0.0f);
		inSettings->textwriter->draw(str);

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}


	//wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
}


/*void idleProc(){
	// update time
	static rsTimer timer;
	frameTime = timer.tick();

	if(readyToDraw && !isSuspended && !checkingPassword)
		draw();
}*/


#ifdef WIN32
void doSaver(HWND hwnd){
	RECT rect;
	int i, j;
	float x, y, temp;

	// Seed random number generator
	srand((unsigned)time(NULL));

	// Window initialization
	hdc = GetDC(hwnd);
	setBestPixelFormat(hdc);
	hglrc = wglCreateContext(hdc);
	GetClientRect(hwnd, &rect);
	wglMakeCurrent(hdc, hglrc);
	glViewport(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
	aspectRatio = float(rect.right) / float(rect.bottom);
#else
void doSaver(HeliosSaverSettings *inSettings)
{
	int i, j;
	float x, y, temp;
	
	srand((unsigned)time(NULL));
	
	inSettings->aspectRatio = float(inSettings->viewWidth)/float(inSettings->viewHeight);
	glViewport(0, 0, inSettings->viewWidth, inSettings->viewHeight);
#endif
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	gluPerspective(60.0, inSettings->aspectRatio, 0.1, 10000.0);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	glEnable(GL_NORMALIZE);

	// Clear the buffers and test for type of buffer swapping
	/*glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
	glClear(GL_COLOR_BUFFER_BIT);
	unsigned char pixel[1] = {255};
	glRasterPos2i(0, 0);
	glDrawPixels(1, 1, GL_RED, GL_UNSIGNED_BYTE, pixel);
	wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
	glReadPixels(0, 0, 1, 1, GL_RED, GL_UNSIGNED_BYTE, pixel);
	if(pixel[0] == 0){  // Color was truly swapped out of the back buffer
		pfd_swap_exchange = 1;
		pfd_swap_copy = 0;
	}
	else{    // Color remains in back buffer
		pfd_swap_exchange = 0;
		pfd_swap_copy = 1;
	}*/

	// Init light texture
	for(i=0; i<LIGHTSIZE; i++){
		for(j=0; j<LIGHTSIZE; j++){
			x = float(i - LIGHTSIZE / 2) / float(LIGHTSIZE / 2);
			y = float(j - LIGHTSIZE / 2) / float(LIGHTSIZE / 2);
			temp = 1.0f - float(sqrt((x * x) + (y * y)));
			if(temp > 1.0f)
				temp = 1.0f;
			if(temp < 0.0f)
				temp = 0.0f;
			inSettings->lightTexture[i][j] = u_int8_t(255.0f * temp * temp);
		}
	}
	glBindTexture(GL_TEXTURE_2D, 1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 1, LIGHTSIZE, LIGHTSIZE, GL_LUMINANCE, 
		GL_UNSIGNED_BYTE, inSettings->lightTexture);
	glBindTexture(GL_TEXTURE_2D, 2);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TEXSIZE, TEXSIZE, GL_RGB, 
		GL_UNSIGNED_BYTE, inSettings->customSpheremap ? inSettings->customSpheremap : spheremap);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glEnable(GL_TEXTURE_2D);

	// Initialize light display list
	glNewList(1, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, 1);
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
	inSettings->elist = new emitter[inSettings->dEmitters];
	inSettings->alist = new attracter[inSettings->dAttracters];
	inSettings->ilist = new ion[inSettings->dIons];
	for (size_t i = 0 ; i < inSettings->dIons ; i++)
	{
		new (&(inSettings->ilist[i])) ion(inSettings);
	}

	// Initialize surface
	if(inSettings->dSurface){
		inSettings->volume = new impCubeVolume;
		// Preview takes to long to initialize because Windows sucks,
		// so initialize smaller surface data structures.
		if(inSettings->doingPreview)
			inSettings->volume->init(14, 14, 14, 125.0f);
		else
			inSettings->volume->init(70, 70, 70, 25.0f);
		inSettings->volume->function = surfaceFunction;
		inSettings->volume->contextInfoForFunction = inSettings;	// add by NZ so surfaceFunction can read inSettings
		inSettings->surface = inSettings->volume->getSurface();
		inSettings->spheres = new impSphere[inSettings->dEmitters + inSettings->dAttracters];
		float sphereScaleFactor = 1.0f / sqrtf(float(2 * inSettings->dEmitters + inSettings->dAttracters));
		for(i=0; i<inSettings->dEmitters; i++)
			inSettings->spheres[i].setThickness(400.0f * sphereScaleFactor);
		for(i=0; i<inSettings->dAttracters; i++)
			inSettings->spheres[i + inSettings->dEmitters].setThickness(200.0f * sphereScaleFactor);
	}
	
	// Change by NZ - color changes must be per-instance and not global
	inSettings->newHsl = rsVec(rsRandf(1.0f), 1.0f, 1.0f);
	inSettings->colorInterp = 1.0f;
	inSettings->colorChange = 0.0f;
	// Change by NZ - put emitters and attractors in random places once we draw
	inSettings->wait = 0.0f;
	inSettings->preinterp = PI;
	inSettings->interpconst = 0.001f;

	// Initialize text
	inSettings->textwriter = new rsText;
	inSettings->readyToDraw = true;
}

#ifdef WIN32
void cleanUp(HWND hwnd){
	// Free memory
	delete[] elist;
	delete[] alist;
	delete[] ilist;
	if(dSurface){
		delete[] spheres;
		delete surface;
		delete volume;
	}

	// Kill device context
	ReleaseDC(hwnd, hdc);
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglrc);
}
#else
void cleanUp(HeliosSaverSettings *inSettings)
{
	delete[] inSettings->elist;
	inSettings->elist = NULL;
	delete[] inSettings->alist;
	inSettings->alist = NULL;
	delete[] inSettings->ilist;
	inSettings->ilist = NULL;
	if (inSettings->dSurface)
	{
		delete[] inSettings->spheres;
		inSettings->spheres = NULL;
		delete inSettings->surface;
		inSettings->surface = NULL;
		delete inSettings->volume;
		inSettings->volume = NULL;
	}
	delete inSettings->textwriter;
	inSettings->textwriter = NULL;
	inSettings->readyToDraw = false;
}
#endif


void setDefaults(HeliosSaverSettings *inSettings){
	inSettings->dIons = 1500;
	inSettings->dSize = 10;
	inSettings->dEmitters = 3;
	inSettings->dAttracters = 3;
	inSettings->dSpeed = 10;
	inSettings->dCameraspeed = 10;
	inSettings->dSurface = 1;
	inSettings->dBlur = 10;
}


// Initialize all user-defined stuff
/*void readRegistry(){
	LONG result;
	HKEY skey;
	DWORD valtype, valsize, val;

	setDefaults();

	result = RegOpenKeyEx(HKEY_CURRENT_USER, registryPath, 0, KEY_READ, &skey);
	if(result != ERROR_SUCCESS)
		return;

	valsize=sizeof(val);

	result = RegQueryValueEx(skey, "Ions", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dIons = val;
	result = RegQueryValueEx(skey, "Size", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dSize = val;
	result = RegQueryValueEx(skey, "Emitters", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dEmitters = val;
	result = RegQueryValueEx(skey, "Attracters", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dAttracters = val;
	result = RegQueryValueEx(skey, "Speed", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dSpeed = val;
	result = RegQueryValueEx(skey, "Cameraspeed", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dCameraspeed = val;
	result = RegQueryValueEx(skey, "Surface", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dSurface = val;
	result = RegQueryValueEx(skey, "Blur", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dBlur = val;
	result = RegQueryValueEx(skey, "FrameRateLimit", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dFrameRateLimit = val;

	RegCloseKey(skey);
}


// Save all user-defined stuff
void writeRegistry(){
    LONG result;
	HKEY skey;
	DWORD val, disp;

	result = RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &skey, &disp);
	if(result != ERROR_SUCCESS)
		return;

	val = dIons;
	RegSetValueEx(skey, "Ions", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = dSize;
	RegSetValueEx(skey, "Size", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = dEmitters;
	RegSetValueEx(skey, "Emitters", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = dAttracters;
	RegSetValueEx(skey, "Attracters", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = dSpeed;
	RegSetValueEx(skey, "Speed", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = dCameraspeed;
	RegSetValueEx(skey, "Cameraspeed", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = dSurface;
	RegSetValueEx(skey, "Surface", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = dBlur;
	RegSetValueEx(skey, "Blur", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = dFrameRateLimit;
	RegSetValueEx(skey, "FrameRateLimit", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));

	RegCloseKey(skey);
}


BOOL aboutProc(HWND hdlg, UINT msg, WPARAM wpm, LPARAM lpm){
	switch(msg){
	case WM_CTLCOLORSTATIC:
		if(HWND(lpm) == GetDlgItem(hdlg, WEBPAGE)){
			SetTextColor(HDC(wpm), RGB(0,0,255));
			SetBkColor(HDC(wpm), COLORREF(GetSysColor(COLOR_3DFACE)));
			return(int(GetSysColorBrush(COLOR_3DFACE)));
		}
		break;
    case WM_COMMAND:
		switch(LOWORD(wpm)){
		case IDOK:
		case IDCANCEL:
			EndDialog(hdlg, LOWORD(wpm));
			break;
		case WEBPAGE:
			ShellExecute(NULL, "open", "http://www.reallyslick.com/", NULL, NULL, SW_SHOWNORMAL);
		}
	}
	return FALSE;
}


void initControls(HWND hdlg){
	char cval[16];

	SendDlgItemMessage(hdlg, IONS, UDM_SETRANGE, 0, LPARAM(MAKELONG(DWORD(30000), DWORD(0))));
	SendDlgItemMessage(hdlg, IONS, UDM_SETPOS, 0, LPARAM(dIons));

	SendDlgItemMessage(hdlg, SIZE, TBM_SETRANGE, 0, LPARAM(MAKELONG(DWORD(1), DWORD(100))));
	SendDlgItemMessage(hdlg, SIZE, TBM_SETPOS, 1, LPARAM(dSize));
	SendDlgItemMessage(hdlg, SIZE, TBM_SETLINESIZE, 0, LPARAM(1));
	SendDlgItemMessage(hdlg, SIZE, TBM_SETPAGESIZE, 0, LPARAM(5));
	sprintf(cval, "%d", dSize);
	SendDlgItemMessage(hdlg, SIZETEXT, WM_SETTEXT, 0, LPARAM(cval));

	SendDlgItemMessage(hdlg, EMITTERS, TBM_SETRANGE, 0, LPARAM(MAKELONG(DWORD(1), DWORD(10))));
	SendDlgItemMessage(hdlg, EMITTERS, TBM_SETPOS, 1, LPARAM(dEmitters));
	SendDlgItemMessage(hdlg, EMITTERS, TBM_SETLINESIZE, 0, LPARAM(1));
	SendDlgItemMessage(hdlg, EMITTERS, TBM_SETPAGESIZE, 0, LPARAM(2));
	sprintf(cval, "%d", dEmitters);
	SendDlgItemMessage(hdlg, EMITTERTEXT, WM_SETTEXT, 0, LPARAM(cval));

	SendDlgItemMessage(hdlg, ATTRACTERS, TBM_SETRANGE, 0, LPARAM(MAKELONG(DWORD(1), DWORD(10))));
	SendDlgItemMessage(hdlg, ATTRACTERS, TBM_SETPOS, 1, LPARAM(dAttracters));
	SendDlgItemMessage(hdlg, ATTRACTERS, TBM_SETLINESIZE, 0, LPARAM(1));
	SendDlgItemMessage(hdlg, ATTRACTERS, TBM_SETPAGESIZE, 0, LPARAM(2));
	sprintf(cval, "%d", dAttracters);
	SendDlgItemMessage(hdlg, ATTRACTERTEXT, WM_SETTEXT, 0, LPARAM(cval));

	SendDlgItemMessage(hdlg, SPEED, TBM_SETRANGE, 0, LPARAM(MAKELONG(DWORD(1), DWORD(100))));
	SendDlgItemMessage(hdlg, SPEED, TBM_SETPOS, 1, LPARAM(dSpeed));
	SendDlgItemMessage(hdlg, SPEED, TBM_SETLINESIZE, 0, LPARAM(1));
	SendDlgItemMessage(hdlg, SPEED, TBM_SETPAGESIZE, 0, LPARAM(5));
	sprintf(cval, "%d", dSpeed);
	SendDlgItemMessage(hdlg, SPEEDTEXT, WM_SETTEXT, 0, LPARAM(cval));

	SendDlgItemMessage(hdlg, CAMERASPEED, TBM_SETRANGE, 0, LPARAM(MAKELONG(DWORD(0), DWORD(100))));
	SendDlgItemMessage(hdlg, CAMERASPEED, TBM_SETPOS, 1, LPARAM(dCameraspeed));
	SendDlgItemMessage(hdlg, CAMERASPEED, TBM_SETLINESIZE, 0, LPARAM(1));
	SendDlgItemMessage(hdlg, CAMERASPEED, TBM_SETPAGESIZE, 0, LPARAM(5));
	sprintf(cval, "%d", dCameraspeed);
	SendDlgItemMessage(hdlg, CAMERASPEEDTEXT, WM_SETTEXT, 0, LPARAM(cval));

	CheckDlgButton(hdlg, SURFACE, dSurface);

	SendDlgItemMessage(hdlg, BLUR, TBM_SETRANGE, 0, LPARAM(MAKELONG(DWORD(0), DWORD(100))));
	SendDlgItemMessage(hdlg, BLUR, TBM_SETPOS, 1, LPARAM(dBlur));
	SendDlgItemMessage(hdlg, BLUR, TBM_SETLINESIZE, 0, LPARAM(1));
	SendDlgItemMessage(hdlg, BLUR, TBM_SETPAGESIZE, 0, LPARAM(5));
	sprintf(cval, "%d", dBlur);
	SendDlgItemMessage(hdlg, BLURTEXT, WM_SETTEXT, 0, LPARAM(cval));

	initFrameRateLimitSlider(hdlg, FRAMERATELIMIT, FRAMERATELIMITTEXT);
}


BOOL screenSaverConfigureDialog(HWND hdlg, UINT msg,
										 WPARAM wpm, LPARAM lpm){
	int ival;
	char cval[16];

    switch(msg){
    case WM_INITDIALOG:
        InitCommonControls();
        readRegistry();
        initControls(hdlg);
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wpm)){
        case IDOK:
			dIons = SendDlgItemMessage(hdlg, IONS, UDM_GETPOS, 0, 0);
			dSize = SendDlgItemMessage(hdlg, SIZE, TBM_GETPOS, 0, 0);
			dEmitters = SendDlgItemMessage(hdlg, EMITTERS, TBM_GETPOS, 0, 0);
            dAttracters = SendDlgItemMessage(hdlg, ATTRACTERS, TBM_GETPOS, 0, 0);
			dSpeed = SendDlgItemMessage(hdlg, SPEED, TBM_GETPOS, 0, 0);
			dCameraspeed = SendDlgItemMessage(hdlg, CAMERASPEED, TBM_GETPOS, 0, 0);
			dSurface = (IsDlgButtonChecked(hdlg, SURFACE) == BST_CHECKED);
			dBlur = SendDlgItemMessage(hdlg, BLUR, TBM_GETPOS, 0, 0);
			dFrameRateLimit = SendDlgItemMessage(hdlg, FRAMERATELIMIT, TBM_GETPOS, 0, 0);
			writeRegistry();
            // Fall through
        case IDCANCEL:
            EndDialog(hdlg, LOWORD(wpm));
            break;
		case DEFAULTS:
			setDefaults();
			initControls(hdlg);
			break;
		case ABOUT:
			DialogBox(mainInstance, MAKEINTRESOURCE(DLG_ABOUT), hdlg, DLGPROC(aboutProc));
			break;
		}
        return TRUE;
	case WM_HSCROLL:
		if(HWND(lpm) == GetDlgItem(hdlg, SIZE)){
			ival = SendDlgItemMessage(hdlg, SIZE, TBM_GETPOS, 0, 0);
			sprintf(cval, "%d", ival);
			SendDlgItemMessage(hdlg, SIZETEXT, WM_SETTEXT, 0, LPARAM(cval));
		}
		if(HWND(lpm) == GetDlgItem(hdlg, EMITTERS)){
			ival = SendDlgItemMessage(hdlg, EMITTERS, TBM_GETPOS, 0, 0);
			sprintf(cval, "%d", ival);
			SendDlgItemMessage(hdlg, EMITTERTEXT, WM_SETTEXT, 0, LPARAM(cval));
		}
		if(HWND(lpm) == GetDlgItem(hdlg, ATTRACTERS)){
			ival = SendDlgItemMessage(hdlg, ATTRACTERS, TBM_GETPOS, 0, 0);
			sprintf(cval, "%d", ival);
			SendDlgItemMessage(hdlg, ATTRACTERTEXT, WM_SETTEXT, 0, LPARAM(cval));
		}
		if(HWND(lpm) == GetDlgItem(hdlg, SPEED)){
			ival = SendDlgItemMessage(hdlg, SPEED, TBM_GETPOS, 0, 0);
			sprintf(cval, "%d", ival);
			SendDlgItemMessage(hdlg, SPEEDTEXT, WM_SETTEXT, 0, LPARAM(cval));
		}
		if(HWND(lpm) == GetDlgItem(hdlg, CAMERASPEED)){
			ival = SendDlgItemMessage(hdlg, CAMERASPEED, TBM_GETPOS, 0, 0);
			sprintf(cval, "%d", ival);
			SendDlgItemMessage(hdlg, CAMERASPEEDTEXT, WM_SETTEXT, 0, LPARAM(cval));
		}
		if(HWND(lpm) == GetDlgItem(hdlg, BLUR)){
			ival = SendDlgItemMessage(hdlg, BLUR, TBM_GETPOS, 0, 0);
			sprintf(cval, "%d", ival);
			SendDlgItemMessage(hdlg, BLURTEXT, WM_SETTEXT, 0, LPARAM(cval));
		}
		if(HWND(lpm) == GetDlgItem(hdlg, FRAMERATELIMIT))
			updateFrameRateLimitSlider(hdlg, FRAMERATELIMIT, FRAMERATELIMITTEXT);
		return TRUE;
    }
    return FALSE;
}


LONG screenSaverProc(HWND hwnd, UINT msg, WPARAM wpm, LPARAM lpm){
	switch(msg){
	case WM_CREATE:
		readRegistry();
		doSaver(hwnd);
		readyToDraw = 1;
		break;
	case WM_DESTROY:
		readyToDraw = 0;
		cleanUp(hwnd);
		break;
	}
	return defScreenSaverProc(hwnd, msg, wpm, lpm);
}*/
