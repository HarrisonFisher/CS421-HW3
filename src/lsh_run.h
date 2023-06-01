// lsh_run.h
#ifndef LSH_RUN_H
#define LSH_RUN_H

// Include other necessary headers...
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <jansson.h>


// Function prototypes
char* read_file(const char* filename);
char* execute_bash_script(char *script, char **args, int args_count);
char** get_args_from_str(const char* input_str, int* args_count);
char* getScript(const char* filename, const char* category, const char* language, const char* scriptType);
int lsh_run_exc(char **args);


char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the string
    char* buffer = (char*)malloc(size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer
    fread(buffer, 1, size, file);
    fclose(file);

    // Add null terminator
    buffer[size] = '\0';

    return buffer;
}


char* execute_bash_script(char *script, char **args, int args_count) {
  // Allocate memory for the args array based on args_count
  char **new_args = (char**)malloc((args_count+5) * sizeof(char*));

  // Set the first elements of the new_args array
  new_args[0] = "/bin/bash";
  new_args[1] = "-c";
  new_args[2] = script;
  new_args[3] = "";
  int j = 4;

  // Loop through the command line arguments and add them to the new_args array
  for (int i = 0; i < args_count; i++) {
    new_args[j++] = args[i];
  }

  // Set the last element of the new_args array to NULL
  new_args[j] = NULL;

  // Run the script as a separate process using execvp()
//   execvp(new_args[0], new_args);

    int pipefd[2];
    pid_t pid;
    char* buffer;
    size_t buffer_size = 1024; // Initial buffer size

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        close(pipefd[0]); // Close unused read end of the pipe
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
        execvp(new_args[0], new_args);
        perror("execvp"); // execvp only returns if there was an error
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]); // Close unused write end of the pipe
        size_t nbytes = 0;
        buffer = malloc(buffer_size);
        while (1) {
            int read_result = read(pipefd[0], buffer + nbytes, buffer_size - nbytes); // Read output from the read end of the pipe
            if (read_result == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            nbytes += read_result;
            if (read_result == 0 || nbytes == buffer_size) {
                break;
            }
            buffer_size *= 2; // Double the buffer size if it's not enough
            buffer = realloc(buffer, buffer_size);
        }
        buffer[nbytes] = '\0'; // Null-terminate the string
        return buffer; // Return the buffer containing the output
    }
}

char** get_args_from_str(const char* input_str, int* args_count) {
    // Create a mutable copy of the input string
    char* mutable_str = strdup(input_str);
    if (mutable_str == NULL) {
        // Error handling for memory allocation failure
        return NULL;
    }

    // Extract command-line arguments from mutable_str
    char** args = NULL;
    int count = 0;
    char* token = strtok(mutable_str, " ");
    while (token != NULL) {
        args = (char**)realloc(args, (count + 1) * sizeof(char*));
        args[count] = strdup(token);
        if (args[count] == NULL) {
            // Error handling for memory allocation failure
            free(mutable_str);
            for (int i = 0; i < count; i++) {
                free(args[i]);
            }
            free(args);
            return NULL;
        }
        count++;
        token = strtok(NULL, " ");
    }

    // Set args_count and return args
    *args_count = count;

    // Free the mutable string
    free(mutable_str);

    return args;
}


char* getScript(const char* filename, const char* category, const char* language, const char* scriptType) {
    char* script = NULL;
    json_error_t error;
    json_t* root = json_load_file(filename, 0, &error);

    if (!root) {
        fprintf(stderr, "Error loading JSON file: %s\n", error.text);
        return NULL;
    }

    json_t* category_obj = json_object_get(root, category);
    if (!json_is_object(category_obj)) {
        fprintf(stderr, "Category '%s' not found in JSON file.\n", category);
        json_decref(root);
        return NULL;
    }

    json_t* language_obj = json_object_get(category_obj, language);
    if (!json_is_object(language_obj)) {
        fprintf(stderr, "Language '%s' not found in JSON file.\n", language);
        json_decref(root);
        return NULL;
    }

    json_t* script_obj = json_object_get(language_obj, scriptType);
    if (!json_is_string(script_obj)) {
        fprintf(stderr, "Script not found for language '%s' and type '%s'.\n", language, scriptType);
        json_decref(root);
        return NULL;
    }

    script = strdup(json_string_value(script_obj));

    json_decref(root);
    return script;
}

