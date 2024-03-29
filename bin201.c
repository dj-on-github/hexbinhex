
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
fprintf(stderr,"Usage: bin201 [-w <width>][-b][-h][-o <out filename>] [filename]\n");
fprintf(stderr,"  -w <width> Sets the number of bits per output line\n");
fprintf(stderr,"  -B         Reverses the order of bits in each byte to big endian\n");
fprintf(stderr,"  -L         Outputs bits as little endian (default)\n");
fprintf(stderr,"  -s         Add a space between every 8 bits\n");
fprintf(stderr,"Convert binary data to ascii binary (01001001).\n");
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
    int j;
	
	FILE *ifp;
	FILE *ofp;
	int using_outfile;
	int using_infile;
	char filename[1000];
	char infilename[1000];
	
    int width=32;
    int littleendian = 1;
    int gotB = 0;
    int gotL = 0;    
    int abyte;
    int spaces = 0;
    int verbose = 0;
    
	/* Defaults */
	using_outfile = 0;       /* use stdout instead of outputfile*/

    filename[0] = (char)0;
	infilename[0] = (char)0;

	/* get the options and arguments */
    int longIndex;

    char optString[] = "o:k:w:BLsvh";
    static const struct option longOpts[] = {
    { "output", no_argument, NULL, 'o' },
    { "width", required_argument, NULL, 'w' },
    { "bigendian", no_argument, NULL, 'B' },
    { "littleendian", no_argument, NULL, 'L' },
    { "spacebetweenbytes", no_argument, NULL, 's'},
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
            case 'w':
                width=atoi(optarg);
                if (width<1) width=32;
                break;
            case 'f':
                using_infile = 1;
                strcpy(infilename,optarg);
                break;
            case 'B':
                littleendian = 0;
                gotB = 1;
                break;
            case 'L':
                littleendian = 1;
                gotL = 1;
                break;
            case 's':
                spaces = 1;
                break;
            case 'v':
                verbose = 1;
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
        if (littleendian==0) fprintf(stderr, "Big endian bit packing assumed\n");
        if (littleendian==1) fprintf(stderr, "Little endian bit packing assumed\n");
        if (using_infile==1) {
            fprintf(stderr,"Reading binary data from file: %s\n", infilename);
        }
        if (using_outfile==1) {
            fprintf(stderr,"Writing ASCII binary data to file: %s\n", filename);
        }
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
    unsigned char outbuffer[10000];
    int outindex = 0;
    int bitcount = 0;
    int done=0;
    size_t len;
    int lastcharnl = 0;
    char binch;
    int first = 1;

    do {
        if (using_infile==1)
            len = fread(buffer, 1, 512 , ifp);
        else
            len = fread(buffer, 1, 512 , stdin);
            
        if (len == 0) break;
        
        for (i=0;i<len;i++) {
            abyte = buffer[i];
            for (j=0;j<8;j++) {
                if (littleendian == 1) {
                    if (((abyte >> j) & 0x01) == 0) {
                        binch = '0';
                    } else {
                        binch = '1';
                    }
                } else {
                    if (((abyte >> (7-j)) & 0x01) == 0) {
                        binch = '0';
                    } else {
                        binch = '1';
                    }
                }
                bitcount++;

                sprintf((char *)&((outbuffer[outindex])),"%c",binch);  
                outindex+=1;
                
                if (bitcount==width) {
                    sprintf((char *)&(outbuffer[outindex]),"\n");
                    outindex++;
                    bitcount=0;
                    lastcharnl = 1;
                    first = 1;
                }
                else {
                    lastcharnl = 0;
                    if ((spaces==1) && ((bitcount % 8)==0) && (bitcount > 1) && (first==0)) {
                        sprintf((char *)&((outbuffer[outindex]))," ");  
                        outindex++;
                    }
                    else
                    {
                        first = 0;
                    }
                }
            }
        }
        
        if (using_outfile)
            fwrite(outbuffer, outindex,1,ofp);
        else
            fwrite(outbuffer, outindex,1,stdout);
        
        outindex = 0;
      
        
    } while (done==0);
    
    if (lastcharnl==0) {
        if (using_outfile)
            fprintf(ofp,"\n");
        else
            printf("\n");
    }
    
    if (using_outfile==1) fclose(ofp);

    return 0;    
}


