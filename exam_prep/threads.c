#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pthread_t tid[2];

// example function for use in thread
void *doSomeThing(void *arg) {
  unsigned long i = 0;
  pthread_t id = pthread_self();

  if (pthread_equal(id, tid[0])) {
    puts("First thread processing");
  } else {
    puts("Second thread processing");
  }

  for (i = 0; i < (0xFFFFFFFF); i++) {
    // time consuming loop that does nothing
  };

  if (pthread_equal(id, tid[0])) {
    puts("First thread finished");
  } else {
    puts("Second thread finished");
  }

  return NULL;
}

// example thread creation and wait for threads
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

int main(void) { sleep_example(); }