int lsh_run_exc(char **args){
    char *file = args[1];
    char *test = args[2];
    char *extension = strrchr(file, '.');
    if (extension != NULL) {
        extension++;
    }

    // Compile
    char *Compile_Bash = getScript(test, "Bash Script", extension, "Compile");
    char* CompileArgs[] = {file, NULL};
    char *Compile_Output = execute_bash_script(Compile_Bash, CompileArgs, 1);

    // Read the JSON file
    FILE* test_file = fopen(test, "r");
    if (!test_file) {
        printf("Failed to open the JSON file.\n");
        return 1;
    }

    // Parse the JSON data
    json_t* root;
    json_error_t error;
    root = json_loadf(test_file, 0, &error);
    fclose(test_file);
    if (!root) {
        printf("JSON parsing error: %s\n", error.text);
        return 1;
    }

    // Get the "Test Cases" object
    json_t* testCases = json_object_get(root, "Test Cases");
    if (!json_is_object(testCases)) {
        printf("Failed to get 'Test Cases' object.\n");
        json_decref(root);
        return 1;
    }

    int testsPassed = 0;
    int testsFailed = 0;

    // Loop through the test cases
    const char* key;
    json_t* value;
    json_object_foreach(testCases, key, value) {
        // Get the test inputs and expected result
        if (!json_is_array(value)) {
            printf("Invalid test case format for '%s'.\n", key);
            continue;
        }

        json_t* inputs = json_array_get(value, 0);
        json_t* expectedResult = json_array_get(value, 1);

        if (!json_is_string(inputs) || !json_is_string(expectedResult)) {
            printf("Invalid test case format for '%s'.\n", key);
            continue;
        }

        const char* inputsStr = json_string_value(inputs);
        const char* expectedResultStr = json_string_value(expectedResult);

        int args_count = 0;
        char **Run_Args = get_args_from_str(inputsStr, &args_count);
        // Resize args to make room for new element
        Run_Args = (char**) realloc(Run_Args, (args_count + 1) * sizeof(char*));
        // Shift elements to make room for "Calculator.c"
        for (int i = args_count; i > 0; i--) {
            Run_Args[i] = Run_Args[i-1];
        }
        // Set first element to point to "Calculator.c"
        Run_Args[0] = (char*) malloc(strlen(file) + 1);
        strcpy(Run_Args[0], file);
        args_count += 1;

        // Run
        char *Run_Bash = getScript(test, "Bash Script", extension, "Execute");
        char *Run_Output = execute_bash_script(Run_Bash, Run_Args, args_count);
        size_t len = strlen(Run_Output);
        if (len > 0 && Run_Output[len-1] == '\n') {
            Run_Output[len-1] = '\0';
        }

        // Print test
        printf("Test:            %s\n", key);
        printf("Input:           %s\n", inputsStr);
        printf("Expected Result: %s\n", expectedResultStr);
        printf("Actual Result:   %s\n", Run_Output);
        

        // Compare actual result with expected result
        if (strcmp(Run_Output, expectedResultStr) == 0) {
            printf("Result:          %s\n\n", "PASSED");
            testsPassed++;
        } else {
            printf("Result:          %s\n\n", "FAILED");
            testsFailed++;
        }

        // Free memory allocated for args
        for (int i = 0; i < args_count; i++) {
            free(Run_Args[i]);
        }
        free(Run_Args);
    }

    
    // Exit
    char *Exit_Bash = getScript(test, "Bash Script", extension, "Exit");
    char* ExitArgs[] = {file, NULL};
    char *Exit_Output = execute_bash_script(Exit_Bash, ExitArgs, 1);


    // Display test summary
    printf("------------------------\n");
    printf("Test Summary:\n");
    printf("Tests Passed: %d\n", testsPassed);
    printf("Tests Failed: %d\n\n", testsFailed);

    // Cleanup
    json_decref(root);

    return 1;
}

#endif  // LSH_RUN_H
