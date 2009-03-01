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


#ifndef IMPCUBETABLE_H
#define IMPCUBETABLE_H


/****************************************

OpenGL coordinate system

Y
|
|
|
|
|
/--------X
/
/
Z


Indices for vertices and edge on cube


2-----6-----6
/|          /|
3 |        11 |
/  1        /  9
3-----7-----7   |
|   |       |   |
|   0-----4-|---4
2  /       10  /
| 0         | 8
|/          |/
1-----5-----5


*****************************************/

// corners
// left-right-bottom-top-far-near notation
#define LBF 0x01
#define LBN 0x02
#define LTF 0x04
#define LTN 0x08
#define RBF 0x10
#define RBN 0x20
#define RTF 0x40
#define RTN 0x80



class impCubeTable{
public:
    // This table describes the sequence of edges to visit
    // in order to build triangle strips in a cube  There are
    // a maximum of 16 integers that can result from a cube
    // configuration.  We have 17 integers in each row so that
    // each row can end with a 0.  The zero signifies the end
    // of the data in each row.
    int** cubetable;
    // 256 x 6 array of true/false values.  For each of the 256
    // entries in the cubetable, this table indicates which
    // neighboring cubes will also contain parts of the surface.
    bool** crawltable;

    impCubeTable(){
        cubetable = new int*[256];
        int i;
        for(i=0; i<256; i++)
            cubetable[i] = new int[17];

        ec[0][0] = 0;   ec[0][1] = 1;
        ec[1][0] = 0;   ec[1][1] = 2;
        ec[2][0] = 1;   ec[2][1] = 3;
        ec[3][0] = 2;   ec[3][1] = 3;
        ec[4][0] = 0;   ec[4][1] = 4;
        ec[5][0] = 1;   ec[5][1] = 5;
        ec[6][0] = 2;   ec[6][1] = 6;
        ec[7][0] = 3;   ec[7][1] = 7;
        ec[8][0] = 4;   ec[8][1] = 5;
        ec[9][0] = 4;   ec[9][1] = 6;
        ec[10][0] = 5;  ec[10][1] = 7;
        ec[11][0] = 6;  ec[11][1] = 7;

        vc[0][0] = 0;  vc[0][1] = 1;  vc[0][2] = 4;
        vc[1][0] = 0;  vc[1][1] = 5;  vc[1][2] = 2;
        vc[2][0] = 1;  vc[2][1] = 3;  vc[2][2] = 6;
        vc[3][0] = 2;  vc[3][1] = 7;  vc[3][2] = 3;
        vc[4][0] = 4;  vc[4][1] = 9;  vc[4][2] = 8;
        vc[5][0] = 5;  vc[5][1] = 8;  vc[5][2] = 10;
        vc[6][0] = 6;  vc[6][1] = 11;  vc[6][2] = 9;
        vc[7][0] = 7;  vc[7][1] = 10;  vc[7][2] = 11;

        makecubetable();

        crawltable = new bool*[256];
        for(i=0; i<256; i++)
            crawltable[i] = new bool[6];

        makecrawltable();
    };

    ~impCubeTable() {
        int i;
        for(i=0; i<256; i++)
            delete[] cubetable[i];
        delete[] cubetable;
        
        for(i=0; i<256; i++)
            delete[] crawltable[i];
        delete[] crawltable;
    };
    
private:
        // edge connectivity
        // This array defines which vertices are connected by each edge.
        int ec[12][2];

    // vertex connectivity
    // This array defines which edges extend from each vertex.
    int vc[8][3];

