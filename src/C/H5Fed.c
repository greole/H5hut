/*
  Copyright 2007-2009
 	Paul Scherrer Institut, Villigen, Switzerland;
 	Benedikt Oswald;
 	Achim Gsell
 	All rights reserved.
 
  Authors
 	Achim Gsell
  
  Warning
	This code is under development.
 
 */

#include <stdarg.h>
#include <hdf5.h>
#include "h5core/h5_core.h"
#include "H5Fed.h"

h5_err_t
H5FedOpenMesh (
	h5_file_t * const f,
	const h5_id_t mesh_id,
	const h5_oid_t mesh_type_id
	) {
	SET_FNAME ( f, __func__ );
	return h5t_open_mesh ( f, mesh_id, mesh_type_id );
}

h5_err_t
H5FedCloseMesh (
	h5_file_t * const f
	) {
	return h5_error_not_implemented ( f, __FILE__, __func__, __LINE__ );
}

h5_err_t
H5FedSetLevel (
	h5_file_t * const f,
	const h5_id_t level_id
	) {
	SET_FNAME ( f, __func__ );
	return h5t_open_level ( f, level_id );
}

h5_err_t
H5FedLinkMeshToStep (
	h5_file_t * f,
	const h5_id_t mesh_id
	) {
	return h5_error_not_implemented ( f, __FILE__, __func__, __LINE__ );
}