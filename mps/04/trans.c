/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
  int rows, cols, block;

  if (M == 32 && N == 32) {
    int i, j, temp1, temp2;
    block = 8;

    for (rows = 0; rows < N; rows += block) {
      for (cols = 0; cols < M; cols += block) {
        for (j = cols; j < cols + block; j++) {
          for (i = rows; i < rows + block; i++) {
            if (i == j) {
              temp1 = A[i][i]; //store to temp1 to reduce missing
              temp2 = i; //Save the position at the diagonal
            } else {
              B[i][j] = A[j][i];
            }
          }
          if (rows == cols) {
            B[temp2][temp2] = temp1;
          }
        }
      }
    }
  }

  if (M == 64 && N == 64) {
    int i;
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    block = 8;
    
    for (rows = 0; rows < 64; rows += block) {
      for (cols = 0; cols < 64; cols += block) {
        for (i = 0; i < block; i++) {
          if (i == 0) {
            temp5 = A[rows][cols+4];
            temp6 = A[rows][cols+5];
            temp7 = A[rows][cols+6];
            temp8 = A[rows][cols+7];          
          }
          temp1 = A[rows+i][cols];
          temp2 = A[rows+i][cols+1];
          temp3 = A[rows+i][cols+2];
          temp4 = A[rows+i][cols+3];
          B[cols][rows+i] = temp1;
          B[cols][rows+i+64] = temp2;
          B[cols][rows+i+128] = temp3;
          B[cols][rows+i+192] = temp4;
        }
        for (i = block - 1; i > 0; i--) {
          temp1 = A[rows+i][cols+4];
          temp2 = A[rows+i][cols+5];
          temp3 = A[rows+i][cols+6];
          temp4 = A[rows+i][cols+7];
          B[cols+4][rows+i] = temp1;
          B[cols+4][rows+i+64] = temp2;
          B[cols+4][rows+i+128] = temp3;
          B[cols+4][rows+i+192] = temp4;
        }
        B[cols+4][rows] = temp5;
        B[cols+4][rows+64] = temp6;
        B[cols+4][rows+128] = temp7;
        B[cols+4][rows+192] = temp8;
      }
    }
  }
  
  if (M == 61 && N == 67) {
    int i, j;
    block = 16;
    
    for (rows = 0; rows < N; rows += block) {
      for (cols = 0; cols < M; cols += block) {
        for (j = 0; cols + j  <  M && j < block; j++) {
          for (i = 0; rows + i < N && i < block; i++) {
            B[cols+j][rows+i] = A[rows+i][cols+j];
          }
        }
      }
    }
  }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    /* registerTransFunction(trans, trans_desc);  */

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

