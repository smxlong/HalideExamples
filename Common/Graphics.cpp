#include <cstdlib>
#include <cstdio>
#include <algorithm>

#include "Graphics.h"
#include "Vec.h"

using namespace Halide;

SDL_Window* mainWindow;
SDL_Renderer* mainRenderer;
SDL_Texture* mainTexture;

ImageParam image(type_of<float>(), 2);
Param<float> minvalue;
Param<float> maxvalue;
Func imageConverter;
Func imageConverterMinMaxProvided;

namespace HalideExamples {

void GetImageMinMax(Halide::Image<float>& image, float& min, float& max) {
	int w = image.width();
	int h = image.height();
	min = image(0, 0);
	max = min;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			float val = image(x, y);
			if (val < min) min = val;
			else if (val > max) max = val;
		}
	}
}

Func InitializeDiffuseShader(Image<float>& input, Param<float> &lx, Param<float>& ly, Param<float>& lz) {
	Func shade;
	Var x, y;

	// Compute the surface normal as the cross product of the tangent vectors along X and Y
	Vec tangentX(1, 0, (input(x + 1, y) - input(x - 1, y)) / 2);
	Vec tangentY(0, 1, (input(x, y + 1) - input(x, y - 1)) / 2);
	Vec normal = cross(tangentX, tangentY).normalized();

	// Compute the vector to the light source
	Vec l = (Vec(lx, ly, lz) - Vec(x, y, input(x, y))).normalized();

	// Compute the diffuse illumination as the dot product of light vector and normal vector
	// (proportional to cos(a) between the two)
	Expr diffuse = dot(l, normal);

	shade(x, y) = diffuse;

	// Now schedule it.

	// Split the space into 256x256 blocks for parallelization
	Var xi, yi, xo, yo;
	Var tx, ty, nx, ny, ti;
	shade.tile(x, y, tx, ty, nx, ny, 256, 256);

	// Split the blocks into smaller 32x16 tiles, vectorize and unroll
	shade.tile(nx, ny, xo, yo, xi, yi, 32, 16)
		.vectorize(xi)
		.unroll(yi);

	// Run all blocks in parallel
	shade.fuse(tx, ty, ti);
	shade.parallel(ti);

	return shade;
}

Func InitializeSpecularShader(Image<float>& input, Param<float> &lx, Param<float>& ly, Param<float>& lz, Param<float> &ex, Param<float>& ey, Param<float>& ez) {
	Func shade;
	Var x, y;

	// Compute the surface normal as the cross product of the tangent vectors along X and Y
	Vec tangentX(1, 0, (input(x + 1, y) - input(x - 1, y)) / 2);
	Vec tangentY(0, 1, (input(x, y + 1) - input(x, y - 1)) / 2);
	Vec normal = cross(tangentX, tangentY).normalized();

	// Compute the vector to the light source
	Vec l = (Vec(lx, ly, lz) - Vec(x, y, input(x, y))).normalized();

	// Compute the diffuse illumination as the dot product of light vector and normal vector
	// (proportional to cos(a) between the two)
	Expr diffuse = dot(l, normal);

	// Now calculate specular reflection

	// Reflect an eye ray about the normal
	Vec eye = Vec(x - ex, y - ey, input(x, y) - ez).normalized();
	Vec reflect = eye - 2 * dot(eye, normal) * normal;

	// If the angle is "very close", i.e. the reflected ray intersects the spherical light source,
	// add a highlight
	Expr specular = select(dot(l, reflect) > 0.98f, 0.5f, 0);

	// The result is the sum of diffuse and specular, normalized to 0..1 range
	shade(x, y) = (diffuse + specular) / 1.5f;

	// Now schedule it.

	// Split the space into 256x256 blocks for parallelization
	Var xi, yi, xo, yo;
	Var tx, ty, nx, ny, ti;
	shade.tile(x, y, tx, ty, nx, ny, 256, 256);

	// Split the blocks into smaller 32x16 tiles, vectorize and unroll
	shade.tile(nx, ny, xo, yo, xi, yi, 32, 16)
		.vectorize(xi)
		.unroll(yi);

	// Run all blocks in parallel
	shade.fuse(tx, ty, ti);
	shade.parallel(ti);

	return shade;
}

