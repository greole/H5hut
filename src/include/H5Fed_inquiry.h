/*
  Header file for declaring the H5Fed application programming
  interface (API) in the C language.
  
  Copyright 2006-2007
 	Paul Scherrer Institut, Villigen, Switzerland;
 	Benedikt Oswald;
 	Achim Gsell
 	All rights reserved.
 
  Authors
 	Achim Gsell
  
  Warning
	This code is under development.
 
 */
#ifndef __H5FED_INQUIRY_H
#define __H5FED_INQUIRY_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
  Get number of meshes of given type.

  \param[in]	f	File handle
  \param[in]	type_id	Type of mesh we want the number of.

  \return	Number of meshes of type \c type_id or error code.
 */
static inline h5_ssize_t
H5FedGetNumTetrahedralMeshes ( 
	h5_file_t* const f
	) {
	H5_API_ENTER1 (h5_err_t, "f=0x%p", f);
	H5_API_RETURN (h5t_get_num_tetmeshes (f));
}

static inline h5_ssize_t
H5FedGetNumTriangleMeshes ( 
	h5_file_t* const f
	) {
	H5_API_ENTER1 (h5_err_t, "f=0x%p", f);
	H5_API_RETURN (h5t_get_num_trimeshes (f));
}

/*!
  Get the number of hierarchical mesh levels.

  \param[in]	f	File handle

  \return	Number of hierarchical mesh levels or error code.
 */
static inline h5_ssize_t
H5FedGetNumLevels (
	h5_file_t* const f
	) {
	H5_API_ENTER1 (h5_ssize_t, "f=0x%p", f);
	H5_API_RETURN (h5t_get_num_leaf_levels (f));
}

/*!
  Get current mesh levels.

  \param[in]	f	File handle

  \return	ID of current mesh levels or error code.
 */
static inline h5t_lvl_idx_t
H5FedGetLevel (
	h5_file_t* const f
	) {
	H5_API_ENTER1 (h5t_lvl_idx_t, "f=0x%p", f);
	H5_API_RETURN (h5t_get_level (f));
}

/*!
  Returns the number of vertices used for defining the (sub-)mesh
  at current level on this compute node.

  \param[in]	f	file handle

  \return	Number of vertices or error code.
*/
static inline h5_ssize_t
H5FedGetNumVertices (
	h5_file_t* const f		/*!< file handle		*/
	) {
	H5_API_ENTER1 (h5_ssize_t, "f=0x%p", f);
	H5_API_RETURN (h5t_get_num_vertices (f, -1));
}

/*!
  Returns the number of vertices used for defining the (sub-)mesh
  at current level on compute node \c cnode.

  \param[in]	f	file handle
  \param[in]	cnode	compute node

  \return	Number of vertices or error code.
*/
static inline h5_ssize_t
H5FedGetNumVerticesCnode (
	h5_file_t* const f,
	const int cnode
	) {
	H5_API_ENTER2 (h5_ssize_t, "f=0x%p, cnode=%d", f, cnode);
	H5_API_RETURN (h5t_get_num_vertices (f, cnode));
}

/*!
  Returns the number of vertices used for defining the (sub-)mesh
  at current level overl all compute nodes.

  \param[in]	f	file handle

  \return	Total number of vertices or error code.
*/
static inline h5_ssize_t
H5FedGetNumVerticesTotal (
	h5_file_t* const f
	) {
	H5_API_ENTER1 (h5_ssize_t, "f=0x%p", f);
	H5_API_RETURN (h5t_get_num_vertices (f, -1));
}

/*!
  Returns the number of elements present in the (sub-)mesh
  at current level on this compute node.

  \param[in]	f	file handle

  \return	Number of elements or error code.
*/
static inline h5_ssize_t
H5FedGetNumElements (
	h5_file_t* const f
	) {
	H5_API_ENTER1 (h5_ssize_t, "f=0x%p", f);
	// MLH: can't use field from opaque h5_file_t!
	//h5_ssize_t num = h5t_get_num_elems (f, f->myproc);
	H5_API_RETURN (h5t_get_num_elems (f, -1));
}

/*!
  Returns the number of elements present in the (sub-)mesh
  at current level on compute node \c cnode.

  \param[in]	f	file handle
  \param[in]	cnode	Compute node

  \return	Number of elements or error code.
*/
static inline h5_ssize_t
H5FedGetNumElementsCnode (
	h5_file_t* const f,
	const int cnode
	) {
	H5_API_ENTER2 (h5_ssize_t, "f=0x%p, cnode=%d", f, cnode);
	H5_API_RETURN (h5t_get_num_elems (f, cnode));
}

/*!
  Returns the number of elements present in the mesh
  at current level over all compute nodes.

  \param[in]	f	File handle.

  \return	Number of elements or error code.
*/
static inline h5_ssize_t
H5FedGetNumElementsTotal (
	h5_file_t* const f
	) {
	H5_API_ENTER1 (h5_ssize_t, "f=0x%p", f);
	H5_API_RETURN (h5t_get_num_elems (f, -1));
}

#ifdef __cplusplus
}
#endif

#endif
