#include <stdio.h>
#include <stdlib.h>

#include "H5hut.h"

const char* FNAME = "simple_triangle.h5";

typedef struct vertex {
	h5_float64_t P[3];
} vertex_t; 

typedef struct elem {
	h5_loc_idx_t vids[3];
} elem_t;
	       
vertex_t Vertices[] = {
	{ {-1.0,  0.0,  0.0} },
	{ { 1.0,  0.0,  0.0} },
	{ { 0.0,  1.0,  0.0} },
	{ { 0.0, -1.0,  0.0} }
};

elem_t Elems[] = {
	{ { 0, 1, 2 } },
	{ { 0, 1, 3 } }
};

const int num_vertices = sizeof (Vertices) / sizeof (Vertices[0]);
const int num_elems = sizeof (Elems) / sizeof (Elems[0]);

int
main (
	int argc,
	char* argv[]
	) {
	/* abort program on errors in library */
	H5SetErrorHandler (H5AbortErrorhandler);
	H5SetVerbosityLevel (5);

	/* open file and add mesh */
	h5_file_t* const f = H5OpenFile (FNAME, H5_O_WRONLY, 0);
	H5FedAddTriangleMesh (f);

	/* store vertices */
	H5FedBeginStoreVertices (f, num_vertices);
	int i;
	for (i = 0; i < num_vertices; i++) {
		H5FedStoreVertex (f, -1, Vertices[i].P);
	}
	H5FedEndStoreVertices (f);

	/* store elements */
	H5FedBeginStoreElements (f, num_elems);
	for (i = 0; i < num_elems; i++) {
		H5FedStoreElement (f, Elems[i].vids);
	}
	H5FedEndStoreElements (f);

	/* add 1. Level */
	H5FedAddLevel(f);
	H5FedBeginRefineElements (f);
	H5FedRefineElement (f, 0);
	H5FedEndRefineElements (f);

	H5FedCloseMesh (f);
	H5CloseFile (f);
	return 0;
}
