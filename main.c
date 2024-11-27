#include "cross_pipe.h"
#include <stdio.h>

int main() {
    Pipe pipe = pipe_open("echo Hello, World!", "r");

    if (!pipe.stream) {
        fprintf(stderr, "Failed to open pipe.\n");
        return 1;
    }
    if(pipe_is_done(&pipe)){
        printf("pipe was done fast\n");
    }

    char buffer[128];
    while (1) {
        int available = pipe_available_bytes(&pipe);
        if (available > 0) {
            size_t bytes_read = pipe_read(&pipe, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("Output: %s", buffer);
            }
        } else if (feof(pipe.stream)) {
            break;
        }
    }

    int exit_code = pipe_close(&pipe);
    printf("Process exit code: %d\n", exit_code);

    return 0;
}
