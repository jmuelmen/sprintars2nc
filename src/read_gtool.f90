subroutine open_sprintars ( fname_c, err )  bind ( C )
  USE ISO_C_BINDING
  
  character (kind=c_char, len=1), dimension (1024), intent (in) :: fname_c
  character (len=1024) :: fname_fortran
  integer (kind = c_int), intent(out) :: err
  integer :: i
  
  err = z'c0ffee'

  fname_fortran = " "

  loop_string: do i=1, 1024
     if ( fname_c (i) == c_null_char ) then
        exit loop_string
     else
        fname_fortran (i:i) = fname_c (i)
     end if
  end do loop_string
   
  write(*,*) TRIM(fname_fortran)

  open (11, file=TRIM(fname_fortran), form='unformatted', status='old', &
       convert = 'big_endian', err = 8, iostat = err)

  return
  
8 write( *, * ) 'i/o error # ', err, ', on input file' 
  stop 

end subroutine open_sprintars


subroutine read_sprintars_tstep_3d (buffer, idim, jdim, kdim, &
     eof, err)  bind ( C )
  USE ISO_C_BINDING

  integer (kind = c_int), intent(in)  :: idim, jdim, kdim
  integer (kind = c_int), intent(out) :: err, eof
  real (kind=c_float), dimension (idim * jdim * kdim), intent (out) :: buffer

  character head*1024

  real, allocatable :: sdat(:,:,:)

  err = z'c0ffee'

  allocate(sdat(idim, jdim, kdim))
  read (11, err = 8, end = 9, iostat = err) head
  read (11, err = 8, end = 9, iostat = err) sdat

  do i = 1, idim
     do j = 1, jdim
        do k = 1, kdim
           buffer(((i - 1) * jdim + (j - 1)) * kdim + k) = sdat(i, j, k)
        end do
     end do
  end do

  deallocate(sdat)
  eof = 0
  return
        
8 write( *, * ) 'i/o error # ', err, ' on input file' 
  stop 
9 eof = 1
  return 

end subroutine read_sprintars_tstep_3d

! subroutine read_sprintars (idim, jdim, kdim )
!   integer, intent(in)  :: idim,jdim,kdim
! !  real (kind=c_float), dimension (idim * jdim *kdim), intent (out) :: buffer

!   character head*1024
!   real, allocatable :: sdat(:,:,:)
!   real sdat2(640,320,57)

!   write(*,*) 'hello'
!   write(*,*) idim

!   allocate(sdat(idim, jdim, kdim))
!   write(*,*) 'hello'
!   read (11) head
!   read (11) sdat2
!   write(*,*) head
!   write(*,*) 'goodbye'
! end subroutine read_sprintars

! program test

!   open (11,file='frainl_3hr',form='unformatted',status='old')
!   call read_sprintars(1,2,3)

! !   character frain*1024
! !   character fsnow*1024
! !   character fps*1024

! !   integer idim,jdim,kdim,ddim
! !   parameter (idim=640,jdim=320,kdim=57,ddim=1)
! !   real      rain(idim,jdim,kdim),snow(idim,jdim,kdim),ps(idim,jdim)

! !   frain = 'frainl_3hr'
! !   fsnow = 'fsnowl_3hr'
! !   fps = 'ps_3hr'

! !   call open_sprintars(frain, fsnow, fps)
! !   call read_sprintars(rain, snow, ps)
! !   call read_sprintars(rain, snow, ps)

! !   write(*,*) ps

! end program test
