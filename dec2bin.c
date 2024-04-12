
/*
    dec2bin - A utility to convert decimal numbers to binary numbers.
    
    Copyright (C) 2024  David Johnston

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
    fprintf(stderr,"Usage: dec2bin [-b][-w <width>][-h][-o <out filename>] [filename]\n");
    fprintf(stderr,"  -w <width> gives size of binary output numbers in bytes,\n");
    fprintf(stderr,"             default 4 bytes. Must be from 1 to 8.\n");
    fprintf(stderr,"  -b Output numbers as big-endian binary.\n");
    fprintf(stderr,"             Default is little endian\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"Convert binary data to decimal.\n");
    fprintf(stderr,"  Author: David Johnston, dj@deadhat.com\n");
    fprintf(stderr,"\n");
}

void new_digits(char *digits, int *digitcount) {
    *digitcount = 0;
};

void add_digit(char ch, char *digits, int *digitcount) {
    int i;
    i = *digitcount;
    digits[i] = ch;
    i++;
    *digitcount = i;
};

// The digits if any in the digits buffer should be all the digits for a given number
// because it was followed by a non digit or it got to the end of a file.
// So we convert the number to binary and output it.
void finish_digits(char *digits, int *digitcount, int *using_outfile, FILE *ofp, int *bwidth, int *bigendian) {
    uint64_t thenumber;
    unsigned char outbytes[16];
    unsigned char *numberptr;
    int result;
    int i;

    if (*digitcount > 0) {
        digits[*digitcount]=0x00;
        result = sscanf(digits,"%" SCNu64 ,&thenumber);
       
        if (result) {
            numberptr = (unsigned char *)(&thenumber);

            if (*bigendian == 1) {
                for (i=0; i<*bwidth; i++) {
                    outbytes[i] = numberptr[*bwidth-i-1];
                }
            } else {
                for (i=0; i<*bwidth; i++) {
                    outbytes[i] = numberptr[i];
                }
            }

            if (*using_outfile == 1) {
                fwrite(outbytes, 1, *bwidth, ofp);
            } else {
                fwrite(outbytes, 1, *bwidth, stdout);
            }
        }
    }
    *digitcount = 0;

};

/********
* main() is mostly about parsing and qualifying the command line options.
*/

int main(int argc, char** argv)
{
    int opt;
	int i;
	
	FILE *ifp;
	FILE *ofp;
	int using_outfile;
	int using_infile;
	char filename[1000];
	char infilename[1000];
	
	//int linewidth;
    int bwidth=4;
    int bigendian=0; 

	/* Defaults */
	using_outfile = 0;       /* use stdout instead of outputfile*/

    filename[0] = (char)0;
	infilename[0] = (char)0;

	/* get the options and arguments */
    int longIndex;

    char optString[] = "bo:k:w:h";
    static const struct option longOpts[] = {
    { "output", required_argument, NULL, 'o' },
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
                bwidth=atoi(optarg);
                if ((bwidth < 1) || (bwidth > 8)) {
                    fprintf(stderr,"Error: width must from 1 to 8 inclusive.\n");
                    exit(1);
                }
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

    char buffer[2048];
    char *bufferptr;
    char digits[256];
    //unsigned char outbuffer[10000];
    //int outindex = 0;
    //int bytecount = 0;
    int done=0;
    //int decimalval=0;
    //unsigned char c=0;
    //size_t len;
    char ch;
    int gotten;
    int digitcount;
    int lastone_adigit = 1;

    //int lastcharnl = 0;
    digitcount = 0;
    new_digits(digits, &digitcount);

    do {
        if (using_infile==1)
            gotten = fread(buffer, 1, sizeof(buffer), ifp);
        else
            gotten = fread(buffer, 1, sizeof(buffer), stdin);
            
        if (gotten == 0) {
            finish_digits(digits, &digitcount, &using_outfile, ofp, &bwidth, &bigendian);  // mop up any last number 
            done = 1;
            break;
        }

        bufferptr = buffer;
        
        for (i=0; i<gotten; i++) {
            ch = *bufferptr++;

            if  ((ch > 0x2F) && (ch < 0x3a)) { // while it's a digit
                if (lastone_adigit==0) {
                    new_digits(digits, &digitcount); // A new number if it
                }                                    //  follows a non number
                add_digit(ch, digits, &digitcount);  // Add the digit to the number
                lastone_adigit=1;              // and remember that it was a number
            }
            else {
                if (digitcount > 0) {
                    finish_digits(digits, &digitcount, &using_outfile, ofp, &bwidth, &bigendian);  // If we didn't get a digit char
                }
                lastone_adigit=0;                    // then finish up any numbers we
            }                                            // already collected
        }

    } while (done==0);

    if (using_outfile==1) fclose(ofp);
    
}


