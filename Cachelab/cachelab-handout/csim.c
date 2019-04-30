#include "cachelab.h"
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
typedef unsigned long long address;
/**
 * cParms struct, holds the information of the cache
 * @param s 2^s bits stored
 * @param b Block size
 * @param E Number of cache lines
 * @param B Line block size
 * @param hits # of hits the cache has had
 * @param miss # of misses the cache has had
 * #param evictions # of evictions the cache has had
 */
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
 * @param recentUse value to store hold recently the data was used
 * @param valid Whether or not this line is in use
 * @param a Address of the operation on this line
 */
typedef struct {
	//Recently used tag
	int recentUse;
	//Valid line
	int valid;
	//Memory address
	address a;
} cacheLine;

/**
 * Holds a 1D array of cacheLines
 * @param lines Array of cacheLines in the set
 */
typedef struct {
	cacheLine *lines;

} cacheSet;
/**
 * Actual cache, holds a 1D array of sets to represent cache
 * @param sets Array of cacheSets in the cache
 */
typedef struct {

	cacheSet *sets;
} cache;
/**
 * Checks if the given line array has the correct char array ending character
 * @param line cacheLine to examine
 */
void checkLine(char *line) {
	//Loop through size of the cache line

	for (int i = 0; i < 100; i++) {

		//Get the current element in the line
		char el = line[i];
		//If the end of the char array
		if (el == '\0') {
			i=100;

		}
		//Else look if we are at new line, set it to be the end of the array
		else if (el == '\n') {
			line[i] = '\0';
		}
	}
}
/**
 * Builds a cache with the given # of sets and lines
 * @param sets # of sets for cache to be constructed with
 * @param lines # of lines per set for the cache to be constructed with
 * @return Returns a newly constructed cache
 */
cache buildCache(int sets, int lines) {
	//Declare structs
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
		//Initialize struct variables
		for (int j = 0; j < lines; j++) {
			l.recentUse = 0;
			l.a = 0;
			l.valid = 0;

			s.lines[j] = l;
		}
	}
	return c;
}
/**
 * Retrieves the index of the next empty line
 * @param set cacheSet to look for next empty line
 * @param lines # of lines within the set
 * @return index of next empty line, -1 if it does not exist
 */
int nextEmptyLine(cacheSet set, int lines) {
	cacheLine l;
	//Loop the set lines
	for (int i = 0; i < lines; i++) {
		l = set.lines[i];
		//Return the next empty line
		if (l.valid == 0) {
			return i;
		}

	}
	return -1;
}

/**
 * Enact LRU policy on passed in set, evicting the oldest data
 * @param set cacheSet to pass in
 * @param lines # of lines for set
 * @param used Array of used lines
 * @return Index of oldest data to evict
 */
int enforceLRUPolicy(cacheSet set, int lines, int *used) {
	//Get the oldest oldest
	int max = set.lines[0].recentUse;
	//Get the youngest data
	int min = set.lines[0].recentUse;

	int lastUsed = 0;
	//Loop through the lines, checking to see when they were last recently used
	for (int i = 1; i < lines; i++) {
		if (min > set.lines[i].recentUse) {
			lastUsed = i;
			min = set.lines[i].recentUse;
		}
		if (max < set.lines[i].recentUse) {
			max = set.lines[i].recentUse;
		}

	}
	//Update which is the oldest and youngest
	used[0] = min;
	used[1] = max;
	//Return index of the oldest data
	return lastUsed;

}
/**
 * Modifies the cache c based on the instruction stored in the address a, as well as updates the total
 * hits, misses, and evictions in cParms parms
 * @param c Cache to update
 * @param parms cParms struct to update stats on
 * @param a Unsigned long long address that holds instruction
 */
