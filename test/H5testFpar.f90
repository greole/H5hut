! ==============
!
! Sample Fortran program that uses HDF5 bindings
!
! ==============

      program H5testFpar
      implicit none
      include 'mpif.h'
      include 'H5PartF90.inc'
      INTEGER*8 file
      INTEGER*8 nstep
      INTEGER*8 ndata
      INTEGER*8 npoints,pointoffset
      INTEGER*8 step
      INTEGER*8 I
      INTEGER*8 J
      REAL*8,ALLOCATABLE:: X(:),Y(:),Z(:),PX(:),PY(:),PZ(:)
      INTEGER*8,ALLOCATABLE:: ID(:)
      REAL*8 REALTIME(1)
      INTEGER myproc,nprocs
      INTEGER ierr
      INTEGER*8 rc
      INTEGER:: comm
      
      call MPI_Init(ierr)
      comm = MPI_COMM_WORLD
      call MPI_Comm_rank(comm,myproc,ierr)
      call MPI_Comm_size(comm,nprocs,ierr)

      file = h5pt_openw_par("testfilef.h5",comm)
      print *,"Opened file testfilef.h5 for writing"
      npoints = 1024
      nstep = 10
      ALLOCATE(X(npoints),Y(npoints),Z(npoints))
      ALLOCATE(PX(npoints),PY(npoints),PZ(npoints))
      ALLOCATE(ID(npoints))
      print *,"    Npoints=",npoints," nsteps=",nstep
      print *,"    writing X,Y,Z,PX,PY,PZ,ID"
      print *,"    ... initialize the data arrays"
      do I=1,npoints
         X(INT(I))=0.0
         Y(I)=1.0+REAL(I)
         Z(I)=100.0+REAL(I)*2.0
         ID(I)=I
      enddo
      print *,"Tell h5pt how many particles are stored in the file"
!     set the number of points
      rc = h5pt_setnpoints(file,npoints)
      print *,"write an attribute string"
!     write an annotation to the file
      rc=h5pt_writefileattrib_string(file,"Annotation","Testing 1 2 3")
      print *,"and now write the steps"
      do I=1,nstep
!        Set the step number
         print *,"Write Step ",I
         rc = h5pt_setstep(file,I)
!        Now start writing the data arrays for this step
         rc = h5pt_writedata_r8(file,"x",X)
         rc = h5pt_writedata_r8(file,"y",Y)
         rc = h5pt_writedata_r8(file,"z",Z)
         rc = h5pt_writedata_r8(file,"px",PX)
         rc = h5pt_writedata_r8(file,"py",PY)
         rc = h5pt_writedata_r8(file,"pz",PZ)
         rc = h5pt_writedata_i8(file,"id",ID)
         do J=1,npoints
            ID(J)=ID(J)+10
         enddo
!        And write a simple floatingpoint attribute associated with this timestep
         REALTIME(1) = REAL(I)*0.1
         rc = h5pt_writestepattrib_r8(file,"RealTime",REALTIME,1_8)
      enddo
      print *,"Done writing, now close the file"
      rc = h5pt_close(file)

! **************** Clean out some variables ***************
      nstep=0
!      npoints=0
      do I=1,npoints
         X(I)=-1.0
         Y(I)=-1.0
         Z(I)=-1.0
         ID(I)=0
      enddo

! *****************Now Reopen for Reading ******************


      print *,"Open file for reading"
      file = h5pt_openr_par("testfilef.h5",comm)
      print *,"  Opened testfilef.h5"
      nstep = h5pt_getnsteps(file)
      print *,"  Nsteps = ",nstep
      rc = h5pt_setstep(file,1_8)
      print *,"now get the number of datasets"
      ndata = h5pt_getndatasets(file)
      print *,"  Ndata=",ndata
      npoints = h5pt_getnpoints(file)
      print *,"  Total NP=",npoints
      npoints = npoints/nprocs
      pointoffset = npoints*myproc;
      rc = h5pt_setview(file,pointoffset,pointoffset+npoints)
      print *,"  Pointoffset=",pointoffset,"  Local NP=",npoints
! Now we need to set our view to read correct points

      
      do step=1,nstep
         print *,"Read step ",step
!        set the current step
         rc = h5pt_setstep(file,step)
         rc = h5pt_readdata_i8(file,"id",ID)
!        read the Z data from the current step
!        rc = h5prt_readdata(file,step,X,Y,Z,PX,PY,PZ,ID)
         do J=1,10
            print *,"    ID(",J,")==",ID(J)
         enddo
      enddo

      rc = h5pt_close(file)
      print *,"done"      
      call MPI_Finalize()
      
      end
