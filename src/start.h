// This header is an auto-included prelude for each solver. It provides nothing
// which isn't found basically by default in a normal hosted environment.

// Flag this header as a system header. This will prevent warnings for unused
// statics, as well as for noreturn functions which GCC thinks can return.
#pragma GCC system_header

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#define NULL ((void*)0)

// System calls. We will only use three: read, write, and exit.

// extern int read(int fd, const void* buffer, int size);
// extern int write(int fd, const void* buffer, int size);
// extern __attribute__((noreturn)) void exit(int code);

static int read(int fd, const void* buffer, int size) {
  int result;
  asm volatile("int $0x80"
               : "=a"(result)
               : "a"(3), "b"(fd), "c"(buffer), "d"(size)
               : "memory");
  return result;
}

static int write(int fd, const void* buffer, int size) {
  int result;
  asm volatile("int $0x80"
               : "=a"(result)
               : "a"(4), "b"(fd), "c"(buffer), "d"(size)
               : "memory");
  return result;
}

static __attribute__((noreturn)) void exit(int code) {
  asm volatile("int $0x80" : : "a"(1), "b"(code));
}

static unsigned int strlen(const char* c_string) {
  const char* i = c_string;
  while (*i) i++;
  return i - c_string;
}

// GCC automatically generates calls to __udivdi3 and __umoddi3 when attempting
// division with `unsigned long long` variables, so we need to provide
// a definition. However, it seems that these magic symbols don't get GC'd
// properly when unused, so uncommenting this code causes all solvers to get
// bigger.

struct divmod_result {
  unsigned long long quotient;
  unsigned long long remainder;
};

__attribute__((pure)) struct divmod_result divmod_(unsigned long long a,
                                                   unsigned int b) {
  unsigned long long accumulator = 0;
  unsigned long long value = b;
  unsigned long long factor = 1;
  while ((value << 1) <= a) {
    value <<= 1;
    factor <<= 1;
  }
  while (factor) {
    if (value <= a) {
      a -= value;
      accumulator += factor;
    }
    value >>= 1;
    factor >>= 1;
  }
  return (struct divmod_result){.quotient = accumulator, .remainder = a};
}

__attribute__((pure)) unsigned long long __udivdi3(unsigned long long a,
                                                   unsigned long b) {
  return divmod_(a, b).quotient;
}

__attribute__((pure)) unsigned long long __umoddi3(unsigned long long a,
                                                   unsigned long b) {
  return divmod_(a, b).remainder;
}
