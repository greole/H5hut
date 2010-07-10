#include <string.h>
#include <stdlib.h>

#include "h5core/h5_core.h"
#include "h5_core_private.h"

/*!
  \note
  A partition must not be part of another partition.

  A partition must not divide another partition into two pieces.

  After handling the ghost zones, the partition must not be empty

  We must track the overall size somewhere. This is a good place to do it. (?)
*/

static void
_normalize_partition (
	struct h5b_partition *p	/*!< IN/OUT: partition */
	) {
	h5_int64_t x;

	if ( p->i_start > p->i_end ) {
		x = p->i_start;
		p->i_start = p->i_end;
		p->i_end = x;
	}
	if ( p->j_start > p->j_end ) {
		x = p->j_start;
		p->j_start = p->j_end;
		p->j_end = x;
	}
	if ( p->k_start > p->k_end ) {
		x = p->k_start;
		p->k_start = p->k_end;
		p->k_end = x;
	}
}

/*!
  \ingroup h5block_private

  \internal

  Gather layout to all processors

  \return	H5_SUCCESS or error code
*/
static h5_err_t
_allgather (
	h5_file_t *const f		/*!< IN: file handle */
	) {
#ifdef PARALLEL_IO
	struct h5b_partition *partition = &f->b->user_layout[f->myproc];
	struct h5b_partition *layout = f->b->user_layout;

	TRY( h5priv_mpi_allgather(f,
		partition, 1, f->b->partition_mpi_t,
		layout, 1, f->b->partition_mpi_t, f->comm) );
#endif
	return H5_SUCCESS;
}

/*!
  \ingroup h5block_private

  \internal

  Get dimension sizes of block.  These informations are stored inside the
  block structure.
*/
static void
_get_dimension_sizes (
	h5_file_t *const f			/*!< IN: file handle */
	) {
	int proc;
	struct h5b_fdata *b = f->b;
	struct h5b_partition *partition = b->user_layout;

	b->i_max = 0;
	b->j_max = 0;
	b->k_max = 0;

	for ( proc = 0; proc < f->nprocs; proc++, partition++ ) {
		if ( partition->i_end > b->i_max ) b->i_max = partition->i_end;
		if ( partition->j_end > b->j_max ) b->j_max = partition->j_end;
		if ( partition->k_end > b->k_max ) b->k_max = partition->k_end;
	}
}

#define _NO_GHOSTZONE(p,q) ( (p->i_end < q->i_start) \
  			  ||   (p->j_end < q->j_start) \
			  ||   (p->k_end < q->k_start) )


/*!
  \ingroup h5block_private

  \internal

  Check whether two partitions have a common ghost-zone.

  \return value != \c 0 if yes otherwise \c 0
*/
static int
_have_ghostzone (
	const struct h5b_partition *p,	/*!< IN: partition \c p */
	const struct h5b_partition *q	/*!< IN: partition \c q */
	) {
	return ( ! ( _NO_GHOSTZONE ( p, q ) || _NO_GHOSTZONE ( q, p ) ) );
}

/*!
  \ingroup h5block_private

  \internal

  Calculate volume of partition.

  \return volume
*/
static h5_int64_t
_volume_of_partition (
	const struct h5b_partition *p	/*!< IN: partition */
	) {
	return (p->i_end - p->i_start)
		* (p->j_end - p->j_start)
		* (p->k_end - p->k_start);

}

#define MIN( x, y ) ( (x) <= (y) ? (x) : (y) )  
#define MAX( x, y ) ( (x) >= (y) ? (x) : (y) )  

/*!
  \ingroup h5block_private

  \internal

  Calc volume of ghost-zone.

  \return volume
*/
static h5_int64_t
_volume_of_ghostzone (
	const struct h5b_partition *p, /*!< IN: ptr to first partition */
	const struct h5b_partition *q  /*!< IN: ptr to second partition */
	) {

	h5_int64_t dx = MIN ( p->i_end, q->i_end )
		- MAX ( p->i_start, q->i_start ) + 1;
	h5_int64_t dy = MIN ( p->j_end, q->j_end )
		- MAX ( p->j_start, q->j_start ) + 1;
	h5_int64_t dz = MIN ( p->k_end, q->k_end )
		- MAX ( p->k_start, q->k_start ) + 1;

	return dx * dy * dz;
}

