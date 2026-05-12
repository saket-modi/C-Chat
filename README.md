## Running locally
- **Windows:** This repository was developed on Windows using Cygwin for 64-bit machines to access UNIX/POSIX compliant headers such as <code><unistd.h></code> and <code><sys/socket.h></code>. It won't compile natively on Windows using MSYS2 or MinGW.
    - Install Cygwin with the following packages (packages can be selected during the installation process via the setup):
    - <code>gcc-core</code>, <code>gcc-g++</code>, <code>cygwin-devel</code>
- **Unix/Linux-based:** The code should compile natively on POSIX compliant systems. All libraries used are present in the C stdlib.

### Compiling
Run <code>gcc src/server.c -o server</code> and <code>gcc src/client.c -o client</code>

### Execution
- Execute the <code>server</code> first. The <code>client</code> attempts to connect to <code>localhost:6767</code> as part of its initial <code>connect()</code> call. If the server is not <code>bind()</code>ed (or in fact, even if the server isn't <code>listen()</code>ing to requests on the port) to the host yet, the client will run into an error.
- Multiple number of terminal windows may be opened and the <code>client</code> may be run to involve multiple clients. The maximum number of clients is defined in the <code>MAX_CLIENTS</code> macro under <code>server.c</code>. This is passed to the <code>listen()</code> function in:
```c
get_listener_fd(char* port)
```