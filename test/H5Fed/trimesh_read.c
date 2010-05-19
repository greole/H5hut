#include <stdio.h>
#include <stdlib.h>

#include "H5Part.h"
#include "H5Fed.h"

const h5_oid_t MESH_TYPE = H5_TRIANGLE_MESH;
const char* FNAME = "simple_triangle.h5";

static h5_err_t
traverse_vertices (
	h5_file_t* const f
	) {
	/* get number of vertices we have to expect */
	h5_size_t num_vertices_expect = H5FedGetNumVerticesTotal (f);

	/* get iterator for co-dim 2 entities, i.e. vertices */
	h5t_entity_iterator_t* iter = H5FedBeginTraverseEntities (f, 2);

	/* iterate */
	h5_id_t local_id;
	h5_size_t num_vertices = 0;
	while ((local_id = H5FedTraverseEntities ( f, iter )) >= 0) {
		h5_float64_t P[3];
		H5FedGetVertexCoordByID (f, local_id, P);
		char v[256];
		snprintf (v, sizeof(v), "=%llx=", local_id);
		printf ("| %-18s | (%f, %f, %f) |\n",
			 v, P[0], P[1], P[2]);
		num_vertices++;
	}

	/* done */
	H5FedEndTraverseEntities (f, iter);

	/* report error if we got a different number then expected */
	if (num_vertices != num_vertices_expect) {
		fprintf (stderr, "!!! Got %lld vertices, but expected %lld.\n",
			  num_vertices, num_vertices_expect);
	}

	printf ("    Number of vertices on level: %lld\n", num_vertices);
	return H5_SUCCESS;
}

static h5_err_t
traverse_edges (
	h5_file_t* const f
	) {
	printf ( "Travering edges on level %lld:\n", H5FedGetLevel(f) );

	/* get iterator for co-dim 1 entities, i.e. edges */
	h5t_entity_iterator_t* iter = H5FedBeginTraverseEntities (f, 1);

	/* iterate */
	h5_id_t local_id;
	h5_size_t num_edges = 0;
	while ((local_id = H5FedTraverseEntities (f, iter)) >= 0) {
		char v[256];
		char k[256];
		h5_id_t local_vids[4];
		snprintf ( k, sizeof(k), "=%llx=", local_id );
		H5FedGetVertexIndicesOfEntity ( f, local_id, local_vids );
		snprintf ( v, sizeof(v), "=[%lld,%lld]=",
				   local_vids[0], local_vids[1] );
		printf ( "| %-18s | %-18s |\n", k, v );
		num_edges++;
	}

	/* done */
	H5FedEndTraverseEntities ( f, iter );

	printf ("    Number of edges: %lld\n", num_edges);
	return H5_SUCCESS;
}

static h5_err_t
traverse_elems (
	h5_file_t* const f
	) {
	/* get number of elements we have to expect */
	h5_size_t num_elems_expect = H5FedGetNumElementsTotal (f);

	/* get iterator for co-dim 0 */
	h5t_entity_iterator_t* iter = H5FedBeginTraverseEntities (f, 0);

	/* iterate over all co-dim 0 entities, i.e. elements */
	h5_id_t local_id;
	h5_size_t num_elems = 0;
	while ((local_id = H5FedTraverseEntities (f, iter)) >= 0) {
		char v[256];
		char t[256];
		h5_id_t local_vids[4];
		snprintf (t, sizeof(t), "=%llx=", local_id);
		H5FedGetVertexIndicesOfEntity (f, local_id, local_vids);
		snprintf (v, sizeof(v), "=[%lld,%lld,%lld]=",
			   local_vids[0], local_vids[1], local_vids[2]);
		printf ("| %-18s | %-18s |\n", t, v);
		num_elems++;
	}

	/* done */
	H5FedEndTraverseEntities (f, iter);

	/* report error if we got a different number then expected */
	if (num_elems != num_elems_expect) {
		fprintf (stderr, "!!! Got %lld elements, but expected %lld.\n",
			  num_elems, num_elems_expect);
		exit(1);
	}

	printf ("    Number of elements on level: %lld\n", num_elems);
	return H5_SUCCESS;
}

static h5_err_t
traverse_level (
	h5_file_t* const f,
	const h5_id_t level_id
	) {
	printf ("    Setting level to %lld\n", level_id);
	H5FedSetLevel (f, level_id);
	traverse_vertices (f);
	traverse_edges (f);
	traverse_elems (f);
	return H5_SUCCESS;
}

static h5_err_t
traverse_mesh (
	h5_file_t* const f,
	const h5_id_t mesh_id,
	const h5_oid_t mesh_type
	) {
	/* open mesh and get number of levels */
	printf ("    Opening mesh with id %lld\n", mesh_id);
	H5FedOpenMesh (f, mesh_id, mesh_type);
	h5_size_t num_levels = H5FedGetNumLevels (f);
	printf ("    Number of levels in mesh: %lld\n", num_levels);

	/* loop over all levels */
	h5_id_t level_id;
	for (level_id = 0; level_id < num_levels; level_id++) {
		traverse_level (f, level_id);
	}
	/* done */
	H5FedCloseMesh (f);
	return H5_SUCCESS;
}

int
main (
	int argc,
	char* argv[]
	) {

	/* abort program on error, so we don't have to handle them */
	H5SetErrorHandler (H5AbortErrorhandler);
	H5SetVerbosityLevel (4);

	/* open file and get number of meshes */
	h5_file_t* f = H5OpenFile (FNAME, H5_O_RDONLY, 0);
	h5_size_t num_meshes = H5FedGetNumMeshes (f, MESH_TYPE);
	printf ("    Number of meshes: %lld\n", num_meshes);

	/* loop over all meshes */
	h5_id_t mesh_id;
	for (mesh_id = 0; mesh_id < num_meshes; mesh_id++) {
		traverse_mesh (f, mesh_id, MESH_TYPE);
	}

	/* done */
	H5CloseFile (f);
	return 0;
}
