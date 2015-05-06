/**
 * @file
 * Implementation of the parser for JSON.
 *
 * @verbatim
 * SPSPS
 * Stacy's Pathetically Simple Parsing System
 * https://github.com/sprowell/spsps
 *
 * Copyright (c) 2014, Stacy Prowell
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * @endverbatim
 */

#include <json.h>

/**
 * Provide a main method to test the JSON parser.
 * @param argc				Number of arguments.
 * @param argv				Arguments.
 */
int main(int argc, char * argv[]) {
	// If a first argument is provided, it is the file name.  If no first
	// argument is provided, then read from standard in.
	FILE * input = stdin;
	if (argc > 1) {
		input = fopen(argv[1], "rt");
		if (input == NULL) {
			fprintf(stderr, "ERROR: Unable to read from file %s.\n", argv[1]);
			exit(1);
		}
	}

	// Now we have to read from the stream.  We read and parse JSON.
	Parser parser = spsps_new(argv[1], input);
	json_value * value = json_parse_value(parser);
	if (value != NULL) {
		// Print the value!
		json_stream(stdout, value, 0);
		fprintf(stdout, "\n");
		// Free the value!
		json_free_value(value);
	}
	spsps_free(parser);
	// Done.
	if (argc > 1) {
		fclose(input);
	}
	exit(0);
}
