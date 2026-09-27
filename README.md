# ++C

C with "classes"

++C is a CRT and OOP framework for C. It provides "Classes" via macros, and other features expected from a common CRT. It is not C standard compliant. It requires no dependencies (other than win32 APIs or Linux syscalls)

## Usage

### Demo

```c
#include <class.pph> // for classes
#include <app.pph> // base app interface
#include <sio.h> // simple IO

CLASS(Demo, Application)
{
    BASE(Application) // add base class
    FIELD(bool, showVersion)
    METHOD(bool, check_state, (void *self))
};

CLASS_EXPORT(Demo); // export the class

CONSTRUCTOR(Demo)
{
    SUPER_CTOR(self, Application); // calls the base class constructor
    self->showVersion = false;
    METHOD_LINK(base, Demo, run); // link the methods you need into the class
    METHOD_LINK(Demo, check_state);
}

DECONSTRUCTOR(Demo)
{
    SUPER_DTOR(self, Application); // calls the base class deconstructor
}

CLASS_INFO(Demo, Application); // generates the RTTI metadata

IMPLEMENT(Demo, int, run, (void *self))
{
    printf("Hello, World!\n");
    return 0;
}
```

```c
int program(parr_t csArgs)
{
    // heap-allocate class
    Demo *demo = NEW(Demo);

    // RTTI checked cast
    Application *app = AS(Application, demo);

    // dynamic invoking
    CALL1(app, parse_args, &csArgs);
    int exit_code = CALL0(app, main);

    // run destructor + free
    REMOVE(demo);

    return exit_code;
}
```

## Build

### Prerequisites

- CMake 3.25+
- Linux
  - GCC 13+ or Clang 16+
  - as / GNU Assembler / GCC / Clang
- Windows
  - MSVC 2022+ or Clang (targeting MinGW or MSVC ABI)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release

cmake --build build

./build/main
```

Windows (MSVC)

```batch
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

.\build\Release\main.exe
```

## Features

### Dynamic containers:

- `parr_t` Pointer array
- `dstr_t` Dynamic string
- `iarr_t` Dynamic integer array

### Signals


#### signal

```c
#include <sig.h> // read the header for more info

static void handle_sigint(int sig) {
    // ...
}

int program(parr_t csArgs) {
    if (signal(SIGINT, handle_sigint) == SIG_ERR)
        return 1;

    // ...
}
```

#### sigaction

```c
#include <sig.h>

static void on_sigterm(int sig)
{
    // ...
}

bool setup_signals(void)
{
    sigaction_t act = {0};
    sigaction_t old_act = {0};

    act.sa_handler = on_sigterm;
    act.sa_flags = SA_RESTART;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGTERM, &act, &old_act) < 0)
    {
        return false;
    }
    return true;
}
```

#### raise

```c
#include <sig.h>

static void custom_handler(int sig)
{
    // ...
}

int program(parr_t csArgs)
{
    signal(SIGUSR1, custom_handler);

    raise(SIGUSR1);

    return 0;
}
```

#### Ignoring a signal

```c
signal(SIGINT, SIG_IGN);
```

### Streams

#### Files

```c
#include <fio.pph>
#include <sio.h>

void fwrite_demo(void)
{
    // "w", "w+", "r", "r+", "a", "a+"
    Stream *file = fopen("log.txt", "w");
    if (!file)
    {
        puts("Failed to open file for writing.", SIOERR);
        return;
    }

    fprintf(file, "Application started: %s\n", "DemoApp");
    fprintf(file, "Count: %d, Float: %f\n", 42, 3.14159);

    fwrite(file, "static text.\n");

    CALL0(file, close);
    REMOVE(file);
}
```

#### Standard IO

```c
#include <fio.pph>

void stdio_demo(void)
{
    fprintf(stdout(), "Hello, World!\n");
    fprintf(stderr(), "Uh oh!: code %d\n", 500);

    dstr_t input = {0};
    freadline(stdin(), &input);
    fprintf(stdout(), "You entered: %s\n", input.data);
    dstr_free(&input);
}
```

#### Memory

```c
#include <mio.pph>
#include <fio.pph>   // FS_SEEK_SET and fprintf
#include <sio.h>

void memory_stream_demo(void)
{
    MemoryStream *ms = NEW(MemoryStream);
    Stream *stream = AS(Stream, ms);

    // write formatted data into memory
    fprintf(stream, "Message: %s (ID: %d)", "MemoryStream Test", 101);

    // rewind back to the beginning
    CALL2(stream, seek, 0, FS_SEEK_SET);

    // read bytes back from the memory stream
    char buffer[64] = {0};
    i64 bytes_read = CALL2(stream, read, buffer, sizeof(buffer) - 1);
    printf("Read %lld bytes from memory: %s\n", bytes_read, buffer);

    // inspect the underlying buffer directly
    const char *raw_buf = mstream_get_buffer(ms);
    printf("Raw contents: %s\n", raw_buf);

    REMOVE(ms);
}
```

### Paths

```c
#include <fs_path.pph>
#include <dstr.h>
#include <sio.h>

void path_demo(void)
{
    dstr_t root_str = dstr_new("var/logs");
    FSPath *path = fs_path(root_str);
    dstr_free(&root_str);

    // join paths (handles separators)
    CALL1(path, join, "app/debug.log");

    // output normalized path
    dstr_t full_path = CALL0(path, data);
    printf("Full Path: %s\n", full_path.data);
    dstr_free(&full_path);

    // extract directory (parent) and filename (base)
    FSPath *parent = CALL0(path, parent);
    FSPath *base   = CALL0(path, base);

    dstr_t parent_str = CALL0(parent, data);
    dstr_t base_str   = CALL0(base, data);

    printf("Parent dir: %s\n", parent_str.data);
    printf("filename:    %s\n", base_str.data);

    dstr_free(&parent_str);
    dstr_free(&base_str);

    REMOVE(parent);
    REMOVE(base);
    REMOVE(path);
}
```

```c
#include <fs_path.pph>
#include <sio.h>

void file_ops_demo(void)
{
    dstr_t name = dstr_new("test_file.tmp");
    FSPath *p = fs_path(name);
    dstr_free(&name);

    // create an empty file
    if (CALL0(p, touch))
    {
        puts("File created successfully.", SIOOUT);
    }

    // delete the file
    if (CALL0(p, remove))
    {
        puts("File deleted successfully.", SIOOUT);
    }

    REMOVE(p);
}
```
