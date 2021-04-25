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

#define WRITE 1 // from 2, which will cause buffer overflow
#define READ 0

void task_3() {
  int pipe1[2], pipe2[2]; // pipe2 should have two elements
  pipe(pipe1);
  pipe(pipe2);
  if (fork() == 0) {
    dup2(pipe1[WRITE], STDOUT_FILENO);
    dup2(pipe2[READ],
         STDIN_FILENO); // switched these two, so we don't overwrite pipe2[READ]
    close(pipe1[READ]);
    close(pipe1[WRITE]);
    exec(A);
  }
  if (fork() != 0) {
    dup2(pipe1[READ], STDIN_FILENO);
    dup2(pipe2[WRITE], STDOUT_FILENO);
    close(pipe1[WRITE]);
    close(pipe2[READ]);
    exec(B);
  }
  close(pipe1[READ]);
  close(pipe1[WRITE]);
  close(pipe2[READ]);
  close(pipe2[WRITE]);
}

int main() {
  task_2();
  // sleep(10);

  return 0;
}