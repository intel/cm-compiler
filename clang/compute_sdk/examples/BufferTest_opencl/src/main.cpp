/*========================== begin_copyright_notice ============================

Copyright (C) 2009-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#define __CL_ENABLE_EXCEPTIONS
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#include <iostream>
#include <ctime>
#include <limits>
#include <cmath>
#include <math.h>
#include <CL/cl.hpp>
#include "basic.hpp"
#include "utils.h"
#include "oclobject.hpp"
#include "cmdparser.hpp"

// #include <windows.h>

using namespace std;

static string GetDeviceString(cl::Device device);
static bool SupportsOpenCL2(cl::Device device);
double totalTime( cl_event relEvent );

unsigned char *LoadImageAsBMP( const std::wstring& file_name, cl_int2 &size )
{
    const unsigned int bit32 = 32;
    const unsigned int bit24 = 24;
    unsigned int bytesPerPixel = 0;
    // read whole file
    std::vector<char>    input_data;
    readFile( file_name, input_data );

    // offset of the start for data
    int data_offset = *(int*)&input_data[10];
    // get image size
    size = *(cl_int2*)&input_data[18];
    // get image bits per pixel
    cl_short bits = *(cl_short*)&input_data[28];

    // calc pixel_pitch as width for 32bit images
    int pixel_pitch = size.s[0];

    if( bits != bit32 && bits != bit24 )
    {
        throw Error( "Bad format of " + wstringToString( file_name ) + " BMP file. Only 32 and 24 bits per pixel are supported" );
    }

    bytesPerPixel = bits / 8;
    if( size.s[0] * size.s[1] * bytesPerPixel + data_offset > (int)input_data.size() )
    {
        throw Error( "Could not read " + wstringToString( file_name ) + " BMP file." );
    }
    // prepare storage for pixels
    unsigned char *p = new unsigned char[size.s[1] * pixel_pitch * bytesPerPixel];
    // convert BMP ABGR pixels into RGBA and
    // bottom-top format into top-bottom
    for( int y = 0; y<size.s[1]; ++y )
    {
        for( int x = 0; x < size.s[0]; ++x )
        {
            unsigned char* pSrc = (unsigned char*)&input_data[data_offset + ( y * size.s[0] + x ) * bytesPerPixel];
            unsigned char* pDst = p + bytesPerPixel * ( y*pixel_pitch + x );
            pDst[0] = pSrc[1];
            pDst[1] = pSrc[2];
            pDst[2] = pSrc[3];
            if( bits == 32 )
            {
                pDst[3] = pSrc[0];
            }
        }
    }
    return p;
};

void run_box_blur_Kernel(OpenCLBasic& oclobjects, string inputFile1, string outputFile, int radius, float factor)
{
	// Define the Variables
	cl_int          err = 0;            // opencl error code
	cl_mem          input1;             // input image2d image
	cl_mem          outputImage;        // output buffer that contains processed image
	cl_image_format format;             // structure to define image format
	cl_image_format intermediate;       // structure to define image format
	cl_image_desc   desc;               // structure to define image description
	cl_ulong        start = 0, end = 0; // Variables for Time Measurement
	double          perf_start = 0, perf_stop = 0; // Variables for Time Measurement
	double          timeFor20Blur = 0.0, timeForBlur = 0.0; // Variables for Time Measurement
	double          timeFor20BoxBlur = 0.0; // Variables for Time Measurement

	// HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	// CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	// WORD saved_attributes;

	// /* Save current attributes */
	// GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
	// saved_attributes = consoleInfo.wAttributes;

	// Define the Variables
	bool is2Device = SupportsOpenCL2(cl::Device(oclobjects.device));
	if (!is2Device)
	{
		// SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
		printf("\nThis Device does not Support OpenCL 2.0+. Quit BoxBlur\n");
		/* Restore original attributes */
		// SetConsoleTextAttribute(hConsole, saved_attributes);
	}

    //Load the the 24/32 bit Image
	//input_data1 = LoadImageAsBMP(wcstring1, image_size);
	//STEP 1:
	//SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
	printf("\nStarting OpenCL 2.0 implementation\n");

    unsigned width = 128*1024*1024;

    unsigned char *surf0;
    unsigned char *surf1;
    unsigned char *surf2;


    surf0 = new unsigned char[width];// (unsigned char*)malloc(width);
    surf1 = new unsigned char[width]; // (unsigned char*)malloc(width);
    surf2 = new unsigned char[width]; // (unsigned char*)malloc(width);

    for (int i = 0; i < width; i++)
    {
        surf0[i] = 0x55;
        surf1[i] = 0xaa;
        surf2[i] = 0x55;
    }

    //Build the opencl and build the kernel
	OpenCLProgramOneKernel BoxBlur(
		oclobjects, // opencl objects like platform, device, context and queue
		L"BufferTest_genx",
		"",
		"BufferTest", //"BoxBlur_Image"
		"-cmc");

    // Create buffer from bitmap image, and empty buffer to receive output
    cl_int		clErr;
    cl_mem cl_InBuf = clCreateBuffer(oclobjects.context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, width, surf0, &clErr);
    cl_mem cl_OutBuf = clCreateBuffer(oclobjects.context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, width, surf1, &clErr);

    // execute box-blur
	{
		cl_event    BoxBlurExecution = NULL; // event to measure execution time
		size_t      globalsize[2] = { width, 1 };  //Six pixel per-work_item
		size_t      localsize[2] = { 1, 1 };  //?
		//setup the kernel argument
		err = clSetKernelArg(BoxBlur.kernel, 0, sizeof(cl_mem), &cl_InBuf);
		SAMPLE_CHECK_ERRORS(err);
		err = clSetKernelArg(BoxBlur.kernel, 1, sizeof(cl_mem), &cl_OutBuf);
		SAMPLE_CHECK_ERRORS(err);
        SAMPLE_CHECK_ERRORS(err);
        perf_start = time_stamp();
		// submit kernel to execute
		err = clEnqueueNDRangeKernel(oclobjects.queue, BoxBlur.kernel, 2, NULL, globalsize, localsize, 0, NULL, &BoxBlurExecution);
		SAMPLE_CHECK_ERRORS(err);
		err = clWaitForEvents(1, &BoxBlurExecution);
		SAMPLE_CHECK_ERRORS(err);

		perf_stop = time_stamp();

		timeFor20Blur = (perf_stop - perf_start) * 1000;

		//Profiling information: To use the profiling function in OCL, and compare the values uncomment the code below
		clGetEventProfilingInfo( BoxBlurExecution, CL_PROFILING_COMMAND_START, sizeof( start ), &start, NULL );
		clGetEventProfilingInfo( BoxBlurExecution, CL_PROFILING_COMMAND_END, sizeof( end ), &end, NULL );
		timeFor20BoxBlur = (cl_double)( end - start )*(cl_double)( 1e-06 );
		err = clReleaseEvent(BoxBlurExecution);
		SAMPLE_CHECK_ERRORS(err);
	}

    {
         clEnqueueReadBuffer(oclobjects.queue, cl_OutBuf, CL_TRUE, 0,
            width, surf1, 0, NULL, NULL);
    }
    {
        if (!memcmp(surf1, surf2, width))
        {
            printf("PASSED\n");
        }
        else
        {
            for (int i = 0; i < width; i++)
            {
                if (surf1[i] == surf2[i])
                {
                    //printf("(%d: output: %x, expect: %x)\t", i, surf1[i], surf2[i]);
					printf("(%d)\t", i);
                }
            }

            printf("FAILED\n");
            throw std::runtime_error("failed");
        }
    }

	//Release the various Buffers
    delete[] surf0;
    delete[] surf1;
    clReleaseMemObject(cl_InBuf);
	clReleaseMemObject(cl_OutBuf);
}

