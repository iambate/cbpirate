#define _GNU_SOURCE
#include<papi.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/cdefs.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<time.h>
#include<unistd.h>
#define MB 1024*1024
#define MEM 0
#define BW 1

// Printing as per the flags
//#define DEBUG
//#define DEBUGV
//#define COUNTERS

// Global variables
// TODO: many variables are global. Keep only need.
// Rest should be passed as args to threads
char *glb_mem_cache_hog, **glb_cmd, *glb_target_cpus, *glb_filename;
int glb_no_of_ways, glb_no_of_sets, glb_nthreads=1, glb_pthread_exit_status;
int glb_no_of_hot_ways, glb_chunk_size, glb_no_of_chunk_lines, glb_no_of_bw_req;
int glb_total_events, *glb_events[2];
size_t glb_cache_size, glb_block_size, glb_size_cache_hog;
long glb_running_event_no, glb_bw_running_event_no, glb_interval;


void bw_req(int no_of_bw_req, int no_warm_req){
    static int start_point=0;
    int new_no_req = no_of_bw_req;
    if(no_warm_req > no_of_bw_req) {
        start_point = ((2*glb_no_of_ways) + (no_of_bw_req - no_warm_req) + start_point)%(2*glb_no_of_ways);
        new_no_req = no_warm_req;
    }
    for(int i=0; i< new_no_req;i++){
#ifdef DEBUGVV
        printf("In bw req, storing at %p\n", &glb_mem_cache_hog[start_point*2*MB]);
#endif
        glb_mem_cache_hog[start_point*2*MB] = (char)(i*new_no_req);
        start_point=(start_point+1)%(2*glb_no_of_ways);
    }
}

void* change_bw(void *tno) {
    int this_thread_no = *(int *)tno;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(3, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    while(glb_pthread_exit_status==0) {
        if(glb_bw_running_event_no!=glb_running_event_no) {
#ifdef DEBUGV
            printf("In change bw, no_of_bw_req: %d\n", glb_no_of_bw_req);
#endif
            bw_req(glb_no_of_bw_req, glb_no_of_hot_ways);
            glb_bw_running_event_no = glb_running_event_no;
        }
    }
    return NULL;
}
void* keep_cache_hot(void *tno){

    int index, tmp_no_of_hot_ways;
    int this_thread_no = *(int *)tno;
    int cpu_no = 2 + this_thread_no % 2;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_no, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);

    // 1st line in cache of every way will be handle by bw_req
    int starting_i = (this_thread_no == 0)?1:0;
    char tmp __attribute__((unused));
    while(glb_pthread_exit_status==0) {
#ifdef DEBUGVVV
        printf("In keep cache with tid %d, no of hot ways: %d\n",
                this_thread_no, glb_no_of_hot_ways);
#endif
        // Use read mutex lock to read
        tmp_no_of_hot_ways = glb_no_of_hot_ways;
        for(int j = 0; j < tmp_no_of_hot_ways; j++) {
            index = this_thread_no*glb_chunk_size + j*(glb_cache_size/glb_no_of_ways);
            for(int i = starting_i; i < glb_no_of_chunk_lines; i++) {
                tmp = glb_mem_cache_hog[index];
                index += glb_block_size;
            } // for i end
        } // for j end
    } // while end
    return NULL;
}

void read_events(){
    FILE *f = fopen(glb_filename, "r");
    int wb, mem;
    fscanf(f, "%d\n", &glb_total_events);
    glb_events[MEM] = (int *)malloc(glb_total_events*sizeof(int));
    glb_events[BW] = (int *)malloc(glb_total_events*sizeof(int));
    for(int i=0;i<glb_total_events;i++) {
        fscanf(f, "%d\t%d\n", &(glb_events[MEM][i]), &(glb_events[BW][i]));
    }
    fclose(f);
}

