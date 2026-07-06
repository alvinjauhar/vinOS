
typedef __builtin_va_list va_list;

#define va_start(va, last) __builtin_va_start(va, last)
#define va_end(va) __builtin_va_end(va)
#define va_arg(va, type) __builtin_va_arg(va, type)
