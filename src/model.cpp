#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *model_file, const char *diffuse_file) : verts_(), faces_() {
    std::ifstream in;
    in.open (model_file, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } 
        else if(!line.compare(0, 3, "vt "))
        {
            iss >> trash >> trash;
            Vec2f uv;
            for(int i = 0; i < 2; i++) iss >> uv[i];
            tex_coords_.push_back({uv.x, 1.0f-uv.y});
        }
        else if (!line.compare(0, 2, "f ")) {
            std::vector<int> fs;
            int f,t,n;
            iss >> trash;
            int cnt = 0;
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt.push_back(--f);
                fs.push_back(f);
                facet_tex.push_back(--t);
                facet_nrm.push_back(--n);
                cnt++;
            }
            faces_.push_back(fs);
            if (3!=cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                in.close();
                return;
            }
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;

    load_texture(model_file, diffuse_file, diffuse_map);
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return facet_vrt.size()/3;
}

std::vector<int> Model::face(const int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

void Model::load_texture(std::string filename, const std::string suffix, TGAImage &img) {
    size_t dot = filename.find_last_of(".");
    if (dot==std::string::npos) return;
    std::string texfile = suffix;
    std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
}

Vec2f Model::uv(const int iface, const int nthvert)
{
    return tex_coords_[facet_tex[iface*3+nthvert]];
}