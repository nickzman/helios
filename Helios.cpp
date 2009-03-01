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


#include <windows.h>
#include <stdio.h>
#include "../Savergl/Savergl.h"
#include <math.h>
#include <time.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <regstr.h>
#include <commctrl.h>
#include "../../rsMath/rsMath.h"
#include "../../rgbhsl/rgbhsl.h"
#include "../../Implicit/impCubeVolume.h"
#include "../../Implicit/impCrawlPoint.h"
#include "../../Implicit/impSphere.h"
#include <list>
#include "spheremap.h"
#include "resource.h"


#define LIGHTSIZE 64
#define PI 3.14159265359f
#define PIx2 6.28318530718f


class particle;
class emitter;
class attracter;
class ion;



// Global variables
LPCTSTR registryPath = ("Software\\Really Slick\\Helios");
HGLRC hglrc;
HDC hdc;
int readyToDraw = 0;
unsigned char lightTexture[LIGHTSIZE][LIGHTSIZE];
float elapsedTime = 0.0f;
emitter *elist;
attracter *alist;
ion *ilist;
rsVec newRgb;
float billboardMat[16];
// Parameters edited in the dialog box
int dIons;
int dSize;
int dEmitters;
int dAttracters;
int dSpeed;
int dCameraspeed;
int dSurface;
int dWireframe;
int dBlur;
int dPriority;

impCubeVolume* volume;
impSurface* surface;
impSphere* spheres;



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
	pos = rsVec(myRandf(1000.0f) - 500.0f, myRandf(1000.0f) - 500.0f, myRandf(1000.0f) - 500.0f);
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
	pos = rsVec(myRandf(1000.0f) - 500.0f, myRandf(1000.0f) - 500.0f, myRandf(1000.0f) - 500.0f);
}


class ion:public particle{
public:
	float speed;

	ion();
	~ion(){};
	void start();
	void update();
	void draw();
};

ion::ion(){
	float temp;

	pos = rsVec(0.0f, 0.0f, 0.0f);
	rgb = rsVec(0.0f, 0.0f, 0.0f);
	temp = myRandf(2.0f) + 0.4f;
	size = float(dSize) * temp;
	speed = float(dSpeed) * 12.0f / temp;
}

void ion::start(){
	int i = myRandi(dEmitters);
	pos = elist[i].pos;
	float offset = elapsedTime * speed;
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

	rgb = newRgb;
}