int main(int argc, const char** argv)
{
    string inputFile1 = "";
    string inputFile2 = "";
    string outputFile = "";
    float alpha = 0.0;
    string temp = "";
    int gamma = 0;
    try
    {
        // Define and parse command-line arguments.
        CmdParserCommon cmd( argc, argv );
        cmd.parse();
        // Immediatly exit if user wanted to see the usage information only.
        if( cmd.help.isSet() )
        {
            return EXIT_SUCCESS;
        }

        // Create the necessary OpenCL objects up to device queue.
        // we use CL_QUEUE_PROFILING_ENABLE to be able geting kernel execution time to comapre sRGBA and RGBA performance
        OpenCLBasic oclobjects(
            cmd.platform.getValue(),
            cmd.device_type.getValue(),
            cmd.device.getValue(),
            CL_QUEUE_PROFILING_ENABLE );

        temp = cmd.alpha.getValue();
        alpha = (float)atof( temp.c_str() );
        if( alpha < 0.0 || alpha > 1.0 ){
            printf( "\nFAILURE: Value of alpha must be between 0 and 1. \nValue of alpha %.6f", alpha );
            return EXIT_FAILURE;
        }

        inputFile1 = cmd.input1_Filename.getValue();
        if( inputFile1.substr( inputFile1.find_last_of( "." ) + 1 ) != "bmp" )
        {
            printf( "\nFAILURE: Expected input file format is a 32-bit bmp file" );
            return EXIT_FAILURE;
        }

        inputFile2 = cmd.input2_Filename.getValue();
        if( inputFile2.substr( inputFile2.find_last_of( "." ) + 1 ) != "bmp" )
        {
            printf( "\nFAILURE: Expected input file format is a 32-bit bmp file" );
            return EXIT_FAILURE;
        }

        outputFile = cmd.output_Filename.getValue();
        if( outputFile.substr( outputFile.find_last_of( "." ) + 1 ) != "bmp" )
        {
            printf( "\nWARNING: Expected output file format is a 32-bit bmp file." );
        }

		// run picture processing using RGBA image format and print execution time
		printf( "\nRun Box Blur on input Image1...\n" );
	    run_box_blur_Kernel( oclobjects, inputFile1, outputFile, 3, 0.1111f );

		return EXIT_SUCCESS;

    }
    catch( const Error& error )
    {
        cerr << "[ ERROR ] Sample application specific error: " << error.what() << "\n";
        return EXIT_FAILURE;
    }
    catch( const exception& error )
    {
        cerr << "[ ERROR ] " << error.what() << "\n";
        return EXIT_FAILURE;
    }
    catch( ... )
    {
        cerr << "[ ERROR ] Unknown/internal error happened.\n";
        return EXIT_FAILURE;
    }
}

