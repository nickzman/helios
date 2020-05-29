/*
 * Copyright (C) 2005-2010  Terence M. Welsh
 *
 * This file is part of rsText.
 *
 * rsText is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * rsText is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef RSTEXT_H
#define RSTEXT_H



#ifdef WIN32
	#include <windows.h>
#endif
#include <vector>
#include <string>
#include <sstream>
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>



class rsText{
public:
	GLuint texture;  // texture object
	GLuint listbase;  // first display list

	rsText();
	~rsText(){}
	void draw(std::string &str);
	void draw(std::vector<std::string> &strvec);
};



template<class T> inline std::string to_string(const T & Value){
    std::stringstream ss;
    ss << Value;
    return ss.str();
}



#endif
