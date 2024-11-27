#ifndef CROSS_PIPE_H
#define CROSS_PIPE_H

#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>

    typedef struct {
        FILE *stream;       // File pointer for standard I/O
        HANDLE hFile;       // File handle
        HANDLE hProcess;    // Process handle (for checking status)
    } Pipe;
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/wait.h>

    typedef struct {
        FILE *stream;       // File pointer for standard I/O
        pid_t pid;          // Process ID (for checking status)
    } Pipe;
#endif

// Open a pipe to execute a command
static inline Pipe pipe_open(const char *command, const char *mode) {
    Pipe pipe;
#ifdef _WIN32
    pipe.stream = _popen(command, mode);
    if (pipe.stream) {
        pipe.hFile = (HANDLE)_get_osfhandle(_fileno(pipe.stream));
        pipe.hProcess = NULL;  // Not directly available from _popen
    } else {
        pipe.hFile = INVALID_HANDLE_VALUE;
        pipe.hProcess = NULL;
    }
#else
    pipe.stream = popen(command, mode);
    if (pipe.stream) {
        // Extract the process ID from the implementation
        // Note: This assumes popen is implemented with fork()
        pipe.pid = -1; // No direct way to get the PID from popen
    }
#endif
    return pipe;
}

// Check how many bytes are available to read (non-blocking)
static inline int pipe_available_bytes(Pipe *pipe) {
#ifdef _WIN32
    if (!pipe->stream || pipe->hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Invalid pipe handle.\n");
        return -1;
    }

    DWORD bytes_available = 0;
    if (!PeekNamedPipe(pipe->hFile, NULL, 0, NULL, &bytes_available, NULL)) {
        DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE) {
            // Pipe is closed, process likely finished
            return 0;
        }
        fprintf(stderr, "PeekNamedPipe failed: %lu\n", error);
        return -1;
    }
    return (int)bytes_available;
#else
    int available = 0;
    int fd = fileno(pipe->stream); // Get the file descriptor
    if (ioctl(fd, FIONREAD, &available) == -1) {
        perror("ioctl failed");
        return -1;
    }
    return available;
#endif
}

// Check if the process is done (non-blocking)
static inline int pipe_is_done(Pipe *pipe) {
#ifdef _WIN32
    if (!pipe->stream || pipe->hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Invalid pipe handle.\n");
        return -1;
    }

    DWORD exit_code;
    if (!GetExitCodeProcess(pipe->hProcess, &exit_code)) {
        fprintf(stderr, "GetExitCodeProcess failed: %lu\n", GetLastError());
        return -1;
    }

    return exit_code != STILL_ACTIVE;  // 1 if done, 0 if running
#else
    if (pipe->pid <= 0) {
        fprintf(stderr, "Invalid process ID.\n");
        return -1;
    }

    int status;
    pid_t result = waitpid(pipe->pid, &status, WNOHANG); // Non-blocking check
    if (result == 0) {
        return 0; // Still running
    } else if (result > 0) {
        return 1; // Process done
    } else {
        perror("waitpid failed");
        return -1;
    }
#endif
}

// Read data from the pipe (blocking)
static inline size_t pipe_read(Pipe *pipe, char *buffer, size_t buffer_size) {
    return fread(buffer, 1, buffer_size, pipe->stream);
}

// Close the pipe and get the exit code
static inline int pipe_close(Pipe *pipe) {
    int exit_code;
#ifdef _WIN32
    exit_code = _pclose(pipe->stream);
    pipe->stream = NULL;
    pipe.hFile = INVALID_HANDLE_VALUE;
    pipe.hProcess = NULL;
#else
    exit_code = pclose(pipe->stream);
    pipe->stream = NULL;
#endif
    return exit_code;
}

#endif // CROSS_PIPE_H
