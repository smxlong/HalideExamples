#ifndef HalideExamples_Block_h
#define HalideExamples_Block_h

namespace HalideExamples {

template <typename T, unsigned int WIDTH, unsigned int HEIGHT=1, unsigned int DEPTH=1, unsigned int DEPTH2=1>
class Block {
public:
	typedef T type;

	unsigned int width() const {
		return WIDTH;
	}

	unsigned int height() const {
		return HEIGHT;
	}

	unsigned int depth() const {
		return DEPTH;
	}

	unsigned int depth2() const {
		return DEPTH2;
	}

	T& operator()(unsigned int x, unsigned int y = 0, unsigned int z = 0, unsigned int w = 0) {
		return data[w * WIDTH * HEIGHT * DEPTH + z * WIDTH * HEIGHT + y * WIDTH + x];
	}

	const T& operator()(unsigned int x, unsigned int y = 0, unsigned int z = 0, unsigned int w = 0) const {
		return data[w * WIDTH * HEIGHT * DEPTH + z * WIDTH * HEIGHT + y * WIDTH + x];
	}

	T data[WIDTH * HEIGHT * DEPTH * DEPTH2];
};

}

#endif // HalideExamples_Block_h