void ion::update(){
	int i;
	int startOver = 0;
	static float startOverDistance;
	static rsVec force, tempvec;
	static float length, temp;

	force = rsVec(0.0f, 0.0f, 0.0f);
	for(i=0; i<dEmitters; i++){
		tempvec = pos - elist[i].pos;
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
	startOverDistance = speed * elapsedTime;
	for(i=0; i<dAttracters; i++){
		tempvec = alist[i].pos - pos;
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
		start();
	else{
		force.normalize();
		pos += (force * elapsedTime * speed);
	}
}

void ion::draw(){
	glColor3f(rgb[0], rgb[1], rgb[2]);
	glPushMatrix();
		glTranslatef(pos[0] * billboardMat[0] + pos[1] * billboardMat[4] + pos[2] * billboardMat[8],
			pos[0] * billboardMat[1] + pos[1] * billboardMat[5] + pos[2] * billboardMat[9],
			pos[0] * billboardMat[2] + pos[1] * billboardMat[6] + pos[2] * billboardMat[10]);
		glScalef(size, size, size);
		glCallList(1);
	glPopMatrix();
}


void setTargets(int whichTarget){
	int i;

	switch(whichTarget){
	case 0:  // random
		for(i=0; i<dEmitters; i++)
			elist[i].settargetpos(rsVec(rsVec(myRandf(1000.0f) - 500.0f, myRandf(1000.0f) - 500.0f, myRandf(1000.0f) - 500.0f)));
		for(i=0; i<dAttracters; i++)
			alist[i].settargetpos(rsVec(rsVec(myRandf(1000.0f) - 500.0f, myRandf(1000.0f) - 500.0f, myRandf(1000.0f) - 500.0f)));
		break;
	case 1:{  // line (all emitters on one side, all attracters on the other)
		float position = -500.0f, change = 1000.0f / float(dEmitters + dAttracters - 1);
		for(i=0; i<dEmitters; i++){
			elist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
			position += change;
		}
		for(i=0; i<dAttracters; i++){
			alist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
			position += change;
		}
		break;
		}
	case 2:{  // line (emitters and attracters staggered)
		float change;
		if(dEmitters > dAttracters)
			change = 1000.0f / float(dEmitters * 2 - 1);
		else
			change = 1000.0f / float(dAttracters * 2 - 1);
		float position = -500.0f;
		for(i=0; i<dEmitters; i++){
			elist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
			position += change * 2.0f;
		}
		position = -500.0f + change;
		for(i=0; i<dAttracters; i++){
			alist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
			position += change * 2.0f;
		}
		break;
		}
	case 3:{  // 2 lines (parallel)
		float change = 1000.0f / float(dEmitters * 2 - 1);
		float position = -500.0f;
		float height = -525.0f + float(dEmitters * 25);
		for(i=0; i<dEmitters; i++){
			elist[i].settargetpos(rsVec(rsVec(position, height, -50.0f)));
			position += change * 2.0f;
		}
		change = 1000.0f / float(dAttracters * 2 - 1);
		position = -500.0f;
		height = 525.0f - float(dAttracters * 25);
		for(i=0; i<dAttracters; i++){
			alist[i].settargetpos(rsVec(rsVec(position, height, 50.0f)));
			position += change * 2.0f;
		}
		break;
		}
	case 4:{  // 2 lines (skewed)
		float change = 1000.0f / float(dEmitters * 2 - 1);
		float position = -500.0f;
		float height = -525.0f + float(dEmitters * 25);
		for(i=0; i<dEmitters; i++){
			elist[i].settargetpos(rsVec(rsVec(position, height, 0.0f)));
			position += change * 2.0f;
		}
		change = 1000.0f / float(dAttracters * 2 - 1);
		position = -500.0f;
		height = 525.0f - float(dAttracters * 25);
		for(i=0; i<dAttracters; i++){
			alist[i].settargetpos(rsVec(rsVec(10.0f, height, position)));
			position += change * 2.0f;
		}
		break;
		}
	case 5:  // random distribution across a plane
		for(i=0; i<dEmitters; i++)
			elist[i].settargetpos(rsVec(rsVec(myRandf(1000.0f) - 500.0f, 0.0f, myRandf(1000.0f) - 500.0f)));
		for(i=0; i<dAttracters; i++)
			alist[i].settargetpos(rsVec(rsVec(myRandf(1000.0f) - 500.0f, 0.0f, myRandf(1000.0f) - 500.0f)));
		break;
	case 6:{  // random distribution across 2 planes
		float height = -525.0f + float(dEmitters * 25);
		for(i=0; i<dEmitters; i++)
			elist[i].settargetpos(rsVec(rsVec(myRandf(1000.0f) - 500.0f, height, myRandf(1000.0f) - 500.0f)));
		height = 525.0f - float(dAttracters * 25);
		for(i=0; i<dAttracters; i++)
			alist[i].settargetpos(rsVec(rsVec(myRandf(1000.0f) - 500.0f, height, myRandf(1000.0f) - 500.0f)));
		break;
		}
	case 7:{  // 2 rings (1 inside and 1 outside)
		float angle = 0.5f, cosangle, sinangle;
		float change = PIx2 / float(dEmitters);
		for(i=0; i<dEmitters; i++){
			angle += change;
			cosangle = cosf(angle) * 200.0f;
			sinangle = sinf(angle) * 200.0f;
			elist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
		}
		angle = 1.5f;
		change = PIx2 / float(dAttracters);
		for(i=0; i<dAttracters; i++){
			angle += change;
			cosangle = cosf(angle) * 500.0f;
			sinangle = sinf(angle) * 500.0f;
			alist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
		}
		break;
		}
	case 8:{  // ring (all emitters on one side, all attracters on the other)
		float angle = 0.5f, cosangle, sinangle;
		float change = PIx2 / float(dEmitters + dAttracters);
		for(i=0; i<dEmitters; i++){
			angle += change;
			cosangle = cosf(angle) * 500.0f;
			sinangle = sinf(angle) * 500.0f;
			elist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
		}
		for(i=0; i<dAttracters; i++){
			angle += change;
			cosangle = cosf(angle) * 500.0f;
			sinangle = sinf(angle) * 500.0f;
			alist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
		}
		break;
		}
	case 9:{  // ring (emitters and attracters staggered)
		float change;
		if(dEmitters > dAttracters)
			change = PIx2 / float(dEmitters * 2);
		else
			change = PIx2 / float(dAttracters * 2);
		float angle = 0.5f, cosangle, sinangle;
		for(i=0; i<dEmitters; i++){
			cosangle = cosf(angle) * 500.0f;
			sinangle = sinf(angle) * 500.0f;
			elist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
			angle += change * 2.0f;
		}
		angle = 0.5f + change;
		for(i=0; i<dAttracters; i++){
			cosangle = cosf(angle) * 500.0f;
			sinangle = sinf(angle) * 500.0f;
			alist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
			angle += change * 2.0f;
		}
		break;
		}
	case 10:  // 2 points
		for(i=0; i<dEmitters; i++)
			elist[i].settargetpos(rsVec(rsVec(500.0f, 100.0f, 50.0f)));
		for(i=0; i<dAttracters; i++)
			alist[i].settargetpos(rsVec(rsVec(-500.0f, -100.0f, -50.0f)));
		break;
	}
}


float surfaceFunction(float* position){
	static int i;
	static float value;
	static int points = dEmitters + dAttracters;

	value = 0.0f;
	for(i=0; i<points; i++)
		value += spheres[i].value(position);

	return(value);
}


void draw(){
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
	preCameraInterp += float(dCameraspeed) * elapsedTime * 0.01f;
	cameraInterp = 0.5f - (0.5f * cosf(preCameraInterp));
	cameraDistance = (1.0f - cameraInterp) * oldCameraDistance
		+ cameraInterp * targetCameraDistance;
	if(preCameraInterp >= PI){
		oldCameraDistance = targetCameraDistance;
		targetCameraDistance = -myRandf(1300.0f) - 200.0f;
		preCameraInterp = 0.0f;
	}
	glLoadIdentity();
	glTranslatef(0.0, 0.0, cameraDistance);

	// then do rotation
	static rsVec radialVel = rsVec(0.0f, 0.0f, 0.0f);
	static rsVec targetRadialVel = radialVel;
	static rsQuat rotQuat = rsQuat(0.0f, 0.0f, 0.0f, 1.0f);
	rsVec radialVelDiff = targetRadialVel - radialVel;
	float changeRemaining = radialVelDiff.normalize();
	float change = float(dCameraspeed) * 0.0002f * elapsedTime;
	if(changeRemaining > change){
		radialVelDiff *= change;
		radialVel += radialVelDiff;
	}
	else{
		radialVel = targetRadialVel;
		if(myRandi(2)){
			targetRadialVel = rsVec(myRandf(1.0f), myRandf(1.0f), myRandf(1.0f));
			targetRadialVel.normalize();
			targetRadialVel *= float(dCameraspeed) * myRandf(0.002f);
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
	rotMat.get(billboardMat);

	// Calculate new color
	static rsVec oldHsl, newHsl = rsVec(myRandf(1.0f), 1.0f, 1.0f), targetHsl;
	static float colorInterp = 1.0f, colorChange;
	colorInterp += elapsedTime * colorChange;
	if(colorInterp >= 1.0f){
		if(!myRandi(3) && dIons >= 100)  // change color suddenly
			newHsl = rsVec(myRandf(1.0f), 1.0f - (myRandf(1.0f) * myRandf(1.0f)), 1.0f);
		oldHsl = newHsl;
		targetHsl = rsVec(myRandf(1.0f), 1.0f - (myRandf(1.0f) * myRandf(1.0f)), 1.0f);
		colorInterp = 0.0f;
		// amount by which to change colorInterp each second
		colorChange = myRandf(0.005f * float(dSpeed)) + (0.002f * float(dSpeed));
	}
	else{
		float diff = targetHsl[0] - oldHsl[0];
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
		hsl2rgb(newHsl[0], newHsl[1], 1.0f, newRgb[0], newRgb[1], newRgb[2]);
	}

	// Release ions
	if(ionsReleased < dIons){
		releaseTime -= elapsedTime;
		while(ionsReleased < dIons && releaseTime <= 0.0f){
			ilist[ionsReleased].start();
			ionsReleased ++;
			// all ions released after 2 minutes
			releaseTime += 120.0f / float(dIons);
		}
	}

	// Set interpolation value for emitters and attracters
	static float wait = 0.0f;
	static float preinterp = PI, interp;
	static float interpconst = 0.001f;
	wait -= elapsedTime;
	if(wait <= 0.0f){
		preinterp += elapsedTime * float(dSpeed) * interpconst;
		interp = 0.5f - (0.5f * cosf(preinterp));
	}
	if(preinterp >= PI){
		// select new taget points (not the same pattern twice in a row)
		static int newTarget = 0, lastTarget;
		lastTarget = newTarget;
		newTarget = myRandi(10);
		if(newTarget == lastTarget)
			newTarget ++;
		setTargets(newTarget);
		preinterp = 0.0f;
		interp = 0.0f;
		wait = 10.0f;  // pause after forming each new pattern
		interpconst = 0.001f;
		if(!myRandi(4))  // interpolate really fast sometimes
			interpconst = 0.1f;
	}


	// Update particles
	for(i=0; i<dEmitters; i++){
		elist[i].interppos(interp);
		elist[i].update();
	}
	for(i=0; i<dAttracters; i++){
		alist[i].interppos(interp);
		alist[i].update();
	}
	for(i=0; i<ionsReleased; i++)
		ilist[i].update();

	// Calculate surface
	if(dSurface){
		for(i=0; i<dEmitters; i++)
			spheres[i].setPosition(elist[i].pos[0], elist[i].pos[1], elist[i].pos[2]);
		for(i=0; i<dAttracters; i++)
			spheres[dEmitters+i].setPosition(alist[i].pos[0], alist[i].pos[1], alist[i].pos[2]);
		std::list<impCrawlPoint> crawlpointlist;
		float center[3];
		for(i=0; i<dEmitters+dAttracters; i++){
			spheres[i].center(center);
			crawlpointlist.push_back(impCrawlPoint(center));
		}
		surface->reset();
		static float valuetrig = 0.0f;
		valuetrig += elapsedTime;
		volume->surfacevalue = 0.45f + 0.05f * cosf(valuetrig);
		volume->make_surface(crawlpointlist);
	}

	// Draw
	// clear the screen
	if(dBlur){  // partially
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0f, 0.0f, 0.0f, 0.5f - (float(sqrt(sqrt(double(dBlur)))) * 0.15495f));
		glPushMatrix();
		glLoadIdentity();
		glBegin(GL_TRIANGLE_STRIP);
			glVertex3f(-5.0f, -4.0f, -3.0f);
			glVertex3f(5.0f, -4.0f, -3.0f);
			glVertex3f(-5.0f, 4.0f, -3.0f);
			glVertex3f(5.0f, 4.0f, -3.0f);
		glEnd();
		glPopMatrix();
	}
	else  // completely
		glClear(GL_COLOR_BUFFER_BIT);

	// Draw ions
	glBlendFunc(GL_ONE, GL_ONE);
	glBindTexture(GL_TEXTURE_2D, 1);
	for(i=0; i<ionsReleased; i++)
		ilist[i].draw();

	// Draw surfaces
	float brightFactor;
	float surfaceColor[3] = {0.0f, 0.0f, 0.0f};
	if(dSurface){
		glBindTexture(GL_TEXTURE_2D, 2);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		// find color for surfaces
		if(dIons >= 100){
			if(dWireframe)
				brightFactor = 2.0f / (float(dBlur + 30) * float(dBlur + 30));
			else
				brightFactor = 4.0f / (float(dBlur + 30) * float(dBlur + 30));
			for(i=0; i<100; i++){
				surfaceColor[0] += ilist[i].rgb[0] * brightFactor;
				surfaceColor[1] += ilist[i].rgb[1] * brightFactor;
				surfaceColor[2] += ilist[i].rgb[2] * brightFactor;
			}
			glColor3fv(surfaceColor);
		}
		else{
			if(dWireframe)
				brightFactor = 200.0f / (float(dBlur + 30) * float(dBlur + 30));
			else
				brightFactor = 400.0f / (float(dBlur + 30) * float(dBlur + 30));
			glColor3f(newRgb[0] * brightFactor, newRgb[1] * brightFactor, newRgb[2] * brightFactor);
		}
		// draw the surface
		glPushMatrix();
			glMultMatrixf(billboardMat);
			if(dWireframe){
				glDisable(GL_TEXTURE_2D);
				surface->draw_wireframe();
				glEnable(GL_TEXTURE_2D);
			}
			else
				surface->draw();
		glPopMatrix();
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}

	// If graphics card does a true buffer swap instead of a copy swap
	// then everything must get drawn on both buffers
	if(dBlur && pfd_swap_exchange){
		wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0f, 0.0f, 0.0f, 0.5f - (float(sqrt(sqrt(double(dBlur)))) * 0.15495f));
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
				if(dWireframe){
					glDisable(GL_TEXTURE_2D);
					surface->draw_wireframe();
					glEnable(GL_TEXTURE_2D);
				}
				else
					surface->draw();
			glPopMatrix();
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
		}
	}

	wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
}


void IdleProc(){
	static DWORD thisTime = timeGetTime(), lastTime;

	// update time
	lastTime = thisTime;
    thisTime = timeGetTime();
    if(thisTime >= lastTime)
		elapsedTime = float(thisTime - lastTime) * 0.001f;
    // else use elapsedTime from last frame

	if(readyToDraw && !checkingPassword)
		draw();
}


void doSaver(HWND hwnd){
	int i, j;
	float x, y, temp;
	RECT rect;

	// Seed random number generator
	srand((unsigned)time(NULL));
	rand(); rand(); rand(); rand(); rand();
	rand(); rand(); rand(); rand(); rand();
	rand(); rand(); rand(); rand(); rand();
	rand(); rand(); rand(); rand(); rand();

	// Window initialization
	hdc = GetDC(hwnd);
	SetBestPixelFormat(hdc);
	hglrc = wglCreateContext(hdc);
	GetClientRect(hwnd, &rect);
	wglMakeCurrent(hdc, hglrc);
	glViewport(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, float(rect.right) / float(rect.bottom), 0.1, 10000.0f);
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	// Clear the buffers and test for type of buffer swapping
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
	glClear(GL_COLOR_BUFFER_BIT);
	unsigned char pixel[1] = {255};
	glRasterPos2i(0, 0);
	glDrawPixels(1, 1, GL_RED, GL_UNSIGNED_BYTE, pixel);
	wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
	glReadPixels(0, 0, 1, 1, GL_RED, GL_UNSIGNED_BYTE, pixel);
	if(pixel[0] == 0){  // Color was swapped out of the back buffer
		pfd_swap_exchange = 1;
		pfd_swap_copy = 0;
	}
	else{    // Color remains in back buffer
		pfd_swap_exchange = 0;
		pfd_swap_copy = 1;
	}

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
			lightTexture[i][j] = unsigned char(255.0f * temp * temp);
		}
	}
	glBindTexture(GL_TEXTURE_2D, 1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 1, LIGHTSIZE, LIGHTSIZE, GL_LUMINANCE, 
		GL_UNSIGNED_BYTE, lightTexture);
	glBindTexture(GL_TEXTURE_2D, 2);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TEXSIZE, TEXSIZE, GL_RGB, 
		GL_UNSIGNED_BYTE, spheremap);
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
	elist = new emitter[dEmitters];
	alist = new attracter[dAttracters];
	ilist = new ion[dIons];

	// Initialize surface
	if(dSurface){
		volume = new impCubeVolume;
		volume->init(50, 50, 50, 35.0f);
		volume->function = surfaceFunction;
		surface = new impSurface;
		surface->init(6000);
		volume->surface = surface;
		spheres = new impSphere[dEmitters + dAttracters];
		float sphereScaleFactor = 1.0f / float(sqrt(double(2 * dEmitters + dAttracters)));
		for(i=0; i<dEmitters; i++)
			spheres[i].setScale(400.0f * sphereScaleFactor);
		for(i=0; i<dAttracters; i++)
			spheres[i + dEmitters].setScale(200.0f * sphereScaleFactor);
	}
}

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


