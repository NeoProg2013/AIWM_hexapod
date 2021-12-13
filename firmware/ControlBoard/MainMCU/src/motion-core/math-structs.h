/// ***************************************************************************
/// @file    math-structs.h
/// @author  NeoProg
/// ***************************************************************************
#ifndef _MATH_STRUCTS_H_
#define _MATH_STRUCTS_H_


typedef struct {
    float x;
    float y;
    float z;
} v3d_t;

typedef struct {
    float x;
    float z;
} v2d_t;

typedef struct {
    v3d_t n;
    float y;
} s3d_t;


#endif // _MATH_STRUCTS_H_
