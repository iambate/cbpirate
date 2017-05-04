#include<papi.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<sys/cdefs.h>
#include<time.h>
#include<unistd.h>

int main(int argc, char *argv[]){

    char *mem_cache_hog;
    int opt, no_of_ways, no_of_sets, size_cache_hog, index;
    float ipc, rtime, ptime;
    long long ins;
    size_t cache_size, block_size;

    while(-1 != (opt = getopt(argc, argv, "s:w:n:b:"))){
        switch(opt) {
            case 's':
                cache_size = atoi(optarg);
                break;
            case 'w':
                no_of_ways = atoi(optarg);
                break;
            case 'n':
                no_of_sets = atoi(optarg);
                break;
            case 'b':
                block_size = atoi(optarg);
                break;
        }
    }
    printf("size %d\n", cache_size);
    printf("no of ways %d\n", no_of_ways);
    printf("no of sets %d\n", no_of_sets);
    size_cache_hog = no_of_ways*1024*1024*2;
    mem_cache_hog = (char *)mmap(NULL, size_cache_hog, \
                                 PROT_READ | PROT_WRITE, \
                                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, \
                                 -1, 0);
    
    PAPI_ipc(&rtime, &ptime, &ins, &ipc);
    index=0;
    for(int i=0; i< no_of_sets; i++) {
        mem_cache_hog[index] = (char)i;
        index+=block_size;
    }
    PAPI_ipc(&rtime, &ptime, &ins, &ipc);
    printf("First time CPI %f\n", 1/ipc);

    for(int j=0; j<10; j++) {
        char tmpchar __attribute__((unused));
        index=0;
        PAPI_ipc(&rtime, &ptime, &ins, &ipc);
        for(int i=0; i< no_of_sets; i++) {
            tmpchar = mem_cache_hog[index];
            index+=block_size;
        }
        PAPI_ipc(&rtime, &ptime, &ins, &ipc);
        printf("CPI %f\n", 1/ipc);
    }

    index=0;
    PAPI_ipc(&rtime, &ptime, &ins, &ipc);
    for(int j=0; j <no_of_ways; j++) {
        for(int i=0; i< no_of_sets; i++) {
            mem_cache_hog[index] = (char)i+5;
            index+=block_size;
        }
        index = (j+1)*(1024*1024*2);
    }
    PAPI_ipc(&rtime, &ptime, &ins, &ipc);
    printf("With Bandwidth CPI %f\n", 1/ipc);

    munmap(mem_cache_hog, size_cache_hog);
    //PAPI_ipc(&rtime, &ptime, &ins, &ipc);
}
