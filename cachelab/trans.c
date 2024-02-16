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
	if(N == 32)
	{
		for(int i=0;i<N;i+=8)
		{
			for(int j=0;j<M;j+=8)
			{
				for(int k=i;k<i+8;k++)
				{
					int a_0 = A[k][j];
					int a_1 = A[k][j+1];
					int a_2 = A[k][j+2];
					int a_3 = A[k][j+3];
					int a_4 = A[k][j+4];
					int a_5 = A[k][j+5];
					int a_6 = A[k][j+6];
					int a_7 = A[k][j+7];

					B[j][k] = a_0;
					B[j+1][k] = a_1;
					B[j+2][k] = a_2;
					B[j+3][k] = a_3;
					B[j+4][k] = a_4;
					B[j+5][k] = a_5;
					B[j+6][k] = a_6;
					B[j+7][k] = a_7;
				}
			}
		}
	}
	else if(N == 64)
	{
		for(int i=0;i<N;i+=8)
		{
			for(int j=0;j<M;j+=8)
			{
				for(int k=i;k<i+4;k++)
				{
					int a_0 = A[k][j];
					int a_1 = A[k][j+1];
					int a_2 = A[k][j+2];
					int a_3 = A[k][j+3];
					int a_4 = A[k][j+4];
					int a_5 = A[k][j+5];
					int a_6 = A[k][j+6];
					int a_7 = A[k][j+7];

					B[j][k] = a_0;
					B[j+1][k] = a_1;
					B[j+2][k] = a_2;
					B[j+3][k] = a_3;
					B[j][k+4] = a_7;
					B[j+1][k+4] = a_6;
					B[j+2][k+4] = a_5;
					B[j+3][k+4] = a_4;
				}
				for(int l=0;l<4;l++)
				{
					int a_0 = A[i+4][j+3-l];
					int a_1 = A[i+5][j+3-l];
					int a_2 = A[i+6][j+3-l];
					int a_3 = A[i+7][j+3-l];
					int a_4 = A[i+4][j+4+l];
					int a_5 = A[i+5][j+4+l];
					int a_6 = A[i+6][j+4+l];
					int a_7 = A[i+7][j+4+l];

					B[j+4+l][i] = B[j+3-l][i+4];
					B[j+4+l][i+1] = B[j+3-l][i+5];
					B[j+4+l][i+2] = B[j+3-l][i+6];
					B[j+4+l][i+3] = B[j+3-l][i+7];

					B[j+3-l][i+4] = a_0;
					B[j+3-l][i+5] = a_1;
					B[j+3-l][i+6] = a_2;
					B[j+3-l][i+7] = a_3;
					B[j+4+l][i+4] = a_4;
					B[j+4+l][i+5] = a_5;
					B[j+4+l][i+6] = a_6;
					B[j+4+l][i+7] = a_7;
				}
			}
		}
	}
	else
	{
		for(int i=0;i<N;i+=16)
		{
			for(int j=0;j<M;j+=16)
			{
				for(int k=i;k<i+16 && k<N;k++)
				{
					int tmp_pos = -1;
					int tmp_val = 0;
					int l;
					for(l = j;l<j+16 && l<M;l++)
					{
						if(l == k)
						{
							tmp_pos = k;
							tmp_val = A[k][k];
						}
						else
							B[l][k] = A[k][l];
					}
					if(tmp_pos != -1)
						B[tmp_pos][tmp_pos] = tmp_val;
				}
			}
		}
	}
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
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
    registerTransFunction(trans, trans_desc); 

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

