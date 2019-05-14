/*
 * Copyright (c) 2017, Intel Corporation
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
#include <gflags/gflags.h>

// @brief message for help argument
static const char help_message[] = "Print a usage message.";

// @brief message for input image argument
static const char input_image_message[] = "Path to an 24bit .bmp RGB image.  Default value: Cold-mountain.bmp";

// @brief message for output image argument
static const char output_image_message[] = "Path to save 24bit skewed RGB .raw output image and deskew bitmap file. Default: no output file";

// @brief message for max-frames
static const char max_frames_message[] = "Maximum number of frames to run.";

//@brief message for sppbits selected
static const char sppbits_message[] = "Only 10 and 12 bits allowed for Gain Offset. Default: 10.";

//@brief message for skew angle
static const char skewangle_message[] = "Skew angle for the Warp Affine. Default: 5 degree";

// @brief Define flag for showing help message
DEFINE_bool(h, false, help_message);

// @brief Define parameter for input image file
DEFINE_string(i, "fruit.bmp", input_image_message);

// @brief Define parameter for output image file
DEFINE_string(o, "", output_image_message);

// @brief Define parameter for maximum number of frames to run
// Default is 10
DEFINE_int32(maxframes, 10, max_frames_message);

// @brief Define flag for sppbits
DEFINE_int32(sppbits, 10, sppbits_message);

// @brief Define flag for skewangle
DEFINE_int32(skewangle, 5, skewangle_message);

static void showUsage() {
   std::cout << std::endl;
   std::cout << "hw_x64.ScanPreProcess [OPTION]" << std::endl;
   std::cout << "Options:" << std::endl;
   std::cout << std::endl;
   std::cout << "   -h                          " << help_message << std::endl;
   std::cout << "   -i <filename>               " << input_image_message << std::endl;
   std::cout << "   -o <filename>               " << output_image_message << std::endl;
   std::cout << "   --maxframes <integer>       " << max_frames_message <<  std::endl;
   std::cout << "   --sppbits <integer>         " << sppbits_message << std::endl;
   std::cout << "   --skewangle <integer>       " << skewangle_message << std::endl;
   std::cout << std::endl;
   std::cout << std::endl;
}

bool ParseCommandLine(int argc, char *argv[])
{
   //gflags::ParseCommandLineNonHelpFlags(&argc, &argv, true);
   google::ParseCommandLineNonHelpFlags(&argc, &argv, true);
   if (FLAGS_h) {
      showUsage();
      return false;
   }

   if (FLAGS_i.empty()) {
      throw std::logic_error("Parameter -i is not set");
   }

   return true;
}
