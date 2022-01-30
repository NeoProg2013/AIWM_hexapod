/// ***************************************************************************
/// @file    math-structs.h
/// @author  NeoProg
/// ***************************************************************************
#ifndef _MATH_STRUCTS_H_
#define _MATH_STRUCTS_H_

// Vector 3D
typedef struct {
    float x;
    float y;
    float z;
} v3d_t;

// Rotate 3D
typedef v3d_t r3d_t;

// Point 3D
typedef v3d_t p3d_t;

// Vector 2D
typedef struct {
    float x;
    float z;
} v2d_t;

// Surface
typedef struct {
    v3d_t n; // Normal vector
    p3d_t a; // Any point on surface A(x0, y0, z0)
} s3d_t;


#endif // _MATH_STRUCTS_H_
