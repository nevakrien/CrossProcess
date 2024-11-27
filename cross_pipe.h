#ifndef CROSS_PIPE_H
#define CROSS_PIPE_H

#include <stdio.h>

#define LOG_ERROR(x) perror(x);

#ifdef _WIN32
    #include <windows.h>
    typedef struct {
        FILE *stream;       // File pointer for standard I/O
        HANDLE process;     // Process handle for checking status
    } Pipe;
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <errno.h>
    typedef struct {
        FILE *stream;       // File pointer for standard I/O
    } Pipe;
#endif

// Open a pipe to execute a command
static inline Pipe pipe_open(const char *command, const char *mode) {
    Pipe pipe;
#ifdef _WIN32
    pipe.stream = _popen(command, mode);
    if (pipe.stream) {
        pipe.process = (HANDLE)_get_osfhandle(_fileno(pipe.stream));
    }
#else
    pipe.stream = popen(command, mode);
#endif
    return pipe;
}

// Check how many bytes are available to read (non-blocking)
static inline int pipe_available_bytes(Pipe *pipe) {
#ifdef _WIN32
    DWORD bytes_available = 0;
    if (!PeekNamedPipe(pipe->process, NULL, 0, NULL, &bytes_available, NULL)) {
        LOG_ERROR("PeekNamedPipe failed");
        return -1;
    }
    return (int)bytes_available;
#else
    int available = 0;
    int fd = fileno(pipe->stream); // Get the file descriptor
    if (ioctl(fd, FIONREAD, &available) == -1) {
        LOG_ERROR("ioctl failed");
        return -1;
    }
    return available;
#endif
}

// Read data from the pipe (blocking)
static inline size_t pipe_read(Pipe *pipe, char *buffer, size_t buffer_size) {
    return fread(buffer, 1, buffer_size, pipe->stream);
}

// Get the exit code of the process (after the pipe is closed)
static inline int pipe_get_exit_code(Pipe *pipe) {
#ifdef _WIN32
    DWORD exit_code;
    if (!GetExitCodeProcess(pipe->process, &exit_code)) {
        LOG_ERROR("GetExitCodeProcess failed");
        return -1;
    }
    return (int)exit_code;
#else
    LOG_ERROR("Exit code unavailable until pipe is closed.\n");
    return -1;
#endif
}

// Close the pipe
static inline int pipe_close(Pipe *pipe) {
#ifdef _WIN32
    return _pclose(pipe->stream);
#else
    return pclose(pipe->stream);
#endif
}

#endif // CROSS_PIPE_H
