/* Copyright (C) The Scalable Software Infrastructure Project. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the name of the project nor the names of its contributors 
      may be used to endorse or promote products derived from this software 
      without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE SCALABLE SOFTWARE INFRASTRUCTURE PROJECT
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SCALABLE SOFTWARE INFRASTRUCTURE
   PROJECT BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
   OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
        #include "lis_config.h"
#else
#ifdef HAVE_CONFIG_WIN_H
        #include "lis_config_win.h"
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lis.h"

#undef __FUNC__
#define __FUNC__ "main"
LIS_INT main(LIS_INT argc, char* argv[])
{
  LIS_INT           err;
  LIS_INT           nprocs,mtype,my_rank;
  int               int_nprocs,int_my_rank;
  LIS_INT           nsol;
  LIS_MATRIX        A,A0,B;
  LIS_VECTOR        x,y;
  LIS_REAL          evalue0;
  LIS_SCALAR        shift, *tmpa;
  LIS_ESOLVER       esolver;
  LIS_SOLVER        solver;
  LIS_REAL          residual;
  LIS_INT           iters;
  double            times;
  double      itimes,ptimes,p_c_times,p_i_times;
  LIS_PRECON        precon;
  LIS_INT      *ptr,*index;
  LIS_SCALAR      *value;
  LIS_INT           nsolver;
  char              esolvername[128];
  LIS_INT           mode;

  LIS_DEBUG_FUNC_IN;
    
  lis_initialize(&argc, &argv);

#ifdef USE_MPI
  MPI_Comm_size(MPI_COMM_WORLD,&int_nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&int_my_rank);
  nprocs = int_nprocs;
  my_rank = int_my_rank;
#else
  nprocs  = 1;
  my_rank = 0;
#endif
    
  if( argc < 4 )
    {
      if( my_rank==0 ) 
  {
    printf("Usage: %s matrix_filename eigenvalue_filename eigenvector_filename [options]\n", argv[0]);
  }
      CHKERR(1);
    }

  if( my_rank==0 )
    {
      printf("\n");
      printf("number of processes = %d\n",nprocs);
    }

#ifdef _OPENMP
  if( my_rank==0 )
    {
      printf("max number of threads = %d\n",omp_get_num_procs());
      printf("number of threads = %d\n",omp_get_max_threads());
    }
#endif
    
  /* create matrix and vectors */
  lis_matrix_create(LIS_COMM_WORLD,&A);
  lis_input_matrix(A,argv[1]);
  lis_vector_duplicate(A,&x);
  lis_esolver_create(&esolver);
  lis_esolver_set_option("-e si -ss 1 -eprint mem",esolver);
  lis_esolver_set_optionC(esolver);
  lis_esolve(A, x, &evalue0, esolver);
  lis_esolver_get_esolver(esolver,&nsol);
  lis_esolver_get_esolvername(nsol,esolvername);
  lis_esolver_get_residualnorm(esolver, &residual);
  lis_esolver_get_iters(esolver, &iters);
  lis_esolver_get_timeex(esolver,&times,&itimes,&ptimes,&p_c_times,&p_i_times);
  if( my_rank==0 ) {
    printf("%s: mode number              = %d\n", esolvername, 0);
#ifdef _LONG__DOUBLE
    printf("%s: eigenvalue               = %Le\n", esolvername, evalue0);
#else
    printf("%s: eigenvalue               = %e\n", esolvername, evalue0);
#endif
#ifdef _LONGLONG
    printf("%s: number of iterations     = %lld\n",esolvername, iters);
#else
    printf("%s: number of iterations     = %d\n",esolvername, iters);
#endif
    printf("%s: elapsed time             = %e sec.\n", esolvername, times);
    printf("%s:   preconditioner         = %e sec.\n", esolvername, ptimes);
    printf("%s:     matrix creation      = %e sec.\n", esolvername, p_c_times);
    printf("%s:   linear solver          = %e sec.\n", esolvername, itimes);
#ifdef _LONG__DOUBLE
    printf("%s: relative residual 2-norm = %Le\n\n",esolvername, residual);
#else
    printf("%s: relative residual 2-norm = %e\n\n",esolvername, residual);
#endif
  }

  lis_vector_create(LIS_COMM_WORLD,&y);
  lis_matrix_create(LIS_COMM_WORLD,&B);
  lis_esolver_get_evalues(esolver,y);
  lis_esolver_get_evectors(esolver,B);

  /* output eigenvalues */
  lis_output_vector(y,LIS_FMT_MM,argv[2]);

  /* output eigenvectors */
  lis_output_matrix(B,LIS_FMT_MM,argv[3]);

  lis_esolver_destroy(esolver);
  lis_matrix_destroy(A);
  lis_vector_destroy(x);
  lis_matrix_destroy(B);
  lis_vector_destroy(y);

  lis_finalize();

  LIS_DEBUG_FUNC_OUT;

  return 0;
}

 
