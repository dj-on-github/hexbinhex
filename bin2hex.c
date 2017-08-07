
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
#define _BSD_SOURCE 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>

void display_usage() {
fprintf(stderr,"Usage: bin2hex [-w <width>][-h][-o <out filename>] [filename]\n");
fprintf(stderr,"\n");
fprintf(stderr,"Convert binary data to hexadecimal.\n");
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
	
	int linewidth;
    int width;
    
    int abyte;

	/* Defaults */
	using_outfile = 0;       /* use stdout instead of outputfile*/

    width = 32;
	linewidth = 32;
    
    filename[0] = (char)0;
	infilename[0] = (char)0;

	/* get the options and arguments */
    int longIndex;

    char optString[] = "o:k:w:h";
    static const struct option longOpts[] = {
    { "output", no_argument, NULL, 'o' },
    { "width", required_argument, NULL, 'w' },
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
    unsigned char outbuffer[10000];
    int outindex = 0;
    int bytecount = 0;
    int done=0;
    size_t len;
    int lastcharnl = 0;
    
    do {
        if (using_infile==1)
            len = fread(buffer, 1, 2048 , ifp);
        else
            len = fread(buffer, 1, 2048 , stdin);
            
        if (len == 0) break;
        
        for (i=0;i<len;i++) {
            abyte = buffer[i];
            bytecount++;
            
            sprintf((char *)&((outbuffer[outindex])),"%02X",abyte);  
            outindex+=2;
            
            if (bytecount==width) {
                sprintf((char *)&(outbuffer[outindex]),"\n");
                outindex++;
                bytecount=0;
                lastcharnl = 1;
            }
            else {
                lastcharnl = 0;
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
    
}


