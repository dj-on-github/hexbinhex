
/*
    bin2hex - A utility to convert binary data to hex data.
    
    Copyright (C) 2017  David Johnston

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
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>

void display_usage() {
fprintf(stderr,"Usage: bin2nistoddball [-l <bits_per_symbol 1-8>][-B|-L][-v][-h][-o <out filename>] [filename]\n");
fprintf(stderr,"       -l , --length <bits_per_symbol 1-8> Set the number of bits to encode in eat output byte\n");
fprintf(stderr,"       -r , --reverse                      Interpret input binary data as big endian (MSB first) (default is little endian)\n");
fprintf(stderr,"       -B , --bigendian                    Unpack output multi-bit symbols as big-endian (msb first)\n");
fprintf(stderr,"       -L , --littleendian                 Unpack output multi-bit symbols as little-endian (lsb first) (default)\n");
fprintf(stderr,"       -v , --verbose                      Output information to stderr\n");
fprintf(stderr,"       -h , --help                         Output this information\n");
fprintf(stderr,"\n");
fprintf(stderr,"Convert binary data to NIST Oddball SP800-90B one-symbol-per-byte format.\n");
fprintf(stderr,"  Author: David Johnston, dj@deadhat.com\n");
fprintf(stderr,"\n");
}

void printsample(unsigned char *thesample)
{
    int tempindex;
    int j;
    int i;
   tempindex = 0;
    for (j=0;j<16;j++)
    {
            for (i=0;i<16;i++) printf("%02X",thesample[tempindex++]);
            printf("\n");
    }
}

/********
* main() is mostly about parsing and qualifying the command line options.
*/

int main(int argc, char** argv)
{
    int opt;
	int i;
	
	FILE *ifp;
	FILE *ofp;
	int using_outfile = 0;  /* use stdout instead of outputfile*/
	int using_infile;
	char filename[1000];
	char infilename[1000];
	
    int bps = 1;   
    int abyte;
    
    int littleendian=1;
    int gotL=0;
    int gotB=0;
    int verbose = 0;
    int reverse = 0;

	/* Zero out the strings */    
    filename[0] = (char)0;
	infilename[0] = (char)0;

	/* get the options and arguments */
    int longIndex;

    char optString[] = "o:k:l:w:BLrvh";
    static const struct option longOpts[] = {
    { "output", no_argument, NULL, 'o' },
    { "reverse", no_argument, NULL, 'r' },
    { "bigendian", no_argument, NULL, 'B' },
    { "littleendian", no_argument, NULL, 'L' },
    { "bits_per_symbol", required_argument, NULL, 'l' },
    { "verbose", no_argument, NULL, 'v' },
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
            case 'l':
                bps = atoi(optarg);
                if ((bps < 1) || (bps > 8)) {
                    perror("Error, bits per symbol bust be between 1 and 8");
                    display_usage();
                    exit(-1);
                };
                break;
            case 'r':
                reverse=1;
                break;
            case 'L':
                littleendian=1;
                gotL=1;
                break;
            case 'B':
                littleendian=0;
                gotB=1;
                break;
            case 'v':
                verbose=1;
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
    
    if (gotB==1 && gotL==1) {
        fprintf(stderr,"ERROR, Can't be both big endian (-B) and little endian (-L) at the same time\n");
        exit(-1);
    }

    if (optind < argc) {
        strcpy(infilename,argv[optind]);
        using_infile = 1;
    }
        
    if (verbose==1) {
        fprintf(stderr,"Verbose mode enabled\n");
        if (reverse==1) fprintf(stderr, "Input data interpreted as big-endian (msb first)\n");
        if (littleendian==0) fprintf(stderr, "Output multi-bit symbols encoded as big endian (MSB first)\n");
        if (littleendian==1) fprintf(stderr, "Output multi-bit symbols encoded as little endian (LSB first) (default)\n");
        if (((gotB==1) || (gotL==1)) && (bps > 1)){
            printf("Warning: -L and -B arguments have so effect with 1 bit output symbols\n"); 
        }
        if (using_infile==1) {
            fprintf(stderr,"Reading binary data from file: %s\n", infilename);
        }
        if (using_outfile==1) {
            fprintf(stderr,"Writing NIST 1 symbol per byte data to file: %s\n", filename);
        }
        fprintf(stderr,"Bits per symbol = %d\n",bps); 
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

    unsigned char buffer[2048];

    #define BITFIFO_SIZE 20000
    unsigned char bitfifo[BITFIFO_SIZE];
    int bitfifo_head = 0;
    int bitfifo_tail = 0;
    int bitfifo_entries = 0;

    unsigned char outbuffer[20000];
    int outindex = 0;
    int bytecount = 0;
    int done=0;
    size_t len;
    unsigned char abit;

    int symbol_count;
    int j;
    int runcount = 0;
    int last_abyte = -1;
    int max_runcount = 0;
    int location = 0;
    do {
        if (using_infile==1)
            len = fread(buffer, 1, 2048 , ifp);
        else
            len = fread(buffer, 1, 2048 , stdin);
            
        if (len == 0) break;

        // Read in buffer bytes into the FIFO of bits. One bit per byte.        
        for (i=0;i<len;i++) {
            abyte = buffer[i];
            for(j=0;j<8;j++) {
                if (reverse==0) {
                    abit = (abyte & 0x01);
                    abyte = abyte >> 1;
                } else {
                    abit = (abyte & 0x80) >> 7;
                    abyte = abyte << 1;
                }

                bitfifo_head = (bitfifo_head +1) % BITFIFO_SIZE;
                bitfifo_entries++;
                bitfifo[bitfifo_head] = abit;
            }
        }

        // Work out how many full symbols are in the FIFO.
        symbol_count = bitfifo_entries / bps;

        //Pull bits from the FIFO and Write out the symbols (of 1 to 8 bits) as bytes;
        for (i=0;i<symbol_count;i++) {
            abyte = 0;
            // Read the bits of one symbol and put into an output byte.
            for (j=0;j<bps;j++) {
                bitfifo_tail = (bitfifo_tail +1) % BITFIFO_SIZE;
                bitfifo_entries--;
                abit = bitfifo[bitfifo_tail];
                
                if (littleendian==1) {
                    abyte = abyte | (abit << j);
                } else {
                    abyte = (abyte << 1) | abit;
                }
            }

            bytecount++;
            outbuffer[outindex] = abyte;
            outindex += 1;

            if (abyte==last_abyte) {
                runcount++;
                if (runcount > max_runcount) {
                    max_runcount = runcount;
                }
            } else {
                runcount = 1;
                last_abyte = abyte;
            }
            location++;
            //if (runcount > 35) {
            //    fprintf(stderr,"Runcount = %d, location = %d\n",runcount,location);
            //}
        }
        
        if (using_outfile)
            fwrite(outbuffer, outindex,1,ofp);
        else
            fwrite(outbuffer, outindex,1,stdout);
        
        outindex = 0;
        
    } while (done==0);
    
    if (using_outfile==1) fclose(ofp);
    //fprintf(stderr,"max_runcount = %d\n",max_runcount);
}