cParms insert(cache c, cParms parms, address a) {
	int cacheChecked = 1;
	//Get # of lines
	int lines = parms.E;
	//Get # of cache hits
	int recentHits = parms.hits;

	//Remove offset and block size
	int tag = 64 - (parms.s + parms.b);
	//Get beginning of address by shifting by the offset
	address start = a >> (parms.s + parms.b);
	//Get the address of where we are accessing
	address accessIndex = (a << tag) >> (tag + parms.b);
	//Get the corresponding set
	cacheSet set = c.sets[accessIndex];
	//Begin looking through cache
	for (int i = 0; i < lines; i++) {
		//If the data is valid aka already in use
		if (set.lines[i].valid == 1) {
			//If we are the correct tag
			if (set.lines[i].a == start) {
				//Update stats and recently used level
				set.lines[i].recentUse++;
				parms.hits++;
			}

		}
		//If the cache has been checked and the current line is not in use
		else if (set.lines[i].valid == 0 && cacheChecked) {
			cacheChecked = 0;
		}

	}
	//Update miss if there are no hits, thus there must've been a miss
	if (recentHits == parms.hits) {
		parms.miss++;
	} else {
		//Return the cParms struct if there is an update
		return parms;
	}
	//Allocate memory for the lines that have been accessed
	int *accessed = (int *) malloc(sizeof(int) * 2);
	//Get the number of lines mondified after enforcing the cache policy and retrieve the status of accessed lines
	int linesModified = enforceLRUPolicy(set, parms.E, accessed);
	//If cache has been fully checked
	if (cacheChecked == 1) {
		//Update evictions and insert the new address into the line
		parms.evictions++;
		set.lines[linesModified].a = start;
		//Update the recently used status
		set.lines[linesModified].recentUse = accessed[1] + 1;
	} else {
		//Get the next open spot in the set
		int nextLine = nextEmptyLine(set, parms.E);
		//Update what is stored in that line with the new address
		set.lines[nextLine].a = start;
		set.lines[nextLine].valid = 1;
		//Update the recently used status
		set.lines[nextLine].recentUse = accessed[1] + 1;
	}
	//Free memory
	free(accessed);
	return parms;

}
/**
 * Frees up the memory associated with a cache c, # of cacheSets sets, and # of cacheLines lines
 * @param c Cache to free
 * @param sets # of cacheSets in Cache c
 * @param lines # of cacheLines in cacheSets in Cache C
 *
 */
void destroyCache(cache c, int sets, int lines) {
	//Loop through all sets in the cache
	for (int i = 0; i < sets; i++) {
		cacheSet currentSet = c.sets[i];
		//Free the memory
		free(currentSet.lines);

	}
	//Free the set structs associated with the cache
	free(c.sets);
}
int main(int argc, char **argv) {
	cParms cacheParms;
	//Trace file pointer
	char *tFile= "";
	//Variable to hold current comand line argument
	char argument;
	int verbosity;

	//Loop through the command line arguments, making sure they exist and are correct
	while ((argument = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
		if (argument == 's') {
			cacheParms.s = atoi(optarg);
			cacheParms.S = 1 << cacheParms.s;
		}
		if (argument == 'E') {
			cacheParms.E = atoi(optarg);
		}
		if (argument == 'b') {
			cacheParms.b = atoi(optarg);
			cacheParms.B = 1 << cacheParms.b;
		}
		if (argument == 't') {
			tFile = optarg;
		}
		if (argument == 'v') {
			verbosity =1;
		}
	}
	cacheParms.hits = 0;
	cacheParms.miss = 0;
	cacheParms.evictions = 0;
	FILE *file;
	file = fopen(tFile, "r");
	cache simCache = buildCache(cacheParms.S, cacheParms.E);

	//While file is valid
	if (file != 0) {
		char instruction;
		address a;
		int size;
		//Look for instructions
		while (fscanf(file, " %c %llx, %d", &instruction, &a, &size) == 3) {
			int recentHits = cacheParms.hits;
			int recentMisses = cacheParms.miss;
			int recentEvictions = cacheParms.evictions;
			//If it is a move opertaion
			if (instruction == 'M') {
				//Print out special output if verbosity is set
				if (verbosity == 1) {
					printf("M %llx,%d", a, size);
					for (int i = 0; i < 2; i++) {
						cacheParms = insert(simCache, cacheParms, a);
						if (recentMisses != cacheParms.miss) {
							printf(" miss");
						} else if (recentHits != cacheParms.hits) {
							printf(" hit");
						} else if (recentEvictions != cacheParms.evictions) {
							printf(" eviction");
						}
						recentHits = 0;
						recentMisses = 0;
						recentEvictions = 0;
					}
					printf("\n");
				} else {
					cacheParms = insert(simCache, cacheParms, a);
					cacheParms = insert(simCache, cacheParms, a);
				}

			}
			//If it is a store opertaion
			else if (instruction == 'S') {
				//Update cache with new instruction
				cacheParms = insert(simCache, cacheParms, a);
				//Print out special output if verbosity is set

				if (verbosity == 1) {
					printf("S %llx,%d", a, size);
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
			//If it is a load operation
			else if (instruction == 'L') {
				//Update cache with new instruction

				cacheParms = insert(simCache, cacheParms, a);
				//Print out special output if verbosity is set

				if (verbosity == 1) {
					printf("L %llx, %d", a, size);
					if (recentMisses != cacheParms.miss) {
						printf(" miss");
					} else if (recentHits != cacheParms.hits) {
						printf(" hit");
					} else if (recentEvictions != cacheParms.evictions) {
						printf(" eviction");
					}
				}
			}
		}
	} else {
		// File unreadable
		printf("failed to open file");
		fprintf(stderr, "ERROR: Can't open file: %s\n", tFile);
		return 1;
	}
	//Close the file reader
	fclose(file);
	//Free up the memory taken by the cache
	destroyCache(simCache, cacheParms.S, cacheParms.E);
	//Print the summary
	printSummary(cacheParms.hits, cacheParms.miss, cacheParms.evictions);

}