string GetDeviceString(cl::Device device)
{
    string device_string;
    device_string += string("Device: ") +
                     device.getInfo<CL_DEVICE_NAME>() +
                     string("\nVendor: ") +
                     device.getInfo<CL_DEVICE_VENDOR>();
    return device_string;
}

bool SupportsOpenCL2(cl::Device device)
{
    // Checks if the device supports OpenCL 2.0
    string dev_version = device.getInfo<CL_DEVICE_VERSION>();
    // The format of the version string is defined in the spec as
    // "OpenCL <major>.<minor> <vendor-specific>"
    dev_version = dev_version.substr(string("OpenCL ").length());
    dev_version = dev_version.substr(0, dev_version.find('.'));

    string lang_version = device.getInfo<CL_DEVICE_OPENCL_C_VERSION>();
    // The format of the version string is defined in the spec as
    // "OpenCL C <major>.<minor> <vendor-specific>"
    lang_version = lang_version.substr(string("OpenCL C ").length());
    lang_version = lang_version.substr(0, lang_version.find('.'));

    return (stoi(dev_version) >= 2 && stoi(lang_version) >= 2);
}

cl::Device GetDevice()
{
    // Find the GPU device if there is one.  Otherwise, use the CPU.
    cl::Platform platform = platform.getDefault();
    vector<cl::Device> devices;
    try
    {
        platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    }
    catch (cl::Error)
    {
        platform.getDevices(CL_DEVICE_TYPE_CPU, &devices);
    }
    return devices[0];
}
