#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>	/* va_arg - System dependent ?! */
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <hdf5.h>

#include "h5_core.h"
#include "h5_core_private.h"

const char * H5_OID_NAMES[] = {
	"N.N.",
	"vertex",
	"edge",
	"triangle",
	"tetrahedron"
};

h5_err_t
_h5t_handle_get_global_entity_id_err (
	h5_file *f,
	const h5_id_t * const global_vids
	) {
	struct h5t_fdata *t = &f->t;
	switch ( t->mesh_type ) {
	case H5_OID_TETRAHEDRON:
		return _h5t_error_global_tet_id_nexist ( global_vids );
	case H5_OID_TRIANGLE:
		return _h5t_error_global_tri_id_nexist ( global_vids );
	default:
		return h5_error_internal( __FILE__, __func__, __LINE__ );
	}
}
	
h5_err_t
_h5t_handle_get_local_entity_id_err (
	h5_file *f,
	const h5_id_t * const local_vids
	) {
	struct h5t_fdata *t = &f->t;
	switch ( t->mesh_type ) {
	case H5_OID_TETRAHEDRON:
		return _h5t_error_local_tet_id_nexist ( local_vids );
	case H5_OID_TRIANGLE:
		return _h5t_error_local_triangle_id_nexist ( local_vids );
	default:
		return h5_error_internal( __FILE__, __func__, __LINE__ );
	}
}

h5_err_t
_h5t_error_illegal_object_type (
	h5_file_t * const f,
	h5_oid_t oid ) {
	struct h5t_fdata *t = &f->t;
	switch ( t->mesh_type ) {
	case H5_OID_TETRAHEDRON:
		return h5_error_internal( __FILE__, __func__, __LINE__ );
	case H5_OID_TRIANGLE:
		switch ( oid ) {
		case H5_OID_TETRAHEDRON:
			return h5_error (
				H5_ERR_INVAL,
				"Illegal topological entity tetrahedron"
				" in triangle mesh." );
		default:
			return h5_error_internal( __FILE__, __func__, __LINE__ );
		}
	default:
		return h5_error_internal( __FILE__, __func__, __LINE__ );
	}
}
