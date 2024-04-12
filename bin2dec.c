
/*
    bin2dec - A utility to convert binary numbers to decimal numbers.
    
    Copyright (C) 2023  David Johnston

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    -----
    
    Contact. David Johnston dj@deadhat.com
*/
/* make isnan() visible */
#define _BSD_SOURCE 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <getopt.h>

void display_usage() {
fprintf(stderr,"Usage: bin2dec [-b][-w <width>][-h][-o <out filename>] [filename]\n");
fprintf(stderr,"       -w <width>        : Set the number of bytes for each number, 1-8 (default 4)\n");
fprintf(stderr,"       -b                : Use big endian order (Default little endian)\n");
fprintf(stderr,"       -o <out filename> : send output to a file (default stdout)\n");
fprintf(stderr,"       -h                : Print this help\n"); 
fprintf(stderr,"\n");
fprintf(stderr,"Convert binary data to decimal.\n");
fprintf(stderr,"  Author: David Johnston, dj@deadhat.com\n");
fprintf(stderr,"\n");
}

/********
* main() is mostly about parsing and qualifying the command line options.
*/

int main(int argc, char** argv)
{
    int opt;
	int i;
	int j;
	
	FILE *ifp;
	FILE *ofp;
	int using_outfile;
	int using_infile;
	char filename[1000];
	char infilename[1000];
	
	//int linewidth;
    int width;
    int bigendian=0; 

	/* Defaults */
	using_outfile = 0;       /* use stdout instead of outputfile*/

    width = 4;
	//linewidth = 32;
    
    filename[0] = (char)0;
	infilename[0] = (char)0;

	/* get the options and arguments */
    int longIndex;

    char optString[] = "bo:k:w:h";
    static const struct option longOpts[] = {
    { "output", no_argument, NULL, 'o' },
    { "width", required_argument, NULL, 'w' },
    { "bigendian", no_argument, NULL, 'b' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
    };

    opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    while( opt != -1 ) {
        switch( opt ) {
            case 'o':
                using_outfile = 1;
                strcpy(filename,optarg);
                break;
            case 'w':
                width=atoi(optarg);
                break;
            case 'b':
                bigendian = 1;
                break;
            case 'f':
                using_infile = 1;
                strcpy(infilename,optarg);
                break;
                
            case 'h':   /* fall-through is intentional */
            case '?':
                display_usage();
                exit(0);
                 
            default:
                /* You won't actually get here. */
                break;
        }
         
        opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
    } // end while
    
    if (optind < argc) {
        strcpy(infilename,argv[optind]);
        using_infile = 1;
    }

	/* Range check the var args */
    if ((width < 1) || (width > 8)) {
        fprintf(stderr,"Error: width must be between 1 and 8");
        exit(1);
    }

	/* open the output file if needed */

	if (using_outfile==1)
	{
		ofp = fopen(filename, "w");
		if (ofp == NULL) {
			perror("failed to open output file for writing");
			exit(1);
		}
	}

	/* open the input file if needed */
	if (using_infile==1)
	{
        ifp =  fopen(infilename, "rb");
		    
		if (ifp == NULL) {
			perror("failed to open input file for reading");
			exit(1);
		}
	}

    unsigned char buffer[2048];
    size_t len;
    
    // Get an integer number of bytes less than or equal to 2048
    int read_size = 0;
    read_size = (int)(2048/width);
    
    unsigned char *bufptr;
    unsigned char word[8];
    uint64_t *wordptr;
    wordptr = (uint64_t *)word;

    do {
        if (using_infile==1) {
            len = fread(buffer, width, read_size , ifp);
        }
        else {
            len = fread(buffer, width, read_size , stdin);
        }
            
        if (len == 0) {
            exit(0);
        }
        
        bufptr = buffer;

        for(i=0;i<len;i++) {

            *wordptr = 0;
            if (bigendian==0) {
                for (j=0;j<width;j++) {
                    word[j] = *bufptr++; 
                }
            } else {
                for (j=0;j<width;j++) {
                    word[width-j-1] = *bufptr++; 
                }
            }

            if (using_outfile==1) {
                fprintf(ofp, "%" PRIu64 "\n",*wordptr); 
            } else {
                fprintf(stdout, "%" PRIu64 "\n",*wordptr); 
            }
        }
        
    } while (1);
    
    if (using_outfile==1) fclose(ofp);
    
}


