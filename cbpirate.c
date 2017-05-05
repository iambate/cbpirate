#include<papi.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<sys/cdefs.h>
#include<time.h>
#include<unistd.h>
#define MB 1024*1024

char *mem_cache_hog;
void bw_req(int no_of_bw_req, int no_warm_req){
    static int start_point=0;
    int new_no_req = no_of_bw_req;
    if(no_warm_req > no_of_bw_req) {
        start_point = (24 + (no_of_bw_req - no_warm_req) + start_point)%24;
        new_no_req = no_warm_req;
    }
    for(int i=0; i< new_no_req;i++){
        mem_cache_hog[start_point*2*MB] = (char)i;
        start_point=(start_point+1)%24;
    }
}

int main(int argc, char *argv[]){

    int opt, no_of_ways, no_of_sets;
    float ipc, rtime, ptime;
    long index;
    long long ins;
    size_t cache_size, block_size, size_cache_hog;
    long long value[3]={0, 0, 0};
    int array[3]={PAPI_L3_TCM, PAPI_TOT_INS, PAPI_TOT_CYC};

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
    size_cache_hog = 2*no_of_ways*1024*1024*2;
    printf("size_cache_hog %ld\n", size_cache_hog);
    mem_cache_hog = (char *)mmap(NULL, size_cache_hog, \
                                 PROT_READ | PROT_WRITE, \
                                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, \
                                 -1, 0);
    if (mem_cache_hog==NULL) {
        fprintf(stderr, "mmap failed");
        exit(-1);
    }

    PAPI_start_counters(array,3);
    index=0;
    for(int i=0; i< no_of_sets; i++) {
        mem_cache_hog[index] = (char)i;
        index+=block_size;
    }
    PAPI_read_counters(value, 3);
    printf("First time CPI %f\n", value[3]/value[2]);

    for(int j=0; j<10; j++) {
        char tmpchar __attribute__((unused));
        index=0;
        PAPI_read_counters(value, 3);
        for(int i=0; i< no_of_sets; i++) {
            tmpchar = mem_cache_hog[index];
            index+=block_size;
        }
        PAPI_read_counters(value, 3);
        printf("First time CPI %f cache misses %d\n", (float)value[2]/(float)value[1], value[0]);
    }

    PAPI_read_counters(value, 3);
    bw_req(36, 0);
    PAPI_read_counters(value, 3);
    printf("WITH bandwidth CPI %f, cache misses %ld\n", ((float)value[2])/((float)value[1]), value[0]);

    PAPI_ipc(&rtime, &ptime, &ins, &ipc);
    bw_req(12, 0);
    PAPI_read_counters(value, 3);
    printf("WITH bandwidth CPI %f, cache misses %ld\n", ((float)value[2])/((float)value[1]), value[0]);

    PAPI_read_counters(value, 3);
    bw_req(36, 0);
    PAPI_read_counters(value, 3);
    printf("WITH bandwidth CPI %f, cache misses %ld\n", ((float)value[2])/((float)value[1]), value[0]);

    PAPI_read_counters(value, 3);
    bw_req(12, 0);
    PAPI_read_counters(value, 3);
    printf("WITH bandwidth CPI %f, cache misses %ld\n", ((float)value[2])/((float)value[1]), value[0]);
    sleep(5);
    munmap(mem_cache_hog, size_cache_hog);
}
