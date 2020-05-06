//Bryce Nesius

#include "cachelab.h"
#include "getopt.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>

typedef struct
{
	int valid;
	int tag;
	int last_update;
} line_t;

typedef struct
{
	int num_lines;
	line_t **lines;
} set_t;

typedef struct
{
	int num_sets;
	set_t **sets;
} cache_t;

typedef struct
{
	char operator;
	long address;
	int memSize;
	int s;
	int b;
	int E;
} parameter_t;

typedef struct
{
	int hits;
	int misses;
	int evictions;
} summary_t;

void dump_cache(cache_t *cache)
{
	for (int set_idx = 0; set_idx < cache->num_sets; set_idx++)
	{
		printf("Set %d\n", set_idx);
		set_t *set = cache->sets[set_idx];
		for (int line_idx = 0; line_idx < set->num_lines; line_idx++)
		{
			line_t *line = set->lines[line_idx];
			printf("\tLine %d: %d %d %d\n", line_idx, line->valid,
				   line->tag, line->last_update);
		}
	}
}

void perform_cache_op(cache_t *cache, parameter_t *params, summary_t *summary, int verbose)
{
	if (verbose)
	{
		printf("%c %10lx %d\n", params->operator, params->address, params->memSize);
	}
	
	// Simulate one cache operation.
	
	int index;
	int result;
	unsigned long long int temp;

	//find type
	int tagSize = (64 - (params->s + params->b));
	unsigned long long inputTag = params->address >> (param->s + param->b);
	temp = params->address << (tagSize);
	unsigned long long setIndex = temp >> (tagSize + params->b);

	for (index = 0; index < params->E; index++) {
		if(cache->sets->lines[index]->valid) {
			if(cache->sets->lines[index]->tag == inputTag) {
				cache->sets->lines[index]->last_update ++;
				summary->hits ++;
			}
		}
	}
}

int main(int argc, char **argv)
{
	//command arg options
	int setPower = 0;
	int E = 1; // Must be at least one.
	// int help = 0;
	int blockPower = 0;
	int verbose = 0;
	char *filePath;
	int option;
	parameter_t parameters;

	while ((option = getopt(argc, argv, "s:E:b:t:v")) != -1)
	{
		switch (option)
		{
		case 's':
			setPower = atoi(optarg);
			parameters.s = setPower;
			break;
		case 'E':
			E = atoi(optarg);
			parameters.E = E;
			break;
		case 'b':
			blockPower = atoi(optarg);
			parameters.b = blockPower;
			break;
		case 't':
			filePath = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		}
	}

	//track data
	summary_t cacheSummary;
	cacheSummary.evictions = 0;
	cacheSummary.hits = 0;
	cacheSummary.misses = 0;

	int B = pow(2, blockPower); //bytes per block
	int S = pow(2, setPower);	//sets in the cache

	printf("S = %d, E = %d, B = %d\n", S, E, B);

	cache_t *cache = (cache_t *)malloc(sizeof(cache_t)); //allocates memory (cache)
	cache->num_sets = S;								 //tracks # of sets
	cache->sets = (set_t **)malloc(S * sizeof(set_t *)); //allocates *'s to sets

	for (int set_idx = 0; set_idx < S; set_idx++)
	{
		set_t *set = (set_t *)malloc(sizeof(set_t));		  //allocates one set
		set->num_lines = E;									  //tracks # of lines per set
		set->lines = (line_t **)malloc(E * sizeof(line_t *)); //allocate * to lines

		for (int line_idx = 0; line_idx < E; line_idx++)
		{
			line_t *line = (line_t *)malloc(sizeof(line_t)); //allocates one line
			line->valid = 0;								 //not valid
			line->tag = INT_MAX;							 //bonus tag bits
			line->last_update = INT_MAX;					 //modified in far future
			set->lines[line_idx] = line;					 //store pointer to line
		}
		cache->sets[set_idx] = set; //store * to a set
	}
	dump_cache(cache);

	//read file
	FILE *traceFile = fopen(filePath, "r");
	char line[80];
	while (fgets(line, 80, traceFile) != NULL)
	{
		if (line[0] == 'I')
		{
			continue; // Skip instruction operations.
		}
		parameters.operator= line[1]; // Grab the operation.
		sscanf(line + 3, "%lx,%u", &parameters.address, &parameters.memSize);
		perform_cache_op(cache, &parameters, &cacheSummary, verbose);
	}
	fclose(traceFile);

	printSummary(cacheSummary.hits, cacheSummary.misses, cacheSummary.evictions);
	return 0;
}
