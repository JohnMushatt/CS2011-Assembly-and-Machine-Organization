#include "cachelab.h"
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

typedef unsigned long long address;
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
/**
 * Structure to hold data for a single cache line
 */
typedef struct {
	//Recently used tag
	int recentUse;
	//Valid line
	int valid;
	//Memory address
	address a;
	//Cache block
	char *block;
} cacheLine;

/**
 * Holds a 1D array of cacheLines
 */
typedef struct {
	cacheLine *lines;

} cacheSet;
/**
 * Actual cache, holds a 1D array of sets to represent cache
 */
typedef struct {

	cacheSet *sets;
} cache;

cache buildCache(int sets, int lines) {
	//Initialize structs
	cache c;
	cacheSet s;
	cacheLine l;
	//Allocate memory enough memory for cacheSets
	c.sets = (cacheSet *) malloc(sizeof(cacheSet) * sets);
	//Allocate memory for lines per set
	for (int i = 0; i < sets; i++) {
		//Allocate enough memory to hold cacheLine
		s.lines = (cacheLine *) malloc(sizeof(cacheLine) * lines);
		//Default initialization
		c.sets[i] = s;

		for (int j = 0; j < lines; j++) {
			l.recentUse = 0;
			l.a = 0;
			l.valid = 0;

			s.lines[j] = l;
		}
	}
	return c;
}
int nextEmptyLine(cacheSet set, int lines) {
	cacheLine l;
	for (int i = 0; i < lines; i++) {
		l = set.lines[i];
		if (l.valid == 0) {
			return i;
		}

	}
	return -1;
}
/**
 *
 */
int enforceLRUPolicy(cacheSet set, int lines, int *used) {
	int max = set.lines[0].recentUse;
	int min = set.lines[0].recentUse;

	int lastUsed = 0;

	for (int i = 1; i < lines; i++) {
		if (min > set.lines[i].recentUse) {
			lastUsed = i;
			min = set.lines[i].recentUse;
		}
		if (max < set.lines[i].recentUse) {
			max = set.lines[i].recentUse;
		}

	}
	used[0] = min;
	used[1] = max;
	return lastUsed;

}
cParms insert(cache c, cParms parms, address a) {
	int cacheChecked = 1;
	int lines = parms.E;
	int recentHits = parms.hits;

	//Remove offset and block size
	int tag = 64 - parms.s - parms.b;
	//Get beginning of address by shifting by the offset
	address start = a >> (parms.s + parms.b);
	//Get the address of where we are accessing
	address accessIndex = (a << tag) >> (tag + parms.b);
	//Get the corresponding set
	cacheSet set = c.sets[accessIndex];

	for (int i = 0; i < lines; i++) {

		if (set.lines[i].valid == 1) {

			if (set.lines[i].a == start) {
				set.lines[i].recentUse++;
				parms.hits++;
			}

		} else if (set.lines[i].valid == 0 && cacheChecked) {
			cacheChecked = 0;
		}

	}

	if (recentHits == parms.hits) {
		parms.miss++;
	} else {
		return parms;
	}

	int *accessed = (int *) malloc(sizeof(int) * 2);
	int linesModified = enforceLRUPolicy(set, parms.E, accessed);
	if (cacheChecked == 1) {
		parms.evictions++;
		set.lines[linesModified].a = start;
		set.lines[linesModified].recentUse = accessed[1] + 1;
	} else {
		int nextLine = nextEmptyLine(set, parms.E);
		set.lines[nextLine].a = start;
		set.lines[nextLine].valid = 1;
		set.lines[nextLine].recentUse = accessed[1] + 1;
	}
	free(accessed);
	return parms;

}

int main(int argc, char **argv) {
	cParms cacheParms;
	//Trace file pointer
	char tFile;
	//Variable to hold current comand line argument
	char argument;
	int verbosity;
	//Loop through the command line arguments, making sure they exist and are correct
	while (argument = getopt(argc, argv, "s:E:b:t:vh") != -1) {
		if (argument == 's') {
			cacheParms.s = atoi(optarg);
		}
		if (argument == 'E') {
			cacheParms.E = atoi(optarg);
		}
		if (argument == 'b') {
			cacheParms.b = atoi(optarg);

		}
		if (argument == 't') {
			tFile = optarg;
		}
		if (argument) {
			verbosity = atoi(optarg);
		} else {
			exit(1);
		}
	}

	FILE *file;
	file = fopen(tFile, "r");
	cache simCache = buildCache(cacheParms.S, cacheParms.E);

	if (file != 0) {
		char instruction;
		address a;
		int size;

		while (fscanf(file, " %c %llx, %d", &instruction, &a, &size) == 3) {
			int recentHits = cacheParms.hits;
			int recentMisses = cacheParms.miss;
			int recentEvictions = cacheParms.evictions;

			if (instruction == 'M') {
				if (verbosity == 1) {
					printf("M %llx, %d", a, size);
					for (int i = 0; i < 2; i++) {
						cacheParms = insert(simCache, cacheParms, a);
						if (recentMisses != cacheParms.miss) {
							printf(" miss");
						} else if (recentHits != cacheParms.hits) {
							printf(" hit");
						} else if (recentEvictions != cacheParms.evictions) {
							printf(" eviction");
						}
						recentHits=0;
						recentMisses=0;
						recentEvictions=0;
					}
					printf("\n");
				}
				else {
					cacheParms = insert(simCache,cacheParms,a);
					cacheParms = insert(simCache,cacheParms,a);
				}
			}
			if (instruction == 'S') {
				cacheParms = insert(simCache, cacheParms, a);
				if (verbosity == 1) {
					printf("S %llx, %d", a, size);
					if (recentMisses != cacheParms.miss) {
						printf(" miss");
					} else if (recentHits != cacheParms.hits) {
						printf(" hit");
					} else if (recentEvictions != cacheParms.evictions) {
						printf(" eviction");
					}
					printf("\n");
				}
			}
			if(instruction == 'L') {
				cacheParms = insert(simCache,cacheParms,a);
				if(verbosity ==1) {
					printf("L %llx, %d",a,size);
					if(recentMisses!= cacheParms.miss) {
						printf(" miss");
					}
					else if(recentHits != cacheParms.hits) {
						printf(" hit");
					}
					else if(recentEvictions !=cacheParms.evictions) {
						printf(" eviction");
					}
				}
			}
		}
	}
	else {
		// File unreadable
				fprintf( stderr, "ERROR: Can't open file: %s\n", tFile);
				return 1;
	}
	fclose(file);
	printSummary(cacheParms.hits, cacheParms.miss, cacheParms.evictions);
}
