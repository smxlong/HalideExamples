#include <ImageConverter.h>

using namespace HalideExamples;
using namespace Halide;

int main() {
	Image<float> input(16, 16);
	Image<uint32_t> output(16, 16);
	ImageParam iparam(type_of<float>(), 2);
	Func ic = ImageConverter(iparam);

	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 16; ++x) {
			input(x, y) = 177.23f * (y * 16 + x) - 17.123f;
		}
	}

	iparam.set(input);
	ic.realize(output);
	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 16; ++x) {
			printf("%X,", output(x, y));
		}
		printf("\n");
	}
	return 0;
}
