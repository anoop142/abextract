/* 
Anoop S 

abextract - ADB backup extractor   

Tar extraction is done is using "tar" utility.
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

#define CHUNK 16384



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



int main(int argc, char **argv)
{
    const char *help =  "usage: abextract backup.ab\n";
    if(argc == 2) {
        if(strncmp(argv[1],"-h",3)== 0) 
        {
            fprintf(stdout,help);
            exit(0);
        } 
        
            FILE *source ;
            FILE *dest;
            int ret;

            /* open input file */
            source = fopen(argv[1],"r");
            if( source == NULL){
                perror(argv[1]);
                exit(0);
            }
            /* Open output file */
            char out_file[128];
            int skip_ext = strlen(argv[1])-3;
            strncpy(out_file,argv[1],skip_ext);
            strncat(out_file,"_unpacked.tar",14);
            dest = fopen(out_file,"w");


            /* avoid end-of-line conversions */
            SET_BINARY_MODE(source);
            SET_BINARY_MODE(dest);


            ret = inf(source, dest);
            if (ret != Z_OK)
                zerr(ret);
            char tar_command[128]="tar xf ";
            strcat(tar_command,out_file);
            if(system(tar_command) != -1)
            {
                fprintf(stdout,"upacked successfully!\n");
                remove(out_file);
            }
            else{
                perror("abextract");
            }   

        fclose(source);
        fclose(dest); 

    }
    
    else {
        fputs(help, stderr);
        return 1;
    }
    
}
