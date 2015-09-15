#include "ImageConverter.h"

using namespace Halide;

namespace HalideExamples {

Halide::Func ImageConverter(Halide::ImageParam image) {
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
	Expr scaled = val * cast<uint32_t>(0x010101);
	rescaled(x, y) = scaled;

	imgmin.compute_root();
	imgmax.compute_root();

	Var xo, yo, xi, yi;

	//rescaled.tile(x, y, xo, yo, xi, yi, 32, 8);
	//rescaled.vectorize(xi);
	//rescaled.unroll(yi);

	return rescaled;
}

}
