#include <cstdlib>
#include <cstdio>
#include <algorithm>

#include "Graphics.h"

using namespace Halide;

SDL_Window* mainWindow;
SDL_Surface* mainSurface;

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
	Expr val = cast<uint8_t>(255.0f * (image(x, y) - minvalue) * scale + 0.5f);
	rescaled(x, y) = cast<uint32_t>(val) | (cast<uint32_t>(val) << 8) | (cast<uint32_t>(val) << 16);

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

	uint32_t pixfmtraw = SDL_GetWindowPixelFormat(mainWindow);
	SDL_PixelFormat* pixfmt = SDL_AllocFormat(pixfmtraw);
	mainSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);
}

void TerminateGraphics() {
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();
}

void DisplayImage(Halide::Image<float>& image) {
	::image.set(image);

	SDL_LockSurface(mainSurface);
	uint8_t* pixels = reinterpret_cast<uint8_t*>(mainSurface->pixels);

	// Create buffer_t to wrap the surface
	buffer_t pixbuf = { 0 };
	pixbuf.host = pixels;
	pixbuf.extent[0] = SCREEN_WIDTH;
	pixbuf.extent[1] = SCREEN_HEIGHT;
	pixbuf.stride[0] = 1;
	pixbuf.stride[1] = mainSurface->pitch / 4;
	pixbuf.elem_size = 4;

	Buffer output(type_of<uint32_t>(), &pixbuf);
	imageConverter.realize(output);

	SDL_UnlockSurface(mainSurface);
	SDL_BlitSurface(mainSurface, 0, SDL_GetWindowSurface(mainWindow), 0);
	SDL_UpdateWindowSurface(mainWindow);
}

}
