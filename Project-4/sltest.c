#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "time.h"

#define BILLION 1000000000L
#define INSERT_YIELD 0x01
#define DELETE_YIELD 0x02
#define SEARCH_YIELD 0x04

static long long counter;
//set the number of threads and iterations as 1 for default case
static long long nthreads = 1;
static long long niterations = 1;

static pthread_mutex_t m_lock;
volatile static int s_lock = 0;

static char sync_form = 'a';
static int opt_yield = 0;
struct SortedListElement {
  struct SortedListElement *prev;
  struct SortedListElement *next;
  const char *key;
};

typedef struct SortedListElement SortedList_t;
typedef struct SortedListElement SortedListElement_t;

static SortedList_t* sort_list;
static SortedList_t** sort_list_l;  
static int list_num = 0;


//a helper 2D array that stores the element
char ** rand_variables;
int * int_map;

//there are 33 of them
const char charset[] = "0123456789";


void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
  SortedListElement_t *p = list;
  SortedListElement_t *n = list -> next;
  while(n!=list){
    if(strcmp(element->key, n->key) <= 0)
      break;
    p = n;
    n = n->next;
  }
  if(opt_yield & INSERT_YIELD)
    pthread_yield();
  element->prev = p;
  element->next = n;
  p->next = element;
  n->prev = element;
}

int SortedList_delete( SortedListElement_t *element)
{
  
  if(element == NULL)
    return -1;
  SortedListElement_t *n = element->next;
  SortedListElement_t *p = element->prev;
  if(n->prev!=element)
    return -1;
  if(p->next != element)
    return -1;
  if(opt_yield & DELETE_YIELD)
    pthread_yield();
  n->prev = p;
  p->next = n;
  element->next = NULL;
  element->prev = NULL;
  free(element);
  return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
  SortedListElement_t *n = list->next;
  if(opt_yield & SEARCH_YIELD)
      pthread_yield();
   while(n!=list)
   {
     if (strcmp(n->key, key)==0)
	return n;  
     n = n->next;
   }
  return NULL;
}

int SortedList_length(SortedList_t *list)
{
  int counter = 0;
  SortedListElement_t *n = list->next;
  if(opt_yield & SEARCH_YIELD)
    pthread_yield();
  while(n!=list)
    {
      counter ++;
      n = n->next;
    }
  return counter;
}


static struct option long_options[] = {
 {"threads", required_argument, NULL, 't'},
 {"iterations", required_argument, NULL, 'i'},
 {"iter", required_argument, NULL, 'I'},
 {"sync", required_argument, NULL, 's'},
 {"lists", required_argument, NULL, 'l'},
 {"yield", required_argument, NULL, 'y'},
 {0,0,0,0}
};


