#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int> > face_uv_indices_;
	std::vector<Vec2f> uvs_;
	std::vector<Vec3f> normals_;
public:
	Model(const char* filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	std::vector<int> face(int idx);
	std::vector<int> face_uv_indices(int idx);
	Vec2f uv(int i);
	Vec3f normal(int i);
};

#endif //__MODEL_H__