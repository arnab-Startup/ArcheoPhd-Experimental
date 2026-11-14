#ifndef __EventToken_H__
#define __EventToken_H__

#ifndef __eventregistrationtoken_defined
#define __eventregistrationtoken_defined
typedef struct EventRegistrationToken {
    __int64 value;
} EventRegistrationToken;
#endif

#ifdef DEFINE_ENUM_FLAG_OPERATORS
#undef DEFINE_ENUM_FLAG_OPERATORS
#endif

#ifdef __cplusplus
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) \
extern "C++" { \
inline ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((int)a) | ((int)b)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) |= ((int)b)); } \
inline ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((int)a) & ((int)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) &= ((int)b)); } \
inline ENUMTYPE operator ~ (ENUMTYPE a) { return ENUMTYPE(~((int)a)); } \
inline ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((int)a) ^ ((int)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) ^= ((int)b)); } \
}
#else
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE)
#endif

#endif // __EventToken_H__
