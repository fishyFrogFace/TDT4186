#include <unistd.h>

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
    close(pipe2[WRITE]); // change to pipe2 here, or else we close the whole
                         // pipe for the parent process
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