/*!
  \ingroup h5block_private

  \internal

  Dissolve ghost-zone by moving the X coordinates.  Nothing will be changed
  if \c { p->i_start <= q->i_end <= p->i_end }.  In this case \c -1 will be
  returned.

  \return H5_SUCCESS or -1
*/
static h5_int64_t
_dissolve_X_ghostzone (
	struct h5b_partition *p,	/*!< IN/OUT: ptr to first partition */
	struct h5b_partition *q	/*!< IN/OUT: ptr to second partition */
	) {

	if ( p->i_start > q->i_start )
		return _dissolve_X_ghostzone( q, p );

	if ( q->i_end <= p->i_end )  /* no dissolving		*/
		return -1;

	p->i_end = ( p->i_end + q->i_start ) >> 1;
	q->i_start = p->i_end + 1;
	return 0;
}

/*!
  \ingroup h5block_private

  \internal

  Dissolve ghost-zone by moving the Y coordinates.  Nothing will be changed
  if \c { p->j_start <= q->j_end <= p->j_end }.  In this case \c -1 will be
  returned.

  \return H5_SUCCESS or -1
*/
static h5_int64_t
_dissolve_Y_ghostzone (
	struct h5b_partition *p,	/*!< IN/OUT: ptr to first partition */
	struct h5b_partition *q	/*!< IN/OUT: ptr to second partition */
	) {

	if ( p->j_start > q->j_start )
		return _dissolve_Y_ghostzone( q, p );

	if ( q->j_end <= p->j_end )    /* no dissolving		*/
		return -1;

	p->j_end = ( p->j_end + q->j_start ) >> 1;
	q->j_start = p->j_end + 1;
	return 0;
}

/*!
  \ingroup h5block_private

  \internal

  Dissolve ghost-zone by moving the Z coordinates.  Nothing will be changed
  if \c { p->k_start <= q->k_end <= p->k_end }.  In this case \c -1 will be
  returned.

  \return H5_SUCCESS or -1
*/
static h5_int64_t
_dissolve_Z_ghostzone (
	struct h5b_partition *p,	/*!< IN/OUT: ptr to first partition */
	struct h5b_partition *q	/*!< IN/OUT: ptr to second partition */
	) {

	if ( p->k_start > q->k_start )
		return _dissolve_Z_ghostzone( q, p );

	if ( q->k_end <= p->k_end )    /* no dissolving		*/
		return -1;

	p->k_end = ( p->k_end + q->k_start ) >> 1;
	q->k_start = p->k_end + 1;
	return 0;
}

/*!
  \ingroup h5block_private

  \internal

  Dissolve ghost-zone for partitions \p and \q.

  Dissolving is done by moving either the X, Y or Z plane.  We never move
  more than one plane per partition.  Thus we always have three possibilities
  to dissolve the ghost-zone.  The "best" is the one with the largest
  remaining volume of the partitions.

  \return H5_SUCCESS or error code.
*/
static h5_int64_t
_dissolve_ghostzone (
	h5_file_t *const f,
	struct h5b_partition *p,	/*!< IN/OUT: ptr to first partition */
	struct h5b_partition *q	/*!< IN/OUT: ptr to second partition */
	) {

	struct h5b_partition p_;
	struct h5b_partition q_;
	struct h5b_partition p_best;
	struct h5b_partition q_best;
	h5_int64_t vol;
	h5_int64_t max_vol = 0;

	p_ = *p;
	q_ = *q;
	if ( _dissolve_X_ghostzone ( &p_, &q_ ) == 0 ) {
		vol = _volume_of_partition ( &p_ ) 
			+ _volume_of_partition ( &q_ );
		if ( vol > max_vol ) {
			max_vol = vol;
			p_best = p_;
			q_best = q_;
		}
	}

	p_ = *p;
	q_ = *q;
	if ( _dissolve_Y_ghostzone ( &p_, &q_ ) == 0 ) {
		vol = _volume_of_partition ( &p_ )
			+ _volume_of_partition ( &q_ );
		if ( vol > max_vol ) {
			max_vol = vol;
			p_best = p_;
			q_best = q_;
		}
	}
	p_ = *p;
	q_ = *q;

	if ( _dissolve_Z_ghostzone ( &p_, &q_ ) == 0 ) {
		vol = _volume_of_partition ( &p_ )
			+ _volume_of_partition ( &q_ );
		if ( vol > max_vol ) {
			max_vol = vol;
			p_best = p_;
			q_best = q_;
		}
	}
	if ( max_vol <= 0 ) {
		return h5_error (f,
			H5_ERR_LAYOUT,
			"Cannot dissolve ghostzones in specified layout!" );
	}
	*p = p_best;
	*q = q_best;

	return H5_SUCCESS;
}

