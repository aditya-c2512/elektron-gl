#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

Model *model = NULL;

#define width 800
#define height 800
#define depth 255
Vec3f m2v(Matrix41 m) 
{
    return Vec3f((m[0][0])/m[3][0], (m[1][0])/m[3][0], (m[2][0])/m[3][0]);
}

Matrix41 v2m(Vec3f v) 
{
    Matrix41 m;
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity();
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}


void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) 
{
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) 
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) 
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x=x0; x<=x1; x++) 
    {
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.-t) + y1*t;
        if (steep) 
        {
            image.set(y, x, color);
        } else 
        {
            image.set(x, y, color);
        }
    }
}
Vec3f barycentric(Vec2f& p0,Vec2f& p1,Vec2f& p2,Vec3f& P)
{
    Vec3f u = cross(Vec3f(p2.x-p0.x,
                        p1.x-p0.x,
                        p0.x-P.x),
                    Vec3f(p2.y-p0.y,
                        p1.y-p0.y,
                        p0.y-P.y)
                    );
    if(std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
    return Vec3f(1.0f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) 
{
    Vec3f s[2];
    for (int i=2; i--; ) 
    {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAImage& diffuse, Vec2f* uvs, float intensity) 
{
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) 
    {
        for (int j=0; j<2; j++) 
        {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) 
    {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) 
        {
            Vec3f bc_screen  = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            P.z = 0;
            Vec2f uv;
            for(int i=0; i<3; i++) 
            {
                P.z += pts[i][2]*bc_screen[i];
                uv.x += uvs[i].x*bc_screen[i];
                uv.y += uvs[i].y*bc_screen[i];
            }
            TGAColor color = diffuse.get(uv.x, uv.y);
            color = TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, 255);
            if (zbuffer[int(P.x+P.y*width)]<P.z) 
            {
                zbuffer[int(P.x+P.y*width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

Vec3f world2screen(Vec3f v) 
{
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}

int main(int argc, char** argv) 
{
    if (3==argc) 
    {
        model = new Model(argv[1],argv[2]);
    } else 
    {
        model = new Model("assets/obj/test.obj","assets/textures/test/diffuse.tga");
    }

    float *zbuffer = new float[width*height];
    for (int i=width*height; i--; zbuffer[i] = -depth);

    Vec3f camera(0,0,3);

    Matrix Projection = Matrix::identity();
    Matrix ViewPort = viewport(width/8, height/8, width*3/4, height*3/4);
    // Matrix ViewPort = viewport(0, 0, width, height);
    Projection[3][2] = -1.f/camera.z;
    
    TGAImage image(width, height, TGAImage::RGB);
    Vec3f light_dir(0,0,-1);
    TGAImage diffuse = model->diffuse();
    for (int i=0; i<model->nfaces(); i++) 
    {
        std::vector<int> face = model->face(i);
        Vec3f world_coords[3];
        Vec3f pts[3];
        for (int j=0; j<3; j++) 
        {
            Vec3f v = model->vert(face[j]);
            pts[j] = world2screen(m2v(Projection * v2m(v)));
            world_coords[j]  = v;
        }
        Vec3f n = cross((world_coords[2]-world_coords[0]),(world_coords[1]-world_coords[0]));
        n.normalize(); 
        float intensity = n*light_dir;
        if(intensity > 0)
        {
            Vec2f uvs[3];
            for (int k=0; k<3; k++) {
                uvs[k] = model->uv(i, k) * diffuse.get_width();
            }
            // std::cout << pts[0] << "\t||\t" << pts[1] << "\t||\t" << pts[2] << std::endl;
            triangle(pts, zbuffer, image, diffuse, uvs, intensity);
        }
    }
    
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("renders/lesson_04.tga");
    delete model;
    return 0;
}

