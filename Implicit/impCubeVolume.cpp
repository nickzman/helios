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


#include "impCubeVolume.h"


void impCubeVolume::make_surface(){
    int i, j, k;

    if(surface == NULL)
        return;

    // find gradient value at every corner
    for(i=0; i<=whl[0]; i++){
        for(j=0; j<=whl[1]; j++){
            for(k=0; k<=whl[2]; k++)
                function(corners[i][j][k].position, &(corners[i][j][k].value));
        }
    }

    // polygonize surface
    for(i=0; i<whl[0]; i++){
        for(j=0; j<whl[1]; j++){
            for(k=0; k<whl[2]; k++){
                cubes[i][j][k].index = findcubetableindex(i, j, k);
                polygonize(i, j, k);
            }
        }
    }

    // zero-out edges' done labels
    for(i=0; i<whl[0]; i++){
        for(j=0; j<=whl[1]; j++){
            for(k=0; k<=whl[2]; k++){
                edges[0][i][j][k].done = 0;
            }
        }
    }
    for(i=0; i<=whl[0]; i++){
        for(j=0; j<whl[1]; j++){
            for(k=0; k<=whl[2]; k++){
                edges[1][i][j][k].done = 0;
            }
        }
    }
    for(i=0; i<=whl[0]; i++){
        for(j=0; j<=whl[1]; j++){
            for(k=0; k<whl[2]; k++){
                edges[2][i][j][k].done = 0;
            }
        }
    }
}


void impCubeVolume::make_surface(float eyex, float eyey, float eyez){
    int i, j, k;
    int index;
    cubelistelem* currentcube;
    float xdist, ydist, zdist;

    if(surface == NULL)
        return;

    // find gradient value at every corner
    for(i=0; i<=whl[0]; i++){
        for(j=0; j<=whl[1]; j++){
            for(k=0; k<=whl[2]; k++)
                function(corners[i][j][k].position, &(corners[i][j][k].value));
                // corners[i][j][k].value = function(corners[i][j][k].position);
        }
    }

    // erase list from last frame
    cubelist.clear();

    // polygonize surface
    for(i=0; i<whl[0]; i++){
        for(j=0; j<whl[1]; j++){
            for(k=0; k<whl[2]; k++){
                index = findcubetableindex(i, j, k);
                if(index != 0 && index != 255){
                    cubelist.push_back(cubelistelem(index));
                    currentcube = &(cubelist.back());
                    currentcube->position[0] = i;
                    currentcube->position[1] = j;
                    currentcube->position[2] = k;
                    xdist = cubes[i][j][k].x - eyex;
                    ydist = cubes[i][j][k].y - eyey;
                    zdist = cubes[i][j][k].z - eyez;
                    currentcube->depth = xdist * xdist + ydist * ydist + zdist * zdist;
                }
            }
        }
    }

    // sort the cubes
    cubelist.sort();

    // polygonize surface
    polygonize();

    // zero-out edges' done labels
    for(i=0; i<whl[0]; i++){
        for(j=0; j<=whl[1]; j++){
            for(k=0; k<=whl[2]; k++){
                edges[0][i][j][k].done = 0;
            }
        }
    }
    for(i=0; i<=whl[0]; i++){
        for(j=0; j<whl[1]; j++){
            for(k=0; k<=whl[2]; k++){
                edges[1][i][j][k].done = 0;
            }
        }
    }
    for(i=0; i<=whl[0]; i++){
        for(j=0; j<=whl[1]; j++){
            for(k=0; k<whl[2]; k++){
                edges[2][i][j][k].done = 0;
            }
        }
    }
}


inline void impCubeVolume::crawl_sort(int x, int y, int z){
    findcornervalues(x, y, z);
    int index = findcubetableindex(x, y, z);

    // quit if this cube has been done or does not intersect surface
    if(cubes[x][y][z].done || index == 0 || index == 255)
        return;

    // save index for uncrawling
    cubes[x][y][z].index = index;

    // mark this cube as completed
    cubes[x][y][z].done = 1;

    // add cube to list
    cubelist.push_back(cubelistelem(index));
    cubelistelem* currentcube = &(cubelist.back());
    currentcube->position[0] = x;
    currentcube->position[1] = y;
    currentcube->position[2] = z;

    // polygonize adjacent cubes
    if(crawltable[index][0] && x > 0)
        crawl_sort(x-1, y, z);
    if(crawltable[index][1] && x < whl[0]-1)
        crawl_sort(x+1, y, z);
    if(crawltable[index][2] && y > 0)
        crawl_sort(x, y-1, z);
    if(crawltable[index][3] && y < whl[1]-1)
        crawl_sort(x, y+1, z);
    if(crawltable[index][4] && z > 0)
        crawl_sort(x, y, z-1);
    if(crawltable[index][5] && z < whl[2]-1)
        crawl_sort(x, y, z+1);
}