    int nextedge(int vertex, int edge){
        if(vc[vertex][0] == edge)
            return(vc[vertex][1]);
        if(vc[vertex][1] == edge)
            return(vc[vertex][2]);
        if(vc[vertex][2] == edge)
            return(vc[vertex][0]);

        return(-1);
    };
    void addtotable(int row, int edgecount, int *edgelist){
        static int lastrow = -1;
        static int totalcount = 0;

        if(row != lastrow)
            totalcount = 0;

        // enter the number of vertices into the cubetable
        cubetable[row][totalcount] = edgecount;

        // The edges are listed in counterclockwise order in the edgelist.
        // Notice how they are added to the cubetable out of order, in
        // a zig-zag pattern the way a triangle strip is drawn.
        // There can be at most 7 vertices in one of these triangle strips.
        switch(edgecount){
            case 3:
                cubetable[row][totalcount+1] = edgelist[0];
                cubetable[row][totalcount+2] = edgelist[1];
                cubetable[row][totalcount+3] = edgelist[2];
                break;
            case 4:
                cubetable[row][totalcount+1] = edgelist[0];
                cubetable[row][totalcount+2] = edgelist[1];
                cubetable[row][totalcount+3] = edgelist[3];
                cubetable[row][totalcount+4] = edgelist[2];
                break;
            case 5:
                cubetable[row][totalcount+1] = edgelist[0];
                cubetable[row][totalcount+2] = edgelist[1];
                cubetable[row][totalcount+3] = edgelist[4];
                cubetable[row][totalcount+4] = edgelist[2];
                cubetable[row][totalcount+5] = edgelist[3];
                break;
            case 6:
                cubetable[row][totalcount+1] = edgelist[0];
                cubetable[row][totalcount+2] = edgelist[1];
                cubetable[row][totalcount+3] = edgelist[5];
                cubetable[row][totalcount+4] = edgelist[2];
                cubetable[row][totalcount+5] = edgelist[4];
                cubetable[row][totalcount+6] = edgelist[3];
                break;
            case 7:
                cubetable[row][totalcount+1] = edgelist[0];
                cubetable[row][totalcount+2] = edgelist[1];
                cubetable[row][totalcount+3] = edgelist[6];
                cubetable[row][totalcount+4] = edgelist[2];
                cubetable[row][totalcount+5] = edgelist[5];
                cubetable[row][totalcount+6] = edgelist[3];
                cubetable[row][totalcount+7] = edgelist[4];
                break;
        }

        totalcount += (edgecount + 1);
        lastrow = row;
    };
    void makecubetable(){
        int i, j, k;
        int currentvertex;
        int currentedge;
        bool vertices[8];  // true if on low side of gradient (outside of surface)
        bool edges[12];
        bool edgesdone[12];
        int edgelist[7];  // final list of egdes used in a triangle strip
        int edgecount;

        // Set cubetable values to zero
        // A zero will indicate that there are no more triangle strips to build
        for(i=0; i<256; i++){
            for(j=0; j<17; j++){
                cubetable[i][j] = 0;
            }
        }

        // For each vertex combination
        for(i=0; i<256; i++){
            // identify the vertices on the low side of the gradient
            int andbit;
            for(j=0; j<8; j++){
                andbit = 1;
                for(k=0; k<j; k++)
                    andbit *= 2;
                if(i & andbit)
                    vertices[j] = 1;
                else
                    vertices[j] = 0;
            }

            // Identify the edges that cross threshold value
            // These are edges that connect 1 turned-on and 1 turned-off vertex
            for(j=0; j<12; j++){
                if((vertices[ec[j][0]] + vertices[ec[j][1]]) == 1)
                    edges[j] = 1;
                else
                    edges[j] = 0;
                edgesdone[j] = 0;  // no edges have been used yet
            }

            // Construct lists of edges that form triangle strips
            // try starting from each edge (no need to try last 2 edges)
            for(j=0; j<10; j++){
                currentedge = j;
                edgecount = 0;
                // if this edge contains a surface vertex and hasn't been used
                while(edges[currentedge] && !edgesdone[currentedge]){
                    // add edge to list
                    edgelist[edgecount] = currentedge;
                    edgecount ++;
                    edgesdone[currentedge] = 1;
                    // find that edge's vertex on low side of gradient
                    if(vertices[ec[currentedge][0]])
                        currentvertex = ec[currentedge][0];
                    else
                        currentvertex = ec[currentedge][1];
                    // move along gradiant boundary to find next edge
                    currentedge = nextedge(currentvertex, currentedge);
                    while(!edges[currentedge]){
                        if(currentvertex != ec[currentedge][0])
                            currentvertex = ec[currentedge][0];
                        else
                            currentvertex = ec[currentedge][1];
                        currentedge = nextedge(currentvertex, currentedge);
                    }
                }
                // if a surface has been created add it to the table
                // and start over to try to make another surface
                if(edgecount)
                    addtotable(i, edgecount, edgelist);
            }
        }
    };
    void makecrawltable(){
        int i, j, k;
        bool vertices[8];  // vertices below gradient threshold get turned on
        bool edges[12];  // edges that cross gradient threshold get turned on

        // For each vertex combination
        for(i=0; i<256; i++){
            // identify the vertices on the low side of the gradient
            int andbit;
            for(j=0; j<8; j++){
                andbit = 1;
                for(k=0; k<j; k++)
                    andbit *= 2;
                if(i & andbit)
                    vertices[j] = 1;
                else
                    vertices[j] = 0;
            }

            // Identify the edges that cross threshold value
            // These are edges that connect 1 turned-on and 1 turned-off vertex
            for(j=0; j<12; j++){
                if((vertices[ec[j][0]] + vertices[ec[j][1]]) == 1)
                    edges[j] = 1;
                else
                    edges[j] = 0;
            }

            // -x
            if(edges[0] || edges[1] || edges[2] || edges[3])
                crawltable[i][0] = true;
            else
                crawltable[i][0] = false;
            // +x
            if(edges[8] || edges[9] || edges[10] || edges[11])
                crawltable[i][1] = true;
            else
                crawltable[i][1] = false;
            // -y
            if(edges[0] || edges[4] || edges[5] || edges[8])
                crawltable[i][2] = true;
            else
                crawltable[i][2] = false;
            // +y
            if(edges[3] || edges[6] || edges[7] || edges[11])
                crawltable[i][3] = true;
            else
                crawltable[i][3] = false;
            // -z
            if(edges[1] || edges[4] || edges[6] || edges[9])
                crawltable[i][4] = true;
            else
                crawltable[i][4] = false;
            // +z
            if(edges[2] || edges[5] || edges[7] || edges[10])
                crawltable[i][5] = true;
            else
                crawltable[i][5] = false;
        }
    };
};


#endif
