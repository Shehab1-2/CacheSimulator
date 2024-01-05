#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Global variables for tracking cache statistics
int readCount = 0;
int writeCount = 0;
int hits = 0;
int misses = 0;
char policy;  // Cache replacement policy ('f' for FIFO, 'l' for LRU)
int counter = 0;  // Counter for tracking usage order

// Structure to represent a line in the cache
struct Line {
	unsigned long long int tag;  // Tag part of the address
	int age;  // Age of the line for replacement policy
	int lastUsed;  // Counter value when the line was last used
};

// Structure to represent a set in the cache
struct Set {
	struct Line *lines;  // Array of lines
};

// Structure to represent the entire cache
struct Cache {
	struct Set *sets;  // Array of sets
	int cacheSize;  // Total size of the cache
	int lineSize;  // Size of each line
	int linesPerSet;  // Number of lines per set
	int numSets;  // Number of sets in the cache
	int offsetBits;  // Number of offset bits
	int setBits;  // Number of set index bits
};

// Recursive function to calculate log base 2
int loga(int a){
	return (a > 1) ? 1 + loga(a / 2) : 0;
}

// Function to create and initialize a cache
struct Cache *createCache(int cacheSize, int lineSize, int numSets, int linesPerSet){
	struct Cache *cache = malloc(sizeof(struct Cache));
	cache->offsetBits = loga(lineSize);
	cache->setBits = loga(numSets);
	cache->sets = malloc(sizeof(struct Set) * numSets);
	cache->lineSize = lineSize;
	cache->numSets = numSets;
	cache->linesPerSet = linesPerSet;
	for (int i = 0; i < numSets; i++){
		cache->sets[i].lines = malloc(sizeof(struct Line) * linesPerSet);
		for (int j = 0; j < linesPerSet; j++){
			cache->sets[i].lines[j].age = 20000000;
			cache->sets[i].lines[j].tag = 0;
		}
	}
	return cache;
}

// Function to free the memory allocated for the cache
void destroyCache(struct Cache *cache){
	for (int i = 0; i < cache->numSets; i++){
		free(cache->sets[i].lines);
	}
	free(cache->sets);
	free(cache);
}

// Function to simulate cache usage
void useCache(struct Cache *cache, unsigned long long int hex, char type, bool prefetchNext, bool curPre){
	counter++;
	unsigned long long int set = cache->setBits == 0 ? 0 : (hex << (64 - cache->offsetBits - cache->setBits)) >> (64 - cache->setBits);
	unsigned long long int tag = hex >> (cache->offsetBits + cache->setBits);
	int oldestAge = 0;
	int oldestAgeLine = 0;
	int oldestTimeUsed = 1000000;
	int oldestTimeLine = 0;
	bool match = false;
	for (int i = 0; i < cache->linesPerSet; i++){
		if (cache->sets[set].lines[i].age < 10000000 && cache->sets[set].lines[i].tag == tag){
			match = true;
			if (!curPre) { hits++; }
			if (type == 'W') {
				if (!curPre) { writeCount++; }
			} else {
				cache->sets[set].lines[i].lastUsed = counter;
			}
		}
		cache->sets[set].lines[i].age++;
		if (cache->sets[set].lines[i].age > oldestAge){
			oldestAge = cache->sets[set].lines[i].age;
			oldestAgeLine = i;
		}
		if (cache->sets[set].lines[i].lastUsed < oldestTimeUsed){
			oldestTimeUsed = cache->sets[set].lines[i].lastUsed;
			oldestTimeLine = i;
		}
	}
	if (match == false){
		if (!curPre) { misses++; }
		readCount++;
		if (type == 'W') {
			if (!curPre) { writeCount++; }
		}
		if (policy == 'f'){
			cache->sets[set].lines[oldestAgeLine].age = 0;
			cache->sets[set].lines[oldestAgeLine].tag = tag;
			cache->sets[set].lines[oldestAgeLine].lastUsed = counter;
		} else {
			cache->sets[set].lines[oldestTimeLine].age = 0;
			cache->sets[set].lines[oldestTimeLine].tag = tag;
			cache->sets[set].lines[oldestTimeLine].lastUsed = counter;
		}
		if (prefetchNext){
			hex += cache->lineSize;
			useCache(cache, hex, 'R', false, true);
		}
	}
}

// Main function
int main(int argc, char **argv) {
	// Parsing command line arguments to get cache parameters
	int cacheSize = atoi(argv[1]);
	int lineSize = atoi(argv[4]);
	int linesPerSet = cacheSize / lineSize;
	if (sscanf(argv[2], "assoc:%d", &linesPerSet) == 0){
		if (strcmp(argv[2], "direct") == 0){
			linesPerSet = 1;
		}
	}
	int numSets = cacheSize / (lineSize * linesPerSet);
	struct Cache *cache = createCache(cacheSize, lineSize, numSets, linesPerSet);
	char string1[40];
	char type = 0;
	if (strcmp(argv[3], "fifo") == 0){
		policy = 'f';
	} else {
		policy = 'l';
	}
	char string2[40];
	char buffer[80];
	FILE *file = fopen(argv[5], "r");
	while (fgets(buffer, 80, file) != 0){
		if (strlen(buffer) < 6){
			continue;
		}
		sscanf(buffer, "%s %c %s", string1, &type, string2);
		unsigned long long int hex = strtoul(string2, NULL, 16);
		useCache(cache, hex, type, false, false);
	}
	printf("Prefetch 0\n");
	printf("Memory reads: %d\n", readCount);
	printf("Memory writes: %d\n", writeCount);
	printf("Cache hits: %d\n", hits);
	printf("Cache misses: %d\n", misses);
	struct Cache *cache2 = createCache(cacheSize, lineSize, numSets, linesPerSet);
	rewind(file);
	readCount = 0;
	writeCount = 0;
	hits = 0;
	misses = 0;
	while (fgets(buffer, 80, file) != 0){
		if (strlen(buffer) < 6){
			continue;
		}
		sscanf(buffer, "%s %c %s", string1, &type, string2);
		unsigned long long int hex = strtoul(string2, NULL, 16);
		useCache(cache2, hex, type, true, false);
	}
	printf("Prefetch 1\n");
	printf("Memory reads: %d\n", readCount);
	printf("Memory writes: %d\n", writeCount);
	printf("Cache hits: %d\n", hits);
	printf("Cache misses: %d\n", misses);
	destroyCache(cache);
	destroyCache(cache2);
}
