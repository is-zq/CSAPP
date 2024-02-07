#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#define ADDR_LEN 64
#define swap(a,b) do { \
	char tmp = a; \
	a = b; \
	b = tmp; \
} while(0)

bool vflag = false;

struct Line
{
	int use_cnt;
	bool valid;
	int tag;
};

struct Cache
{
	int use_cnter;
	int s;
	int E;
	int b;
	int hit_count;
	int miss_count;
	int eviction_count;
	struct Line** buffer;
};

void Usage()
{
	printf("Usage: ./csim [-v] -s <s> -E <E> -b <b> -t <tracefile>\n");
}

void Dec2Bin(char* des,unsigned src)
{
	int i = 0;
	int len = ADDR_LEN;
	while(src != 0)
	{
		des[i++] = '0' + (src % 2);
		src /= 2;
	}
	while(i < len)
		des[i++] = '0';
	des[i] = '\0';

	int mid = ADDR_LEN / 2;
	--len;
	for(int i=0;i<mid;i++)
		swap(des[i],des[len-i]);
}

void LineInit(struct Line* line)
{
	line->use_cnt = INT_MAX;
	line->valid = false;
	line->tag = 0;
}

void CacheInit(struct Cache* cache,int s,int E,int b)
{
	int S = pow(2,s);
	cache->use_cnter = 0;
	cache->s = s;
	cache->E = E;
	cache->b = b;
	cache->hit_count = 0;
	cache->miss_count = 0;
	cache->eviction_count = 0;
	cache->buffer = (struct Line**)malloc(S * sizeof(struct Line*));
	for(int i=0;i<S;i++)
	{
		cache->buffer[i] = (struct Line*)malloc(E * sizeof(struct Line));
		for(int j=0;j<E;j++)
			LineInit(&cache->buffer[i][j]);
	}
}

void CacheDestroy(struct Cache* cache)
{
	int S = pow(2,cache->s);
	for(int i=0;i<S;i++)
		free(cache->buffer[i]);
	free(cache->buffer);
}

void LoadOrStore(struct Cache* cache,char* addr)
{
	int s = cache->s,E = cache->E,b = cache->b;
	int set_ind = 0,tag = 0;
	int cnt = 0,pow = 1;
	int i = ADDR_LEN - b - 1;
	while(cnt < s)
	{
		set_ind += (addr[i--] - '0') * pow;
		pow *= 2;
		++cnt;
	}
	pow = 1;
	while(i >= 0)
	{
		tag += (addr[i--] - '0') * pow;
		pow *= 2;
	}

	struct Line* set = cache->buffer[set_ind];
	struct Line* free_line = NULL;
	struct Line* LRU_line = &set[0];
	int min_use = LRU_line->use_cnt;
	for(int i=0;i<E;i++)
	{
		if(set[i].valid && set[i].tag == tag)
		{
			++cache->hit_count;
			set[i].use_cnt = cache->use_cnter++;
			if(vflag)
				printf(" hit");
			return;
		}

		if(free_line == NULL && set[i].valid == false)
			free_line = &set[i];

		if(set[i].use_cnt < min_use)
		{
			LRU_line = &set[i];
			min_use = LRU_line->use_cnt;
		}
	}
	++cache->miss_count;
	if(vflag)
		printf(" miss");

	if(free_line != NULL)
	{
		free_line->valid = true;
		free_line->tag = tag;
		free_line->use_cnt = cache->use_cnter++;
		return;
	}
	else
	{
		LRU_line->valid = true;
		LRU_line->tag = tag;
		LRU_line->use_cnt = cache->use_cnter++;
		++cache->eviction_count;
		if(vflag)
			printf(" eviction");
	}
}

int main(int argc,char* argv[])
{
	int op;
	int s,E,b = 0;
	char t[24] = {0};
	while((op = getopt(argc,argv,"vs:E:b:t:")) != -1)
	{
		switch(op)
		{
			case 'v':
				vflag = true;
				break;
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 't':
				strncpy(t,optarg,24);
				break;
			default:
				Usage();
				return -1;
				break;
		}
	}
	if(s == 0 || E == 0 || b == 0 || strlen(t) == 0)
	{
		Usage();
		return -1;
	}

	FILE* fp = fopen(t,"r");
	struct Cache cache;
	CacheInit(&cache,s,E,b);
	size_t n = 24;
	char* line = (char*)malloc(n);
	while(getline(&line,&n,fp) != -1)
	{
		if(line[0] == 'I')
			continue;
		else
		{
			line[strlen(line)-1] = '\0';
			if(vflag)
				printf("%s",line+1);
			char addr[64] = {0};
			int i = 3;
			while(line[i] != ',')
				i++;
			line[i] = '\0';
			unsigned d;
			sscanf(&line[3],"%x",&d);
			Dec2Bin(addr,d);
			LoadOrStore(&cache,addr);
			if(line[1] == 'M')
				LoadOrStore(&cache,addr);
			if(vflag)
				printf("\n");
		}
	}

	printSummary(cache.hit_count, cache.miss_count, cache.eviction_count);
	CacheDestroy(&cache);
	free(line);
	fclose(fp);
    return 0;
}
