/* uarray2.h
 * Author: Yike Li, Stephanie Xu
 * Date: 02/12/2022
 *
 * Purpose: support a 2-dimensional array data structure, including
 *          functions to create the 2-d array, to access and modify element 
 *          by index, etc.
 *
 */

#ifndef UARRAY2_INCLUDED
#define UARRAY2_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <mem.h>
#include <assert.h>
#include "uarray.h"

#define T UArray2_T
typedef struct T *T;
typedef void UArray2_applyfun(int i, int j, T array2, void *elem, void *cl);
typedef void UArray2_mapfun(T array2, UArray2_applyfun apply, void *cl);


/* UArray2_new
 * Purpose: Allocates, initializes, and returns a new 2-d array
 * Parameters: 1) width: an integer that stores the width of the 2-d array
 *             2) height: an integer that stores the height of the 2-d array
 *             3) size: an integer that stores the size (in bytes)
 *                      of each element
 * Return: a new 2-d array of (width * height) elements that each occupies 
 *         (size) bytes and of bounds on x-axis zero through (width - 1) 
 *         and on y-axis zero through (height-1), unless either width
 *         or height is zero. 
 * 
 * Note: It is a checked runtime error (CRE) for width or height to be  
 *       negative and size to be non-positive.
 */
extern T     UArray2_new(int width, int height, int size);

/* UArray2_free
 * Purpose: deallocates and clears the object of UArray2_T
 * Parameters: the address of an object of type UArray2_T
 * Return: None
 * 
 * Note: It is a CRE for uarray2 or *uarray2 to be null.
 */
extern void  UArray2_free(T *uarray2);

/* UArray2_width
 * Purpose: returns the number of columns in the 2-d array
 * Parameters: an object of type UArray2_T
 * Return: an integer that stores the number of columns in the 2-d array
 * 
 * Note: It is a CRE for uarray2 to be null.
 */
extern int   UArray2_width(T uarray2);

/* UArray2_height
 * Purpose: returns the number of rows in the 2-d array
 * Parameters: an object of type UArray2_T
 * Return: an integer that stores the number of rows in the 2-d array
 * 
 * Note: It is a CRE for uarray2 to be null.
 */
extern int   UArray2_height(T uarray2);

/* UArray2_size
 * Purpose: returns the size (in bytes) of each element in 2-d array
 * Parameters: an object of type UArray2_T
 * Return: the size (in bytes) of each element in 2-d array
 *
 * Note: It is a CRE for uarray2 to be null.
 */
extern int   UArray2_size(T uarray2);

/* UArray2_at
 * Purpose: for accessing the element at the given index (col, row) 
 *          in the 2-d array
 * Parameters: 1) uarray2: an object of UArray2_T type
 *             2) col: an integer of the column index
 *             3) row: an integer of the row index
 * Return: the address of the element at the index (col, row) in the 2-d array
 * 
 * Note: It is a CRE for uarray2 to be null, or the value of col and row to 
 *       be out of range of [0, width - 1] or [0, height - 1].
 */
extern void *UArray2_at(T uarray2, int col, int row);

/* UArray2_map_row_major
 * Purpose: call the apply function on each element in a 2-d array, beginning
 *          at index (0,0), followed by a row major order
 * Parameters: 1) uarray2: an object of UArray2_T type
 *             2) apply(): an address of the function to be applied on 
 *                each element in 2-D array 
 *             3) cl: an address to store the return value of the applied 
 *                function
 * Return: none
 * 
 * Note: It is a CRE for uarray2 to be null, or the applied function's 
 *       arguments is incompatible with the expected format
 */
extern void  UArray2_map_row_major(T uarray2, UArray2_applyfun apply, void *cl);

/* UArray2_map_col_major
 * Purpose: call the apply function on each element in a 2-d array, beginning
 *          at index (0,0), followed by a column major order
 * Parameters: 1) uarray2: an object of UArray2_T type
 *             2) apply(): an address of the function to be applied on 
 *                each element in 2-D array 
 *             3) cl: an address to store the return value of the applied 
 *                function
 * Return: none
 * 
 * Note: It is a CRE for uarray2 to be null, or the applied function's 
 *       arguments is incompatible with the expected format
 */
extern void  UArray2_map_col_major(T uarray2, UArray2_applyfun apply, void *cl);

#undef T
#endif /* UARRAY2 */