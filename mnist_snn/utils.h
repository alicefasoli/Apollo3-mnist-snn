#ifndef __UTILS_H__
#define __UTILS_H__

#include "parameters.h"

double min(double matrix[N_FIRST_LAYER])
{
    double min = matrix[0];
    for (int i = 0; i < N_FIRST_LAYER; i++)
    {
        if (min > matrix[i])
        {
            min = matrix[i];
        }
    }
    return min;
}

double max(double matrix[N_FIRST_LAYER])
{
    double max = matrix[0];
    for (int i = 0; i < N_FIRST_LAYER; i++)
    {
        if (max < matrix[i])
        {
            max = matrix[i];
        }
    }
    return max;
}

double interp(double x, double xp[2], double fp[2])
{
    double x_interpoled;

    double yi = fp[0] + ((x - xp[0]) / (xp[1] - xp[0])) * (fp[1] - fp[0]);
    if (yi < fp[0])
    {
        x_interpoled = fp[0];
    }
    else if (yi > fp[1])
    {
        x_interpoled = fp[1];
    }
    else
    {
        x_interpoled = yi;
    }

    return x_interpoled;
}

// Returns the indices of the maximum values along an axis
int argmax(double *list, int lenList)
{
    int index = 0;
    double max = list[0];
    for (int i = 1; i < lenList; i++)
    {
        if (max < list[i])
        {
            max = list[i];
            index = i;
        }
    }
    return index;
}

// 1-D array dot product
// double dotProduct1D(double first[N_SECOND_LAYER][N_FIRST_LAYER], int row, double second[N_FIRST_LAYER][t + 1], int col)
// {
//     double dotProduct = 0.0;
//     for (int i = 0; i < N_FIRST_LAYER; i++)
//     {
//         dotProduct = dotProduct + (first[row][i] * second[i][col]);
//     }

//     return dotProduct;
// }

#endif
