#include <vector>
#include <cmath>
#include <cstring>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

#define     WIDTH   800
#define     HEIGHT  800

void line(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor* color)
{
    /* ATTEMPT 1 */
    // for(float t = 0; t < 1; t += 0.01)
    // {
    //     int x = x0 + (x1-x0)*t;
    //     int y = y0 + (y1-y0)*t;
    //     image.set(x, y, color);
    // }

    /* ATTEMPT 2 */
    // for(int x = x0; x <= x1; x++)
    // {
    //     float t = (x-x0)/(float)(x1-x0);
    //     int y = y0*(1.0f-t) + y1*t;
    //     image.set(x, y, color);
    // }

    /* ATTEMPT 3 */
    // bool steep = false;
    // if(std::abs(x0-x1) < std::abs(y0-y1))
    // {
    //     std::swap(x0, y0);
    //     std::swap(x1, y1);
    //     steep = true;
    // }
    // if(x0 > x1)
    // {
    //     std::swap(x0, x1);
    //     std::swap(y0, y1);
    // }
    // for(int x = x0; x <= x1; x++)
    // {
    //     float t = (x-x0)/(float)(x1-x0);
    //     int y = y0*(1.0f-t) + y1*t;
    //     if(steep) image.set(y, x, color);
    //     else image.set(x, y, color);
    // }

    /* ATTEMPT 4 */
    // bool steep = false;
    // if(std::abs(x0-x1) < std::abs(y0-y1))
    // {
    //     std::swap(x0, y0);
    //     std::swap(x1, y1);
    //     steep = true;
    // }
    // if(x0 > x1)
    // {
    //     std::swap(x0, x1);
    //     std::swap(y0, y1);
    // }
    // int dx = x1-x0;
    // int dy = y1-y0;
    // float derr = std::abs(dy/(float)(dx));
    // float err = 0;
    // int y = y0;
    // for(int x = x0; x <= x1; x++)
    // {
    //     if(steep) image.set(y, x, color);
    //     else image.set(x, y, color);

    //     err += derr;
    //     if(err > 0.5f)
    //     {
    //         y += (y1 > y0 ? 1 : -1);
    //         err -= 1.0f;
    //     }
    // }

    /*ATTEMPT 5 */
    bool steep = false;
    if(std::abs(x0-x1) < std::abs(y0-y1))
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if(x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1-x0;
    int dy = y1-y0;
    int derr2 = std::abs(dy)*2;
    int err2 = 0;
    int y = y0;
    for(int x = x0; x <= x1; x++)
    {
        if(steep) image.set(y, x, *color);
        else image.set(x, y, *color);

        err2 += derr2;
        if(err2 > dx)
        {
            y += (y1 > y0 ? 1 : -1);
            err2 -= dx*2;
        }
    }
}

int main(int argc, char** argv) 
{
    Model* model;
    
    if(2 == argc) model = new Model(argv[1]);
    else model = new Model("assets/obj/test.obj");

    const TGAColor white = TGAColor(255, 255, 255, 255);
    const TGAColor red   = TGAColor(255, 0,   0,   255);

    TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);
    
    for(int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        for(int j = 0; j < 3; j++)
        {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j+1)%3]);

            int x0 = (v0.x+1.0f)*WIDTH/2.0f;
            int y0 = (v0.y+1.0f)*HEIGHT/2.0f;
            int x1 = (v1.x+1.0f)*WIDTH/2.0f;
            int y1 = (v1.y+1.0f)*HEIGHT/2.0f;

            line(x0, y0, x1, y1, image, &white);
        }
    }
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("renders/output.tga");
    delete model;
    return 0;
}