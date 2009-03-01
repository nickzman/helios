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


#ifndef IMPELEMENT_H
#define IMPELEMENT_H


// This is a generic class from which specific elements are
// derived.  An element would be a sphere or torus or anything
// that defines an implicit surface.


#include <math.h>


class impElement{
public:
    float mat[16];
    float invmat[16];

    impElement(){
        mat[0] = mat[5] = mat[10] = mat[15]
        = invmat[0] = invmat[5] = invmat[10] = invmat[15] = 1.0f;
        mat[1] = mat[2] = mat[3] = mat[4]
            = mat[6] = mat[7] = mat[8] = mat[9]
            = mat[11] = mat[12] = mat[13] = mat[14]
            = invmat[1] = invmat[2] = invmat[3] = invmat[4]
            = invmat[6] = invmat[7] = invmat[8] = invmat[9]
            = invmat[11] = invmat[12] = invmat[13] = invmat[14] = 0.0f;
    };
    // ~impElement(){};
    inline void setPosition(float x, float y, float z){
        mat[12] = x;
        mat[13] = y;
        mat[14] = z;
        invmat[12] = -x;
        invmat[13] = -y;
        invmat[14] = -z;
    };
    inline void setPosition(float* position){
        mat[12] = position[0];
        mat[13] = position[1];
        mat[14] = position[2];
        invmat[12] = -position[0];
        invmat[13] = -position[1];
        invmat[14] = -position[2];
    };
    inline void setMatrix(float* m){
        for(int i=0; i<16; i++)
            mat[i] = m[i];

        invertMatrix();
    };
    inline void invertMatrix(){
        float rmat[4][8];
        float a, b;
        int i, j, k;

        // inititialize reduction matrix
        rmat[0][0] = mat[0];
        rmat[1][0] = mat[1];
        rmat[2][0] = mat[2];
        rmat[3][0] = mat[3];
        rmat[0][1] = mat[4];
        rmat[1][1] = mat[5];
        rmat[2][1] = mat[6];
        rmat[3][1] = mat[7];
        rmat[0][2] = mat[8];
        rmat[1][2] = mat[9];
        rmat[2][2] = mat[10];
        rmat[3][2] = mat[11];
        rmat[0][3] = mat[12];
        rmat[1][3] = mat[13];
        rmat[2][3] = mat[14];
        rmat[3][3] = mat[15];
        rmat[0][4] = 1.0f;
        rmat[1][4] = 0.0f;
        rmat[2][4] = 0.0f;
        rmat[3][4] = 0.0f;
        rmat[0][5] = 0.0f;
        rmat[1][5] = 1.0f;
        rmat[2][5] = 0.0f;
        rmat[3][5] = 0.0f;
        rmat[0][6] = 0.0f;
        rmat[1][6] = 0.0f;
        rmat[2][6] = 1.0f;
        rmat[3][6] = 0.0f;
        rmat[0][7] = 0.0f;
        rmat[1][7] = 0.0f;
        rmat[2][7] = 0.0f;
        rmat[3][7] = 1.0f;

        // perform reductions
        for(i=0; i<4; i++){
            a = rmat[i][i];
            if(a == 0.0f)
                return;  // matrix is singular, can't be inverted
            a = 1.0f / a;
            for(j=0; j<8; j++)
                rmat[i][j] = rmat[i][j] * a;
            for(k=0; k<4; k++){
                if((k-i) != 0){
                    b = rmat[k][i];
                    for(j=0; j<8; j++)
                        rmat[k][j] = rmat[k][j] - b * rmat[i][j];
                }
            }
        }

        // extract inverted matrix
        invmat[0] = rmat[0][4];
        invmat[1] = rmat[1][4];
        invmat[2] = rmat[2][4];
        invmat[3] = rmat[3][4];
        invmat[4] = rmat[0][5];
        invmat[5] = rmat[1][5];
        invmat[6] = rmat[2][5];
        invmat[7] = rmat[3][5];
        invmat[8] = rmat[0][6];
        invmat[9] = rmat[1][6];
        invmat[10] = rmat[2][6];
        invmat[11] = rmat[3][6];
        invmat[12] = rmat[0][7];
        invmat[13] = rmat[1][7];
        invmat[14] = rmat[2][7];
        invmat[15] = rmat[3][7];
    };
    // returns the value of this element at a given position
    // "position" is an array of 3 floats
    inline float value(float* position){ return(0.0f); };
    // assigns a center of the element's volume to "position"
    inline void center(float* position){
        position[0] = mat[12];
        position[1] = mat[13];
        position[2] = mat[14];
    };
};




#endif
