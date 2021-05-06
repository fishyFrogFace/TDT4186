#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pthread_t tid[2];

struct thd_info {
  char *proc_info;
  int proc_number;
};

struct thd_info thd = {"hello, this is string", 8};

/* The two threads execute concurrently, with no guarantees about which thread
 * reaches the usage of thd->proc_info first. So the second thread might set
 * thd->proc_info to null before the first thread has the chance to print it.*/
void *thread_function() {
  pthread_t id = pthread_self();

  if (pthread_equal(id, tid[0])) {
    puts("First thread processing");
  } else {
    puts("Second thread processing");
  }

  if (pthread_equal(id, tid[0])) {
    for (unsigned long i = 0; i < (0xFFFFFFFF); i++) {
      /* Time consuming loop that ensures that the first
       * thread does not check thd.proc_info before after it
       * is mutated by second thread. */
    };
    if (thd.proc_info) {
      puts(thd.proc_info);
    } else {
      puts("Proc info was null");
    }
  } else {
    thd.proc_info = NULL;
    puts("mutated proc_info");
  }

  if (pthread_equal(id, tid[0])) {
    puts("First thread finished");
    /* this won't work with local variables, the address on the stack will not
     * be available once the thread exits */
    static int retval1 = 7;
    pthread_exit(&retval1);
  } else {
    puts("Second thread finished");
    static int retval2 = 8;
    pthread_exit(&retval2);
  }
}

int task_4_problem(void) {
  int *retval[2] = {0, 0};

  for (int i = 0; i < 2; i++) {
    int error = pthread_create(&(tid[i]), NULL, &thread_function, NULL);
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

  return 0;
}

void *thread_function_solution() {
  pthread_t id = pthread_self();

  if (pthread_equal(id, tid[0])) {
    puts("First thread processing");
  } else {
    puts("Second thread processing");
  }

  if (pthread_equal(id, tid[0])) {
    for (unsigned long i = 0; i < (0xFFFFFFFF); i++) {
      /* Time consuming loop that ensures that the first
       * thread does not check thd.proc_info before after it
       * is mutated by second thread. */
    };
    if (thd.proc_info) {
      puts(thd.proc_info);
    } else {
      puts("Proc info was null");
    }
  } else {
    thd.proc_info = NULL;
    puts("mutated proc_info");
  }

  if (pthread_equal(id, tid[0])) {
    puts("First thread finished");
    pthread_exit((void *)7);
  } else {
    puts("Second thread finished");
    pthread_exit((void *)8);
  }
}

int task_4_solution(void) {
  long *retval[2] = {0, 0};

  for (int i = 0; i < 2; i++) {
    int error =
        pthread_create(&(tid[i]), NULL, &thread_function_solution, NULL);
    if (error != 0)
      printf("Can't create thread :[%s]\n", strerror(error));
    else
      printf("Thread created successfully: %lu\n", tid[i]);
  }

  for (int i = 0; i < 2; i++) {
    pthread_join(tid[i], (void **)&retval[i]);
  }

  puts("All threads have finished");

  for (int i = 0; i < 2; i++) {
    printf("Thread %lu returned %ld\n", tid[i], (long)retval[i]);
  }

  return 0;
}

int main() {
  // task_4_problem();
  task_4_solution();
  return 0;
}