#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_() {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) { // v -0.511812 -0.845392 0.127809
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++) iss >> v[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 2, "f ")) { // f 28/5/28 30/7/30 27/8/27 format: (vertex/uv coord index/...)
            std::vector<int> f;
            std::vector<int> face_uv_idx;
            int itrash, idx, uv_idx;
            iss >> trash;
            while (iss >> idx >> trash >> uv_idx >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                uv_idx--;
                f.push_back(idx);
                face_uv_idx.push_back(uv_idx);
            }
            faces_.push_back(f);
            face_uv_indices_.push_back(face_uv_idx);
        }
        else if (!line.compare(0, 4, "vt  ")) { // vt  0.532 0.923 0.000 format: (u/v/...)
            Vec2f uv;
            iss >> trash >> trash;

            for (int i = 0; i < 2; i++) iss >> uv[i]; 

            iss >> trash; // trash the trailing zero
            uvs_.push_back(uv);
        }
        else if (!line.compare(0, 4, "vn  ")) {  // vn  0.001 0.482 -0.876
            Vec3f normal;
            iss >> trash >> trash;

            for (int i = 0; i < 3; i++) iss >> normal[i];
            normals_.push_back(normal);
        }  
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

std::vector<int> Model::face_uv_indices(int idx) {
    return face_uv_indices_[idx];
}

Vec2f Model::uv(int i) {
    return uvs_[i];
}

Vec3f Model::normal(int i) {
    return normals_[i];
}