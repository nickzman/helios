/*
* Copyright (C) 2002  Terence M. Welsh
 *
 * rsMath is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation.
 *
 * rsMath is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef RSMATH_H
#define RSMATH_H



#include <math.h>
#include "rsDefines.h"
#include "rsRand.h"


class rsVec{
public:
    float v[3];

    rsVec() {};
    rsVec(float xx, float yy, float zz){
        v[0] = xx;
        v[1] = yy;
        v[2] = zz;
    };
    float length(){
        return(float(sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])));
    };
    float normalize(){
        float length = float(sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]));
        if(length == 0.0f){
            v[1] = 1.0f;
            return(0.0f);
        }
        float reciprocal = 1.0f / length;
        v[0] *= reciprocal;
        v[1] *= reciprocal;
        v[2] *= reciprocal;
        // Really freakin' stupid compiler bug fix for VC++ 5.0
        /*v[0] /= length;
        v[1] /= length;
        v[2] /= length;*/
        return(length);
    };
    float dot(rsVec vec1){
        return(v[0] * vec1[0] + v[1] * vec1[1] + v[2] * vec1[2]);
    };
    void cross(rsVec vec1, rsVec vec2){
        v[0] = vec1[1] * vec2[2] - vec2[1] * vec1[2];
        v[1] = vec1[2] * vec2[0] - vec2[2] * vec1[0];
        v[2] = vec1[0] * vec2[1] - vec2[0] * vec1[1];
    };
    void scale(float scale){
        v[0] *= scale;
        v[1] *= scale;
        v[2] *= scale;
    };
    int almostEqual(rsVec vec, float tolerance){
        if(sqrt(double((v[0]-vec[0])*(v[0]-vec[0])
                       + (v[1]-vec[1])*(v[1]-vec[1])
                       + (v[2]-vec[2])*(v[2]-vec[2])))
           <= double(tolerance))
            return 1;
        else
            return 0;
    };

    float & operator [] (int i) {return v[i];}
    const float & operator [] (int i) const {return v[i];}
    rsVec & operator = (const rsVec &vec)
    {v[0]=vec[0];v[1]=vec[1];v[2]=vec[2];return *this;};
    rsVec operator + (const rsVec &vec)
    {return(rsVec(v[0]+vec[0], v[1]+vec[1], v[2]+vec[2]));};
    rsVec operator - (const rsVec &vec)
    {return(rsVec(v[0]-vec[0], v[1]-vec[1], v[2]-vec[2]));};
    rsVec operator * (const float &mul)
    {return(rsVec(v[0]*mul, v[1]*mul, v[2]*mul));};
    rsVec operator / (const float &div)
    {float rec = 1.0f/div; return(rsVec(v[0]*rec, v[1]*rec, v[2]*rec));};
    rsVec & operator += (const rsVec &vec)
    {v[0]+=vec[0];v[1]+=vec[1];v[2]+=vec[2];return *this;};
    rsVec & operator -= (const rsVec &vec)
    {v[0]-=vec[0];v[1]-=vec[1];v[2]-=vec[2];return *this;};
    rsVec & operator *= (const rsVec &vec)
    {v[0]*=vec[0];v[1]*=vec[1];v[2]*=vec[2];return *this;};
    rsVec & operator *= (const float &mul)
    {v[0]*=mul;v[1]*=mul;v[2]*=mul;return *this;};
};


class rsQuat{
public:
    float q[4];

