cmake_minimum_required(VERSION 4.1)
project(mathc)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(mathc
        Buffer.cpp
        BUFFER.H
        CBUFFER.CPP
        CBUFFER.H
        CFUNC.CPP
        CFUNC.H
        CMATRIX.CPP
        CMATRIX.H
        FUNC.CPP
        FUNC.H
        FUNC1.CPP
        MATRIX.CPP
        MATRIX.H
        SPECFUNC.CPP
        VFUNC.CPP
        VFUNC.H)
