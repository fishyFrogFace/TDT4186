#include <pthread.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int task_1() {

  for (int i = 0; i < 3; i++) {
    int pid = fork();

    if (pid > 0) {
      printf("%d created %d\n", getpid(), pid);
    }
  }

  printf("Hello World %d\n", getpid());
  return 0;
}

// W(A) means write(1, "A", sizeof "A")
#define W(x) write(1, #x, sizeof #x);

void task_2() {
  printf("A pid: %d\n", getpid());
  int child = fork();
  printf("B pid: %d\n", getpid());

  if (child) { // if we got an id of a child back (we are in parent)
    wait(NULL);
  }

  printf("C. pid: %d\n", getpid());
}

int main() {
  task_2();
  // sleep(10);

  return 0;
}