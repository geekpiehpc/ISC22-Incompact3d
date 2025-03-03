! -*- mode: f90 -*-
!=======================================================================
! This is part of the 2DECOMP&FFT library
! 
! 2DECOMP&FFT is a software framework for general-purpose 2D (pencil) 
! decomposition. It also implements a highly scalable distributed
! three-dimensional Fast Fourier Transform (FFT).
!
! Copyright (C) 2009-2011 Ning Li, the Numerical Algorithms Group (NAG)
!
!=======================================================================

! This file contains the routines that transpose data from Z to Y pencil

subroutine transpose_z_to_y_real(src, dst, opt_decomp)

  implicit none

  real(mytype), dimension(:,:,:), intent(IN) :: src
  real(mytype), dimension(:,:,:), intent(OUT) :: dst
  TYPE(DECOMP_INFO), intent(IN), optional :: opt_decomp

  TYPE(DECOMP_INFO) :: decomp

#ifdef SHM
  real(mytype) :: work1(*), work2(*)
  POINTER  (work1_p, work1), (work2_p, work2)  ! Cray pointers
#endif

  integer :: s1,s2,s3,d1,d2,d3
  integer :: ierror

  if (present(opt_decomp)) then
     decomp = opt_decomp
  else
     decomp = decomp_main
  end if

  s1 = SIZE(src,1)
  s2 = SIZE(src,2)
  s3 = SIZE(src,3)
  d1 = SIZE(dst,1)
  d2 = SIZE(dst,2)
  d3 = SIZE(dst,3)

  ! rearrange source array as send buffer
#ifdef SHM
  work1_p = decomp%ROW_INFO%SND_P
  call mem_split_zy_real(src, s1, s2, s3, work1, dims(2), &
       decomp%z2dist, decomp)
#else
#ifdef EVEN
  if (.not. decomp%even) then
     call mem_split_zy_real(src, s1, s2, s3, work1_r, dims(2), &
          decomp%z2dist, decomp)
  end if
#else
  ! note the src array is suitable to be a send buffer
  ! so no split operation needed
#endif
#endif

  ! define receive buffer
#ifdef SHM
  work2_p = decomp%ROW_INFO%RCV_P
  call MPI_BARRIER(decomp%ROW_INFO%CORE_COMM, ierror)
#endif

#ifdef SHM
  if (decomp%ROW_INFO%CORE_ME==1) THEN
     call MPI_ALLTOALLV(work1, decomp%z2cnts_s, decomp%z2disp_s, &
          real_type, work2, decomp%y2cnts_s, decomp%y2disp_s, &
          real_type, decomp%ROW_INFO%SMP_COMM, ierror)
  end if
#else
#ifdef EVEN
  if (decomp%even) then
     call MPI_ALLTOALL(src, decomp%z2count, &
          real_type, work2_r, decomp%y2count, &
          real_type, DECOMP_2D_COMM_ROW, ierror)
  else
     call MPI_ALLTOALL(work1_r, decomp%z2count, &
          real_type, work2_r, decomp%y2count, &
          real_type, DECOMP_2D_COMM_ROW, ierror)
  end if
#else
  call MPI_ALLTOALLV(src, decomp%z2cnts, decomp%z2disp, &
       real_type, work2_r, decomp%y2cnts, decomp%y2disp, &
       real_type, DECOMP_2D_COMM_ROW, ierror)
#endif
#endif

  ! rearrange receive buffer
#ifdef SHM
  call MPI_BARRIER(decomp%ROW_INFO%CORE_COMM, ierror)
  call mem_merge_zy_real(work2, d1, d2, d3, dst, dims(2), &
       decomp%y2dist, decomp)
#else
  call mem_merge_zy_real(work2_r, d1, d2, d3, dst, dims(2), &
       decomp%y2dist, decomp)
#endif

  return
end subroutine transpose_z_to_y_real


