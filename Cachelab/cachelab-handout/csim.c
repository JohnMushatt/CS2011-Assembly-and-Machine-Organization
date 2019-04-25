#include "cachelab.h"
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
typedef struct {
	//2^s bits
	int s;
	//Block size
	int b;
	//Cache lines
	int E;
	//Sets
	int S;
	//Line block size
	int B;
	//Number of cache hits
	int hits;
	//Number of cache misses
	int miss;
	//Number of cache evictions
	int evictions;
} cParms;
int main( int argc, char **argv)
{
	cParms *cacheParms;
	//Trace file pointer
	char tFile;
	//Variable to hold current comand line argument
	char argument;
	int verbosity;
	//Loop through the command line arguments, making sure they exist and are correct
	while(argument=getopt(argc,argv,"s:E:b:t:vh")!=-1) {
		if(argument=='s') {
			cacheParms->s=atoi(optarg);
		}
		if(argument=='E') {
			cacheParms->E = atoi(optarg);
		}
		if(argument=='b') {
			cacheParms->b= atoi(optarg);

		}
		if(argument =='t') {
			tFile=optarg;
		}
		if(argument) {
			verbosity = atoi(optarg);
		}
		else {
			exit(1);
		}
	}
}
