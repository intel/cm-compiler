/* * Copyright (c) 2020, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// Author: Elaine Wang
#include <string>
#include <chrono>

// The only CM runtime header file that you need is cm_rt.h.
// It includes all of the CM runtime.
#include "rgb_cvt_def.h"
#include "cm_cvt_color.h"

#define IMG_W 1920
#define IMG_H 1080

int GenRandomNum(char *buf)
{
    srand((unsigned int)time(NULL));
    char *p = buf;
    int loop_num = 1920 * 1080 * 4;

    for (int j = 0; j < loop_num; j++) 
    {
        *p = j % 255;
        p++;
    }

#if 0
    if (vector_num == 1)
        return 0;
    else
    {
        repeat_num = vector_num / loop_num;
        std::cout<<"repeat buffer (size "<<loop_num<<" with "<<repeat_num<<" times\n";

        for (int j = 1; j < repeat_num; j++)
        {
            memcpy(p, buf, loop_num * VECTOR_SIZE * sizeof(float));
            p += (VECTOR_SIZE * loop_num);
        }
        return 0;
    }
#endif
}

//b_size is the number of vector and it's equal to output size 
int CalculateByCpu(char *input, char *out_c)
{
    int i, j;
    for (i = 0; i < IMG_H; i++)
    {
        for (j = 0; j < IMG_W; j++)
        {
            out_c[i * IMG_W * 3+ j * 3] = input[i * IMG_W * 4 + j * 4];
            out_c[i * IMG_W * 3+ j * 3 + 1] = input[i * IMG_W * 4 + j * 4 + 1];
            out_c[i * IMG_W * 3+ j * 3 + 2] = input[i * IMG_W * 4 + j * 4 + 2];
        }
    }

    return 0;
}


int CompareOutPuts(char *V1, char *V2)
{
    int count = 0, i;
    for (i = 0; i < IMG_H * IMG_W * 3; i++)
    {
        if ( V1[i] != V2[i])
        {
            ++count;
            if (count < 10)
            {
                printf("Index %d is different : CPU(%d)\t vs. GPU(%d)\n", i,  V1[i], V2[i]);
            }
        }
    }
    printf("\nTotal %d differences!!\n\n", count);
    return count;
}

int AllocInputOutput(char * &input1, char * &output)
{
    input1 = (char*)CM_ALIGNED_MALLOC(sizeof(char) * IMG_W * IMG_H *4, 0x1000);
    output = (char*)CM_ALIGNED_MALLOC(sizeof(char) * IMG_W * IMG_H * 3, 0x1000);
    if (input1 == nullptr || output == nullptr)
    {
        std::cout<<"Ouf of Memory!\n";
        return -1;
    }
    else
        return 0;
}


void dump_vector(char *v, int num)
{
    std::cout<<"Vector [512] = \n";
    for (int i = 0; i < num; i++)
    {
        std::cout<<v[i]<<" ";
        if ((i + 1)%8 == 0)
            std::cout<<"\n";
    }
    std::cout<<"\n";
}

//#define DUMP_DATA
int main(int argc, char* argv[]) {
    int repeat_run = 10;

    char *input1;
    char *output;
    char *cpu_out = (char *)malloc(3 *IMG_W * IMG_H * sizeof(char));
    AllocInputOutput(input1, output);
    std::cout<<"prepar the data....";
    GenRandomNum(input1);
    std::cout<<"....Done"<<"\n";

    CmCvtColor ccc;
    ccc.CreateKernel();
    ccc.CreateOutputBuffer(output);//Create cm buffer from the host memory

    std::chrono::high_resolution_clock::time_point time1,time2;
    time1 = std::chrono::high_resolution_clock::now();
    int loop_num = repeat_run;
    while (loop_num-- > 0)
    {
        ccc.Cvt2Rgb(input1);
    }
    time2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff;
    diff = time2 - time1;
    std::cout << "Everage time cost "<<diff.count()*1000.0 /(float)repeat_run << "ms\n" ;

    CalculateByCpu(input1, cpu_out);
    CompareOutPuts(cpu_out, output);;;;

    CM_ALIGNED_FREE(input1);
    CM_ALIGNED_FREE(output);
    free(cpu_out);
    return 0;
}
