#ifndef HalideExamples_Graphics_h
#define HalideExamples_Graphics_h

#include <SDL.h>

#include <Halide.h>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
extern SDL_Window* mainWindow;
extern SDL_Renderer* mainRenderer;
extern SDL_Texture* mainTexture;

namespace HalideExamples {

	void InitializeGraphics();
	void TerminateGraphics();
	void DisplayImage(Halide::Image<float>& image);
	void DisplayImage(Halide::Image<float>& image, float min, float max);
	Halide::Func InitializeDiffuseShader(Halide::Image<float>& input, Halide::Param<float> &lx, Halide::Param<float>& ly, Halide::Param<float>& lz);
	Halide::Func InitializeSpecularShader(Halide::Image<float>& input, Halide::Param<float> &lx, Halide::Param<float>& ly, Halide::Param<float>& lz, Halide::Param<float> &ex, Halide::Param<float>& ey, Halide::Param<float>& ez);
}

#endif // HalideExamples_Graphics_h
