#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pthread_t tid[3];
int one = 0;
int two = 0;
int three = 0;

void *thread_function_solution() {
  pthread_t id = pthread_self();

  if (pthread_equal(id, tid[0])) {
    while (one == 1)
      ;
    one = 1;
    puts("First thread: LOCK 1");
    for (unsigned long i = 0; i < (0xFFFFFFFF); i++) {
      /* Time consuming loop */
    };
    while (two == 1)
      ;
    puts("First thread: LOCK 2");
    for (unsigned long i = 0; i < (0xFFFFFFFF); i++) {
      /* Time consuming loop */
    };
    one = 0;
    puts("First thread: OPEN 1");
    two = 0;
    puts("First thread: OPEN 2");
  } else if (pthread_equal(id, tid[1])) {
    while (one == 1)
      ;
    one = 1;
    puts("Second thread: LOCK 1");
    for (unsigned long i = 0; i < (0xFFFFFFFF); i++) {
      /* Time consuming loop */
    };
    while (three == 1)
      ;
    three = 1;
    puts("Second thread: LOCK 3");
    for (unsigned long i = 0; i < (0xFFFFFFFF); i++) {
      /* Time consuming loop */
    };
    three = 0;
    puts("Second thread: OPEN 3");
    one = 0;
    puts("Second thread: OPEN 1");
  } else {
    while (three == 1)
      ;
    three = 1;
    puts("Third thread: LOCK 3");
    while (two == 1)
      ;
    two = 1;
    puts("Third thread: LOCK 2");
    for (unsigned long i = 0; i < (0xFFFFFFFF); i++) {
      /* Time consuming loop */
    };
    two = 0;
    puts("Third thread: OPEN 2");
    three = 0;
    puts("Third thread: OPEN 3");
  }

  if (pthread_equal(id, tid[0])) {
    puts("First thread finished");
    pthread_exit((void *)7);
  } else if (pthread_equal(id, tid[1])) {
    puts("Second thread finished");
    pthread_exit((void *)8);
  } else {
    puts("Third thread finished");
    pthread_exit((void *)9);
  }
}

int task_5_solution(void) {
  long *retval[3] = {0, 0, 0};

  for (int i = 0; i < 3; i++) {
    int error =
        pthread_create(&(tid[i]), NULL, &thread_function_solution, NULL);
    if (error != 0)
      printf("Can't create thread :[%s]\n", strerror(error));
    else
      printf("Thread created successfully: %lu\n", tid[i]);
  }

  for (int i = 0; i < 3; i++) {
    pthread_join(tid[i], (void **)&retval[i]);
  }

  puts("All threads have finished");

  for (int i = 0; i < 3; i++) {
    printf("Thread %lu returned %ld\n", tid[i], (long)retval[i]);
  }

  return 0;
}

void *thread_function() {
  pthread_t id = pthread_self();

  if (pthread_equal(id, tid[0])) {
    one = 1;
    puts("First thread: LOCK 1");
    for (unsigned long i = 0; i < (0xFFFFFFFF); i++) {
      /* Time consuming loop */
    };
    while (two == 1)
      ;
    puts("First thread: LOCK 2");
    puts("First thread: OPEN 2");
    puts("First thread: OPEN 1");
  } else if (pthread_equal(id, tid[1])) {
    three = 1;
    puts("Second thread: LOCK 3");
    for (unsigned long i = 0; i < (0xFFFFFFFF); i++) {
      /* Time consuming loop */
    };
    while (one == 1)
      ;
    puts("Second thread: LOCK 1");
    puts("Second thread: OPEN 1");
    puts("Second thread: OPEN 3");
  } else {
    two = 1;
    puts("Third thread: LOCK 2");
    while (three == 1)
      ;
    puts("Third thread: LOCK 3");
    puts("Third thread: OPEN 3");
    puts("Third thread: OPEN 2");
  }

  if (pthread_equal(id, tid[0])) {
    puts("First thread finished");
    pthread_exit((void *)7);
  } else if (pthread_equal(id, tid[1])) {
    puts("Second thread finished");
    pthread_exit((void *)8);
  } else {
    puts("Third thread finished");
    pthread_exit((void *)9);
  }
}

int task_5(void) {
  long *retval[3] = {0, 0, 0};

  for (int i = 0; i < 3; i++) {
    int error = pthread_create(&(tid[i]), NULL, &thread_function, NULL);
    if (error != 0)
      printf("Can't create thread :[%s]\n", strerror(error));
    else
      printf("Thread created successfully: %lu\n", tid[i]);
  }

  for (int i = 0; i < 3; i++) {
    pthread_join(tid[i], (void **)&retval[i]);
  }

  puts("All threads have finished");

  for (int i = 0; i < 3; i++) {
    printf("Thread %lu returned %ld\n", tid[i], (long)retval[i]);
  }

  return 0;
}

int main() {
  task_5_solution();
  return 0;
}