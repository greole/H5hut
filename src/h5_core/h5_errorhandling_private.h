#ifndef __ERRORHANDLING_PRIVATE_H
#define __ERRORHANDLING_PRIVATE_H

#define h5_error_not_implemented( f, file, func, lino )		     \
	h5_error(						     \
		f,						     \
		H5_ERR_NOT_IMPLEMENTED,				     \
		"%s: Function \"%s\", line %d not yet implemented!", \
		file, func, lino );

#define h5_error_internal( f, file, func, lino )   \
	h5_error(				   \
		f,				   \
		H5_ERR_INTERNAL,		   \
		"%s: Internal error: %s line %d!", \
		file, func, lino )

#define HANDLE_H5_NOENT_ERR( f, name )				\
	h5_error(						\
		f,						\
		H5_ERR_NOENT,				\
		"Object \"%s\" doesn't exists.", name );

#define HANDLE_H5_DATASET_RANK_ERR( f, m, n )			  \
	h5_error(						  \
		f,						  \
		H5_ERR_INVAL,				  \
		"Wrong rank of dataset: Is %d, but should be %d", \
		m, n );

#define HANDLE_H5_GROUP_EXISTS_ERR( f, name )		\
	h5_error(					\
		f,					\
		H5_ERR_INVAL,			\
		"Group \"%s\" already exists", name )

/**************** H5 *********************/

#define HANDLE_H5_BADFD_ERR( f )			\
	h5_error(					\
		f,					\
		H5_ERR_BADFD,				\
		"Called with bad filehandle." );

#define HANDLE_H5_INIT_ERR( f )			\
        h5_error(				\
		f,				\
		H5_ERR_INIT,			\
		"Cannot initialize H5." );

#define HANDLE_H5_NOMEM_ERR( f )		\
	h5_error(				\
		f,				\
		H5_ERR_NOMEM,			\
		"Out of memory." );

#define HANDLE_H5_SETSTEP_ERR( f, rc, step )			\
	h5_error(						\
		f,						\
		h5_get_funcname(),				\
		rc,						\
		"Cannont set step to %lld.", (long long)step );

#define _h5_handle_file_mode_error( f, mode_id )		  \
	h5_error(						  \
		f,						  \
		H5_ERR_BADF,					  \
		"Operation not permitted in mode \"%s\"",	  \
		H5_O_MODES[mode_id] );

#define HANDLE_H5_FILE_ACCESS_TYPE_ERR( f, flags )		\
	h5_error(						\
		f,						\
		H5_ERR_INVAL,					\
		"Invalid file access mode \"%d\".", flags);

#define HANDLE_H5_STEP_EXISTS_ERR( f, step )				\
	h5_error(							\
		f,							\
		H5_ERR_INVAL,						\
		"Step #%lld already exists, step cannot be set "	\
		"to an existing step in write and append mode",		\
		(long long)step );

#define HANDLE_H5_NOENTRY_ERR( f, group_name, type, idx )	     \
	h5_error(						     \
		f,						     \
		H5_ERR_NOENTRY,					     \
		"No entry with index %lld and type %d in group %s!", \
		(long long)idx, type, group_name );


#define HANDLE_H5_OVERFLOW_ERR( f, otype, max )			\
	h5_error(						\
		f,						\
		H5_ERR_INVAL,					\
		"Cannot store more than %lld %s", max, otype );

#define HANDLE_H5_PARENT_ID_ERR( f, otype, oid, pid  )			\
	h5_error(							\
		f,							\
		H5_ERR_INVAL,						\
		"Impossible parent_id %d for %s with global id %d",	\
		pid, otype, oid );

#define HANDLE_H5_OUT_OF_RANGE_ERR( f, otype, oid )	\
	h5_error(					\
		f,					\
		H5_ERR_INVAL,				\
		"%s id %lld out of range",		\
		otype, oid );

/**************** HDF5 *********************/
/* H5A: Attribute */
#define HANDLE_H5A_CLOSE_ERR( f )				\
	h5_error(						\
		f,						\
		H5_ERR_HDF5,					\
		"Cannot terminate access to attribute." );

