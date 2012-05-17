//!-----------------------------------------------------
//!
//! \file mathConsts.h
//! contains the maths constants stuff
//! originally from a boost library in the vault, modified for my use as it no longer seems supported
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_MATHCONSTS_H
#define WIERD_CORE_MATHCONSTS_H


#include <limits>


namespace Math 
{


template <typename T>
static const T pi() { return static_cast<T>(3.14159265358979323846264338327950L); }

template <typename T>
static const T pi_over_2() { return pi<T>() / static_cast<T>(2); }

template <typename T>
static const T degree_to_radian() { return pi<T>() / static_cast<T>(180); }

template <typename T>
static const T radian_to_degree() { return static_cast<T>(1) / degree_to_radian<T>(); }

template <typename T>
static const T infinity() { return std::numeric_limits<T>::infinity(); }


}   // namespace Maths


#endif