subroutine transpose_z_to_y_real_start(request, src, dst, sbuf, rbuf, &
     opt_decomp)

  implicit none

  integer, INTENT(OUT) :: request
  real(mytype), dimension(:,:,:) :: src, dst, sbuf, rbuf
  TYPE(DECOMP_INFO), intent(IN), optional :: opt_decomp

  TYPE(DECOMP_INFO) :: decomp

  integer :: ierror

  if (present(opt_decomp)) then
     decomp = opt_decomp
  else
     decomp = decomp_main
  end if

  sbuf = src

#ifdef EVEN
  call MPI_IALLTOALL(sbuf, decomp%z2count, real_type, &
       rbuf, decomp%y2count, real_type, &
       DECOMP_2D_COMM_ROW, request, ierror)
#else
  call MPI_IALLTOALLV(sbuf, decomp%z2cnts, decomp%z2disp, real_type, &
       rbuf, decomp%y2cnts, decomp%y2disp, real_type, &
       DECOMP_2D_COMM_ROW, request, ierror)
#endif

  return
end subroutine transpose_z_to_y_real_start


subroutine transpose_z_to_y_real_wait(request, src, dst, sbuf, rbuf, &
     opt_decomp)

  implicit none

  integer, INTENT(INOUT) :: request
  real(mytype), dimension(:,:,:) :: src, dst, sbuf, rbuf
  TYPE(DECOMP_INFO), intent(IN), optional :: opt_decomp

  TYPE(DECOMP_INFO) :: decomp

  integer :: d1,d2,d3

  integer :: status(MPI_STATUS_SIZE)
  integer :: ierror

  if (present(opt_decomp)) then
     decomp = opt_decomp
  else
     decomp = decomp_main
  end if

  d1 = SIZE(dst,1)
  d2 = SIZE(dst,2)
  d3 = SIZE(dst,3)

  call MPI_WAIT(request, status, ierror)

  ! rearrange receive buffer
  call mem_merge_zy_real(rbuf, d1, d2, d3, dst, dims(2), &
       decomp%y2dist, decomp)

  return
end subroutine transpose_z_to_y_real_wait


subroutine transpose_z_to_y_complex(src, dst, opt_decomp)

  implicit none

  complex(mytype), dimension(:,:,:), intent(IN) :: src
  complex(mytype), dimension(:,:,:), intent(OUT) :: dst
  TYPE(DECOMP_INFO), intent(IN), optional :: opt_decomp

  TYPE(DECOMP_INFO) :: decomp

#ifdef SHM
  complex(mytype) :: work1(*), work2(*)
  POINTER  (work1_p, work1), (work2_p, work2)  ! Cray pointers
#endif

  integer :: s1,s2,s3,d1,d2,d3
  integer :: ierror

  if (present(opt_decomp)) then
     decomp = opt_decomp
  else
     decomp = decomp_main
  end if

  s1 = SIZE(src,1)
  s2 = SIZE(src,2)
  s3 = SIZE(src,3)
  d1 = SIZE(dst,1)
  d2 = SIZE(dst,2)
  d3 = SIZE(dst,3)

  ! rearrange source array as send buffer
#ifdef SHM
  work1_p = decomp%ROW_INFO%SND_P_c
  call mem_split_zy_complex(src, s1, s2, s3, work1, dims(2), &
       decomp%z2dist, decomp)
#else
#ifdef EVEN
  if (.not. decomp%even) then
     call mem_split_zy_complex(src, s1, s2, s3, work1_c, dims(2), &
          decomp%z2dist, decomp)
  end if
#else
  ! note the src array is suitable to be a send buffer
  ! so no split operation needed
#endif
#endif

  ! define receive buffer
#ifdef SHM
  work2_p = decomp%ROW_INFO%RCV_P_c
  call MPI_BARRIER(decomp%ROW_INFO%CORE_COMM, ierror)
#endif

#ifdef SHM
  if (decomp%ROW_INFO%CORE_ME==1) THEN
     call MPI_ALLTOALLV(work1, decomp%z2cnts_s, decomp%z2disp_s, &
          complex_type, work2, decomp%y2cnts_s, decomp%y2disp_s, &
          complex_type, decomp%ROW_INFO%SMP_COMM, ierror)
  end if