#define HANDLE_H5A_CREATE_ERR( f, s )			\
	h5_error(					\
		f,					\
		H5_ERR_HDF5,				\
		"Cannot create attribute \"%s\".", s );

#define HANDLE_H5A_GET_NAME_ERR( f )		\
	h5_error(				\
		f,				\
		H5_ERR_HDF5,			\
		"Cannot get attribute name." );

#define HANDLE_H5A_GET_NUM_ATTRS_ERR( f )		\
	h5_error(					\
		f,					\
		H5_ERR_HDF5,				\
		"Cannot get number of attributes." );

#define HANDLE_H5A_GET_SPACE_ERR( f )					\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Cannot get a copy of dataspace for attribute." );

#define HANDLE_H5A_GET_TYPE_ERR( f )			\
	h5_error(					\
		f,					\
		H5_ERR_HDF5,				\
		"Cannot get attribute datatype." );

#define HANDLE_H5A_OPEN_IDX_ERR( f, n )				      \
	h5_error(						      \
		f,						      \
		H5_ERR_HDF5,					      \
		"Cannot open attribute specified by index \"%lld\".", \
		(long long)n );

#define HANDLE_H5A_OPEN_NAME_ERR( f, s )				\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Cannot open attribute specified by name \"%s\".", s );

#define HANDLE_H5A_READ_ERR( f )		\
	h5_error(				\
		f,				\
		H5_ERR_HDF5,			\
		"Cannot read attribute" );

#define HANDLE_H5A_WRITE_ERR( f, s )			\
	h5_error(					\
		f,					\
		H5_ERR_HDF5,				\
		"Cannot write attribute \"%s\".", s );

/* H5D: Dataset */
#define HANDLE_H5D_CLOSE_ERR( f )		\
	h5_error(				\
		f,				\
		H5_ERR_HDF5,			\
		"Close of dataset failed." );

#define HANDLE_H5D_CREATE_ERR( f, s )		     \
	h5_error(				     \
		f,				     \
		H5_ERR_HDF5,			     \
		"Cannot create dataset name \"%s\"", \
		s );

#define HANDLE_H5D_GET_SPACE_ERR( f )			\
	h5_error(					\
		f,					\
		H5_ERR_HDF5,				\
		"Cannot get dataspace identifier.");

#define HANDLE_H5D_GET_TYPE_ERR( f )			\
	h5_error(					\
		f,					\
		H5_ERR_HDF5,				\
		"Cannot determine dataset type.");

#define HANDLE_H5D_OPEN_ERR( f, s )			\
	h5_error(					\
		f,					\
		H5_ERR_HDF5,				\
		"Cannot open dataset \"%s\".", s );

#define HANDLE_H5D_READ_ERR( f, s )	     \
	h5_error(			     \
		f,			     \
		H5_ERR_HDF5,		     \
		"Read from dataset \"%s\".", \
		s );

#define HANDLE_H5D_WRITE_ERR( f, s )		   \
	h5_error(				   \
		f,				   \
		H5_ERR_HDF5,			   \
		"Write to dataset \"%s\" failed.", \
		s );

/* H5F: file */
#define HANDLE_H5F_CLOSE_ERR( f )			\
	h5_error(					\
		f,					\
		H5_ERR_HDF5,				\
		"Cannot terminate access to file." );

#define HANDLE_H5F_OPEN_ERR( f, filename, flags )		\
	h5_error(						\
		f,						\
		H5_ERR_HDF5,					\
		"Cannot open file \"%s\" with mode \"%d\"",	\
		filename, flags );

/* H5G: group */
#define HANDLE_H5G_CLOSE_ERR( f )				\
	h5_error(						\
		f,						\
		H5_ERR_HDF5,					\
		"Cannot terminate access to datagroup." );

#define HANDLE_H5G_CREATE_ERR( f, s )			\
	h5_error(					\
		f,					\
		H5_ERR_HDF5,				\
		"Cannot create datagroup \"%s\".", s );

