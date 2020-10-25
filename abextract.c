/* 
Anoop S 

abextract - ADB backup extractor  and repacker. 

Encrypted  backups are not supported!.

*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <zlib.h>
#include <sys/stat.h> 

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 10240
#define CHECK_FILE(fptr,ptr) { if(fptr==NULL) {\
                                perror(ptr); \
                                exit(1);  } }


/* Deflate  */
int def(FILE *source, FILE *dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;
    /* Write backup header */
    const char header[] = "ANDROID BACKUP\n5\n1\nnone";
    fwrite(header,1,sizeof(header),dest);

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}





/* Inflate zlib */
int inf(FILE *source, FILE *dest)
{
    /* skip 24 bytes */
    fseek(source,24,SEEK_SET);
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;


    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;    
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);


    } while (ret != Z_STREAM_END);


    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}


/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zlib_extract: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}





void unpack(char *input){
    char out_file[512];

    FILE *source ;
    FILE *dest;
    int ret;

    /* open input file */
    source = fopen(input,"r");
    CHECK_FILE(source,input);

    
    strncpy(out_file,input,strlen(input)-3);
    out_file[strlen(input)-3] = '\0';
    strncat(out_file,".tar",5);

    /* Open output file */
    dest = fopen(out_file,"w");
    CHECK_FILE(dest,out_file);

    /* avoid end-of-line conversions */
    SET_BINARY_MODE(source);
    SET_BINARY_MODE(dest);


    ret = inf(source, dest);
    if (ret != Z_OK)
        zerr(ret);

    fprintf(stdout,"Success!\n");

    fclose(source);
    fclose(dest); 
}


void print_help(const char *help_unpack, const char *help_pack){
    fprintf(stdout,"%s%s",help_unpack,help_pack);
    exit(0);
}



void pack_zlib(const char *input, const char *output){

    FILE *source;
    FILE *dest;
    int ret;

    source = fopen(input,"r");
    CHECK_FILE(source,input);

    dest = fopen(output,"w");
    CHECK_FILE(dest,output);
    
    /* avoid end-of-line conversions */
    SET_BINARY_MODE(source);
    SET_BINARY_MODE(dest);

     ret = def(source, dest, Z_BEST_COMPRESSION);
    if (ret != Z_OK){
        zerr(ret);
    }
    fprintf(stdout,"Success!\n");
    fclose(source);
    fclose(dest);
}


int main(int argc, char *argv[])
{
    const char *help_unpack =  "Usage :\n\tunpack:       abextract unpack     <backup.ab> \n";
    const char *help_pack =    "\tpack:         abextract pack       <backup.tar> <backup.ab>\n";

    if(argc >= 3) {
        if((strcmp(argv[1],"unpack")) == 0){
            unpack(argv[2]);
        }
        else if((strcmp(argv[1],"pack")) == 0){
            if(argv[3] != NULL){
                pack_zlib(argv[2],argv[3]);
            }
            else{
                print_help(help_unpack,help_pack);
            }           
        }

    }

    else {
        print_help(help_unpack,help_pack);

        }
    
}