/*!
  \ingroup h5block_private

  \internal

  Dissolve all ghost-zones.

  Ghost-zone are dissolved in the order of their magnitude, largest first.

  \note
  Dissolving ghost-zones automaticaly is not trivial!  The implemented 
  algorithmn garanties, that there are no ghost-zones left and that we
  have the same result on all processors.
  But there may be zones which are not assigned to a partition any more.
  May be we should check this and return an error in this case.  Then
  the user have to decide to continue or to abort.

  \b {Error Codes}
  \b H5PART_NOMEM_ERR

  \return H5_SUCCESS or error code.
*/
static h5_int64_t
_dissolve_ghostzones (
	h5_file_t *const f	/*!< IN: file handle */
	) {

	struct h5b_fdata *b = f->b;
	struct h5b_partition *p;
	struct h5b_partition *q;
	int proc_p, proc_q;

	struct list {
		struct list *prev;
		struct list *next;
		struct h5b_partition *p;
		struct h5b_partition *q;
		h5_int64_t vol;
	} *p_begin, *p_el, *p_max, *p_end, *p_save;

	memcpy ( b->write_layout, b->user_layout,
		 f->nprocs * sizeof (*f->b->user_layout) );

	TRY( p_begin = (struct list*)h5priv_alloc(f, NULL, sizeof(*p_begin)) );
	p_max = p_end = p_begin;
	
	memset ( p_begin, 0, sizeof ( *p_begin ) );

	for ( proc_p = 0, p = b->write_layout;
		proc_p < f->nprocs-1;
		proc_p++, p++ ) {
		for ( proc_q = proc_p+1, q = &b->write_layout[proc_q];
			proc_q < f->nprocs;
			proc_q++, q++ ) {

			if ( _have_ghostzone ( p, q ) ) {
				TRY( p_el = (struct list*)h5priv_alloc(f, NULL, sizeof(*p_el)) );

				p_el->p = p;
				p_el->q = q;
				p_el->vol = _volume_of_ghostzone ( p, q );
				p_el->prev = p_end;
				p_el->next = NULL;
				
				if ( p_el->vol > p_max->vol )
					p_max = p_el;

				p_end->next = p_el;
				p_end = p_el;
			}
		}
	}
	while ( p_begin->next ) {
		if ( p_max->next ) p_max->next->prev = p_max->prev;
		p_max->prev->next = p_max->next;
		
		_dissolve_ghostzone ( f, p_max->p, p_max->q );

		free ( p_max );
		p_el = p_max = p_begin->next;

		while ( p_el ) {
			if ( _have_ghostzone ( p_el->p, p_el->q ) ) {
				p_el->vol = _volume_of_ghostzone ( p_el->p, p_el->q );
				if ( p_el->vol > p_max->vol )
					p_max = p_el;
				p_el = p_el->next;
			} else {
				if ( p_el->next )
					p_el->next->prev = p_el->prev;
				p_el->prev->next = p_el->next;
				p_save = p_el->next;
				free ( p_el );
				p_el = p_save;
			}
		}

	}
	free ( p_begin );

	p = &b->user_layout[f->myproc];
	h5_debug (f,
		"PROC[%d]: User layout: %lld:%lld, %lld:%lld, %lld:%lld",
		f->myproc,
		(long long)p->i_start, (long long)p->i_end,
		(long long)p->j_start, (long long)p->j_end,
		(long long)p->k_start, (long long)p->k_end );
	/* more detailed debug output: all procs report their view
	   of all other procs */
	for ( proc_p = 0, p = b->user_layout;
		proc_p < f->nprocs;
		proc_p++, p++ ) {
		h5_debug (f,
			"PROC[%d]: proc[%d]: User layout: %lld:%lld, %lld:%lld, %lld:%lld  ",
			f->myproc, proc_p,
			(long long)p->i_start, (long long)p->i_end,
			(long long)p->j_start, (long long)p->j_end,
			(long long)p->k_start, (long long)p->k_end );
	}

	p = &b->write_layout[f->myproc];
	h5_debug (f,
		"PROC[%d]: Ghost-zone layout: %lld:%lld, %lld:%lld, %lld:%lld",
		f->myproc,
		(long long)p->i_start, (long long)p->i_end,
		(long long)p->j_start, (long long)p->j_end,
		(long long)p->k_start, (long long)p->k_end );
	/* more detailed debug output: all procs report their view
	   of all other procs */
	for ( proc_p = 0, p = b->write_layout;
		proc_p < f->nprocs;
		proc_p++, p++ ) {
		h5_debug (f,
			"PROC[%d]: proc[%d]: Ghost-zone layout: %lld:%lld, %lld:%lld, %lld:%lld  ",
			f->myproc, proc_p,
			(long long)p->i_start, (long long)p->i_end,
			(long long)p->j_start, (long long)p->j_end,
			(long long)p->k_start, (long long)p->k_end );
	}
	return H5_SUCCESS;
}