void global_init(int argc, char *argv[]){

    int opt;
    char *errstr;
    glb_target_cpus = NULL;
    glb_filename = NULL;
    glb_no_of_hot_ways=0;
    glb_running_event_no=0;
    glb_bw_running_event_no=-1;
    while(-1 != (opt = getopt(argc, argv, "s:w:n:b:t:c:f:i:"))){
        switch(opt) {
            case 's':
                glb_cache_size = atoi(optarg);
                break;
            case 'w':
                glb_no_of_ways = atoi(optarg);
                break;
            case 'n':
                glb_no_of_sets = atoi(optarg);
                break;
            case 'b':
                glb_block_size = atoi(optarg);
                break;
            case 't':
                glb_nthreads = atoi(optarg);
                break;
            case 'i':
                glb_interval = atol(optarg);
                break;
            case 'c':
                glb_target_cpus = optarg;
                break;
            case 'f':
                glb_filename = optarg;
                break;
        }
    }

    //CHecking
    if(glb_filename==NULL) {
        errstr = "memory & bandwidth graph file not given\n";
        goto err;
    }
    if(glb_target_cpus==NULL) {
        errstr = "Please provide the cpu on which target should run\n";
        goto err;
    }
    glb_cmd = (char **)malloc((argc-optind+4)*sizeof(char *));
    glb_cmd[0] = "taskset";
    glb_cmd[1] = "-c";
    glb_cmd[2] = glb_target_cpus;
    int index=3;
    for(int i=optind;i<argc;i++) {
        glb_cmd[index] = argv[i];
        index++;
    }
    glb_cmd[index] = NULL;


    glb_size_cache_hog = 2*glb_no_of_ways*2*MB;
    glb_chunk_size = (glb_cache_size/glb_no_of_ways)/glb_nthreads;
    glb_no_of_chunk_lines = (glb_chunk_size/glb_block_size);
    glb_mem_cache_hog = (char *)mmap(NULL, glb_size_cache_hog, \
                                 PROT_READ | PROT_WRITE, \
                                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, \
                                 -1, 0);
    if (glb_mem_cache_hog==NULL) {
        errstr = "mmap failed\n";
        goto err;
    }
    read_events();
#ifdef DEBUG
    printf("no_of_chunk_lines %d\n", glb_no_of_chunk_lines);
    printf("chunk_size %d\n", glb_chunk_size);
    printf("nthreads %d\n", glb_nthreads);
    printf("no_of_hot_ways %d\n", glb_no_of_hot_ways);
#endif
    return;
err:
    fprintf(stderr, "%s", errstr);
    exit(-1);
}

void global_finalize(){
    free(glb_cmd);
    free(glb_events[0]);
    free(glb_events[1]);
    munmap(glb_mem_cache_hog, glb_size_cache_hog);
}


int main(int argc, char *argv[]){

    void *status;
    long index;
    long long ins;
    int forkretval, statusint;

    //PAPI counters
    long long value[5]={0, 0, 0, 0, 0};
    int array[5]={PAPI_L3_TCW, PAPI_TOT_INS, PAPI_TOT_CYC, PAPI_L3_DCW, PAPI_L3_DCA};
    PAPI_start_counters(array,5);

    //Call global init
    global_init(argc, argv);
    glb_pthread_exit_status=0;
    pthread_t *threads = malloc((glb_nthreads+1)*sizeof(pthread_t));
    int *thread_id = malloc((glb_nthreads+1)*sizeof(int));
    for(int i=0;i<glb_nthreads; i++) {
        thread_id[i]=i;
        pthread_create(&threads[i], NULL, keep_cache_hot, (void *)(thread_id+i));
    }
    pthread_create(&threads[glb_nthreads], NULL, change_bw, (void *)(thread_id+glb_nthreads));

    //Before forking start the first event
    PAPI_read_counters(value, 5);
    glb_no_of_hot_ways = glb_events[MEM][glb_running_event_no];
    glb_no_of_bw_req = glb_events[BW][glb_running_event_no];
    


    forkretval = fork();
    if(forkretval < 0) {
        fprintf(stderr, "Internal error: during fork, rc: %d", forkretval);
        exit(-1);
    }
    if(forkretval == 0) { //child
        execvp(glb_cmd[0], glb_cmd);
        fprintf(stderr, "Internal error execvp gave an error\n");
        exit(-1);
    }
    do {
        // usleep is in micro sec, input is in milisecond
        usleep(1000*glb_interval);
        PAPI_read_counters(value, 5);
#ifdef COUNTERS
        printf("CPI: %f   L3 data cache writes: %ld   L3 cache writes: %ld\n" \
                "   L3 data cache access: %ld   No of instructions: %ld\n",
                (float)value[2]/(float)value[1], value[3], value[0], value[4], value[1]);
#endif
        glb_running_event_no = (glb_running_event_no+1) % glb_total_events;
        glb_no_of_hot_ways = glb_events[MEM][glb_running_event_no];
        glb_no_of_bw_req = glb_events[BW][glb_running_event_no];
    } while(waitpid(forkretval, &statusint, WNOHANG)==0);

    glb_pthread_exit_status=1;
    for(int j=0; j<glb_nthreads+1; j++) {
        pthread_join(threads[j], &status);
    }

    // Call global finalize
out:
    global_finalize();
}
