/*
 * Copyright (C) 2002  Terence M. Welsh
 *
 * Implicit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation.
 *
 * Implicit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef IMPSURFACE_H
#define IMPSURFACE_H


#ifdef WIN32
#include <windows.h>
#endif

#include <string.h>
#include <OpenGL/gl.h>


class impSurface{
public:
    impSurface(){
        tristrips = NULL;
        vertices = NULL;
        num_tristrips = 0;
        // init(1000);
        init(6000);
    };
    ~impSurface(){
        delete[] tristrips;
        if(vertices) {
            for(int i=0; i<max_tristrips * 7; i++)
                delete[] vertices[i];
            delete[] vertices;
        }
    };
    void init(int max){
        int i;

        if(tristrips)
            delete[] tristrips;
        if(vertices) {
            for(i=0; i<max_tristrips * 7; i++)
                delete[] vertices[i];
            delete[] vertices;
        }
        
        max_tristrips = max;
        tristrips = new int[max_tristrips];
        vertices = new float*[max_tristrips * 7];
        for(i=0; i<max_tristrips * 7; i++)
            vertices[i] = new float[6];  // 3 for normal vector, 3 for position
    };
    void reset(){ num_tristrips = 0; };
    int addstrip(int length, float* data){
        int i;

        if(num_tristrips == max_tristrips)
            return 0;

        tristrips[num_tristrips] = length;

        for(i=0; i<length; i++)
            memcpy(vertices[num_tristrips * 7 + i], &data[i*6], 6 * sizeof(float));

        num_tristrips++;

        return 1;
    };
    void draw(){
        int i, index;

        for(i=0; i<num_tristrips; i++){
            index = i * 7;
            switch(tristrips[i]){  // gives the number of vertices in strip
                case 3:
                    glBegin(GL_TRIANGLES);
                    glNormal3fv(vertices[index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glEnd();
                    break;
                case 4:
                    glBegin(GL_TRIANGLE_STRIP);
                    glNormal3fv(vertices[index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glEnd();
                    break;
                case 5:
                    glBegin(GL_TRIANGLE_STRIP);
                    glNormal3fv(vertices[index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glEnd();
                    break;
                case 6:
                    glBegin(GL_TRIANGLE_STRIP);
                    glNormal3fv(vertices[index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glEnd();
                    break;
                case 7:
                    glBegin(GL_TRIANGLE_STRIP);
                    glNormal3fv(vertices[index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glEnd();
                    break;
            }
        }
    };
    void draw_wireframe(){
        int i, index;

        for(i=0; i<num_tristrips; i++){
            index = i * 7;
            switch(tristrips[i]){  // gives the number of vertices in strip
                case 3:
                    glBegin(GL_LINE_STRIP);
                    glNormal3fv(vertices[index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glEnd();
                    break;
                case 4:
                    glBegin(GL_LINE_STRIP);
                    glNormal3fv(vertices[index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    // Draw this one extra line so that almost every
                    // line gets drawn and there are few duplicates
                    index -= 2;
                    glNormal3fv(vertices[index]);
                    glVertex3fv(&vertices[index][3]);
                    glEnd();
                    break;
                case 5:
                    glBegin(GL_LINE_STRIP);
                    glNormal3fv(vertices[index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glEnd();
                    break;
                case 6:
                    glBegin(GL_LINE_STRIP);
                    glNormal3fv(vertices[index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glEnd();
                    break;
                case 7:
                    glBegin(GL_LINE_STRIP);
                    glNormal3fv(vertices[index]);
                    glVertex3fv(&vertices[index++][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glNormal3fv(vertices[++index]);
                    glVertex3fv(&vertices[index][3]);
                    glEnd();
                    break;
            }
        }
    };

private:
    int num_tristrips;
    int max_tristrips;
    int* tristrips;
    float** vertices;
};



#endif