h5_err_t
h5bpriv_release_hyperslab (
	h5_file_t *const f			/*!< IN: file handle */
	) {
	if ( f->b->shape > 0 ) {
		TRY( h5priv_close_hdf5_dataspace(f, f->b->shape) );
		f->b->shape = -1;
	}
	if ( f->b->diskshape > 0 ) {
		TRY( h5priv_close_hdf5_dataspace(f, f->b->diskshape) );
		f->b->diskshape = -1;
	}
	if ( f->b->memshape > 0 ) {
		TRY( h5priv_close_hdf5_dataspace(f, f->b->memshape) );
		f->b->memshape = -1;
	}
	return H5_SUCCESS;
}

h5_err_t
h5bpriv_open_block_group (
	h5_file_t *const f		/*!< IN: file handle */
	) {

	struct h5b_fdata *b = f->b;

	if ( b->block_gid >= 0) {
		TRY( h5priv_close_hdf5_group(f, b->block_gid) );
		b->block_gid = -1;
	}

	if ( b->block_gid < 0 ) {
		TRY( b->block_gid = h5priv_open_hdf5_group(f,
					f->step_gid, H5_BLOCKNAME) );
	}

	return H5_SUCCESS;
}

h5_err_t
h5bpriv_have_field_group (
	h5_file_t *const f,			/*!< IN: file handle */
	const char *name
	) {

	char name2[H5_DATANAME_LEN];
	h5_normalize_dataset_name(f, name, name2);

	TRY( h5bpriv_open_block_group ( f ) );

	h5_err_t exists;
	TRY( exists = h5priv_hdf5_have_link(f, f->b->block_gid, name2) );

	return exists;
}

h5_err_t
h5bpriv_open_field_group (
	h5_file_t *const f,			/*!< IN: file handle */
	const char *name
	) {

	char name2[H5_DATANAME_LEN];
	h5_normalize_dataset_name(f, name, name2);
	
	if ( f->b->field_gid >= 0 ) {
		TRY( h5priv_close_hdf5_group(f, f->b->field_gid) );
	}

	TRY( h5bpriv_open_block_group ( f ) );
	TRY( f->b->field_gid = h5priv_open_hdf5_group(f, f->b->block_gid, name2) );

	return H5_SUCCESS;
}

h5_err_t
h5b_3d_set_view (
	h5_file_t *const f,		/*!< IN: File handle		*/
	const h5_size_t i_start,	/*!< IN: start index of \c i	*/ 
	const h5_size_t i_end,		/*!< IN: end index of \c i	*/  
	const h5_size_t j_start,	/*!< IN: start index of \c j	*/ 
	const h5_size_t j_end,		/*!< IN: end index of \c j	*/ 
	const h5_size_t k_start,	/*!< IN: start index of \c k	*/ 
	const h5_size_t k_end		/*!< IN: end index of \c k	*/
	) {

	struct h5b_fdata *b = f->b;
	struct h5b_partition *p = &b->user_layout[f->myproc];
	p->i_start = i_start;
	p->i_end =   i_end;
	p->j_start = j_start;
	p->j_end =   j_end;
	p->k_start = k_start;
	p->k_end =   k_end;

	_normalize_partition ( p );
	TRY( _allgather( f ) );
	_get_dimension_sizes ( f );
	TRY( _dissolve_ghostzones ( f ) );
	TRY( _release_hyperslab ( f ) );
	b->have_layout = 1;

	return H5_SUCCESS;
}

