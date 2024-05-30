#pragma once
#include <cmath>

enum EasingMode {
	Normal,
	CLAMP,
};

float EaseInSine(float t, EasingMode mode = Normal);
float EaseOutSine(float t, EasingMode mode = Normal);
float EaseInOutSine(float t, EasingMode mode = Normal);

float EaseInQuad(float t, EasingMode mode = Normal);
float EaseOutQuad(float t, EasingMode mode = Normal);
float EaseInOutQuad(float t, EasingMode mode = Normal);

float EaseInCubic(float t, EasingMode mode = Normal);
float EaseOutCubic(float t, EasingMode mode = Normal);
float EaseInOutCubic(float t, EasingMode mode = Normal);

float EaseInQuart(float t, EasingMode mode = Normal);
float EaseOutQuart(float t, EasingMode mode = Normal);
float EaseInOutQuart(float t, EasingMode mode = Normal);

float EaseInQuint(float t, EasingMode mode = Normal);
float EaseOutQuint(float t, EasingMode mode = Normal);
float EaseInOutQuint(float t, EasingMode mode = Normal);

float EaseInExpo(float t, EasingMode mode = Normal);
float EaseOutExpo(float t, EasingMode mode = Normal);
float EaseInOutExpo(float t, EasingMode mode = Normal);

float EaseInCirc(float t, EasingMode mode = Normal);
float EaseOutCirc(float t, EasingMode mode = Normal);
float EaseInOutCirc(float t, EasingMode mode = Normal);

float EaseInBack(float t, EasingMode mode = Normal);
float EaseOutBack(float t, EasingMode mode = Normal);
float EaseInOutBack(float t, EasingMode mode = Normal);

float EaseInElastic(float t, EasingMode mode = Normal);
float EaseOutElastic(float t, EasingMode mode = Normal);
float EaseInOutElastic(float t, EasingMode mode = Normal);

float EaseInBounce(float t, EasingMode mode = Normal);
float EaseOutBounce(float t, EasingMode mode = Normal);
float EaseInOutBounce(float t, EasingMode mode = Normal);

