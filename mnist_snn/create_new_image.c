#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mnist.h"
#include "parameters.h"

void create(int num)
{
    int i, j;

    FILE *actual_image;
    actual_image = fopen("./apollo3p_mnist/src/actual_image.h", "wb");

    fprintf(actual_image, "#ifndef __ACTUAL_IMAGE_H__\n");
    fprintf(actual_image, "#define __ACTUAL_IMAGE_H__\n\n");

    fprintf(actual_image, "int actual_label = %d;\n\n", test_label[num]);
    fprintf(actual_image, "int actual_img[28][28]={\n");
    for (i = 0; i < 28; i++)
    {
        fprintf(actual_image, "{");
        for (j = 0; j < 28; j++)
        {
            fprintf(actual_image, "%d", (int)test_image[num][(i * 28) + j]);
            if(j !=  27){
                fprintf(actual_image, ",");
            }
        }
        fprintf(actual_image, "},\n");
    }
    fprintf(actual_image, "};\n\n");

    fprintf(actual_image, "#endif\n");

    fclose(actual_image);
}

int main()
{
    load_mnist();

    srand(time(0));
    int random_number = rand() % NUM_TEST + 1;

    create(random_number);

    return 0;
}
