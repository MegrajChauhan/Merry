#ifndef _MERRY_UTILS_
#define _MERRY_UTILS_

#define _stringify(x) #x
#define _glued(x, y) x##y
#define _toggle(x) (~x) >> 63
#define _MERRY_STRINGIFY_(to_str) _stringify(to_str)
#define _MERRY_CONCAT_(x, y) _glued(x, y)
#define _MERRY_TOGGLE_(x) _toggle(x) // toggle a boolean value

#endif