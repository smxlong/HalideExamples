#include <Graphics.h>
#include <Vec.h>
#include <Random.h>

using namespace Halide;

namespace HalideExamples {

const int MESH_WIDTH = 32;
const int MESH_HEIGHT = 32;
const float SPRING_REST_LENGTH = 2.83464566929f;
const float SPRING_FORCE = 0.02f;
const float FADE = 0.977f;

Vec SpringForce(const Vec& r0, const Vec& r1) {
	Vec dr = r1 - r0;
	Expr len = dr.magnitude();
	Expr f = (len - SPRING_REST_LENGTH) * SPRING_FORCE;
	return f * dr / len;
}

template <typename INPUT>
Func SpringMesh(INPUT input) {
	Func output;
	Var x, y, z;
	output(x, y, z) = input(x, y, z);
	
	// Compute force for interior nodes (those with all four neighbors)
	RDom i(1, input.width() - 2, 1, input.height() - 2);
	Vec r0(input(i.x, i.y, 0), input(i.x, i.y, 1), 0.0f);
	Vec rl(input(i.x - 1, i.y, 0), input(i.x - 1, i.y, 1), 0.0f);
	Vec rr(input(i.x + 1, i.y, 0), input(i.x + 1, i.y, 1), 0.0f);
	Vec ru(input(i.x, i.y - 1, 0), input(i.x, i.y - 1, 1), 0.0f);
	Vec rd(input(i.x, i.y + 1, 0), input(i.x, i.y + 1, 1), 0.0f);
	Vec f = SpringForce(r0, rl) + SpringForce(r0, rr) + SpringForce(r0, ru) + SpringForce(r0, rd);
	output(i.x, i.y, 0) = input(i.x, i.y, 0) + input(i.x, i.y, 2) + f.x;
	output(i.x, i.y, 1) = input(i.x, i.y, 1) + input(i.x, i.y, 3) + f.y;
	output(i.x, i.y, 2) = input(i.x, i.y, 2) + f.x;
	output(i.x, i.y, 3) = input(i.x, i.y, 3) + f.y;
	
	Var xo, yo, xi, yi;
	output.tile(x, y, xo, yo, xi, yi, 32, 8).vectorize(xi).unroll(yi);
	
	// We'll deal with the edge cases later
	return output;
}

Func Renderer(Image<float>& particles, Image<float>& previmage, int width, int height) {
	Func image;
	Var x, y;
	
	image(x, y) = FADE * previmage(x, y);
	RDom i(0, particles.width(), 0, particles.height());
	
	Expr posx = clamp(cast<int>(particles(i.x, i.y, 0) + 0.5f), 0.0f, static_cast<float>(width - 1));
	Expr posy = clamp(cast<int>(particles(i.x, i.y, 1) + 0.5f), 0.0f, static_cast<float>(height - 1));
	image(posx, posy) = 1.0f;

	image.vectorize(x, 32);
	
	return image;
}
	
void RunDemo(int width, int height) {
	Buffer oldbuff(type_of<float>(), MESH_WIDTH, MESH_HEIGHT, 4);
	Buffer newbuff(type_of<float>(), MESH_WIDTH, MESH_HEIGHT, 4);
	Image<float> oldparticles(oldbuff);
	Image<float> newparticles(newbuff);
	
	Buffer previmagebuff(type_of<float>(), width, height);
	Buffer imagebuff(type_of<float>(), width, height);
	Image<float> previmage(previmagebuff);
	Image<float> image(imagebuff);
	
	// Initialize particles. We want the block of particles to take up the middle
	// of the screen -- half the screen height, centered
	float left = (width - height * 0.5f) * 0.5f;
	float stepx = height * 0.5f / (MESH_WIDTH - 1);
	float top = height * 0.25f;
	float stepy = height * 0.5f / (MESH_HEIGHT - 1);
	
	
	for (int y = 0; y < MESH_HEIGHT; ++y) {
		for (int x = 0; x < MESH_WIDTH; ++x) {
			oldparticles(x, y, 0) = left + stepx * x + Random(-0.2f, 0.2f);
			oldparticles(x, y, 1) = top + stepy * y + Random(-0.2f, 0.2f);
			oldparticles(x, y, 2) = 0.0f;//Random(-0.05f, 0.05f);
			oldparticles(x, y, 3) = 0.0f;//Random(-0.05f, 0.05f);
		}
	}
	
	// Main loop
	
	Func spring = SpringMesh(oldparticles);
	Func renderer = Renderer(oldparticles, previmage, width, height);
	int nframe = 0;
	float period = 70.0f;
	while (true) {
		for (int x = 1; x < MESH_WIDTH - 1; ++x) {
			oldparticles(x, 1, 1) = top + stepy + 0.4f * stepy * std::cos(2 * M_PI * nframe / period);
		}
		printf("%d\n", nframe++);
		renderer.realize(image);
		DisplayImage(image);
		spring.realize(newparticles);
		std::swap(*oldbuff.raw_buffer(), *newbuff.raw_buffer());
		std::swap(*previmagebuff.raw_buffer(), *imagebuff.raw_buffer());
	}
	
}

}