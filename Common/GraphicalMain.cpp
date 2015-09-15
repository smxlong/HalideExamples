#include "Graphics.h"

namespace HalideExamples {

void RunDemo(int width, int height);

}

using namespace HalideExamples;

int main(int argc, char** argv) {
	InitializeGraphics();

	RunDemo(SCREEN_WIDTH, SCREEN_HEIGHT);

	TerminateGraphics();

}