h5_err_t
h5b_3d_set_chunk (
	h5_file_t *const f,		/*!< IN: File handle */
	const h5_size_t i,		/*!< IN: size of \c i */ 
	const h5_size_t j,		/*!< IN: size of \c j */  
	const h5_size_t k		/*!< IN: size of \c k */ 
	) {

	if ( i == 0 || j == 0 || k == 0 )
	{
		h5_info(f, "Disabling chunking" );
		TRY( h5priv_set_hdf5_layout_property(f,
					f->b->dcreate_prop, H5D_CONTIGUOUS) );
	} else 
	{
		h5_info(f, "Setting chunk to (%lld,%lld,%lld)",
			(long long)i, (long long)j, (long long)k);
		hsize_t dims[3] = { k, j, i };
		TRY( h5priv_set_hdf5_chunk_property(f,
					f->b->dcreate_prop, 1, dims) );
	}

	return H5_SUCCESS;
}

h5_err_t
h5b_3d_get_chunk (
	h5_file_t *const f,		/*!< IN: File handle */
	const char *field_name, 	/*!< IN: name of dataset */
	h5_size_t *dims			/*!< OUT: array containing the chunk dimensions */
	) {

	CHECK_TIMEGROUP ( f );

	struct h5b_fdata *b = f->b;

	TRY( h5bpriv_open_field_group ( f, field_name ) );

	hid_t dataset_id;
	hid_t plist_id;
	hsize_t hdims[3];

	TRY( dataset_id = h5priv_open_hdf5_dataset(f, b->field_gid, H5_BLOCKNAME_X) );
	TRY( plist_id = h5priv_get_hdf5_dataset_create_plist(f, dataset_id) );
	TRY( h5priv_get_hdf5_chunk_property(f, plist_id, 3, hdims) );
	TRY( h5priv_close_hdf5_property(f, plist_id) );
	TRY( h5priv_close_hdf5_dataset(f, dataset_id) );

	dims[0] = hdims[2];
	dims[1] = hdims[1];
	dims[2] = hdims[0];

	h5_info(f,
		"Found chunk dimensions (%lld,%lld,%lld)",
		(long long)dims[0],
		(long long)dims[1],
		(long long)dims[2] );

	return H5_SUCCESS;
}

h5_err_t
h5b_3d_get_view (
	h5_file_t *const f,	/*!< IN: File handle */
	const int proc,		/*!< IN: Processor to get partition from */
	h5_size_t *i_start,	/*!< OUT: start index of \c i	*/ 
	h5_size_t *i_end,	/*!< OUT: end index of \c i	*/  
	h5_size_t *j_start,	/*!< OUT: start index of \c j	*/ 
	h5_size_t *j_end,	/*!< OUT: end index of \c j	*/ 
	h5_size_t *k_start,	/*!< OUT: start index of \c k	*/ 
	h5_size_t *k_end	/*!< OUT: end index of \c k	*/ 
	) {

	if ( ( proc < 0 ) || ( proc >= f->nprocs ) )
		return h5_error(f, H5_ERR_INVAL, "Invalid processor id %d!", proc);

	struct h5b_partition *p = &f->b->user_layout[(size_t)proc];

	*i_start = p->i_start;
	*i_end =   p->i_end;
	*j_start = p->j_start;
	*j_end =   p->j_end;
	*k_start = p->k_start;
	*k_end =   p->k_end;

	return H5_SUCCESS;
}

h5_err_t
h5b_3d_get_reduced_view (
	h5_file_t *const f,	/*!< IN: File handle */
	const int proc,		/*!< IN: Processor to get partition from */
	h5_size_t *i_start,	/*!< OUT: start index of \c i	*/ 
	h5_size_t *i_end,	/*!< OUT: end index of \c i	*/  
	h5_size_t *j_start,	/*!< OUT: start index of \c j	*/ 
	h5_size_t *j_end,	/*!< OUT: end index of \c j	*/ 
	h5_size_t *k_start,	/*!< OUT: start index of \c k	*/ 
	h5_size_t *k_end	/*!< OUT: end index of \c k	*/ 
	) {

	if ( ( proc < 0 ) || ( proc >= f->nprocs ) )
		return h5_error(f, H5_ERR_INVAL, "Invalid processor id %d!", proc);

	struct h5b_partition *p = &f->b->write_layout[(size_t)proc];

	*i_start = p->i_start;
	*i_end =   p->i_end;
	*j_start = p->j_start;
	*j_end =   p->j_end;
	*k_start = p->k_start;
	*k_end =   p->k_end;

	return H5_SUCCESS;
}

