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


#ifndef IMPSPHERE_H
#define IMPSPHERE_H



#include "impElement.h"



class impSphere : public impElement{
private:
    // This class stores scale information outside of impElement's matrix
    // so that there is no need for a complete matrix multiply in the value() function
    float scale;
    float invscale;
    float scale_quad;

public:
    impSphere(){scale = 1.0f;};
    // ~impSphere(){};

    void setScale(float s){
        scale = s;
        scale_quad = scale*scale;
        invscale = 1.0f / scale;
    };
    float value(float* position){
        // float x = (invmat[12] + position[0]) * invscale;
        // float y = (invmat[13] + position[1]) * invscale;
        // float z = (invmat[14] + position[2]) * invscale;
        // return(1.0f / (x*x + y*y + z*z));
        
        float x = (invmat[12] + position[0]);
        float y = (invmat[13] + position[1]);
        float z = (invmat[14] + position[2]);
        return(scale_quad / (x*x + y*y + z*z));
    };
};



#endif
