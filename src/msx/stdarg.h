#define va_list                 unsigned char *
#define va_start(ap, last)      do { ap = &(last) + (getarg() * 2) - 3; } while(0)
#define va_arg(ap, type)        (*(type*)((ap) -= sizeof(type), (ap) + 1))
#define va_copy(dst, src)       do { (dst) = (src); } while(0)
#define va_end(ap)
