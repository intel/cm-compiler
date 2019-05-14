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
static const char input_image_message[] = "Path to an 24bit .bmp RGB image.  Default value: nest.bmp";

// @brief message for JPEG output image argument
static const char jpeg_output_image_message[] = "Path to save JPEG output image. Default: no output file";

// @brief message for RGB raw output image argument
static const char rgb_output_image_message[] = "Path to save RGB raw output image. Default: no output file";

// @brief message for max-frames
static const char max_frames_message[] = "Maximum number of frames to run.";

// @brief message for jpeg-quality
static const char jpeg_quality_message[] = "JPEG compression quality, range(1, 100). Default value: 90";


// @brief Define flag for showing help message
DEFINE_bool(h, false, help_message);

// @brief Define parameter for input image file
DEFINE_string(i, "nest.bmp", input_image_message);

// @brief Define parameter for jpeg output image file
DEFINE_string(jpegout, "", jpeg_output_image_message);

// @brief Define parameter for output image file
DEFINE_string(rgbout, "", rgb_output_image_message);

// @brief Define parameter for maximum number of frames to run
// Default is 10
DEFINE_int32(maxframes, 1, max_frames_message);

// @brief Define parameter for maximum number of frames to run
// Default is 90
DEFINE_int32(jpegquality, 90, jpeg_quality_message);

static void showUsage() {
   std::cout << std::endl;
   std::cout << "hw_x64.ScanToJpeg [OPTION]" << std::endl;
   std::cout << "Options:" << std::endl;
   std::cout << std::endl;
   std::cout << "   -h                          " << help_message << std::endl;
   std::cout << "   -i <filename>               " << input_image_message << std::endl;
   std::cout << "   --jpegout <filename>         " << jpeg_output_image_message << std::endl;
   std::cout << "   --rgbout <filename>          " << rgb_output_image_message << std::endl;
   std::cout << "   --maxframes <integer>       " << max_frames_message <<  std::endl;
   std::cout << "   --jpegquality <integer>     " << jpeg_quality_message <<  std::endl;
   std::cout << std::endl;
   std::cout << std::endl;
}

bool ParseCommandLine(int argc, char *argv[])
{
   gflags::ParseCommandLineNonHelpFlags(&argc, &argv, true);
   if (FLAGS_h) {
      showUsage();
      return false;
   }

   if (FLAGS_i.empty()) {
      throw std::logic_error("Parameter -i is not set");
   }

   return true;
}
