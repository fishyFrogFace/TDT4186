#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

volatile sig_atomic_t print_flag = false;
volatile sig_atomic_t cleanup_flag = false;
volatile sig_atomic_t print_total = false;

/* To ensure that the signal handlers are reentrant,
 * we avoid making any system calls and leave the
 * user commands and system calls to the main loop. */
void sigalarm_handler(int signum) { print_flag = true; }

void sigint_handler(int signum) { cleanup_flag = true; }

void sigusr1_handler(int signum) { print_total = true; }

void spawn(size_t block_size) {
  int pipe_fd[2] = {0, 0};

  if (pipe(pipe_fd) == 0) {

    // initialize the block of data we want to send
    char *block = malloc(block_size);

    /* initialize the buffer for the data we want to receive
     * and the read counters */
    char *buffer = malloc(block_size);

    uintmax_t bytes_read_per_second = 0;
    uintmax_t total_bytes_read = 0;

    if (block == NULL || buffer == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }

    pid_t child_pid = fork();

    // close the idle ends of the pipe
    if (child_pid == 0) {
      if (close(pipe_fd[0]) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
      }

    } else if (0 < child_pid) {
      if (close(pipe_fd[1]) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
      }

    } else {
      perror("fork");
      exit(EXIT_FAILURE);
    }

    // start reading and writing data
    while (true) {
      if (child_pid == 0) {

        ssize_t bytes_written = write(pipe_fd[1], block, block_size);

        if (bytes_written == -1) {
          perror("write");
          exit(EXIT_FAILURE);
        }

        // printf("The child process wrote %lu bytes\n", bytes_written);

        /* There might not be a point to this, since everything is cleaned when
         * the program is stopped, but it was a fun thing to try */
        if (cleanup_flag == true) {
          puts("Cleaning up and exiting child");
          if (close(pipe_fd[1]) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
          }
          exit(EXIT_SUCCESS);
        }
      }

      else if (child_pid > 0) {

        ssize_t bytes_read = read(pipe_fd[0], buffer, block_size);

        if (bytes_read == -1) {
          perror("read");
          exit(EXIT_FAILURE);
        }

        bytes_read_per_second += bytes_read;
        total_bytes_read += bytes_read;

        /* This check is for determining max block size for this system. Should
         * be commented out when testing bandwidth. */
        /* if (bytes_read < block_size) {
          printf("Bytes read was %zu, block size was %zu\n", bytes_read,
                 block_size);
        } */

        /* printf("The parent process read %zu bytes from child process (%d)\n",
               bytes_read, (int)getpid()); */

        /* There might not be a point to this, since everything is cleaned when
         * the program is stopped, but it was a fun thing to try */
        if (cleanup_flag == true) {
          puts("Cleaning up and exiting parent");
          if (close(pipe_fd[0]) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
          }
          if (wait(&child_pid) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
          }
          exit(EXIT_SUCCESS);
        }

        if (print_flag == true) {
          printf("Bytes read last second: %lu\n", bytes_read_per_second);
          bytes_read_per_second = 0;
          print_flag = false;
          alarm(1);
        }

        if (print_total == true) {
          printf("Bytes read in total: %lu\n", total_bytes_read);
          print_total = false;
        }
      }
    }
  } else {
    perror("pipe");
    exit(EXIT_FAILURE);
  }
}

size_t parse_size_t(char *arg) {
  intmax_t block_size = strtoimax(arg, 0, 10);

  if (block_size > INTMAX_MAX || block_size == INTMAX_MIN) {
    perror("strtoimax");
    exit(EXIT_FAILURE);
  } else if (block_size > SIZE_MAX) {
    puts("Error: block_size cannot be larger than SIZE_MAX");
    exit(EXIT_FAILURE);
  } else if (block_size == 0) {
    puts("Error: block_size cannot be 0 or contain non-numerical characters");
    exit(EXIT_FAILURE);
  } else if (block_size < 0) {
    puts("Error: block_size cannot be a negative number");
    exit(EXIT_FAILURE);
  }

  return block_size;
}

int main(int argc, char *argv[]) {
  printf("My PID is: %d\n", (int)getpid());

  if (2 < argc) {
    puts("Error: too many arguments");
    exit(EXIT_FAILURE);
  } else if (argc < 2) {
    puts("Error: too few arguments");
    exit(EXIT_FAILURE);
  } else {
    puts("The perfect amount of arguments");
    size_t block_size = parse_size_t(argv[1]);

    // register signal handlers
    __sighandler_t set_alarm_handler = signal(SIGALRM, &sigalarm_handler);
    __sighandler_t set_interrupt_handler = signal(SIGINT, &sigint_handler);
    __sighandler_t set_usr1_handler = signal(SIGUSR1, &sigusr1_handler);

    if (set_alarm_handler == SIG_ERR || set_interrupt_handler == SIG_ERR ||
        set_usr1_handler == SIG_ERR) {
      perror("signal");
      exit(EXIT_FAILURE);
    }

    // set an alarm
    alarm(1);

    spawn(block_size);
  }

  return 0;
}