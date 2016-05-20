#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>
#include "time.h"

#define BILLION 1000000000L

static long long counter;
//set the number of threads and iterations as 1 for default case
static long long nthreads = 1;
static long long niterations = 1;
static pthread_mutex_t m_lock;
volatile static int s_lock = 0;
static int opt_yield;
static char sync_form = 'a'; //let a be default value, means implementing regular add

static struct option long_options[] = {
 {"threads", required_argument, NULL, 't'},
 {"iterations", required_argument, NULL, 'i'},
 {"iter", required_argument, NULL, 'I'},
 {"sync", required_argument, NULL, 's'},
 {"yield",no_argument, &opt_yield, 1},
 {0,0,0,0}
};



//implementing the add function
void add(long long *pointer, long long value){
  long long sum = *pointer + value;
  if(opt_yield)
    pthread_yield();
  *pointer = sum;
}


//protected by mutex
void add_m(long long *pointer, long long value){
  pthread_mutex_lock(&m_lock);
  long long sum = *pointer + value;
  if(opt_yield)
    pthread_yield();
  *pointer = sum;
  pthread_mutex_unlock(&m_lock);
}

//protected by spin-lock
void add_s(long long *pointer, long long value){
  while(__sync_lock_test_and_set(&s_lock, 1));
  long long sum = *pointer + value;
  if(opt_yield)
    pthread_yield();
  *pointer = sum;
  __sync_lock_release(&s_lock);
}

//protected by compare and swap
void add_c(long long *pointer, long long value){
  long long orig, sum;
  do{
    orig = *pointer;
    sum = orig + value;
    if(opt_yield)
      pthread_yield();
  }while(__sync_val_compare_and_swap(pointer, orig, sum)!=orig);
}


void* thread_func(void *arg)
{
  // int* thread_id = (int*) arg;
  int i;
  //determine the number of iterations for each thread
  //set it as niterations right now, may change later

  if (sync_form == 'a')
    {
      for (i = 0; i < niterations;  i++)
      {
	add(&counter, 1);
      }
      for(i = 0; i < niterations; i++)
      {
	add(&counter, -1);
      }
    }
  else if (sync_form == 'm')
    {
      for (i = 0; i < niterations;  i++)
      {
	add_m(&counter, 1);
    
      }
      for(i = 0; i < niterations; i++)
      {
	add_m(&counter, -1);
      }
    }
  else if (sync_form == 's')
    {
      for (i = 0; i < niterations;  i++)
      {
	add_s(&counter, 1);
      }
      for(i = 0; i < niterations; i++)
      {
	add_s(&counter, -1);
      }
    }
  else if (sync_form == 'c')
    {
      for (i = 0; i < niterations;  i++)
      {
	add_c(&counter, 1);
      }
      for(i = 0; i < niterations; i++)
      {
	add_c(&counter, -1);
      }
   }
  pthread_exit(0);
}

int 
main(int argc, char * argv[])
{
  int option = 0;
  struct timespec start, stop;
  int ERROR = 0;
  //this is the thread array
  pthread_t *Threads;
  //initialize the counter to 0
  counter = 0;
  //count the starting time
 
  while(option != -1){
    //getting the options
  option = getopt_long(argc, argv, "", long_options, NULL);
  switch (option){
  case 't':{
    //get the number of threads
    nthreads = atol(optarg);
    break;
  }
  case 'i':
  case 'I':
    {
      //get the number of iterations
      niterations = atol(optarg);
      break;
    }
  case 's':
    {
      sync_form =*optarg;
      break;
    }
  }
  }
  if(clock_gettime(CLOCK_MONOTONIC, &start) == -1)
  {
    fprintf(stderr, "Fail to use clock_gettime\n");
    exit(1);
  }
 
  Threads = (pthread_t*)malloc(sizeof(pthread_t)*nthreads);
  if(Threads == NULL)
  {
    fprintf(stderr, "Fail to allocate memory using malloc\n");
    exit(1);
  }
  int a = 0;
  int t;
  int ret;
  for (t = 0; t < nthreads; t++)
  {
    ret = pthread_create(&Threads[t], 0, thread_func, &a);
    if(ret)
    {
      fprintf(stderr, "Fail to create a thread\n");
      exit(1);
    }
  }
  
  for(t = 0; t < nthreads; t++)
  {
    ret = pthread_join(Threads[t], 0);
    if(ret)
    {
      fprintf(stderr, "Fail to join a thread\n");
      exit(1);
    }
  }

  if(clock_gettime(CLOCK_MONOTONIC, &stop) == -1)
  {
    fprintf(stderr, "Fail to use clock_gettime\n");
    exit(1);
  }
  
  long long operations = nthreads*niterations*2;
  fprintf(stdout, "%d threads x %d iterations x (add + subtract) = %d operations\n", nthreads, niterations,operations);
  
  if(counter != 0)
  {
    printf("ERROR: final count = %d\n", counter);
    ERROR = 1;
  }
 
  long long accum = (stop.tv_sec - start.tv_sec)*BILLION + (stop.tv_nsec-start.tv_nsec);
  printf("elapsed time: %d ns\n", accum);
  printf("per operation: %d ns\n", accum/operations);
  //if there are errros exit with a non 0 value
  free(Threads);
  if(ERROR)
    exit(ERROR);
  exit(0);
}