int
h5b_3d_get_proc (
	h5_file_t *const f,		/*!< IN: File handle */
	const h5_int64_t i,		/*!< IN: \c i coordinate */
	const h5_int64_t j,		/*!< IN: \c j coordinate */
	const h5_int64_t k		/*!< IN: \c k coordinate */
	) {

	struct h5b_partition *layout = f->b->write_layout;
	int proc;

	for ( proc = 0; proc < f->nprocs; proc++, layout++ ) {
		if ( (layout->i_start <= i) && (i <= layout->i_end) &&
		     (layout->j_start <= j) && (j <= layout->j_end) &&
		     (layout->k_start <= k) && (k <= layout->k_end) ) 
			return proc;
	}
	
	return -1;
}

h5_ssize_t
h5b_get_num_fields (
	h5_file_t *const f		/*!< IN: File handle */
	) {

	TRY( h5bpriv_open_block_group(f) );
	return h5priv_get_num_objs_in_hdf5_group( f, f->b->block_gid );
}

h5_err_t
h5b_get_field_info_by_name (
	h5_file_t *const f,			/*!< IN: file handle */
	const char *name,			/*!< OUT: field name */
	h5_size_t *grid_rank,			/*!< OUT: grid rank */
	h5_size_t *grid_dims,			/*!< OUT: grid dimensions */
	h5_size_t *field_rank,			/*!< OUT: field rank */
	h5_int64_t *type			/*!< OUT: datatype */
	) {

	hsize_t dims[16]; /* give it plenty of space even though we don't expect rank > 3 */
	hsize_t _grid_rank, _field_rank;
	h5_size_t i, j;

	TRY( h5bpriv_open_field_group(f, name) );

	hid_t dataset_id;
 	hid_t dataspace_id;

	TRY( dataset_id = h5priv_open_hdf5_dataset(f,
			 		f->b->field_gid, H5_BLOCKNAME_X) );
	TRY( dataspace_id = h5priv_get_hdf5_dataset_space(f, dataset_id) );

	TRY( _grid_rank = h5priv_get_dims_of_hdf5_dataspace(f,
						dataspace_id, dims, NULL) );
	if ( grid_rank ) *grid_rank = (h5_size_t) _grid_rank;
 
	if ( grid_dims ) {
		for ( i = 0, j = _grid_rank-1; i < _grid_rank; i++, j-- )
			grid_dims[i] = (h5_size_t)dims[j];
	}

	TRY( _field_rank = h5priv_get_num_objs_in_hdf5_group(f,
						    f->b->block_gid) );
	if ( field_rank ) *field_rank = (h5_size_t) _field_rank;

	hid_t h5type;
	TRY( h5type = h5priv_get_hdf5_dataset_type(f, dataset_id) );

	if ( type )
		TRY( *type = h5_normalize_h5_type(f, h5type) );

	TRY( h5priv_close_hdf5_dataspace(f, dataspace_id) );
	TRY( h5priv_close_hdf5_dataset(f, dataset_id) );

	return H5_SUCCESS;
}

h5_err_t
h5b_get_field_info (
	h5_file_t *const f,			/*!< IN: file handle */
	const h5_size_t idx,			/*!< IN: index of field */
	char *name,				/*!< OUT: field name */
	const h5_size_t len_name,		/*!< IN: buffer size */
	h5_size_t *grid_rank,			/*!< OUT: grid rank */
	h5_size_t *grid_dims,			/*!< OUT: grid dimensions */
	h5_size_t *field_rank,			/*!< OUT: field rank */
	h5_int64_t *type			/*!< OUT: datatype */
	) {

	TRY( h5bpriv_open_block_group(f) );
	TRY( h5priv_get_hdf5_objname_by_idx(
		f,
		f->b->block_gid,
		(hsize_t)idx,
		name,
		(size_t)len_name) );

	return h5b_get_field_info_by_name(f,
				name, grid_rank, grid_dims, field_rank, type);
}

