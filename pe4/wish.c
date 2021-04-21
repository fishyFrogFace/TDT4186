#include <asm-generic/errno-base.h>
#include <errno.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 128
#define TOKEN_TYPE_COMMAND 0
#define TOKEN_TYPE_ARGUMENT 1
#define TOKEN_TYPE_INPUT_REDIRECTION 2
#define TOKEN_TYPE_OUTPUT_REDIRECTION 3
#define TOKEN_TYPE_REDIRECTION_FILENAME 4

// characters to split string on
const char *delimiters = " \t\n";

struct input_info {
  char *input;
  int save_out;
  int save_in;
};

void print_prompt(void) {
  char cwd[PATH_MAX];
  getcwd(cwd, sizeof(cwd));
  printf("\n[%s]$ ", cwd);
}

void print_parsed(char *token, int token_type) {
  switch (token_type) {
  case TOKEN_TYPE_COMMAND:
    printf("\nCommand: %s", token);
    break;
  case TOKEN_TYPE_ARGUMENT:
    printf("\tArgument: %s ", token);
    break;
  case TOKEN_TYPE_INPUT_REDIRECTION:
    printf("\tInput redirection: %s", token);
    break;
  case TOKEN_TYPE_OUTPUT_REDIRECTION:
    printf("\tOutput redirection: %s", token);
    break;
  case TOKEN_TYPE_REDIRECTION_FILENAME:
    printf("\tFilename: %s", token);
    break;
  default:
    printf("Unknown type for  token %s", token);
    break;
  }
}

int get_token_type(char *token, int index) {
  static int previous_token_type = TOKEN_TYPE_COMMAND;
  if (index == 0) {
    previous_token_type = TOKEN_TYPE_COMMAND;
    return TOKEN_TYPE_COMMAND;
  }

  if (strcmp(token, "<") == 0) {
    previous_token_type = TOKEN_TYPE_INPUT_REDIRECTION;
    return TOKEN_TYPE_INPUT_REDIRECTION;
  }

  if (strcmp(token, ">") == 0) {
    previous_token_type = TOKEN_TYPE_OUTPUT_REDIRECTION;
    return TOKEN_TYPE_OUTPUT_REDIRECTION;
  }

  if (previous_token_type == TOKEN_TYPE_INPUT_REDIRECTION) {
    previous_token_type = TOKEN_TYPE_REDIRECTION_FILENAME;
    return TOKEN_TYPE_REDIRECTION_FILENAME;
  }

  if (previous_token_type == TOKEN_TYPE_OUTPUT_REDIRECTION) {
    previous_token_type = TOKEN_TYPE_REDIRECTION_FILENAME;
    return TOKEN_TYPE_REDIRECTION_FILENAME;
  }

  previous_token_type = TOKEN_TYPE_ARGUMENT;
  return TOKEN_TYPE_ARGUMENT;
}

void handle_error(int error) {
  printf("\nError: ");
  switch (error) {
  case EACCES:
    printf("Permission denied (EACCES)\n");
    break;
  case EIO:
    printf("I/O error (EIO)\n");
    break;
  case ENOENT:
    printf("No such file or directory (ENOENT)\n");
    break;
  case ENOEXEC:
    printf("Exec format error (ENOEXEC)\n");
    break;
  case EPERM:
    printf("Operation not permitted (EPERM)\n");
    break;
  case EROFS:
    printf("Read-only filesystem (EROFS)\n");
    break;
  default:
    printf("Unknown error with code %d\n", error);
    perror("execvp");
    break;
  }
}

int execute_command(char **tokens) {
  /* flushing stdout before forking
   * so that we don't get duplicated output */
  fflush(stdout);

  pid_t pid = fork();
  if (pid == 0) {
    int result = execvp(tokens[0], tokens);
    int error = errno;

    if (result == -1) {
      handle_error(error);
      exit(EXIT_FAILURE);

    } else {
      exit(EXIT_SUCCESS);
    }
  }
  // if the fork fails, exit the shell
  else if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);

  } else {
    wait(&pid);
    return EXIT_SUCCESS;
  }
}

