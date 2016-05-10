#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "portaudio.h"

/// Display help information
void help(){
    printf("mumpi - DESCRIPTION GOES HERE\n\n");
    printf("Usage:\n");
    printf("mumpi (-s|--server) string (-u|--username) string (-c|--cert) string [(-h|--help)] [(-v|--verbose)]\n\n");
    printf("Options:\n");
    printf("-h, --help                Displays this information.\n");
    printf("-v, --verbose             Verbose mode on.\n");
    printf("-s, --server <string>     mumble server IP:PORT. Required.\n");
    printf("-u, --username <username> username. Required.\n");
    printf("-c, --cert <cert>         SSL certificate file. Required.\n");
    exit(1);
}

int main(int argc, char *argv[]){
	// Program flow:
	// parse cmd line args
	// init audio device using PortAudio or OpenAL
	// init gpio driver
	// init mumlib
	// connect to mumble server
	//


	//TODO: Use Strings instead of cstrings
	// parse command line args using getopt
    char verbose=0;
    std::string server;
    std::string username;
    std::string cert;

    int next_option;
    const char* const short_options = "hvs:u:c:" ;
    const struct option long_options[] =
        {
            { "help", 0, NULL, 'h' },
            { "verbose", 0, NULL, 'v' },
            { "server", 1, NULL, 's' },
            { "username", 1, NULL, 'u' },
            { "cert", 1, NULL, 'c' },
            { NULL, 0, NULL, 0 }
        };

    // parse options
    while(1) {
        // obtain a option
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);

        if(next_option == -1)
            break; // no more options

        switch(next_option) {

            case 'h' : // -h or --help
                help();
                break;

            case 'v' : // -v or --verbose
                verbose=1;
                break;

            case 's' : // -s or --server
				server = std::string(optarg);
                break;

            case 'u' : // -u or --username
				username = std::string(optarg);
                break;

            case 'c' : // -c or --cert
				cert = std::string(optarg);
                break;

            case '?' : // Invalid option
                help();

            case -1 : // No more options
                break;

            default : // shouldn't happen :-)
                return(1);
        }
    }

    // check for mandatory arguments
    if(server.empty() || username.empty() || cert.empty()){
        printf("Mandatory arguments not specified\n");
        help();
    }





    return 0;
}
