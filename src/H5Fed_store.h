/*
  Header file for declaring the H5Fed application programming
  interface (API) in the C language.
  
  Copyright 2006-2009
 	Paul Scherrer Institut, Villigen, Switzerland;
 	Benedikt Oswald;
 	Achim Gsell
 	All rights reserved.
 
  Authors
 	Achim Gsell
  
  Warning
	This code is under development.
 
 */

#ifndef __H5FED_STORE_H
#define __H5FED_STORE_H

h5_id_t
H5FedAddMesh (
	h5_file_t * const f,
	const h5_oid_t mesh_type_id
	);

h5_id_t
H5FedAddLevel (
	h5_file_t * const f
	);

h5_err_t
H5FedBeginStoreVertices (
	h5_file_t * const f,
	const h5_size_t num
	);

h5_id_t
H5FedStoreVertex (
	h5_file_t * const f,
	const h5_id_t id,
	const h5_float64_t P[3]
	);

h5_err_t
H5FedEndStoreVertices (
	h5_file_t * const f
	);

h5_err_t
H5FedBeginStoreElements (
	h5_file_t * const f,
	const h5_size_t num
	);


h5_id_t
H5FedStoreElement (
	h5_file_t * const f,
	const h5_id_t local_vids[]
	);

h5_err_t
H5FedEndStoreElements (
	h5_file_t * const f
	);

h5_err_t
H5FedBeginRefineElements (
	h5_file_t * const f,
	const h5_size_t num
	);

h5_id_t
H5FedRefineElement (
	h5_file_t * const f,
	const h5_id_t eid
	);

h5_err_t
H5FedEndRefineElements (
	h5_file_t * const f
	);

h5_err_t
H5FedMarkRefineElement (
	h5_file_t * const f,
	const h5_id_t eid
	);


#endif
