#ifndef __H5_TYPES_H
#define __H5_TYPES_H

#include <stdarg.h>
#include <hdf5.h>

/*
  file modes:
  H5_O_RDONLY: only reading allowed
  H5_O_WRONLY: create new file, dataset must not exist
  H5_O_APPEND: allows to append a new datasets to an existing file
  H5_O_RDWR:   dataset may exist
*/

#define H5_O_RDWR		0
#define H5_O_RDONLY		1
#define H5_O_WRONLY		2
#define H5_O_APPEND		3

#define H5_ID_T			H5T_NATIVE_INT64
#define H5_FLOAT64_T		H5T_NATIVE_DOUBLE
#define H5_FLOAT32_T		H5T_NATIVE_FLOAT
#define H5_INT64_T		H5T_NATIVE_INT64
#define H5_INT32_T		H5T_NATIVE_INT32
#define H5_COMPOUND_T		H5T_COMPOUND

extern const char * const H5_O_MODES[];

#ifdef   WIN32
typedef __int64			int64_t;
#endif /* WIN32 */

typedef int64_t			h5_int64_t;
typedef int32_t			h5_int32_t;
typedef int64_t			h5_id_t;
typedef uint64_t		h5_size_t;	/* size in number of elements */
typedef int64_t			h5_ssize_t;	/* size in number of elements */
typedef int64_t			h5_err_t;

typedef double			h5_float64_t;
typedef float			h5_float32_t;

typedef struct h5_complex {
	h5_float64_t		r,i;
} h5_complex_t;

typedef h5_id_t h5_2id_t[2];
typedef h5_id_t h5_3id_t[3];
typedef h5_id_t h5_4id_t[4];
typedef h5_float64_t h5_coord3d_t[3];


struct h5_file;
typedef h5_err_t (*h5_errorhandler_t)(
	struct h5_file * const,
	const char*,
	va_list ap );

#ifndef PARALLEL_IO
typedef unsigned long		MPI_Comm;
#endif

typedef struct h5_idlist {
	int32_t		size;		/* allocated space in number of items */
	int32_t		num_items;	/* stored items	*/
	h5_id_t		*items;
} h5_idlist_t;

struct h5_idmap;
typedef struct h5_idmap h5_idmap_t;

/**
   \struct h5_file

   This is an essentially opaque datastructure that
   acts as the filehandle for all practical purposes.
   It is created by H5PartOpenFile<xx>() and destroyed by
   H5PartCloseFile().  
*/
struct h5_file {
	hid_t	file;			/* file id -> fid		*/
	unsigned mode;			/* file access mode		*/
	int	empty;

	h5_err_t	__errno;	/* error number			*/
	const char *	__funcname;	/* H5Block/Fed/Part API function*/

	/* MPI */

	MPI_Comm comm;			/* MPI communicator		*/
	int	nprocs;			/* number of processors		*/
	int	myproc;			/* The index of the processor	
					   this process is running on.	*/

	/* HDF5 */
	hid_t	xfer_prop;		/* file transfer properties	*/
	hid_t	create_prop;		/* file create properties	*/
	hid_t	access_prop;		/* file access properties	*/
	hid_t	root_gid;		/* id of root group		*/
	hid_t	step_gid;		/* id of current step		*/

	/* step internal data						*/
	char	prefix_step_name[256];	/* Prefix of step name		*/
	int	width_step_idx;		/* pad step index with 0 up to this */
	char	step_name[128];		/* full step name		*/
	h5_int64_t step_idx;		/* step index			*/
	int	is_new_step;

	struct h5u_fdata *u;
	struct h5b_fdata *b;
	struct h5t_fdata *t;
};

typedef struct h5_file h5_file_t;

enum h5_oid {
	H5_OID_VERTEX = 1,
	H5_OID_EDGE = 2,
	H5_OID_TRIANGLE = 3,
	H5_OID_TETRAHEDRON = 4,
	H5_OID_MAX = 5	
};
typedef enum h5_oid h5_oid_t;

#define H5_MAX_VERTICES_PER_ELEM H5_OID_TETRAHEDRON

#define H5_TRIANGLE_MESH	 (H5_OID_TRIANGLE)
#define H5_TETRAHEDRAL_MESH	 (H5_OID_TETRAHEDRON)

#endif
