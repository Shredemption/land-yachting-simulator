#ifndef MATH_H
#define MATH_H

#include <cmath>

inline float easeOutCubic(float start, float end, float alpha)
{
    return start + (end - start) * (1 - std::pow((1 - alpha), 3));
};

inline float easeOutBack(float start, float end, float alpha, float overshootFactor)
{
    return start + (end - start) * (1 + (overshootFactor + 1) * std::pow((alpha - 1.0f), 3.0f) + overshootFactor * std::pow((alpha - 1.0f), 2.0f));
}

inline float easeInOutCirc(float start, float end, float alpha)
{
    return start + (end - start) * (alpha < 0.5f ? (1.0f - std::sqrt(1.0f - std::pow(2.0f * alpha, 2.0f))) / 2.0f : (std::sqrt((1.0f - std::pow(-2.0f * alpha + 2.0f, 2.0f))) + 1.0f) / 2);
}

inline float easeInOutQuad(float start, float end, float alpha)
{
    return start + (end - start) * (alpha < 0.5f ? 2.0f * alpha * alpha : 1.0f - std::pow(-2.0f * alpha + 2.0f, 2.0f) / 2.0f);
}
#endif