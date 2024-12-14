#ifndef _DJS_MISC_H_
#define _DJS_MISC_H_

/*
    This file contains the algorithm for listing all cliques
    according to the algorithm of Jain et al. specified in
    "The power of pivoting for exact clique counting." (WSDM 2020).

    This code is a modified version of the code of quick-cliques-1.0 library for counting
    maximal cliques by Darren Strash (first name DOT last name AT gmail DOT com).

    Original author: Darren Strash (first name DOT last name AT gmail DOT com)

    Copyright (c) 2011 Darren Strash. This code is released under the GNU Public License (GPL) 3.0.

    Modifications Copyright (c) 2020 Shweta Jain

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include "LinkedList.h"
#include "degeneracy_helper.h"

// Some memory allocator moved here
#ifdef ALLOW_ALLOC_ZERO_BYTES
#define Malloc(x) malloc(x)
#define Calloc(x, y) calloc(x, y)
#define Free(x) free(x)
#else
#define Malloc(x) malloc(max(x, 1))
#define Calloc(x, y) calloc(max(x, 1), max(y, 1))
#define Free(x) free(x)
#endif // ALLOW_ALLOC_ZERO_BYTES

#define max(x, y) (x > y ? x : y)
#define min(x, y) (x < y ? x : y)
#define MAX_CSIZE 400


void moveFromRToXDegeneracyCliques(int vertex, int *vertexSets, int *vertexLookup, int *pBeginX, int *pBeginP,
                                   int *pBeginR);

void moveToRDegeneracyCliques(int vertex, int *vertexSets, int *vertexLookup, int **neighborsInP, int *numNeighbors,
                              int *pBeginX, int *pBeginP, int *pBeginR, int *pNewBeginX, int *pNewBeginP,
                              int *pNewBeginR);

void fillInPandXForRecursiveCallDegeneracyCliques(int vertex, int orderNumber, int *vertexSets, int *vertexLookup,
                                                  NeighborListArray **orderingArray, int **neighborsInP,
                                                  int *numNeighbors, int *pBeginX, int *pBeginP, int *pBeginR,
                                                  int *pNewBeginX, int *pNewBeginP, int *pNewBeginR);

int findBestPivotNonNeighborsDegeneracyCliques(int **pivotNonNeighbors, int *numNonNeighbors, int *vertexSets,
                                               int *vertexLookup, int **neighborsInP, int *numNeighbors, int beginX,
                                               int beginP, int beginR);

#endif