#define HANDLE_H5G_GET_OBJINFO_ERR( f, s )				\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Cannot get information about object \"%s\".", s );

#define HANDLE_H5G_OPEN_ERR( f, parent_id, grp_name )			\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Cannot open group \"%s/%s\".", parent_id, grp_name );

/* H5P: property */
#define HANDLE_H5P_CLOSE_ERR( f, s )					\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Cannot terminate access to property list \"%s\".", s );

#define HANDLE_H5P_CREATE_ERR( f )			\
	h5_error(					\
		f,					\
		H5_ERR_HDF5,				\
		"Cannot create property list." );

#define HANDLE_H5P_SET_DXPL_MPIO_ERR( f )				\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"MPI: Cannot set data transfer mode." );


#define HANDLE_H5P_SET_FAPL_MPIO_ERR( f )				\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Cannot store IO communicator information to the "	\
		"file access property list.");

/* H5S: dataspace */
#define HANDLE_H5S_CREATE_SIMPLE_ERR( f, rank )		 \
	h5_error(					 \
		f,					 \
		H5_ERR_HDF5,				 \
		"Cannot create dataspace with rank %d.", \
		rank );

#define HANDLE_H5S_CREATE_SIMPLE_3D_ERR( f, dims )			\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Cannot create 3d dataspace with dimension sizes "	\
		"\"(%lld,%lld,%lld)\".",				\
		(long long)dims[0], (long long)dims[1], (long long)dims[2] );

#define HANDLE_H5S_CLOSE_ERR( f )				\
	h5_error(						\
		f,						\
		H5_ERR_HDF5,					\
		"Cannot terminate access to dataspace." ); 

#define HANDLE_H5S_GET_SELECT_NPOINTS_ERR(f)				\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Cannot determine the number of elements"		\
		"in dataspace selection." ); 

#define HANDLE_H5S_GET_SIMPLE_EXTENT_NPOINTS_ERR( f )			\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Cannot determine number of elements in dataspace." ); 

#define HANDLE_H5S_SELECT_HYPERSLAB_ERR( f )				\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Cannot set select hyperslap region or add the "	\
		"specified region" );

#define HANDLE_H5S_GET_SIMPLE_EXTENT_DIMS_ERR( f )		\
	h5_error(						\
		f,						\
		H5_ERR_HDF5,				\
		"Cannot get dimension sizes of dataset" );

/* H5T:  type */
#define HANDLE_H5T_ARRAY_CREATE_ERR( f, type_name, rank )		\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Can't create array datatype object with base "		\
		"datatype %s and rank %d",				\
		type_name, rank );

#define HANDLE_H5T_CREATE_ERR( f, class_name, obj_name )		\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Can't create datatype object of class %s to handle %s.", \
		class_name, obj_name );

#define HANDLE_H5T_INSERT_ERR( f, field_name, type_name )		\
	h5_error(							\
		f,							\
		H5_ERR_HDF5,						\
		"Can't insert field %s to compound datatype to handle %s", \
		field_name, type_name );

#define HANDLE_H5T_CLOSE_ERR( f )		\
	h5_error(				\
		f,				\
		H5_ERR_HDF5,			\
		"Cannot release datatype." );

/* MPI */
#define HANDLE_MPI_ALLGATHER_ERR( f )		\
	h5_error(				\
		f,				\
		 H5_ERR_MPI,			\
		"Cannot gather data." );

#define HANDLE_MPI_COMM_SIZE_ERR( f )					\
	h5_error(							\
		f,							\
		H5_ERR_MPI,						\
		"Cannot get number of processes in my group." );

#define HANDLE_MPI_COMM_RANK_ERR( f )					\
	h5_error(							\
		f,							\
		H5_ERR_MPI,						\
		"Cannot get rank of the calling process in my group." );

#define HANDLE_MPI_UNAVAILABLE_ERR( f )		\
	h5_error(				\
		f,				\
		H5_ERR_MPI,			\
		"MPI not available" );

#endif