!
!  Copyright (c) 2006-2016, The Regents of the University of California,
!  through Lawrence Berkeley National Laboratory (subject to receipt of any
!  required approvals from the U.S. Dept. of Energy) and the Paul Scherrer
!  Institut (Switzerland).  All rights reserved.!
!
!  License: see file COPYING in top level of source distribution.
!
  INTERFACE

     !> \addtogroup h5_log_f
     !! @{
     
     !>
     !! Set verbosity level to \c level.
     !!
     !! Verbosity levels are:
     !! - \c H5_VERBOSE_NONE: be quiet
     !! - \c H5_VERBOSE_ERROR: output error messages
     !! - \c H5_VERBOSE_WARN: output error messages and warning
     !! - \c H5_VERBOSE_INFO: output error messages, warnings and informational messages
     !!
     !! The default verbosity level ist \c H5_VERBOSE_ERROR.
     !!
     !! \return \c H5_SUCCESS
     !!
     !! \see h5_get_verbosity_level()

     SUBROUTINE h5_set_verbosity_level ( level )
       INTEGER*8, INTENT(IN) :: level      !< the level from 0 (no output) to 5 (most detailed)
     END SUBROUTINE h5_set_verbosity_level

     !>
     !! Get verbosity level.
     !!
     !! \return   verbosity level
     !!
     !! \see h5_set_verbosity_level()

     INTEGER*8 FUNCTION h5_get_verbosity_level ()
     END FUNCTION h5_get_verbosity_level

     !> @}
  END INTERFACE
