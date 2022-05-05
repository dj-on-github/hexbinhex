
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
fprintf(stderr,"Usage: nistoddball2bin [-l <bits_per_symbol 1-8>][-B|-L][-v][-h][-o <out filename>] [filename]\n");
fprintf(stderr,"       -l n          Set the number of symbol bits per byte encoded in the input data. Must be between 1 to 8\n");
fprintf(stderr,"       -B            Interpret the input symbols at being big endian (MSB first)\n");
fprintf(stderr,"       -L            Interpret the input symbols at being little endian (LSB first) (default)\n");
fprintf(stderr,"       -v            Verbose mode. Outputs information to stderr\n");
fprintf(stderr,"       -h            Display this information\n");
fprintf(stderr,"       -o filename   Output to file filename instead of stdout\n");
fprintf(stderr,"\n");
fprintf(stderr,"Convert binary data to NIST Oddball SP800-90B one-symbol-per-byte format.\n");
fprintf(stderr,"  Author: David Johnston, dj@deadhat.com\n");
fprintf(stderr,"\n");
fprintf(stderr,"Notes:\n");
fprintf(stderr,"      The NIST format for SP800-90B testing requires symbols to be as one symbol per bit.\n");
fprintf(stderr,"      This means only symbols of sizes 1 through 8 bits can be supported.\n");
fprintf(stderr,"      The bit ordering of the bits within the symbols is not specified by NIST. The -L and -B options allow you to choose.\n");
fprintf(stderr,"      The output binary data by default is in little endian format, with the lower order bits in bytes coming before higher order bits. This can be reversed with the -r option.\n");
}

/********
* main() is mostly about parsing and qualifying the command line options.
*/

int main(int argc, char** argv)
{
    int opt;
	int i;
	
	FILE *ifp;          // Input file pointer
	FILE *ofp;          // Output file pointer
	int using_outfile;      // True if -o <file> option used
	int using_infile;       // True if input filename given
	char filename[1000];    // Let's hope the filename isn't bigger
	char infilename[1000];
	
    int bps;        // Bits per Symbol
    int abyte;      // A Byte. Surprise.

    int littleendian=1;  //Put the first bits in the
                         //lower order of the byte
    int gotL=0;          // 1 when little endian chosen on cmd line
    int gotB=0;          // 1 when bit endian chosen on cmd line
    int verbose = 0;     // Set to non zero for verbose output
    int reverse = 0;
    
	/* Defaults */
	using_outfile = 0;       /* use stdout instead of outputfile*/
    bps = 1;    
    filename[0] = (char)0;
	infilename[0] = (char)0;

	/* get the options and arguments */
    int longIndex;

    char optString[] = "o:k:l:BLrvh";
    static const struct option longOpts[] = {
    { "output", no_argument, NULL, 'o' },
    { "width", required_argument, NULL, 'w' },
    { "bits_per_symbol", required_argument, NULL, 'l' },
    { "littleendian", no_argument, NULL, 'L' },
    { "bigendian", no_argument, NULL, 'B' },
    { "reverse", no_argument, NULL, 'r' },
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
            case 'L':
                littleendian=1;
                gotL=1;
                break;
            case 'B':
                littleendian=0;
                gotB=1;
                break;
            case 'r':
                reverse=1;
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

    //if (reverse == 0) fprintf(stderr,"reverse=0\n");
    //else fprintf(stderr,"reverse=1\n");
    //fflush(stderr);

    do {
        if (using_infile==1)
            len = fread(buffer, 1, 2048 , ifp);
        else
            len = fread(buffer, 1, 2048 , stdin);
            
        if (len == 0) break;

        // Read in symbols into the FIFO of bits. bps bits per byte.        
        for (i=0;i<len;i++) {
            abyte = buffer[i];
            
            // If less that 8 bits per byte, and big endian, shift them
            // to the left so that the MSB can be plucked off from the right.
            if ((littleendian==0) && (bps < 8)) {
                abyte = abyte << (8-bps);
            }
            
            // Pluck off the bits and put into FIFO
            for(j=0;j<bps;j++) {
                if (littleendian==1) {
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

        // Work out how many full bytes are in the FIFO.
        symbol_count = bitfifo_entries / 8;

        //Pull bits from the FIFO and Write out bytes;
        for (i=0;i<symbol_count;i++) {
            abyte = 0;
            // Read the bits of one byte and put into a byte
            for (j=0;j<8;j++) {
                bitfifo_tail = (bitfifo_tail +1) % BITFIFO_SIZE;
                bitfifo_entries--;
                abit = bitfifo[bitfifo_tail];
                if (reverse==0) {
                    abyte = abyte + (abit << j);
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
    
    //if (using_outfile==1) fclose(ofp);
    //printf("max_runcount = %d\n",max_runcount);

    return 0;
}


