#ifndef __BMP_IMAGE_H__
#define __BMP_IMAGE_H__

struct BMPColor {
	union {
		struct {
			unsigned char b, g, r, a;
		};
		unsigned char raw[4];
		unsigned int val;
	};
	int bytespp;

	BMPColor() : val(0), bytespp(1) {
	}

	BMPColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : b(B), g(G), r(R), a(A), bytespp(4) {
	}

	BMPColor(int v, int bpp) : val(v), bytespp(bpp) {
	}

	BMPColor(const BMPColor& c) : val(c.val), bytespp(c.bytespp) {
	}

	BMPColor(const unsigned char* p, int bpp) : val(0), bytespp(bpp) {
		for (int i = 0; i < bpp; i++) {
			raw[i] = p[i];
		}
	}

	BMPColor& operator =(const BMPColor& c) {
		if (this != &c) {
			bytespp = c.bytespp;
			val = c.val;
		}
		return *this;
	}
};

class BMPImage {
protected:
	unsigned char* data;
	int width;
	int height;
	int bytesPerPixel;

public:
	BMPImage(int width, int height, int bytesPerPixel);

	void save(char* fileName);
	void set(int x, int y, BMPColor color);
	int getWidth();
	int getHeight();
};

#endif //__BMP_IMAGE_H__