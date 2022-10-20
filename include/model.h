#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<Vec2f> tex_coords_;
	std::vector<int> facet_vrt{};
    std::vector<int> facet_tex{};  // per-triangle indices in the above arrays
    std::vector<int> facet_nrm{};
	TGAImage diffuse_map;
public:
	Model(const char *model_file, const char *diffuse_file);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	std::vector<int> face(const int idx);
	void load_texture(std::string filename, const std::string suffix, TGAImage &img);
	Vec2f uv(const int face_i, const int vert_n);
	const TGAImage& diffuse() const { return diffuse_map; }
};

#endif //__MODEL_H__
