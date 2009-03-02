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


#ifndef RSRAND_H
#define RSRAND_H



#include <stdlib.h>
#include <limits.h>



// Useful random number macros
// Don't forget to initialize with srand()
inline int myRandi(int x) {
	return(int(random()) % x);
}

inline float myRandf(float x) {
	return(float(random()) / INT_MAX * x);
}



#endif