// tokenize, redirect and call execute command
void handle_input(struct input_info input_info) {

  int index = 1;
  char *output_redirection = NULL;
  char *input_redirection = NULL;

  /* we only support 20 tokens
   * this could have been expanded
   * by creating a dynamically allocated list */
  char *tokens[20] = {NULL};
  char *token = strtok(input_info.input, delimiters);

  tokens[0] = token;
  print_parsed(token, 0);

  while ((token = strtok(NULL, delimiters)) != NULL) {
    int token_type = get_token_type(token, index);

    if (token_type == TOKEN_TYPE_OUTPUT_REDIRECTION) {
      output_redirection = strtok(NULL, delimiters);
      print_parsed(token, token_type);
      token_type = get_token_type(output_redirection, index);
      print_parsed(output_redirection, token_type);
      tokens[index] = NULL;

    } else if (token_type == TOKEN_TYPE_INPUT_REDIRECTION) {
      input_redirection = strtok(NULL, delimiters);
      print_parsed(token, token_type);
      token_type = get_token_type(input_redirection, index);
      print_parsed(input_redirection, token_type);
      tokens[index] = NULL;

    } else {
      tokens[index] = token;
      index += 1;
      print_parsed(token, token_type);
    }
  }

  puts("");
  FILE *out = NULL;
  FILE *in = NULL;

  if (output_redirection != NULL) {

    // if the command fails, the file is still overwritten
    out = fopen(output_redirection, "w");
    if (out == NULL) {
      perror("fopen stdout");
    } else {
      fflush(stdout);

      if (dup2(fileno(out), STDOUT_FILENO) == -1) {
        perror("cannot redirect stdout");
      }
    }
  }

  if (input_redirection != NULL) {

    in = fopen(input_redirection, "r");

    if (in == NULL) {
      perror("fopen stdin");
    } else {
      fflush(stdout);

      if (dup2(fileno(in), STDIN_FILENO) == -1) {
        perror("cannot redirect stdin");
      }
    }
  }

  execute_command(tokens);

  if (output_redirection != NULL && out != NULL) {
    /* flush stdout, close the file
     * and direct stdout back */
    fflush(stdout);
    fclose(out);
    dup2(input_info.save_out, STDOUT_FILENO);

    if (errno != 0) {
      perror("cleanup after output redirection");
      exit(EXIT_FAILURE);
    }

    printf("\nOutput of command was redirected to: %s\n", output_redirection);
  }

  if (input_redirection != NULL && in != NULL) {
    /* flush stdin, close the file
     * and direct stdin back */
    fflush(stdin);
    fclose(in);
    dup2(input_info.save_in, STDIN_FILENO);

    if (errno != 0) {
      perror("cleanup after input redirection");
      exit(EXIT_FAILURE);
    }

    printf("\nInput of command was redirected to: %s\n", input_redirection);
  }
}

/* main program loop
 * manages where we get the input from (user or script file)
 * handles cd and exit
 * and redirects commands to handle_input when applicable */
int main(int argc, char *argv[]) {
  const int save_out = dup(STDOUT_FILENO);
  const int save_in = dup(STDIN_FILENO);
  FILE *script_file = NULL;

  struct input_info input_info = {NULL, save_out, save_in};

  if (argc > 0) {
    if (argc > 2) {
      puts("\nargv: too many arguments");
    } else {
      char *filename = argv[1];
      script_file = fopen(filename, "r");

      if (script_file == NULL) {
        perror("fopen script from argv");
      }
    }
  }

  while (1) {
    char *str = NULL;
    char original[MAX_BUFFER_SIZE];

    print_prompt();
    char input[MAX_BUFFER_SIZE];
    if (script_file == NULL) {
      str = fgets(input, MAX_BUFFER_SIZE, stdin);

    } else {
      str = fgets(input, MAX_BUFFER_SIZE, script_file);
      // print line from file, so it's visible in the terminal
      if (str != NULL) {
        printf("%s", str);
      }
    }

    strcpy(original, str);
    char *token = strtok(str, delimiters);

    input_info.input = original;

    if (token != NULL && str != NULL) {
      if (strcmp(token, "cd") == 0) {

        char *directory = strtok(NULL, delimiters);

        // our cd only takes one argument
        if (strtok(NULL, delimiters) != NULL) {
          puts("cd: too many arguments");

        } else if (chdir(directory) == -1) {
          perror("chdir");
        }
      } else if (strcmp(token, "exit") == 0) {
        // our exit takes no arguments
        if (strtok(NULL, delimiters) != NULL) {
          puts("\nexit: too many arguments");

        } else {
          close(input_info.save_out);
          close(input_info.save_in);
          exit(EXIT_SUCCESS);
        }
      }
      /* filename must be one character or more,
      so token has to contain 3 or more characters */
      else if (strlen(token) > 2 && token[0] == '.' && token[1] == '/') {
        char *filename = &token[2];
        script_file = fopen(filename, "r");

        if (script_file == NULL) {
          perror("fopen script");
        }

        continue;
      } else {
        handle_input(input_info);
      }
    } else if (script_file != NULL) {
      fclose(script_file);
      script_file = NULL;
    }
  }
}
