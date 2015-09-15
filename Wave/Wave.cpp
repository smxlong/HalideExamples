#include <cstdio>

#include <Halide.h>
#include <Graphics.h>

using namespace Halide;

namespace HalideExamples {

////////////////////////// WAVE FUNCTION //////////////////////////

template <typename F1, typename F2, typename F3>
Func WavePropagator(F1 prev, F2 curr, F3 scale) {
	Func next;
	Var x, y, xi, yi, xo, yo;

	////////////////////////// ALGORITHM //////////////////////////

	// Discrete 2D wave equation. Forward time centered space (FTCS). There are far more sophisticated methods.
	next(x, y) = scale(x, y) * (curr(x, y - 1) + curr(x - 1, y) + curr(x + 1, y) + curr(x, y + 1) - 4 * curr(x, y)) + 2 * curr(x, y) - prev(x, y);

	////////////////////////// SCHEDULE //////////////////////////

	// Split the space into 256x256 blocks for parallelization
	Var tx, ty, nx, ny, ti;
	next.tile(x, y, tx, ty, nx, ny, 256, 256);

	// Split the blocks into smaller 32x16 tiles, vectorize and unroll
	next.tile(nx, ny, xo, yo, xi, yi, 32, 16)
		.vectorize(xi)
		.unroll(yi);

	// Run all blocks in parallel
	next.fuse(tx, ty, ti);
	next.parallel(ti);

	return next;
}

////////////////////////// MAIN DEMO FUNCTION //////////////////////////

void RunDemo(int width, int height) {
	std::printf("Hello, world!\n");

	// The wave function takes three inputs:
	//   The previous wave values
	//   The current wave values
	//   The scale buffer (controls wave velocity at each point)
	// It outputs the next wave values.
	//
	// We keep three buffers to store the previous, current, and next wave values, and cycle through
	// them on each iteration. Halide doesn't make this easy, because it forces us to specify a
	// buffer as input to the wave function, but this will not always be the buffer the input comes
	// from. The solution is to go behind Halide's back and swap out the underlying buffer_t structures
	// to cycle through the buffers.

	Buffer buff1(type_of<float>(), width, height);
	Buffer buff2(type_of<float>(), width, height);
	Buffer buff3(type_of<float>(), width, height);
	Image<float> prev(buff1);
	Image<float> curr(buff2);
	Image<float> next(buff3);
	Image<float> scale(width, height);

	// Initialize the wave values
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			prev(x, y) = 0.0f;
			curr(x, y) = 0.0f;
			next(x, y) = 0.0f;
			scale(x, y) = 0.3f;
		}
	}
	// A single drop of water in the center to start
	curr(width / 2, height / 2) = 1.0f;

	// More random drops
	for (int i = 0; i < 1000; ++i) {
		int x = std::rand() % width;
		int y = std::rand() % height;
		curr(x, y) = 1.0f;
	}

	Func wv = WavePropagator(Image<float>(buff1), Image<float>(buff2), scale);

	unsigned int nframes = 0;
	while (true) {
		DisplayImage(curr);
		// Set the min and extent of the output buffer so we compute only the valid region
		buffer_t* rawbuf = buff3.raw_buffer();
		rawbuf->extent[0] -= 2;
		rawbuf->extent[1] -= 2;
		rawbuf->min[0] = 1;
		rawbuf->min[1] = 1;
		rawbuf->host += rawbuf->elem_size * (rawbuf->stride[0] + rawbuf->stride[1]);
		// Compute the output
		wv.realize(buff3);
		// Restore min and extent
		rawbuf = buff3.raw_buffer();
		rawbuf->extent[0] += 2;
		rawbuf->extent[1] += 2;
		rawbuf->min[0] = 0;
		rawbuf->min[1] = 0;
		rawbuf->host -= rawbuf->elem_size * (rawbuf->stride[0] + rawbuf->stride[1]);
		// Cycle through the buffers
		buffer_t* a;
		buffer_t* b;
		buffer_t* c;
		a = buff1.raw_buffer();
		b = buff2.raw_buffer();
		c = buff3.raw_buffer();
		std::swap(*a, *b);
		std::swap(*b, *c);

		++nframes;
	}

}

}
