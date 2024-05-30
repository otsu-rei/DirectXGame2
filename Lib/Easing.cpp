#include "Easing.h"
#include <numbers>

namespace {

	constexpr const float pi = std::numbers::pi_v<float>;

}

/* --- Sine --- */
float EaseInSine(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return 1.0f - std::cos((t * pi) / 2.0f);
}

float EaseOutSine(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return std::sin((t * pi) / 2.0f);
}

float EaseInOutSine(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return -(std::cos(pi * t) - 1.0f) / 2.0f;
}

/* --- Quad --- */
float EaseInQuad(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return t * t;
}

float EaseOutQuad(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return 1.0f - (1.0f - t) * (1.0f - t);
}

float EaseInOutQuad(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return t < 0.5f
		? 2.0f * t * t
		: 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

/* --- Cubic --- */
float EaseInCubic(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return t * t * t;
}

float EaseOutCubic(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return 1.0f - std::pow(1.0f - t, 3.0f);
}

float EaseInOutCubic(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return t < 0.5f
		? 4.0f * t * t * t
		: 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

/* --- Quart --- */
float EaseInQuart(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return t * t * t * t;
}

float EaseOutQuart(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return 1.0f - std::pow(1.0f - t, 4.0f);
}

float EaseInOutQuart(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return t < 0.5f
		? 8.0f * t * t * t * t
		: 1.0f - std::pow(-2.0f * t + 2.0f, 4.0f) / 2.0f;
}

/* --- Quint --- */
float EaseInQuint(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return t * t * t * t * t;
}

float EaseOutQuint(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return 1.0f - std::pow(1.0f - t, 5.0f);
}

float EaseInOutQuint(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return t < 0.5f
		? 16.0f * t * t * t * t * t
		: 1.0f - std::pow(-2.0f * t + 2.0f, 5.0f) / 2.0f;
}

/* --- Expo --- */
float EaseInExpo(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return t == 0.0f
		? 0.0f
		: std::pow(2.0f, 10.0f * t - 10.0f);
}

float EaseOutExpo(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return t == 1.0f
		? 1.0f
		: 1.0f - std::pow(2.0f, -10.0f * t);
}

float EaseInOutExpo(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return t == 0.0f
		? 0.0f
		: t == 1.0f
		? 1.0f
		: t < 0.5f
		? std::pow(2.0f, 20.0f * t - 10.0f) / 2.0f
		: (2.0f - std::pow(2.0f, -20.0f * t + 10.0f)) / 2.0f;
}

/* --- Circ --- */
float EaseInCirc(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return 1.0f - sqrtf(1.0f - std::pow(t, 2.0f));
}

float EaseOutCirc(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return sqrtf(1.0f - std::pow(t - 1.0f, 2.0f));
}

float EaseInOutCirc(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	return t < 0.5f
		? (1.0f - sqrtf(1.0f - std::pow(2.0f * t, 2.0f))) / 2.0f
		: (sqrtf(1.0f - std::pow(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
}

/* --- Back --- */
float EaseInBack(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	const float c1 = 1.70158f;
	const float c2 = c1 + 1.0f;
	return c2 * t * t * t - c1 * t * t;
}

float EaseOutBack(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	const float c1 = 1.70158f;
	const float c2 = c1 + 1.0f;
	return 1.0f + c2 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
}

float EaseInOutBack(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	const float c1 = 1.70158f;
	const float c3 = c1 * 1.525f;
	return t < 0.5f
		? (std::pow(2.0f * t, 2.0f) * ((c3 + 1) * 2.0f * t - c3)) / 2.0f
		: (std::pow(2.0f * t - 2.0f, 2.0f) * ((c3 + 1.0f) * (t * 2.0f - 2.0f) + c3) + 2.0f) / 2.0f;
}

/* --- Elastic --- */
float EaseInElastic(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	const float c4 = (2.0f * pi) / 3.0f;
	return t == 0.0f
		? 0.0f
		: t == 1.0f
		? 1.0f
		: -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4);
}

float EaseOutElastic(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	const float c4 = (2.0f * pi) / 3.0f;
	return t == 0.0f
		? 0.0f
		: t == 1.0f
		? 1.0f
		: std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}

float EaseInOutElastic(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	const float c5 = (2.0f * pi) / 4.5f;
	return t == 0.0f
		? 0.0f
		: t == 1.0f
		? 1.0f
		: t < 0.5f
		? -(std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) / 2.0f
		: (std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
}

/* --- Bounce --- */
float EaseInBounce(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	const float n1 = 7.5625f;
	const float d1 = 2.75f;
	float t2 = 0.0f;
	float t3 = 0.0f;

	t3 = (1.0f - t);

	if (t3 < 1.0f / d1) {
		return 1.0f - (n1 * t3 * t3);

	} else if (t3 < 2.0f / d1) {
		t2 = t3 - 1.5f / d1;
		return 1.0f - (n1 * t2 * t2 + 0.75f);

	} else if (t3 < 2.5f / d1) {
		t2 = t3 - 2.25f / d1;
		return 1.0f - (n1 * t2 * t2 + 0.9375f);

	} else {
		t2 = t3 - 2.625f / d1;
		return 1.0f - (n1 * t2 * t2 + 0.984375f);
	}
}

float EaseOutBounce(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	const float n1 = 7.5625f;
	const float d1 = 2.75f;
	float t2 = 0.0f;

	if (t < 1.0f / d1) {
		return n1 * t * t;

	} else if (t < 2.0f / d1) {
		t2 = t - 1.5f / d1;
		return n1 * t2 * t2 + 0.75f;

	} else if (t < 2.5f / d1) {
		t2 = t - 2.25f / d1;
		return n1 * t2 * t2 + 0.9375f;

	} else {
		t2 = t - 2.625f / d1;
		return n1 * t2 * t2 + 0.984375f;
	}
}

float EaseInOutBounce(float t, EasingMode mode) {
	if (mode == EasingMode::CLAMP) {
		if (t < 0.0f) { t = 0.0f; }
		else if (t > 1.0f) { t = 1.0f; }
	}

	const float n1 = 7.5625f;
	const float d1 = 2.75f;
	float t2 = 0.0f;
	float t3 = 0.0f;

	if (t < 0.5) {
		t3 = (1.0f - t * 2.0f);

		if (t3 < 1.0f / d1) {
			return (1.0f - (n1 * t3 * t3)) / 2.0f;

		} else if (t3 < 2.0f / d1) {
			t2 = t3 - 1.5f / d1;
			return (1.0f - (n1 * t2 * t2 + 0.75f)) / 2.0f;

		} else if (t3 < 2.5f / d1) {
			t2 = t3 - 2.25f / d1;
			return (1.0f - (n1 * t2 * t2 + 0.9375f)) / 2.0f;

		} else {
			t2 = t3 - 2.625f / d1;
			return (1.0f - (n1 * t2 * t2 + 0.984375f)) / 2.0f;
		}

	} else {
		t3 = (2.0f * t - 1.0f);

		if (t3 < 1.0f / d1) {
			return (1.0f + (n1 * t3 * t3)) / 2.0f;

		} else if (t3 < 2.0f / d1) {
			t2 = t3 - 1.5f / d1;
			return (1.0f + (n1 * t2 * t2 + 0.75f)) / 2.0f;

		} else if (t3 < 2.5f / d1) {
			t2 = t3 - 2.25f / d1;
			return (1.0f + (n1 * t2 * t2 + 0.9375f)) / 2.0f;

		} else {
			t2 = t3 - 2.625f / d1;
			return (1.0f + (n1 * t2 * t2 + 0.984375f)) / 2.0f;
		}
	}
}