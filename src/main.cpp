#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

#define     WIDTH   1000
#define     HEIGHT  1000

void line(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color)
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
        if(steep) image.set(y, x, color);
        else image.set(x, y, color);

        err2 += derr2;
        if(err2 > dx)
        {
            y += (y1 > y0 ? 1 : -1);
            err2 -= dx*2;
        }
    }
}

Vec3f barycentric(Vec2i& p0,Vec2i& p1,Vec2i& p2,Vec2i& P)
{
    Vec3f u = Vec3f(p2.x-p0.x,
                    p1.x-p0.x,
                    p0.x-P.x) ^
              Vec3f(p2.y-p0.y,
                    p1.y-p0.y,
                    p0.y-P.y);
    if(std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
    return Vec3f(1.0f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}

void triangle(Vec2i* pts, TGAImage& image, TGAColor color)
{
    // Sort vertices of the triangle by their y-coordinates;
    // Rasterize simultaneously the left and the right sides of the triangle;
    // Draw a horizontal line segment between the left and the right boundary points.
    // if(p0.y > p1.y) std::swap(p0, p1);
    // if(p0.y > p2.y) std::swap(p0, p2);
    // if(p1.y > p2.y) std::swap(p1, p2);

    // int total_height = p2.y-p0.y;
    // for(int i = 0; i < total_height; i++)
    // {
    //     bool second_half = i>(p1.y-p0.y) || p1.y==p0.y;
    //     int segment_height = second_half ? (p2.y-p1.y) : (p1.y-p0.y);

    //     float alpha = (float)i/total_height;
    //     float beta = (float)(i-(second_half ? (p1.y-p0.y) : 0))/segment_height;

    //     Vec2i A = p0 + (p2-p0)*alpha;
    //     Vec2i B = second_half ? p1 + (p2-p1)*beta : p0 + (p1-p0)*beta;
    //     if(A.x>B.x) std::swap(A, B);

    //     for(int j = A.x; j <= B.x; j++)
    //     {
    //         image.set(j, p0.y+i, color);
    //     }
    // }

    Vec2i bboxmin(image.get_width()-1, image.get_height()-1);
    Vec2i bboxmax(0, 0);
    Vec2i clamp(image.get_width()-1, image.get_height()-1);
    for(int i = 0; i < 3; i++)
    {
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));
    
        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }
    Vec2i P;
    for(P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for(P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if(bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            image.set(P.x, P.y, color);
        }
    }
}

int main(int argc, char** argv) 
{
    Model* model;
    
    if(2 == argc) model = new Model(argv[1]);
    else model = new Model("assets/obj/test.obj");

    const TGAColor white    = TGAColor(255, 255, 255, 255);
    const TGAColor red      = TGAColor(255, 0, 0, 255);
    const TGAColor green    = TGAColor(0, 255, 0, 255);
    const TGAColor blue     = TGAColor(0, 0, 255, 255);

    TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);
    Vec3f light_dir(0,0,-1);
    for(int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for(int j = 0; j < 3; j++)
        {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = Vec2i((v.x+1.0f)*WIDTH/2.0f,(v.y+1.0f)*HEIGHT/2.0f);
            world_coords[j] = v;
        }
        Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
        n.normalize();
        float intensity = n * light_dir;
        if(intensity > 0)
        {
            triangle(screen_coords, image, TGAColor(intensity*255,intensity*255,intensity*255,255));
        }
    }
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("renders/lesson_02.tga");
    delete model;
    return 0;
}