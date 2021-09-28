/* Copyright (C) 2021 plushkatze
 *
   ddthin is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ddthin is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with ddthin.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>

const char *argp_program_version = "ddthin 0.0.4";

static char doc[] = "ddthin - sync files thin and sparse";

static char argc_doc[] = "INPUTFILE OUTPUTFILE";

static struct argp_option options[] = {
	{"verbose", 'v', 0, 0, "describe what is happening"},
	{"quiet", 'q', 0, 0, "avoid any ouput, even warnings!"},
	{"blocksize", 'b', "bytes", 0, "use given blocksize"},
	{ 0 }
};

struct arguments
{
	char *args[2];
	int verbose;
	int quiet;
	int blocksize;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch(key)
	{
		case 'v':
			arguments->verbose = 1;
			break;
		case 'q':
			arguments->quiet = 1;
			break;
		case 'b':
			arguments->blocksize = atoi(arg);
			break;
		case ARGP_KEY_ARG:
			if (state->arg_num >= 2)
				argp_usage(state);
			arguments->args[state->arg_num] = arg;
			break;
		case ARGP_KEY_END:
			if (state->arg_num < 2)
				argp_usage(state);
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = {options, parse_opt, argc_doc, doc};


int main(int argc, char* argv[]) 
{
	struct arguments arguments;
	arguments.quiet = 0;
	arguments.verbose = 0;
	arguments.blocksize = 512;
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	if (arguments.blocksize <= 0) {
		printf("ERROR: blocksize must be at least 1\n");
		return(1);
	}

	char inputbuffer[arguments.blocksize];
	char outputbuffer[arguments.blocksize];

	FILE * inputfile = fopen(arguments.args[0], "rb");
	FILE * outputfile = fopen(arguments.args[1], "rb+");
	
	if (inputfile == NULL) {
		perror("Failed to open inputfile");
		return(1);
	}

	if (outputfile == NULL) {
		perror("Failed to open outputfile");
		return(1);
	}

	long int read_pointer = 0;
	long int input_pointer = 0;
	long int write_pointer = 0;
	long int bytes_written = 0;

	// Get filesizes
	// this is not defined in the official standard and will not work on streams
	fseek(inputfile, read_pointer, SEEK_END);
	long int inputfilesize = ftell(inputfile);
	fseek(inputfile, read_pointer, SEEK_SET);
	
	fseek(outputfile, read_pointer, SEEK_END);
	long int outputfilesize = ftell(outputfile);
	fseek(outputfile, read_pointer, SEEK_SET);

	if (inputfilesize != outputfilesize) {
		if (!arguments.quiet) printf("WARNING: filesizes differ!\n");
	}

	if (inputfilesize % arguments.blocksize != 0) {
		if (!arguments.quiet) printf("WARNING: filesize for %s not a multiple of blocksize!\n", arguments.args[0]);
	}

	if (outputfilesize % arguments.blocksize != 0) {
		if (!arguments.quiet) printf("WARNING: filesize for %s not a multiple of blocksize!\n", arguments.args[1]);
	}
	
	int percentagedisplay = 0;

	// we read and write until output is full
	// it might be a blockdevice, so we should never make it bigger
	// input may be bigger or smaller
	while (read_pointer < outputfilesize) {
		percentagedisplay++;

		// TODO: handle read errors
		size_t blocks_read_inputfile = fread(inputbuffer, arguments.blocksize, 1, inputfile);
		size_t blocks_read_outputfile = fread(outputbuffer, arguments.blocksize, 1, outputfile);
		size_t bytes_read_inputfile = arguments.blocksize * blocks_read_inputfile;
		size_t bytes_read_outputfile = arguments.blocksize * blocks_read_outputfile;
		
		// how many bytes were read
		read_pointer += bytes_read_outputfile;
		input_pointer += bytes_read_inputfile;

		// show percent done every 1000 blocks
		if ((arguments.verbose) && (percentagedisplay % 1000)) {
			printf("\rINFO: %.2f%% done. %ldMB processed", ((double)input_pointer/(double)inputfilesize) * 100, input_pointer / 1024 / 1024);
			fflush(stdout);
		}

		// source bigger than destination
		if (bytes_read_outputfile == 0) {
			if (!arguments.quiet) printf("WARNING: reached end of outputfile\n");
			size_t bytes_left = outputfilesize - read_pointer;
			if (!arguments.quiet) printf("WARNING: %zu bytes left in output not enough for full block\n", bytes_left);
			// We can recover from this, but we need to sync both filepositions now
			// both reads are now rolled back in sync, now we change bs to 1
			fseek(inputfile, read_pointer, SEEK_SET);
			fseek(outputfile, read_pointer, SEEK_SET);
			arguments.blocksize = 1;
			if (!arguments.quiet) printf("WARNING: setting blocksize to 1 to recover\n");
			continue;
		}
		
		// source smaller than destination 
		if ((bytes_read_inputfile == 0) && (bytes_read_outputfile > 0)) {
			if (!arguments.quiet) printf("WARNING: reached end of inputfile with data left in output\n");
			size_t bytes_left = inputfilesize - input_pointer;
			if (bytes_left == 0) {
				// we are good, input was fully processed
				// stop loop and close files
				break;
			}
			if (!arguments.quiet) printf("WARNING: %zu bytes left in input not enough for full block\n", bytes_left);
			// We can recover from this, but we need to sync both filepositions now
			// If we read from output the readpointer might have moved, we need to fix this
			read_pointer -= bytes_read_outputfile;
			// both reads are now rolled back in sync, now we change bs to 1
			fseek(inputfile, read_pointer, SEEK_SET);
			fseek(outputfile, read_pointer, SEEK_SET);
			arguments.blocksize = 1;
			if (!arguments.quiet) printf("WARNING: setting blocksize to 1 to recover\n");
			continue;
		}
		
		// blocksize mismatch
		if (bytes_read_outputfile != bytes_read_inputfile) {
			if (!arguments.quiet) printf("WARNING: filesize difference caused inequal blocksize\n");
			// TODO: properly handle this situation
		}

		// compare block
		int blockdiff = memcmp(inputbuffer, outputbuffer, arguments.blocksize);

		if (blockdiff != 0) {
			// We need to write this block to destination
			// move back to beginning of block
			fseek(outputfile, write_pointer, SEEK_SET);
			// TODO: handle write errors
			bytes_written += fwrite(inputbuffer, sizeof(char), bytes_read_inputfile, outputfile);
			write_pointer = read_pointer;
			fseek(outputfile, read_pointer, SEEK_SET);
		} else {
			// source and destination are equal, nothing to do
			write_pointer = read_pointer;
		}
		
	}

	// all blocks processed
	if (inputfilesize > outputfilesize) {
		if (!arguments.quiet) 
			printf("WARNING: inputfile was bigger than output, %ld bytes of input not processed\n", inputfilesize - outputfilesize);
	}
	if (inputfilesize < outputfilesize) {
		if (!arguments.quiet) 
			printf("WARNING: outputfile was bigger than input, %ld bytes in output not processed\n", outputfilesize - inputfilesize);
	}
	if ((!arguments.quiet) && (arguments.verbose)) {
		printf("Copy done. Processed %ld bytes (%ldMB) total\n", read_pointer, read_pointer / 1024 / 1024);
		printf("Total bytes written %ld (%.2f%%)\n", bytes_written, ((double)bytes_written / (double)read_pointer) * 100.0);
	}

	fclose(inputfile);
	fclose(outputfile);
}

