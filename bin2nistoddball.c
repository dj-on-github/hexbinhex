
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
fprintf(stderr,"Usage: bin2nistoddball [-w <width>][-l <bits_per_symbol 1-8>][-h][-o <out filename>] [filename]\n");
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
	int using_outfile;
	int using_infile;
	char filename[1000];
	char infilename[1000];
	
    int width;
    int bps;   
    int abyte;

	/* Defaults */
	using_outfile = 0;       /* use stdout instead of outputfile*/
    width = 32;
    bps = 1;    
    filename[0] = (char)0;
	infilename[0] = (char)0;

	/* get the options and arguments */
    int longIndex;

    char optString[] = "o:k:l:w:h";
    static const struct option longOpts[] = {
    { "output", no_argument, NULL, 'o' },
    { "width", required_argument, NULL, 'w' },
    { "bits_per_symbol", required_argument, NULL, 'l' },
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
                if (width<1) width=32;
                break;
            case 'l':
                bps = atoi(optarg);
                if ((bps < 1) || (bps > 8)) {
                    perror("Error, bits per symbol bust be between 1 and 8");
                    display_usage();
                    exit(-1);
                };
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

    unsigned char buffer[2048];

    #define BITFIFO_SIZE 20000
    unsigned char bitfifo[BITFIFO_SIZE];
    int bitfifo_head = 0;
    int bitfifo_tail = 0;
    int bitfifo_entries = 0;

    unsigned char outbuffer[10000];
    int outindex = 0;
    int bytecount = 0;
    int done=0;
    size_t len;
    unsigned char abit;

    int symbol_count;
    int j;

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
                abit = (abyte & 0x01);
                abyte = abyte >> 1;

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
            // Read the bits of one symbol and put into a byte
            for (j=0;j<bps;j++) {
                bitfifo_tail = (bitfifo_tail +1) % BITFIFO_SIZE;
                bitfifo_entries--;
                abit = bitfifo[bitfifo_tail];
                abyte = (abyte << 1) | abit;
            }

            bytecount++;
            outbuffer[outindex] = abyte;
            outindex += 1;
        }
        
        if (using_outfile)
            fwrite(outbuffer, outindex,1,ofp);
        else
            fwrite(outbuffer, outindex,1,stdout);
        
        outindex = 0;
        
    } while (done==0);
    
    if (using_outfile==1) fclose(ofp);
    
}

