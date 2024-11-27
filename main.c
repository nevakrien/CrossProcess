#include "cross_pipe.h"
#include <stdio.h>

int main() {
    CPipe pipe = cpipe_open("echo Hello, World!", "r");

    if (!pipe.stream) {
        fprintf(stderr, "Failed to open pipe.\n");
        return 1;
    }

    if (cpipe_done(&pipe)){
        printf("wow done fast no output\n");
        return 1;
    }

    char buffer[128];
    while (1) {
        int available = cpipe_available_bytes(&pipe);
        if (available > 0) {
            size_t bytes_read = cpipe_read(&pipe, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("Output: %s", buffer);
            }
        } else if (available==EOF||feof(pipe.stream)) {
            break;
        } else if(available<0){
            fprintf(stderr, "error with available%d.\n",available);
            return 1;
        }
    }

    int exit_code = cpipe_close(&pipe);
    printf("Process exit code: %d\n", exit_code);

    return 0;
}
