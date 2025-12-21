#pragma once

template <class T>
constexpr T max(T op1, T op2)
{
    return op1 > op2 ? op1 : op2;
}

template <class T, class... Args>
constexpr T max(T op1, Args... rest)
{
    return max(op1, max(rest...));
}

template <class T>
constexpr T min(T op1, T op2)
{
    return op1 > op2 ? op2 : op1;
}

template <class T, class... Args>
constexpr T min(T op1, Args... rest)
{
    return min(op1, min(rest...));
}

/**
 * @brief Clamps a value between bounds, including the bounds!
 *
 * @tparam T
 * @param v
 * @param lo
 * @param hi
 * @return T
 */
template <class T>
constexpr T clamp(T v, T lo, T hi)
{
    return max(min(v, hi), lo);
}

template <class T>
constexpr bool within(T v, T lo, T hi)
{
    return v <= hi && v >= lo;
}

