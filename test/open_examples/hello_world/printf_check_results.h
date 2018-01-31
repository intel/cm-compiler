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

// Compares the printf output to the expected output.
// The order of the threads' output is non deterministic and cannot be
// guaranteed; therefore, we check that all the known thread IDs have printed
// the message, that there are no extra thread IDs and that the message
// printed matches the expected output for each thread.
bool CheckPrintfResults(const char * filename) {

    std::string golden_string("   Hello from GPU land");

    // Opens the printf results file
    FILE *printf_output_file = fopen(filename, "r");
    if (printf_output_file == nullptr) {
        perror("Error opening the file");
        return false;
    }

    char buffer[128];
    // Data structure to keep track of the thread IDs we have processed
    bool processed_threads[64] = {false};

    // Reads in the file's contents
    while (fgets(buffer, 128, printf_output_file)) {
        // Creates C++ string
        std::string test_string(buffer);

        // Checks there is a thread ID printed first
        std::size_t found = test_string.find_first_of("0123456789");
        if (found == std::string::npos || found != 0) {
            fclose(printf_output_file);
            perror("Missing thread ID");
            return false;
        }

        std::istringstream input_stream(test_string);
        unsigned int thread_id;
        input_stream >> thread_id;
        if (!input_stream.good()) {
            perror("Error reading in thread ID");
            fclose(printf_output_file);
            return false;
        }

        if (thread_id >= 64) {
            perror("Unexpected thread ID");
            fclose(printf_output_file);
            return false;
        }

        // Checks if already seen this thread ID
        if (processed_threads[thread_id] == true) {
            perror("Duplicate thread ID");
            fclose(printf_output_file);
            return false;
        }

        std::string remainder;
        getline(input_stream, remainder);
        if (remainder.compare(golden_string) != 0) {
            perror("Incorrect output");
            fclose(printf_output_file);
            return false;
        }

        processed_threads[thread_id] = true;
      }

    // Checks if there were any missing threads
    for (int index = 0; index < 64; ++index) {
        if (processed_threads[index] == false) {
            fclose(printf_output_file);
            return false;
        }
    }

    fclose(printf_output_file);

    return true;
}