    rsQuat(){
        q[0] = q[1] = q[2] = 0.0f;
        q[3] = 1.0f;
    };
    rsQuat(float x, float y, float z, float w){
        q[0] = x;
        q[1] = y;
        q[2] = z;
        q[3] = w;
    };
    void make(float a, float x, float y, float z){
        if(a < RSEPSILON && a > -RSEPSILON){
            q[0] = 0.0f;
            q[1] = 0.0f;
            q[2] = 0.0f;
            q[3] = 1.0f;
        }
        else{
            a *= 0.5f;
            float sintheta = sinf(a);
            q[0] = sintheta * x;
            q[1] = sintheta * y;
            q[2] = sintheta * z;
            q[3] = cosf(a);
        }
    };	// angle, axis
    void make(float a, const rsVec &v){
        if(a < RSEPSILON && a > -RSEPSILON){
            q[0] = 0.0f;
            q[1] = 0.0f;
            q[2] = 0.0f;
            q[3] = 1.0f;
        }
        else{
            a *= 0.5f;
            float sintheta = sinf(a);
            q[0] = sintheta * v[0];
            q[1] = sintheta * v[1];
            q[2] = sintheta * v[2];
            q[3] = cosf(a);
        }
    };
    void normalize(){
        float length = float(sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]));

        q[0] /= length;
        q[1] /= length;
        q[2] /= length;
        q[3] /= length;
    };
    void preMult(rsQuat &postQuat){
        // q1q2 = s1v2 + s2v1 + v1xv2, s1s2 - v1.v2
        float tempx = q[0];
        float tempy = q[1];
        float tempz = q[2];
        float tempw = q[3];

        q[0] = tempw * postQuat[0] + postQuat[3] * tempx
            + tempy * postQuat[2] - postQuat[1] * tempz;
        q[1] = tempw * postQuat[1] + postQuat[3] * tempy
            + tempz * postQuat[0] - postQuat[2] * tempx;
        q[2] = tempw * postQuat[2] + postQuat[3] * tempz
            + tempx * postQuat[1] - postQuat[0] * tempy;
        q[3] = tempw * postQuat[3]
            - tempx * postQuat[0]
            - tempy * postQuat[1]
            - tempz * postQuat[2];
    };			// Multiply this quaternion by
                                             // the passed quaternion
                                             // (this * passed)
    void toMat(float *mat){
        float s, xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

        // must have an axis
        if(q[0] == 0.0f && q[1] == 0.0f && q[2] == 0.0f){
            mat[0] = 1.0f;
            mat[1] = 0.0f;
            mat[2] = 0.0f;
            mat[3] = 0.0f;
            mat[4] = 0.0f;
            mat[5] = 1.0f;
            mat[6] = 0.0f;
            mat[7] = 0.0f;
            mat[8] = 0.0f;
            mat[9] = 0.0f;
            mat[10] = 1.0f;
            mat[11] = 0.0f;
            mat[12] = 0.0f;
            mat[13] = 0.0f;
            mat[14] = 0.0f;
            mat[15] = 1.0f;
            return;
        }

        s = 2.0f / (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
        xs = q[0] * s;
        ys = q[1] * s;
        zs = q[2] * s;
        wx = q[3] * xs;
        wy = q[3] * ys;
        wz = q[3] * zs;
        xx = q[0] * xs;
        xy = q[0] * ys;
        xz = q[0] * zs;
        yy = q[1] * ys;
        yz = q[1] * zs;
        zz = q[2] * zs;

        mat[0] = 1.0f - yy - zz;
        mat[1] = xy + wz;
        mat[2] = xz - wy;
        mat[3] = 0.0f;
        mat[4] = xy - wz;
        mat[5] = 1.0f - xx - zz;
        mat[6] = yz + wx;
        mat[7] = 0.0f;
        mat[8] = xz + wy;
        mat[9] = yz - wx;
        mat[10] = 1.0f - xx - yy;
        mat[11] = 0.0f;
        mat[12] = 0.0f;
        mat[13] = 0.0f;
        mat[14] = 0.0f;
        mat[15] = 1.0f;
    };					// Convert quaternion to array of 16 floats
    void fromMat(float *mat){
        float a, b;
        int i;

        a = mat[0] + mat[5] + mat[10];
        if(a > 0.0){
            b = float(sqrt(a + 1.0f));
            q[3] = b * 0.5f;
            b = 0.5f / b;

            q[0] = (mat[6] - mat[9]) * b;
            q[1] = (mat[8] - mat[2]) * b;
            q[2] = (mat[1] - mat[4]) * b;
        }
        else{
            i = 0;
            if(mat[5] > mat[0])
                i = 1;
            if(mat[10] > mat[5])
                i = 2;

            if(i==0){
                b = float(sqrt(mat[0] - mat[5] - mat[10] + 1.0f));
                q[0] *= 0.5f;
                b = 0.5f / b;
                q[3] = (mat[6] - mat[9]) * b;
                q[1] = (mat[1] - mat[4]) * b;
                q[2] = (mat[2] - mat[8]) * b;
            }
            if(i==1){
                b = float(sqrt(mat[5] - mat[10] - mat[0] + 1.0f));
                q[1] *= 0.5f;
                b = 0.5f / b;
                q[3] = (mat[8] - mat[2]) * b;
                q[2] = (mat[6] - mat[9]) * b;
                q[0] = (mat[4] - mat[1]) * b;
            }
            if(i==2){
                b = float(sqrt(mat[10] - mat[0] - mat[5] + 1.0f));
                q[2] *= 0.5f;
                b = 0.5f / b;
                q[3] = (mat[1] - mat[4]) * b;
                q[0] = (mat[8] - mat[2]) * b;
                q[1] = (mat[9] - mat[6]) * b;
            }
        }
    };					// Convert array of 16 floats to quaternion
    void fromEuler(float yaw, float pitch, float roll){
        float cy, cp, cr, sy, sp, sr, cpcy, spsy;

        cy = cosf(yaw * 0.5f);
        cp = cosf(pitch * 0.5f);
        cr = cosf(roll * 0.5f);

        sy = sinf(yaw * 0.5f);
        sp = sinf(pitch * 0.5f);
        sr = sinf(roll * 0.5f);

        cpcy = cp * cy;
        spsy = sp * sy;

        q[3] = cr * cpcy + sr * spsy;
        q[0] = sr * cpcy - cr * spsy;
        q[1] = cr * sp * cy + sr * cp * sy;
        q[2] = cr * cp * sy - sr * sp * cy;
    };	// Convert from hpr angles

    float & operator [] (int i) {return q[i];}
    const float & operator [] (int i) const {return q[i];}
};


