#ifndef HalideExamples_Random_h
#define HalideExamples_Random_h

#include <cstdlib>

namespace HalideExamples {

inline float Random(float min, float max) {
	int ix = std::rand() & 0xFFFFF;
	float x = static_cast<float>(ix) / 0xFFFFF;
	return x * (max - min) + min;
}

}

#endif // HalideExamples_Random_h