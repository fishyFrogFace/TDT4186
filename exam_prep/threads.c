#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pthread_t tid[2];
int global1, global2;

// example function for use in thread
void *doSomeThing(void *arg) {
  pthread_t id = pthread_self();

  if (pthread_equal(id, tid[0])) {
    puts("First thread processing");
  } else {
    puts("Second thread processing");
  }

  for (unsigned long i = 0; i < (0xFFFFFFFF); i++) {
    // time consuming loop that does nothing
  };

  if (pthread_equal(id, tid[0])) {
    puts("First thread finished");
    /* this won't work with local variables, the address on the stack will not
     * be available once the thread exits */
    static int retval1 = 7;
    retval1 = 7;
    global1 = 7;
    pthread_exit(&retval1);
  } else {
    puts("Second thread finished");
    static int retval2 = 8;
    global2 = 8;
    pthread_exit(&retval2);
  }
}

// example thread creation and no wait for threads
int sleep_example(void) {
  for (int i = 0; i < 2; i++) {
    int error = pthread_create(&(tid[i]), NULL, &doSomeThing, NULL);
    if (error != 0)
      printf("Can't create thread :[%s]\n", strerror(error));
    else
      printf("Thread created successfully: %lu\n", tid[i]);
  }

  puts("Main goes to sleep");

  sleep(5);

  puts("Main done sleeping");
  return 0;
}

// example thread creation and wait for threads
int wait_example(void) {
  int *retval[2] = {0, 0};

  for (int i = 0; i < 2; i++) {
    int error = pthread_create(&(tid[i]), NULL, &doSomeThing, NULL);
    if (error != 0)
      printf("Can't create thread :[%s]\n", strerror(error));
    else
      printf("Thread created successfully: %lu\n", tid[i]);
  }

  for (int i = 0; i < 2; i++) {
    pthread_join(tid[i], (void **)&(retval[i]));
  }

  puts("All threads have finished");

  for (int i = 0; i < 2; i++) {
    printf("Thread %lu returned %d\n", tid[i], *retval[i]);
  }

  printf("Global 1: %d\n", global1);
  printf("Global 2: %d\n", global2);

  return 0;
}

int main(void) {
  wait_example();
  return 0;
}