void setDefaults(){
	dIons = 1500;
	dSize = 10;
	dEmitters = 3;
	dAttracters = 3;
	dSpeed = 10;
	dCameraspeed = 10;
	dSurface = 1;
	dWireframe = 0;
	dBlur = 10;
}


// Initialize all user-defined stuff
void readRegistry(){
	LONG result;
	HKEY skey;
	DWORD valtype, valsize, val;

	setDefaults();
	dPriority = 0;

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
	result = RegQueryValueEx(skey, "Wireframe", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dWireframe = val;
	result = RegQueryValueEx(skey, "Blur", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dBlur = val;
	result = RegQueryValueEx(skey, "Priority", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dPriority = val;
	result = RegQueryValueEx(skey, "theVideoMode", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		theVideoMode = val;

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
	//val = dWireframe;
	//RegSetValueEx(skey, "Wireframe", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = dBlur;
	RegSetValueEx(skey, "Blur", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = dPriority;
	RegSetValueEx(skey, "Priority", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = theVideoMode;
	RegSetValueEx(skey, "theVideoMode", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));

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
	CheckDlgButton(hdlg, WIREFRAME, dWireframe);
	SendDlgItemMessage(hdlg, BLUR, TBM_SETRANGE, 0, LPARAM(MAKELONG(DWORD(0), DWORD(100))));
	SendDlgItemMessage(hdlg, BLUR, TBM_SETPOS, 1, LPARAM(dBlur));
	SendDlgItemMessage(hdlg, BLUR, TBM_SETLINESIZE, 0, LPARAM(1));
	SendDlgItemMessage(hdlg, BLUR, TBM_SETPAGESIZE, 0, LPARAM(5));
	sprintf(cval, "%d", dBlur);
	SendDlgItemMessage(hdlg, BLURTEXT, WM_SETTEXT, 0, LPARAM(cval));
	SendDlgItemMessage(hdlg, PRIORITY, TBM_SETRANGE, 0, LPARAM(MAKELONG(DWORD(0), DWORD(2))));
	SendDlgItemMessage(hdlg, PRIORITY, TBM_SETPOS, 1, LPARAM(dPriority));
	SendDlgItemMessage(hdlg, PRIORITY, TBM_SETLINESIZE, 0, LPARAM(1));
	SendDlgItemMessage(hdlg, PRIORITY, TBM_SETPAGESIZE, 0, LPARAM(1));
	sprintf(cval, "%d", dPriority);
	SendDlgItemMessage(hdlg, PRIORITYTEXT, WM_SETTEXT, 0, LPARAM(cval));

	InitVideoModeComboBox(hdlg, VIDEOMODE);
}


BOOL ScreenSaverConfigureDialog(HWND hdlg, UINT msg,
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
			dWireframe = (IsDlgButtonChecked(hdlg, WIREFRAME) == BST_CHECKED);
			dBlur = SendDlgItemMessage(hdlg, BLUR, TBM_GETPOS, 0, 0);
			dPriority = SendDlgItemMessage(hdlg, PRIORITY, TBM_GETPOS, 0, 0);
			theVideoMode = SendDlgItemMessage(hdlg, VIDEOMODE, CB_GETCURSEL, 0, 0);
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
		if(HWND(lpm) == GetDlgItem(hdlg, PRIORITY)){
			ival = SendDlgItemMessage(hdlg, PRIORITY, TBM_GETPOS, 0, 0);
			sprintf(cval, "%d", ival);
			SendDlgItemMessage(hdlg, PRIORITYTEXT, WM_SETTEXT, 0, LPARAM(cval));
		}
		return TRUE;
    }
    return FALSE;
}


LONG ScreenSaverProc(HWND hwnd, UINT msg, WPARAM wpm, LPARAM lpm){
	switch(msg){
	case WM_CREATE:
		readRegistry();
		// Preview takes to long to initialize because Windows sucks
		// So doing bother wasting time on surfaces for preview window.
		if(doingPreview)
			dSurface = 0;
		switch(dPriority){
		case 0:
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
			break;
		case 1:
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
			break;
		case 2:
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
		}
		doSaver(hwnd);
		readyToDraw = 1;
		break;
	case WM_DESTROY:
		readyToDraw = 0;
		cleanUp(hwnd);
		break;
	}
	return DefScreenSaverProc(hwnd, msg, wpm, lpm);
}