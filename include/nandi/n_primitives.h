#if !N_PRIMITIVES_H
#define N_PRIMITIVES_H 1
#include <stdint.h>

typedef int8_t      I8;
typedef int16_t     I16;
typedef int32_t     I32;
typedef int64_t     I64;

typedef uint8_t     U8;
typedef uint16_t    U16;
typedef uint32_t    U32;
typedef uint64_t    U64;

typedef float       F32;
typedef double      F64;

typedef U8          Bool;
#define TRUE ((U8)1)
#define FALSE ((U8)0)

typedef _Atomic(I8)   AtomicI8;
typedef _Atomic(I16)  AtomicI16;
typedef _Atomic(I32)  AtomicI32;
typedef _Atomic(I64)  AtomicI64;

typedef _Atomic(U8)   AtomicU8;
typedef _Atomic(U16)  AtomicU16;
typedef _Atomic(U32)  AtomicU32;
typedef _Atomic(U64)  AtomicU64;

typedef _Atomic(F32)  AtomicF32;
typedef _Atomic(F64)  AtomicF64;

typedef _Atomic(Bool) AtomicBool;

#endif // N_PRIMITIVES_H
