#include "tgaimage.h"
#include "geometry.h"

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

void viewport(int x, int y, int w, int h);
void projection(float coeff=0.f);
void lookAt(Vec3f eye, Vec3f center, Vec3f up);

struct Shader
{
public :
    virtual ~Shader();
    virtual Vec4f vertex(int iface, int nthvert) =0;
    virtual bool fragment(Vec3f bar, TGAColor& color) =0;
};

void triangle(Vec4f* pts, Shader& shader, TGAImage& image, TGAImage& zbuffer);