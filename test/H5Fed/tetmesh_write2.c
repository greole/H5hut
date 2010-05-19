#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "H5Part.h"
#include "H5Fed.h"

const h5_oid_t MESH_TYPE = H5_TETRAHEDRAL_MESH;
const char* FNAME = "simple_tet.h5";
const h5_int32_t num_levels = 11;


typedef struct vertex {
	h5_float64_t P[3];
} vertex_t; 

typedef struct elem {
	h5_id_t vids[4];
} elem_t;
	       
vertex_t Vertices[] = {
	{{-1.0,  0.0,  0.0}},
	{{ 1.0,  0.0,  0.0}},
	{{ 0.0,  1.0,  0.0}},
	{{ 0.0,  0.0,  1.0}}
};

elem_t Elems[] = {
	{{ 0, 1, 2, 3 }}
};

const int num_vertices = sizeof (Vertices) / sizeof (Vertices[0]);
const int num_elems = sizeof (Elems) / sizeof (Elems[0]);

static h5_int32_t
power (
	const h5_int32_t x,
	const h5_int32_t y
	) {
	h5_int32_t p = 1;
	h5_int32_t b = y;
	// bits in b correspond to values of powerN
	// so start with p=1, and for each set bit in b,
	// multiply corresponding table entry
	h5_int32_t powerN = x;

        while (b != 0) {
		if ((b&1) != 0) p *= powerN;
		b >>= 1;
		powerN = powerN * powerN;
	}
	return p;
}

int
main (
	int argc,
	char* argv[]
	) {
	/* abort program on errors in library */
	H5SetErrorHandler (H5AbortErrorhandler);
	H5SetVerbosityLevel (2);

	/* open file and add mesh */
	h5_file_t* const f = H5OpenFile (FNAME, H5_O_WRONLY, 0);
	H5FedAddMesh (f, MESH_TYPE);

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
	H5FedBeginRefineElements (f, 1);
	H5FedRefineElement (f, 0);
	H5FedEndRefineElements (f);

	/* add levels second to num_levels-1 */
	h5_int32_t num_elems_last_level = 1;
	h5_int32_t level_id;
	for (level_id = 2; level_id < num_levels; level_id++) {

		/* refine 4 to the power of level_id-1 elems */
		h5_id_t level_id = H5FedAddLevel(f);
		h5_int32_t num_elems2refine = power (4, level_id-1);
		H5FedBeginRefineElements (f, num_elems2refine);
		for (i = num_elems_last_level;
		     i < num_elems_last_level+num_elems2refine;
		     i++) {
			H5FedRefineElement (f, i);
		}
		H5FedEndRefineElements (f);
		num_elems_last_level += 2 * num_elems2refine;
	}
	H5FedCloseMesh (f);
	H5CloseFile (f);
	return 0;
}
