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

#include "CPipeline.h"
#include "CmdParser.h"

using namespace std;

int main(int argc, char* argv[])
{
   CmTask *copyTask = NULL;

   if (!ParseCommandLine(argc, argv))
      return 0;

   CopyPipeline *cpipeline = new CopyPipeline();

   cpipeline->Init();

   if (cpipeline->GetInputImage(FLAGS_i.c_str()) < 0)
   {
      throw std::runtime_error(std::string("Error in GetInputImage - input file not found"));
   }

   cpipeline->SetLightnessContrast(FLAGS_lightness, FLAGS_contrast);
   cpipeline->AssemblerHigh6Graph(copyTask);
   cpipeline->ExecuteGraph(copyTask, FLAGS_maxframes);

   if (!FLAGS_o.empty())
   {
      cpipeline->SaveOutputImage(FLAGS_o.c_str());
   }

	return 0;
}
