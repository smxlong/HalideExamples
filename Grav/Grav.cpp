#include <Graphics.h>
#include <Vec.h>
#include <Random.h>

using namespace Halide;

namespace HalideExamples {

const int NUM_PARTICLES = 512;
const float GRAVITY = 2.0f;

template <typename INPUT>
Func Gravity(INPUT input) {
	Var i;
	
	// Compute the cumulative force on each particle
	RDom j(0, input.width());
	
	// Compute the gravitational force vector between a pair of
	// particles
	Vec x0(input(i, 0), input(i, 1), input(i, 2));
	Vec x1(input(j, 0), input(j, 1), input(j, 2));
	Vec dx = x1 - x0;
	Expr r2 = dx.magnitudeSquared();
	// Let r2 be no smaller than 1.0f, to avoid particles blasting off from each other when they
	// get too close. This also avoids dividing by zero.
	r2 = Halide::max(1.0f, r2);
	Expr r = Halide::sqrt(r2);
	Vec f = GRAVITY * dx / (r * r2);
	
	// Compute the cumulative force
	Func cumulativeForce;
	cumulativeForce(i) = Tuple(Halide::sum(f.x), Halide::sum(f.y), Halide::sum(f.z));

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

	for (int up = 0; up < 6; ++up) {
		updated.update(up).vectorize(i, 32);
	}
	
	return updated;
}

Func Renderer(Image<float>& particles, int width, int height) {
	Func image;
	Var x, y;
	
	image(x, y) = 0.0f;
	RDom i(0, NUM_PARTICLES);
	
	Expr posx = clamp(cast<int>(particles(i, 0) + 0.5f), 0.0f, static_cast<float>(width - 1));
	Expr posy = clamp(cast<int>(particles(i, 1) + 0.5f), 0.0f, static_cast<float>(height - 1));
	image(posx, posy) = 1.0f;

	image.vectorize(x, 32);
	
	return image;
}
	
void RunDemo(int width, int height) {
	Buffer oldbuff(type_of<float>(), NUM_PARTICLES, 6);
	Buffer newbuff(type_of<float>(), NUM_PARTICLES, 6);
	
	Image<float> oldparticles(oldbuff);
	Image<float> newparticles(newbuff);
	Image<float> rendered(width, height);
	
	for (int i = 0; i < NUM_PARTICLES; ++i) {
		oldparticles(i, 0) = Random(0.0f, static_cast<float>(width - 1));
		oldparticles(i, 1) = Random(0.0f, static_cast<float>(height - 1));
		oldparticles(i, 2) = 0.0f;
		oldparticles(i, 3) = 0.0f;
		oldparticles(i, 4) = 0.0f;
		oldparticles(i, 5) = 0.0f;
	}
	
	Func grav = Gravity(oldparticles);
	Func renderer = Renderer(oldparticles, width, height);
	int nframe = 0;
	while (true) {
		printf("%d\n", nframe++);
		renderer.realize(rendered);
		DisplayImage(rendered);
		grav.realize(newparticles);
		std::swap(*oldbuff.raw_buffer(), *newbuff.raw_buffer());
	}
	
}

}