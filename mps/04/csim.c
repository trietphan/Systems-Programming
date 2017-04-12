#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

typedef struct {
  char valid;
  long tag;
  int lru; //store least recently used for eviction
} line_t;

typedef struct {
  line_t *lines;
} set_t;

typedef struct {
  set_t *sets;
  int num_sets;
  int num_lines;
} cache_t;

void help_messages(char *argv[]);
int cache_init(cache_t *cache, int s, int E);
void process_each_line(long long address);
void read_trace_file(char *trace_name);
int process_args(int argc, char *argv[], int *s, int *E, int *b, char **t);

int main(int argc, char *argv[]) {
  int s = 0, E = 0, b = 0;
  int hits = 0, misses = 0, evics = 0;
  char *t = NULL;
  cache_t cache;
  
  process_args(argc, argv, &s, &E, &b, &t);
  cache_init(&cache, s, E);
  read_trace_file(t);
  printSummary(hits, misses, evics);
  return 0;
}

void help_messages(char *argv[]) {
  printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
  printf("Options:\n");
  printf("  -h         Print this help message.\n");
  printf("  -v         Optional verbose flag.\n");
  printf("  -s <num>   Number of set index bits.\n");
  printf("  -E <num>   Number of lines per set.\n");
  printf("  -b <num>   Number of block offset bits.\n");
  printf("  -t <file>  Trace file.\n\n");
  printf("Examples:\n");
  printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
  printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
}

/* init a cache with given number of s (num_set), E (num_line), and b (block_size) values */
int cache_init(cache_t *cache, int s, int E){
  cache->num_sets = (2 << s);
  cache->num_lines = E;
  cache->sets = (set_t *)malloc(cache->num_sets * sizeof(set_t));
  int i, j;

  for (i = 0; i < cache->num_sets; i++) {
    cache->sets[i].lines = (line_t *)malloc(E * sizeof(line_t));
    for (j = 0; j < cache->num_lines; j++) {
      cache->sets[i].lines[j].valid = 0;
      cache->sets[i].lines[j].tag = 0;
      cache->sets[i].lines[j].lru = 0;
    }
  }
  return 0;
}

int extract_bits(int address, int start, int end) {
  // Returns [start, end) bits from address
  int mask = (1 << (end - start)) - 1;
  return (address >> start) & mask;
}

/* Update the least recently use for eviction */
int lru_update(cache_t cache, int setbits, int cacheLine) {
  return 0;
}

void process_each_line(long long address) {
  
}

void read_trace_file(char *trace_name) {
  char buff[100];
  FILE *tn = fopen(trace_name, "r");
  if (tn == NULL) {
    printf("%s: The trace does not exist", trace_name);
    return;
  }
  while (fgets(buff, 100, tn)) {
    unsigned long long address = 0;
    unsigned length = 0;
    if (buff[1] == 'M' || buff[1] == 'S' || buff[1] == 'L') {
      sscanf(buff+2, "%llx,%u", &address, &length);
      /* printf("%c %llx,%u\n", buff[1], address, length); */
      process_each_line(address);
    }
  }
  fclose(tn);
}

int process_args(int argc, char *argv[], int *s, int *E, int *b, char **t) {
  int opt;
  while ((opt = getopt(argc, argv, "vs:E:b:t:")) != -1) {
      switch(opt) {
      case 's':
        *s = atoi(optarg);
        break;
      case 'b':
        *b = atoi(optarg);
        break;
      case 'E':
        *E = atoi(optarg);
        break;
      case 't':
        *t = optarg;
        break;
      case 'h':
        help_messages(argv);
        break;
      default:
        help_messages(argv);
        exit(-1);
        break;
      }
  }
  if(*s <= 0 || *b <= 0 || *E <= 0 || *t == NULL) {
      help_messages(argv);
      exit(-1);
  }
  return 0;
}
