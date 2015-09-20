#include <Graphics.h>
#include <Vec.h>
#include <Random.h>

using namespace Halide;

namespace HalideExamples {

const int NUM_PARTICLES = 512;
const float TIMESCALE = 1.0f;
const float GRAVITY = 0.01f * TIMESCALE * TIMESCALE;
const float FADE_BASE = 0.987f;
const float FADE = 0.987f; // pow(FADE_BASE, TIMESCALE)
// 

template <typename INPUT>
Func Gravity(INPUT input) {
	Var i;
	
	// Compute the cumulative force on each particle
	RDom j(0, input.width());
	
	// Compute the gravitational acceleration vector between a pair of
	// particles
	Vec x0(input(i, 0), input(i, 1), input(i, 2));
	Vec x1(input(j, 0), input(j, 1), input(j, 2));
	Vec dx = x1 - x0;
	Expr r2 = dx.magnitudeSquared();
	// Let r2 be no smaller than 1.0f, to avoid particles blasting off from each other when they
	// get too close. This also avoids dividing by zero.
	r2 = Halide::max(1.0f, r2);
	Expr r = Halide::sqrt(r2);
	Vec a = GRAVITY * input(j, 6) * dx / (r * r2);
	
	// Compute the cumulative force
	Func cumulativeForce;
	cumulativeForce(i) = Tuple(Halide::sum(a.x), Halide::sum(a.y), Halide::sum(a.z));

	cumulativeForce.vectorize(i, 32);
	cumulativeForce.compute_root();
	
	// Compute the updated positions
	Func updated;
	Var k;
	updated(i, k) = 0.0f;
	updated(i, 0) = input(i, 0) + input(i, 3);
	updated(i, 1) = input(i, 1) + input(i, 4);
	updated(i, 2) = input(i, 2) + input(i, 5);
	updated(i, 3) = input(i, 3) + cumulativeForce(i)[0];
	updated(i, 4) = input(i, 4) + cumulativeForce(i)[1];
	updated(i, 5) = input(i, 5) + cumulativeForce(i)[2];
	updated(i, 6) = input(i, 6);

	for (int up = 0; up < 6; ++up) {
		updated.update(up).vectorize(i, 32);
	}
	
	return updated;
}

Func Renderer(Image<float>& particles, Image<float>& previmage, int width, int height) {
	Func image;
	Var x, y;
	
	image(x, y) = FADE * previmage(x, y);
	RDom i(0, NUM_PARTICLES);
	
	Expr posx = clamp(cast<int>(particles(i, 0) + 0.5f), 0.0f, static_cast<float>(width - 1));
	Expr posy = clamp(cast<int>(particles(i, 1) + 0.5f), 0.0f, static_cast<float>(height - 1));
	image(posx, posy) = 1.0f;

	image.vectorize(x, 32);
	
	return image;
}
	
void RunDemo(int width, int height) {
	Buffer oldbuff(type_of<float>(), NUM_PARTICLES, 7);
	Buffer newbuff(type_of<float>(), NUM_PARTICLES, 7);
	Image<float> oldparticles(oldbuff);
	Image<float> newparticles(newbuff);
	
	Buffer previmagebuff(type_of<float>(), width, height);
	Buffer imagebuff(type_of<float>(), width, height);
	Image<float> previmage(previmagebuff);
	Image<float> image(imagebuff);
	
	// Initialize particles
	
	for (int i = 0; i < NUM_PARTICLES; ++i) {
		oldparticles(i, 0) = Random(0.0f, static_cast<float>(width - 1));
		oldparticles(i, 1) = Random(0.0f, static_cast<float>(height - 1));
		oldparticles(i, 2) = 0.0f;

		float vx = Random(-1.0f, 1.0f);
		float vy = Random(-1.0f, 1.0f);
		while (vx * vx + vy * vy > 1.0f) {
			vx = Random(-1.0f, 1.0f);
			vy = Random(-1.0f, 1.0f);
		}
		
		oldparticles(i, 3) = TIMESCALE * 0.25f * vx;
		oldparticles(i, 4) = TIMESCALE * 0.25f * vy;
		oldparticles(i, 5) = 0.0f;
		oldparticles(i, 6) = Random(0.1f, 10.0f);
	}
	// sun in the middle
	oldparticles(0, 0) = width * 0.5f;
	oldparticles(0, 1) = height * 0.5f;
	oldparticles(0, 2) = 0.0f;
	oldparticles(0, 3) = 0.0f;
	oldparticles(0, 4) = 0.0f;
	oldparticles(0, 5) = 0.0f;
	oldparticles(0, 6) = 1000.0f;
	
	// Main loop
	
	Func grav = Gravity(oldparticles);
	Func renderer = Renderer(oldparticles, previmage, width, height);
	int nframe = 0;
	while (true) {
		printf("%d\n", nframe++);
		renderer.realize(image);
		DisplayImage(image);
		grav.realize(newparticles);
		std::swap(*oldbuff.raw_buffer(), *newbuff.raw_buffer());
		std::swap(*previmagebuff.raw_buffer(), *imagebuff.raw_buffer());
	}
	
}

}