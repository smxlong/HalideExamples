#include <cmath>
#include <cstdio>

#include <Halide.h>
#include <Graphics.h>
#include <Vec.h>

using namespace Halide;

namespace HalideExamples {

const int NUM_PARTICLES = 100000;
const float TIMESCALE = 0.001f;
const float GRAVITY = 1.0f * TIMESCALE;

/////////////////// PARTICLE FOUNTAIN FUNCTION ////////////////////

template <typename F1>
Func ParticleFountain(F1 particles, Param<float> gravity) {
	////////////////////////// ALGORITHM //////////////////////////

	Func output;
	Var x;
	
	// adjust Y velocity
	Expr vely = particles(x, 3) + gravity;
	// move the particle
	output(x) = Tuple(particles(x, 0) + particles(x, 2),
					  particles(x, 1) + vely,
					  vely);

	////////////////////////// SCHEDULE //////////////////////////
	output.vectorize(x, 32);
	
	return output;
}

void InitPlane(buffer_t& buff, Buffer& particleBuff, int plane) {
	buffer_t* rawParticleBuff = particleBuff.raw_buffer();
	buff.host = rawParticleBuff->host + rawParticleBuff->stride[1] * plane;
	buff.extent[0] = rawParticleBuff->extent[0];
	buff.extent[1] = 0;
	buff.stride[0] = rawParticleBuff->stride[0];
	buff.stride[1] = rawParticleBuff->stride[1];
	buff.elem_size = rawParticleBuff->elem_size;
}

float Random(float min, float max) {
	int ix = std::rand() & 0xFFFFF;
	float x = static_cast<float>(ix) / 0xFFFFF;
	return x * (max - min) + min;
}

void CreateParticle(Image<float>& particles, Image<float>& newParticles, int i) {
	// The particles get launched with random directions and speeds from the bottom of the image
	float vel = TIMESCALE * (Random(10.0f, 100.0f) + Random(10.0f, 100.0f) + Random(10.0f, 100.0f));
	float angle = Random(M_PI / 4.0f, 3.0f * M_PI / 4.0f);
	newParticles(i, 0) = particles(i, 0) = SCREEN_WIDTH / 2;
	newParticles(i, 1) = particles(i, 1) = SCREEN_HEIGHT - 1;
	newParticles(i, 2) = particles(i, 2) = vel * std::cos(angle);
	newParticles(i, 3) = particles(i, 3) = -vel * std::sin(angle); // negative direction to go upward
}

////////////////////////// MAIN DEMO FUNCTION //////////////////////////

void RunDemo(int width, int height) {
	// Main buffer which holds the 
	Buffer buff1(type_of<float>(), NUM_PARTICLES, 4);
	Image<float> particles(buff1);
	Buffer buff2(type_of<float>(), NUM_PARTICLES, 4);
	Image<float> newparticles(buff2);
	
	// Initialize the particles
	for (int i = 0; i < NUM_PARTICLES; ++i) {
		CreateParticle(particles, newparticles, i);
	}
	
	Param<float> deltaz;
	deltaz.set(GRAVITY);
	Func par = ParticleFountain(particles, deltaz);

	for (int i = 0; i < 100; ++i) {
		printf("%f,%f,%f,%f\n", particles(0, 0), particles(0, 1), particles(0, 2), particles(0, 3));
		// Make buffers for the X, Y, velY planes
		buffer_t xbuff = { 0 };
		InitPlane(xbuff, buff2, 0);
		buffer_t ybuff = { 0 };
		InitPlane(ybuff, buff2, 1);
		buffer_t velybuff = { 0 };
		InitPlane(velybuff, buff2, 3);
		
		Buffer x(type_of<float>(), &xbuff);
		Buffer y(type_of<float>(), &ybuff);
		Buffer vely(type_of<float>(), &velybuff);
		
		par.realize({x, y, vely});
		std::swap(*buff1.raw_buffer(), *buff2.raw_buffer());
	}
}

}