void* thread_func(void *arg)
{
  int* thread_id = (int*) arg;
  int length;
  if (sync_form == 'a')
    {
      int i;
      for (i = 0; i< niterations; i++)
      {
	SortedListElement_t* element = (SortedListElement_t*)malloc(sizeof(SortedListElement_t));
        element->key = rand_variables[(*thread_id)*niterations+i];
	if(list_num > 1)
	  {
	    int index = int_map[(*thread_id)*niterations+1] % list_num;
	    SortedList_insert(sort_list_l[index], element);
	  }
	else 
	  SortedList_insert(sort_list, element);
      }
 
      if(list_num > 1)
	{
	  for (i = 0; i < list_num; i++)
	    length += SortedList_length(sort_list_l[i]);
	}
      else
	length = SortedList_length(sort_list);

      for (i = 0; i< niterations; i++)
      {
	SortedListElement_t* deleted;
	if (list_num > 1)
	  {
	    int index = int_map[*(thread_id)*niterations+1] % list_num;
	    deleted = SortedList_lookup(sort_list_l[index], rand_variables[(*thread_id)*niterations+i]);
	  }
	else
	    deleted = SortedList_lookup(sort_list, rand_variables[(*thread_id)*niterations+i]);
	SortedList_delete(deleted);
      }
    }
  else if (sync_form == 'm')
    {
      int i;
      for (i=0; i<niterations; i++)
      {
	
	SortedListElement_t* element = (SortedListElement_t*)malloc(sizeof(SortedListElement_t));
        element->key = rand_variables[(*thread_id)*niterations+i];
	if(list_num > 1)
	  {
	    int index = int_map[(*thread_id)*niterations + i] % list_num;
	    pthread_mutex_lock(&m_lock);
	    SortedList_insert(sort_list_l[index], element);
	   }
	else
	  {
	     pthread_mutex_lock(&m_lock);
	    SortedList_insert(sort_list, element);
	  }
	  pthread_mutex_unlock(&m_lock);
      }
            
      if(list_num > 1)
	{
	  for(i=0; i< list_num; i++)
	    length += SortedList_length(sort_list_l[i]);
	}
      else
	length = SortedList_length(sort_list);

      for(i=0; i< niterations; i++)
      {
	SortedListElement_t* deleted;
	if(list_num > 1)
	  {
	    int index = int_map[(*thread_id)*niterations +i] % list_num;
	    pthread_mutex_lock(&m_lock);
	    deleted = SortedList_lookup(sort_list_l[index], rand_variables[(*thread_id)*niterations+i]);
	   }
	else
	  {
	    pthread_mutex_lock(&m_lock);
	    deleted = SortedList_lookup(sort_list, rand_variables[(*thread_id)*niterations +i]);
	  }
 
	SortedList_delete(deleted);
	pthread_mutex_unlock(&m_lock);
      }
    }
  else if (sync_form == 's')
    {
      int i;
      for(i=0; i<niterations; i++)
      {
	SortedListElement_t* element = (SortedListElement_t*)malloc(sizeof(SortedListElement_t));
        element->key = rand_variables[(*thread_id)*niterations+i];
	if(list_num > 1)
	  {
	    int index = int_map[(*thread_id)*niterations + i] % list_num;
	    while(__sync_lock_test_and_set(&s_lock, 1));
	    SortedList_insert(sort_list_l[index], element);
	   }
	else 
	  {
	    while(__sync_lock_test_and_set(&s_lock, 1));
	    SortedList_insert(sort_list, element);
	  }
	   __sync_lock_release(&s_lock);
      }
      
      if(list_num > 1)
	{
	  for (i=0; i< list_num; i++)
	    length+=SortedList_length(sort_list_l[i]);
	}
      else 
	length = SortedList_length(sort_list);

      
      for(i=0; i<niterations; i++)
      {
	SortedListElement_t* deleted;
	if(list_num > 1)
	  {
	    int index = int_map[(*thread_id)*niterations + i] % list_num;
	    while(__sync_lock_test_and_set(&s_lock,1));
	    deleted = SortedList_lookup(sort_list_l[index], rand_variables[(*thread_id)*niterations + i]);
	   }
	else
	  {
	    while(__sync_lock_test_and_set(&s_lock,1));
	    deleted = SortedList_lookup(sort_list, rand_variables[(*thread_id)*niterations+i]);
	  }
        SortedList_delete(deleted); 
	__sync_lock_release(&s_lock);
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
      sync_form = *optarg;
      break;
    }
  case 'y':
    {
      int a = 0;
      while(optarg[a]!='\0')
      {
	if (optarg[a] == 'i')
	{
	  opt_yield |= INSERT_YIELD;
	}
	else if (optarg[a] == 'd')
	{
	  opt_yield |= DELETE_YIELD;
	}
	else if (optarg[a] == 's')
	{
	  opt_yield |= SEARCH_YIELD;
	}
	else
	{
	  fprintf(stderr, "Pass in invalid option argument for yield\n");
	  exit(1);
	}
	a++;
      }
      break;
    }
  case 'l':
    {
      list_num = atoi(optarg);
      break;
    }
  }
  }

  
  if(list_num > 1)
  {
    sort_list_l = (SortedListElement_t**)malloc(sizeof(SortedListElement_t*)*list_num);
    if(sort_list_l == NULL)
    {
      fprintf(stderr, "Fail to use malloc\n");
      exit(1);
    }
    int k;
    for( k = 0; k < list_num; k++)
    {
      sort_list_l[k] = (SortedListElement_t*)malloc(sizeof(SortedListElement_t));
      if(sort_list_l[k] == NULL)
      {
	fprintf(stderr, "Fail to use malloc\n");
	exit(1);
      }
      sort_list_l[k] -> key = NULL;
      sort_list_l[k] -> prev = sort_list_l[k];
      sort_list_l[k] -> next = sort_list_l[k];
    }
    
    int_map = (int*)malloc(sizeof(int)*nthreads*niterations);
    if(int_map == NULL)
    {
      fprintf(stderr, "Fail to use malloc\n");
      exit(1);
    }
  }
  else
  {
    sort_list = (SortedList_t*)malloc(sizeof(SortedList_t));
    sort_list-> key = NULL;
    sort_list->prev = sort_list;
    sort_list->next = sort_list;
  }
  
  
  rand_variables = (char**) malloc (sizeof(int*)*nthreads*niterations);
  if (rand_variables == NULL)
  {
    fprintf(stderr, "Fail to use malloc\n");
    exit(1);
  }

  //initialize the 2D array with random numbers 
  int i;
  for (i = 0; i < nthreads * niterations; i++)
  {
    rand_variables[i] = (char*)malloc(sizeof(char)* 8);
    if (rand_variables[i] == NULL)
    {
      fprintf(stderr, "Fail to use malloc\n");
      exit(1);
    }

    int j;
    //set it to 8
    for( j=0;j<8; j++)
    {
      int key = rand() % 10; //get a random number smaller than 33
      rand_variables[i][j] = charset[key];
    }
    if(list_num > 1)
      int_map[i] = atoi(rand_variables[i]);
  }

  /////////////////For the int map, we just use 
  
  //////////////////////////// Begin the starting time /////////////////////////////////

  
  Threads = (pthread_t*)malloc(sizeof(pthread_t)*nthreads);
  if(Threads == NULL)
  {
    fprintf(stderr, "Fail to allocate memory using malloc\n");
    exit(1);
  }

  
  int *thread_no = (int*)malloc(sizeof(int)*nthreads);
  if(thread_no == NULL)
  {
    fprintf(stderr, "Fail to allocate memory\n");
    exit(1);
  }


  for(int i =0; i < nthreads; i++)
    thread_no[i]=i;
  
  
  if(clock_gettime(CLOCK_MONOTONIC, &start) == -1)
  {
    fprintf(stderr, "Fail to use clock_gettime\n");
    exit(1);
  }  


  
  int ret;
  for (i = 0; i < nthreads; i++)
  {
    ret = pthread_create(&Threads[i], 0, thread_func, &thread_no[i]);
    if(ret)
    {
      fprintf(stderr, "Fail to create a thread\n");
      exit(1);
    }
  }
  
  for(i = 0; i < nthreads; i++)
  {
    ret = pthread_join(Threads[i], 0);
    if(ret)
    {
      fprintf(stderr, "Fail to join a thread\n");
      exit(1);
    }
  }

  //after pthread join. Need to figure out how to get the length
  
  
  if(clock_gettime(CLOCK_MONOTONIC, &stop) == -1)
  {
    fprintf(stderr, "Fail to use clock_gettime\n");
    exit(1);
  }

  long long operations = nthreads*niterations*2;
  fprintf(stdout, "%d threads x %d iterations x (ins + lookul/del) = %d operations\n", nthreads, niterations, operations);

  int counter = 0;
  if(list_num > 1)
    {
      int j;
      for (j = 0; j < list_num; j++)
	counter += SortedList_length(sort_list_l[j]);
      if (counter != 0)
	{
	  printf("ERROR: final count = %d\n", counter);
	  ERROR = 1;
	}
    }
  else
    {
      counter = SortedList_length(sort_list);
      if(counter != 0)
	{
	  printf("ERROR: final count = %d\n", counter);
	  ERROR = 1;
	}
    }
  
  long long accum = (stop.tv_sec - start.tv_sec)*BILLION + (stop.tv_nsec-start.tv_nsec);
  printf("elapsed time: %d ns\n", accum);
  printf("per operation: %d ns\n", accum/operations);
  //if there are errros exit with a non 0 value
  free(Threads);
  free(thread_no);
  free(sort_list);
  if(list_num > 1)
    {
      int i;
      for (i = 0; i < list_num; i++)
	free(sort_list_l[i]);
      free(sort_list_l);
      free(int_map);
    }
  
  for(int i = 0; i<nthreads*nthreads; i++)
    {
      free(rand_variables[i]);
    }
  free(rand_variables);
  if(ERROR)
    exit(ERROR);
  exit(0);
}
