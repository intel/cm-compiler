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

// Compare the top half image of f_out_str with image of f_gold_str1, and
// compare the second half image of f_out_str with f_gold_str2.
// Function returns true if max per-pixel difference between two bmp files
// is less than tolerance.
template<typename type1>
bool CompareBmp(char *f_out_str,
                char *f_gold_str1,
                char *f_gold_str2,
                type1 tolerance) {
  unsigned char header_out[54];
  unsigned char header_gold[54];

  FILE *f_out = fopen(f_out_str, "rb");
  if (f_out == nullptr) {
    perror(f_out_str);
    return false;
  }
  if (fread(header_out, 1, 54, f_out) != 54) {
    perror(f_out_str);
    fclose(f_out);
    return false;
  }

  FILE *f_gold1 = fopen(f_gold_str1, "rb");
  if (f_gold1 == nullptr) {
    perror(f_gold_str1);
    fclose(f_out);
    return false;
  }
  if (fread(header_gold, 1, 54, f_gold1) != 54) {
    perror(f_gold_str1);
    fclose(f_out);
    fclose(f_gold1);
    return false;
  }

  if (header_out[18] != header_gold[18] ||
      header_out[22] != header_gold[22]) {
    fclose(f_out);
    fclose(f_gold1);
    return false;
  }

  FILE *f_gold2 = fopen(f_gold_str2, "rb");
  if (f_gold2 == nullptr) {
    perror(f_gold_str2);
    fclose(f_out);
    fclose(f_gold1);
    return false;
  }
  if (fread(header_gold, 1, 54, f_gold2) != 54) {
    perror(f_gold_str2);
    fclose(f_out);
    fclose(f_gold1);
    fclose(f_gold2);
    return false;
  }

  if (header_out[18] != header_gold[18] ||
      header_out[22] != header_gold[22]) {
    fclose(f_out);
    fclose(f_gold1);
    fclose(f_gold2);
    return false;
  }

  unsigned int width = abs(*reinterpret_cast<short *>(&header_out[18]));
  unsigned int height = abs(*reinterpret_cast<short *>(&header_out[22]));

  unsigned char *img_out = new unsigned char[width*height*3];
  unsigned char *img_gold1 = new unsigned char[width*height*3];
  unsigned char *img_gold2 = new unsigned char[width*height*3];

  if (fread(img_out, 1, width*height*3, f_out) != width*height*3) {
    perror(f_out_str);
    fclose(f_out);
    fclose(f_gold1);
    fclose(f_gold2);
    delete[] img_out;
    delete[] img_gold1;
    delete[] img_gold2;
    return false;
  }

  if (fread(img_gold1, 1, width*height*3, f_gold1) != width*height*3) {
    perror(f_gold_str1);
    fclose(f_out);
    fclose(f_gold1);
    fclose(f_gold2);
    delete[] img_out;
    delete[] img_gold1;
    delete[] img_gold2;
    return false;
  }

  if (fread(img_gold2, 1, width*height*3, f_gold2) != width*height*3) {
    perror(f_gold_str2);
    fclose(f_out);
    fclose(f_gold1);
    fclose(f_gold2);
    delete[] img_out;
    delete[] img_gold1;
    delete[] img_gold2;
    return false;
  }

  fclose(f_out);
  fclose(f_gold1);
  fclose(f_gold2);

  for (unsigned int i = 0; i < height/2; i++) {
    for (unsigned int j = 0; j < width*3; j++) {
      if (abs(img_out[i*width*3 + j] - img_gold1[i*width*3 + j]) > tolerance) {
        delete[] img_out;
        delete[] img_gold1;
        delete[] img_gold2;
        return false;
      }
      if (abs(img_out[(i + height/2)*width*3 + j] -
              img_gold2[(i + height/2)*width*3 + j]) > tolerance) {
        delete[] img_out;
        delete[] img_gold1;
        delete[] img_gold2;
        return false;
      }
    }
  }

  delete[] img_out;
  delete[] img_gold1;
  delete[] img_gold2;

  return true;
}
