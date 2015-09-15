#ifndef HalideExamples_Graphics_h
#define HalideExamples_Graphics_h

#include <SDL.h>

#include <Halide.h>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
extern SDL_Window* mainWindow;
extern SDL_Surface* mainSurface;

namespace HalideExamples {

	void InitializeGraphics();
	void TerminateGraphics();
	void DisplayImage(Halide::Image<float>& image);

}

#endif // HalideExamples_Graphics_h
