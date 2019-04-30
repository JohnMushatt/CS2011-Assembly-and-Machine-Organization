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
/**
 * Transpose function for assignment
 * @param M Number of columns
 * @param N Number of rows
 * @param A 2D matrix to store initial state
 * @param B 2D matrix to store after transposing
 */
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
	//Check if 32x32 matrix
	if (M == 32 && N == 32) {
		int current;
		//Need diagonal to compute the new tranpose
		int diagonal;
		//Triple for loop is to swap values and flip their position in the matrix
		for (int l = 0; l < 4; l++) {
			for (int k = 0; k < 4; k++) {
				for (int i = 8 * l; i < 8 * l + 8; i++) {
					for (int j = 8 * k; j < 8 * k + 8; j++) {
						if (i != j)
							B[j][i] = A[i][j];
						else {
							current = A[j][j];
							diagonal = j;
						}
					}
					//Put value originally in A into B
					if (k == l)
						B[diagonal][diagonal] = current;
				}
			}
		}
	}
	//if 64x64
	else if (M == 64 && N == 64) {
		int current;
		// Same as above, need to store diagonal
		int diagonal;
		//Have remainder for the last element in the matrix row
		int remainder = 0;
		//8 sets for the 64x64 matrix
		int sets = 8;
		int col1;
		int col2;
		int col3;
		int col4;

		for (int i = 0; i < 64; i += 8) {
			for (int j = 0; j < 64; j += 8) {

				for (int k = 0; k < 4; k++) {
					//Begin transposing process

					//Get the current element + 1
					current = A[i + k][j + 1];
					//Get the diagonal
					diagonal = A[i + k][j + 2];
					//Get the last element
					remainder = A[i + k][j + 3];
					//Recompute the number of sets
					sets = A[i + k][j + 4];
					//Get the 1st column
					col1= A[i + k][j + 5];
					//Get the 2nd column
					col2= A[i + k][j + 6];
					//Get the 3rd column
					col3 = A[i + k][j + 7];
					//Get the 4th col
					col4 = A[i + k][j + 0];
					//Assign values back into the new matrix
					B[j + 3][i + k + 4] = sets;
					B[j + 1][i + k + 0] = current;
					B[j + 2][i + k + 0] = diagonal;
					B[j + 3][i + k + 0] = remainder;

					B[j + 0][i + k + 4] = col1;
					B[j + 1][i + k + 4] = col2;
					B[j + 2][i + k + 4] = col3;
					B[j + 0][i + k + 0] = col4;


				}
				//Get the current element + 1
				current = A[i + 5][j + 4];
				//Get the diagonal value
				diagonal = A[i + 6][j + 4];
				//Get the last element of the row
				remainder = A[i + 7][j + 4];
				//Recompute the sets on this line
				sets = A[i + 4][j + 3];
				//Get the first column
				col1= A[i + 5][j + 3];
				//Get the 2nd column
				col2= A[i + 6][j + 3];
				//get the 3rd column
				col3 = A[i + 7][j + 3];
				//Get the 4th column
				col4 = A[i + 4][j + 4];
				//Assign values to the new Matrix B
				B[j + 4][i + 0] = B[j + 3][i + 4];
				B[j + 4][i + 1] = B[j + 3][i + 5];
				B[j + 4][i + 2] = B[j + 3][i + 6];
				B[j + 4][i + 3] = B[j + 3][i + 7];

				B[j + 3][i + 4] = sets;
				B[j + 4][i + 5] = current;
				B[j + 4][i + 6] = diagonal;
				B[j + 4][i + 7] = remainder;

				B[j + 3][i + 5] = col1;
				B[j + 3][i + 6] = col2;
				B[j + 3][i + 7] = col3;
				B[j + 4][i + 4] = col4;


				for (int k = 0; k < 3; k++) {
					//Recompute sets in the matrix
					sets = A[i + 4][j + k];
					//Get the current element in the element +1
					current = A[i + 5][j + 5 + k];
					//Get the diagonal value in the current row
					diagonal = A[i + 6][j + 5 + k];
					//Get the last element in the row
					remainder = A[i + 7][j + 5 + k];
					//get the 1st column
					col1= A[i + 5][j + k];
					//Get the 2nd column
					col2= A[i + 6][j + k];
					//Get the 3rd column
					col3 = A[i + 7][j + k];
					//Get the 4th column
					col4 = A[i + 4][j + 5 + k];

					//Assign values to new matrix B
					B[j + 5 + k][i + 0] = B[j + k][i + 4];
					B[j + k][i + 4] = sets;
					B[j + 5 + k][i + 1] = B[j + k][i + 5];
					B[j + 5 + k][i + 5] = current;
					B[j + k][i + 5] = col1;
					B[j + 5 + k][i + 2] = B[j + k][i + 6];
					B[j + 5 + k][i + 6] = diagonal;
					B[j + k][i + 6] = col2;
					B[j + 5 + k][i + 3] = B[j + k][i + 7];
					B[j + 5 + k][i + 7] = remainder;
					B[j + k][i + 7] = col3;
					B[j + 5 + k][i + 4] = col4;


				}

			}
		}
	}
	//If non uniform dimensions
	else {
		//based on predetermined block size
		int blocksize = 18;
		for (int row = 0; row < N; row += blocksize) {
			for (int col = 0; col < M; col += blocksize) {
				for (int i = row; i < row + blocksize && i < N; ++i) {
					for (int j = col; j < col + blocksize && j < M; ++j) {
						int matrixElement = A[i][j];
						B[j][i] = matrixElement;
					}
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
void trans(int M, int N, int A[N][M], int B[M][N]) {
	int i, j, current;

	for (i = 0; i < N; i++) {
		for (j = 0; j < M; j++) {
			current = A[i][j];
			B[j][i] = current;
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
void registerFunctions() {
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
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
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