Func InitializeImageConverter() {
	// First get min and max of the image
	RDom r(0, image.width(), 0, image.height());

	// Now rescale the image to the range 0..255 and project the value to a RGBA integer value
	Func imgmin;
	imgmin() = minimum(image(r.x, r.y));
	Func imgmax;
	imgmax() = maximum(image(r.x, r.y));
	Expr scale = 1.0f / (imgmax() - imgmin());
	Func rescaled;
	Var x, y;
	Expr val = cast<uint32_t>(255.0f * (image(x, y) - imgmin()) * scale + 0.5f);
	Expr scaled = val * cast<uint32_t>(0x00010101);
	rescaled(x, y) = scaled;

	imgmin.compute_root();
	imgmax.compute_root();

	Var xo, yo, xi, yi;

	rescaled.tile(x, y, xo, yo, xi, yi, 32, 8);
	rescaled.vectorize(xi);
	rescaled.unroll(yi);

	return rescaled;
}

Func InitializeImageConverterMinMaxProvided() {
	// Now rescale the image to the range 0..255 and project the value to a RGBA integer value
	Expr scale = 1.0f / (maxvalue - minvalue);
	Func rescaled;
	Var x, y;
	Expr val = cast<uint32_t>(255.0f * (image(x, y) - minvalue) * scale + 0.5f);
	Expr scaled = val * cast<uint32_t>(0x00010101);
	rescaled(x, y) = scaled;

	Var xo, yo, xi, yi;
	rescaled.tile(x, y, xo, yo, xi, yi, 32, 8);
	rescaled.vectorize(xi);
	rescaled.unroll(yi);

	return rescaled;
}

void InitializeGraphics() {
	imageConverter = InitializeImageConverter();
	imageConverterMinMaxProvided = InitializeImageConverterMinMaxProvided();

	int ec = SDL_Init(SDL_INIT_VIDEO);
	if (ec < 0) {
		std::printf("ERROR: could not initialize SDL (code %d)\n", ec);
		std::exit(1);
	}

	mainWindow = SDL_CreateWindow("HalideExamples - Wave", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);
	if (!mainRenderer) {
		SDL_DestroyWindow(mainWindow);
		SDL_Quit();
		printf("Could not create renderer");
		std::exit(1);
	}
	mainTexture = SDL_CreateTexture(mainRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (!mainTexture) {
		SDL_DestroyRenderer(mainRenderer);
		SDL_DestroyWindow(mainWindow);
		SDL_Quit();
		printf("Could not create texture");
		std::exit(1);
	}
}

void TerminateGraphics() {
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();
}

void DisplayImage(Halide::Image<float>& image) {
	::image.set(image);

	void* vpixels;
	int pitch;
	SDL_LockTexture(mainTexture, 0, &vpixels, &pitch);
	uint8_t* pixels = reinterpret_cast<uint8_t*>(vpixels);

	// Create buffer_t to wrap the surface
	buffer_t pixbuf = { 0 };
	pixbuf.host = pixels;
	pixbuf.extent[0] = SCREEN_WIDTH;
	pixbuf.extent[1] = SCREEN_HEIGHT;
	pixbuf.stride[0] = 1;
	pixbuf.stride[1] = pitch / 4;
	pixbuf.elem_size = 4;

	Buffer output(type_of<uint32_t>(), &pixbuf);
	imageConverter.realize(output);

	SDL_UnlockTexture(mainTexture);
	SDL_RenderCopy(mainRenderer, mainTexture, 0, 0);
	SDL_RenderPresent(mainRenderer);
}

void DisplayImage(Halide::Image<float>& image, float min, float max) {
	::image.set(image);

	void* vpixels;
	int pitch;
	SDL_LockTexture(mainTexture, 0, &vpixels, &pitch);
	uint8_t* pixels = reinterpret_cast<uint8_t*>(vpixels);

	// Create buffer_t to wrap the surface
	buffer_t pixbuf = { 0 };
	pixbuf.host = pixels;
	pixbuf.extent[0] = SCREEN_WIDTH;
	pixbuf.extent[1] = SCREEN_HEIGHT;
	pixbuf.stride[0] = 1;
	pixbuf.stride[1] = pitch / 4;
	pixbuf.elem_size = 4;

	Buffer output(type_of<uint32_t>(), &pixbuf);
	minvalue.set(min);
	maxvalue.set(max);
	imageConverterMinMaxProvided.realize(output);

	SDL_UnlockTexture(mainTexture);
	SDL_RenderCopy(mainRenderer, mainTexture, 0, 0);
	SDL_RenderPresent(mainRenderer);
}


}