class rsMatrix{
public:
    float m[16];
    // 1 0 0 x   0 4 8  12  <-- Column order matrix just like OpenGL
    // 0 1 0 y   1 5 9  13
    // 0 0 1 z   2 6 10 14
    // 0 0 0 1   3 7 11 15

    rsMatrix() {};
    void identity(){
        m[0] = m[5] = m[5] = m[15] = 1.0f;
        m[1] = m[2] = m[3] = m[4] = 0.0f;
        m[6] = m[7] = m[8] = m[9] = 0.0f;
        m[11] = m[12] = m[13] = m[14] = 0.0f;
    };
    void get(float* mat){
        for(int i=0; i<16; i++)
            mat[i] = m[i];
    };
    void preMult(const rsMatrix &postMat){
        float preMat[16];

        preMat[0] = m[0];
        preMat[1] = m[1];
        preMat[2] = m[2];
        preMat[3] = m[3];
        preMat[4] = m[4];
        preMat[5] = m[5];
        preMat[6] = m[6];
        preMat[7] = m[7];
        preMat[8] = m[8];
        preMat[9] = m[9];
        preMat[10] = m[10];
        preMat[11] = m[11];
        preMat[12] = m[12];
        preMat[13] = m[13];
        preMat[14] = m[14];
        preMat[15] = m[15];

        m[0] = preMat[0]*postMat[0] + preMat[4]*postMat[1] + preMat[8]*postMat[2] + preMat[12]*postMat[3];
        m[1] = preMat[1]*postMat[0] + preMat[5]*postMat[1] + preMat[9]*postMat[2] + preMat[13]*postMat[3];
        m[2] = preMat[2]*postMat[0] + preMat[6]*postMat[1] + preMat[10]*postMat[2] + preMat[14]*postMat[3];
        m[3] = preMat[3]*postMat[0] + preMat[7]*postMat[1] + preMat[11]*postMat[2] + preMat[15]*postMat[3];
        m[4] = preMat[0]*postMat[4] + preMat[4]*postMat[5] + preMat[8]*postMat[6] + preMat[12]*postMat[7];
        m[5] = preMat[1]*postMat[4] + preMat[5]*postMat[5] + preMat[9]*postMat[6] + preMat[13]*postMat[7];
        m[6] = preMat[2]*postMat[4] + preMat[6]*postMat[5] + preMat[10]*postMat[6] + preMat[14]*postMat[7];
        m[7] = preMat[3]*postMat[4] + preMat[7]*postMat[5] + preMat[11]*postMat[6] + preMat[15]*postMat[7];
        m[8] = preMat[0]*postMat[8] + preMat[4]*postMat[9] + preMat[8]*postMat[10] + preMat[12]*postMat[11];
        m[9] = preMat[1]*postMat[8] + preMat[5]*postMat[9] + preMat[9]*postMat[10] + preMat[13]*postMat[11];
        m[10] = preMat[2]*postMat[8] + preMat[6]*postMat[9] + preMat[10]*postMat[10] + preMat[14]*postMat[11];
        m[11] = preMat[3]*postMat[8] + preMat[7]*postMat[9] + preMat[11]*postMat[10] + preMat[15]*postMat[11];
        m[12] = preMat[0]*postMat[12] + preMat[4]*postMat[13] + preMat[8]*postMat[14] + preMat[12]*postMat[15];
        m[13] = preMat[1]*postMat[12] + preMat[5]*postMat[13] + preMat[9]*postMat[14] + preMat[13]*postMat[15];
        m[14] = preMat[2]*postMat[12] + preMat[6]*postMat[13] + preMat[10]*postMat[14] + preMat[14]*postMat[15];
        m[15] = preMat[3]*postMat[12] + preMat[7]*postMat[13] + preMat[11]*postMat[14] + preMat[15]*postMat[15];
    };
    void fromQuat(const rsQuat &q){
        float s, xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

        // must have an axis
        if(q[0] == 0.0f && q[1] == 0.0f && q[2] == 0.0f){
            identity();
            return;
        }

        s = 2.0f / (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
        xs = q[0] * s;
        ys = q[1] * s;
        zs = q[2] * s;
        wx = q[3] * xs;
        wy = q[3] * ys;
        wz = q[3] * zs;
        xx = q[0] * xs;
        xy = q[0] * ys;
        xz = q[0] * zs;
        yy = q[1] * ys;
        yz = q[1] * zs;
        zz = q[2] * zs;

        m[0] = 1.0f - yy - zz;
        m[1] = xy + wz;
        m[2] = xz - wy;
        m[3] = 0.0f;
        m[4] = xy - wz;
        m[5] = 1.0f - xx - zz;
        m[6] = yz + wx;
        m[7] = 0.0f;
        m[8] = xz + wy;
        m[9] = yz - wx;
        m[10] = 1.0f - xx - yy;
        m[11] = 0.0f;
        m[12] = 0.0f;
        m[13] = 0.0f;
        m[14] = 0.0f;
        m[15] = 1.0f;
    };

    const float & operator [] (int i) const {return m[i];}
    rsMatrix & operator = (const rsMatrix &mat){
        m[0]=mat[0]; m[1]=mat[1]; m[2]=mat[2]; m[3]=mat[3];
        m[4]=mat[4]; m[5]=mat[5]; m[6]=mat[6]; m[7]=mat[7];
        m[8]=mat[8]; m[9]=mat[9]; m[10]=mat[10]; m[11]=mat[11];
        m[12]=mat[12]; m[13]=mat[13]; m[14]=mat[14]; m[15]=mat[15];
        return *this;
    };
};






#endif  // RSMATH_H