#else
#ifdef EVEN
  if (decomp%even) then
     call MPI_ALLTOALL(src, decomp%z2count, &
          complex_type, work2_c, decomp%y2count, &
          complex_type, DECOMP_2D_COMM_ROW, ierror)
  else
     call MPI_ALLTOALL(work1_c, decomp%z2count, &
          complex_type, work2_c, decomp%y2count, &
          complex_type, DECOMP_2D_COMM_ROW, ierror)
  end if
#else
  call MPI_ALLTOALLV(src, decomp%z2cnts, decomp%z2disp, &
       complex_type, work2_c, decomp%y2cnts, decomp%y2disp, &
       complex_type, DECOMP_2D_COMM_ROW, ierror)
#endif
#endif

  ! rearrange receive buffer
#ifdef SHM
  call MPI_BARRIER(decomp%ROW_INFO%CORE_COMM, ierror)
  call mem_merge_zy_complex(work2, d1, d2, d3, dst, dims(2), &
       decomp%y2dist, decomp)
#else
  call mem_merge_zy_complex(work2_c, d1, d2, d3, dst, dims(2), &
       decomp%y2dist, decomp)
#endif

  return
end subroutine transpose_z_to_y_complex


subroutine transpose_z_to_y_complex_start(request, src, dst, sbuf, &
     rbuf, opt_decomp)

  implicit none

  integer, INTENT(OUT) :: request
  complex(mytype), dimension(:,:,:) :: src, dst, sbuf, rbuf
  TYPE(DECOMP_INFO), intent(IN), optional :: opt_decomp

  TYPE(DECOMP_INFO) :: decomp

  integer :: ierror

  if (present(opt_decomp)) then
     decomp = opt_decomp
  else
     decomp = decomp_main
  end if

  sbuf = src

#ifdef EVEN
  call MPI_IALLTOALL(sbuf, decomp%z2count, &
       complex_type, rbuf, decomp%y2count, &
       complex_type, DECOMP_2D_COMM_ROW, request, ierror)
#else
  call MPI_IALLTOALLV(sbuf, decomp%z2cnts, decomp%z2disp, &
       complex_type, rbuf, decomp%y2cnts, decomp%y2disp, &
       complex_type, DECOMP_2D_COMM_ROW, request, ierror)
#endif

  return
end subroutine transpose_z_to_y_complex_start


subroutine transpose_z_to_y_complex_wait(request, src, dst, sbuf, &
     rbuf, opt_decomp)

  implicit none

  integer, INTENT(INOUT) :: request
  complex(mytype), dimension(:,:,:) :: src, dst, sbuf, rbuf
  TYPE(DECOMP_INFO), intent(IN), optional :: opt_decomp

  TYPE(DECOMP_INFO) :: decomp

  integer :: d1,d2,d3

  integer :: status(MPI_STATUS_SIZE)
  integer :: ierror

  if (present(opt_decomp)) then
     decomp = opt_decomp
  else
     decomp = decomp_main
  end if

  d1 = SIZE(dst,1)
  d2 = SIZE(dst,2)
  d3 = SIZE(dst,3)

  call MPI_WAIT(request, status, ierror)

  ! rearrange receive buffer
  call mem_merge_zy_complex(rbuf, d1, d2, d3, dst, dims(2), &
       decomp%y2dist, decomp)

  return
end subroutine transpose_z_to_y_complex_wait


! pack/unpack ALLTOALL(V) buffers

subroutine mem_split_zy_real(in,n1,n2,n3,out,iproc,dist,decomp)

  implicit none

  integer, intent(IN) :: n1,n2,n3
  real(mytype), dimension(n1,n2,n3), intent(IN) :: in
  real(mytype), dimension(*), intent(OUT) :: out
  integer, intent(IN) :: iproc
  integer, dimension(0:iproc-1), intent(IN) :: dist
  TYPE(DECOMP_INFO), intent(IN) :: decomp

  integer :: i,j,k, m,i1,i2,pos

  do m=0,iproc-1
     if (m==0) then 
        i1 = 1
        i2 = dist(0)
     else
        i1 = i2+1
        i2 = i1+dist(m)-1
     end if

