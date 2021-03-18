    #include <unistd.h>

    int getopt(int argc, char * const argv[],const char *optstring);

    extern char *optarg;
    extern int optind, opterr, optopt;