#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "elektron_gl.h"

#define width 1080
#define height 1080

Model *model = NULL;

Vec3f light_dir(1,1,1);
Vec3f       eye(1,1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);

struct GouraudShader : public Shader {
    Vec3f varying_intensity;

    virtual Vec4f vertex(int iface, int nthvert) 
    {
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir);
        Vec3f gl_Vertex = model->vert(iface, nthvert);
        return Vec4f(Viewport*Projection*ModelView*Matrix(gl_Vertex));
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) 
    {
        float intensity = varying_intensity*bar;
        color = TGAColor(255*intensity, 255*intensity, 255*intensity, 255); 
        return false;
    }
};

int main(int argc, char** argv) 
{
    if (5==argc) 
    {
        model = new Model(argv[1],argv[2],argv[3],argv[4]);
    } else 
    {
        model = new Model("assets/obj/test.obj",
         "assets/textures/test/diffuse.tga",
         "assets/textures/test/normal.tga",
         "assets/textures/test/specular.tga");
    }

    lookAt(eye, center, up);
    viewport(width/8, height/8, width*3/4, height*3/4);
    projection(-1.f/(eye-center).norm());

    light_dir.normalize();

    TGAImage image  (width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::RGB);

    GouraudShader gouraud;
    for (int i=0; i<model->nfaces(); i++) 
    {
        Vec4f screen_coords[3];
        for (int j=0; j<3; j++) 
        {
            screen_coords[j] = gouraud.vertex(i, j);
        }
        triangle(screen_coords, gouraud, image, zbuffer);
    }

    image.flip_vertically();
    zbuffer.flip_vertically();
    image.write_tga_file("renders/render.tga");
    zbuffer.write_tga_file("renders/render_zbuff.tga");

    delete model;
    return 0;
}

