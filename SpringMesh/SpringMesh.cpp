#include <Graphics.h>
#include <Vec.h>
#include <Random.h>

using namespace Halide;

namespace HalideExamples {

const int MESH_WIDTH = 64;
const int MESH_HEIGHT = 64;
const float SPRING_REST_LENGTH = 5.714286;
const float SPRING_FORCE = 0.3f;
const float GRAVITY = 0.0001f;
//const float GRAVITY = 0.0f;
const float FADE = 0.977f;
const float ROOT2 = 1.4142135623f;
const float DEGREES_TO_RADS = 0.0174532925199f;

Vec SpringForce(const Vec& r0, const Vec& r1) {
	Vec dr = r1 - r0;
	Expr len = dr.magnitude();
	Expr f = (len - SPRING_REST_LENGTH) * SPRING_FORCE;
	return f * dr / len;
}

template <typename INPUT, typename COORD>
Vec SpringForce(INPUT& input, COORD& x, COORD& y, Expr dx, Expr dy, float scale=1.0f) {
	Vec r0(input(x, y, 0), input(x, y, 1), 0.0f);
	Expr x1 = clamp(x + dx, 0, input.width() - 1);
	Expr y1 = clamp(y + dy, 0, input.height() - 1);
	Vec r1(input(x1, y1, 0), input(x1, y1, 1), 0.0f);
	Vec dr = r1 - r0;
	Expr len = dr.magnitude();
	Vec f = (len - scale * SPRING_REST_LENGTH) * SPRING_FORCE * dr / len;
	Expr outx = select(x1 == x + dx && y1 == y + dy, f.x, 0.0f);
	Expr outy = select(x1 == x + dx && y1 == y + dy, f.y, 0.0f);
	return Vec(outx, outy, 0);
}

template <typename INPUT>
Func SpringMesh(INPUT input) {
	Func output;
	Var x, y, z;
	output(x, y, z) = input(x, y, z);

	Vec f = SpringForce(input, x, y,  0, -1)
		  + SpringForce(input, x, y, -1,  0)
		  + SpringForce(input, x, y,  1,  0)
		  + SpringForce(input, x, y,  0,  1)
		  + SpringForce(input, x, y, -1, -1, ROOT2)
		  + SpringForce(input, x, y, -1,  1, ROOT2)
		  + SpringForce(input, x, y,  1, -1, ROOT2)
		  + SpringForce(input, x, y,  1,  1, ROOT2);
	output(x, y, 0) = input(x, y, 0) + input(x, y, 2) + f.x;
	output(x, y, 1) = input(x, y, 1) + input(x, y, 3) + f.y + GRAVITY;
	output(x, y, 2) = input(x, y, 2) + f.x;
	output(x, y, 3) = input(x, y, 3) + f.y + GRAVITY;
	
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

	printf("%f\n", stepx);
	
	for (int y = 0; y < MESH_HEIGHT; ++y) {
		for (int x = 0; x < MESH_WIDTH; ++x) {
			oldparticles(x, y, 0) = left + stepx * x;
			oldparticles(x, y, 1) = top + stepy * y;
			oldparticles(x, y, 2) = 0.0f;
			oldparticles(x, y, 3) = 0.0f;
		}
	}
	
	// Rotate the particles by 15 degrees
	const float centerx = SCREEN_WIDTH / 2;
	const float centery = SCREEN_HEIGHT / 2;
	for (int y = 0; y < MESH_HEIGHT; ++y) {
		for (int x = 0; x < MESH_WIDTH; ++x) {
			float a = std::cos(DEGREES_TO_RADS * 15.0f);
			float b = std::sin(DEGREES_TO_RADS * 15.0f);
			// rotate position
			float oldx = oldparticles(x, y, 0) - centerx;
			float oldy = oldparticles(x, y, 1) - centery;
			float newx = a * oldx - b * oldy;
			float newy = b * oldx + a * oldy;
			oldparticles(x, y, 0) = newx + centerx;
			oldparticles(x, y, 1) = newy + centery;
			// and velocity
			oldx = oldparticles(x, y, 2);
			oldy = oldparticles(x, y, 3);
			newx = a * oldx - b * oldy;
			newy = b * oldx + a * oldy;
			oldparticles(x, y, 2) = newx;
			oldparticles(x, y, 3) = newy;
		}
	}
	
	// Main loop
	
	Func spring = SpringMesh(oldparticles);
	Func renderer = Renderer(oldparticles, previmage, width, height);
	int nframe = 0;
	float period = 70.0f;
	while (true) {
		printf("%d\n", nframe++);
		renderer.realize(image);
		if (nframe % 10 == 0) {
			DisplayImage(image);
		}
		spring.realize(newparticles);
		// Let particles bounce off the bottom
		for (int y = 0; y < MESH_WIDTH; ++y) {
			for (int x = 0; x < MESH_HEIGHT; ++x) {
				if (newparticles(x, y, 1) >= height - 1) {
					newparticles(x, y, 1) = 2 * (height - 1) - newparticles(x, y, 1);
					newparticles(x, y, 3) = -newparticles(x, y, 3);
				}
			}
		}
		std::swap(*oldbuff.raw_buffer(), *newbuff.raw_buffer());
		std::swap(*previmagebuff.raw_buffer(), *imagebuff.raw_buffer());
	}
	
}

}