#ifdef SHM
     pos = decomp%z2disp_o(m) + 1
#else
#ifdef EVEN
     pos = m * decomp%z2count + 1
#else
     pos = decomp%z2disp(m) + 1
#endif
#endif

     do k=i1,i2
        do j=1,n2
           do i=1,n1
              out(pos) = in(i,j,k)
              pos = pos + 1
           end do
        end do
     end do
  end do

  return
end subroutine mem_split_zy_real


subroutine mem_split_zy_complex(in,n1,n2,n3,out,iproc,dist,decomp)

  implicit none

  integer, intent(IN) :: n1,n2,n3
  complex(mytype), dimension(n1,n2,n3), intent(IN) :: in
  complex(mytype), dimension(*), intent(OUT) :: out
  integer, intent(IN) :: iproc
  integer, dimension(0:iproc-1), intent(IN) :: dist
  TYPE(DECOMP_INFO), intent(IN) :: decomp

  integer :: i,j,k, m,i1,i2,pos

  do m=0,iproc-1
     if (m==0) then 
        i1 = 1
        i2 = dist(0)
     else
        i1 = i2+1
        i2 = i1+dist(m)-1
     end if

#ifdef SHM
     pos = decomp%z2disp_o(m) + 1
#else
#ifdef EVEN
     pos = m * decomp%z2count + 1
#else
     pos = decomp%z2disp(m) + 1
#endif
#endif

     do k=i1,i2
        do j=1,n2
           do i=1,n1
              out(pos) = in(i,j,k)
              pos = pos + 1
           end do
        end do
     end do
  end do

  return
end subroutine mem_split_zy_complex


subroutine mem_merge_zy_real(in,n1,n2,n3,out,iproc,dist,decomp)

  implicit none

  integer, intent(IN) :: n1,n2,n3
  real(mytype), dimension(*), intent(IN) :: in
  real(mytype), dimension(n1,n2,n3), intent(OUT) :: out
  integer, intent(IN) :: iproc
  integer, dimension(0:iproc-1), intent(IN) :: dist
  TYPE(DECOMP_INFO), intent(IN) :: decomp

  integer :: i,j,k, m,i1,i2, pos

  do m=0,iproc-1
     if (m==0) then
        i1 = 1
        i2 = dist(0)
     else
        i1 = i2+1
        i2 = i1+dist(m)-1
     end if

#ifdef SHM
     pos = decomp%y2disp_o(m) + 1
#else
#ifdef EVEN
     pos = m * decomp%y2count + 1
#else
     pos = decomp%y2disp(m) + 1
#endif
#endif

     do k=1,n3
        do j=i1,i2
           do i=1,n1
              out(i,j,k) = in(pos)
              pos = pos + 1
           end do
        end do
     end do
  end do

  return
end subroutine mem_merge_zy_real


subroutine mem_merge_zy_complex(in,n1,n2,n3,out,iproc,dist,decomp)

  implicit none

  integer, intent(IN) :: n1,n2,n3
  complex(mytype), dimension(*), intent(IN) :: in
  complex(mytype), dimension(n1,n2,n3), intent(OUT) :: out
  integer, intent(IN) :: iproc
  integer, dimension(0:iproc-1), intent(IN) :: dist
  TYPE(DECOMP_INFO), intent(IN) :: decomp

  integer :: i,j,k, m,i1,i2, pos

  do m=0,iproc-1
     if (m==0) then
        i1 = 1
        i2 = dist(0)
     else
        i1 = i2+1
        i2 = i1+dist(m)-1
     end if

#ifdef SHM
     pos = decomp%y2disp_o(m) + 1
#else
#ifdef EVEN
     pos = m * decomp%y2count + 1
#else
     pos = decomp%y2disp(m) + 1
#endif
#endif

     do k=1,n3
        do j=i1,i2
           do i=1,n1
              out(i,j,k) = in(pos)
              pos = pos + 1
           end do
        end do
     end do
  end do

  return
end subroutine mem_merge_zy_complex
