#include <cmath>
#include <iostream>
#include <limits>
#include <cstdlib>
#include "elektron_gl.h"

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

Shader::~Shader() {}

void viewport(int x, int y, int w, int h) 
{
    Viewport  = Matrix::identity(4);
    Viewport[0][3] = x+w/2.f;
    Viewport[1][3] = y+h/2.f;
    Viewport[2][3] = 255.f/2.f;
    Viewport[0][0] = w/2.f;
    Viewport[1][1] = h/2.f;
    Viewport[2][2] = 255.f/2.f;
}
void projection(float coeff)
{
    Projection = Matrix::identity(4);
    Projection[3][2] = coeff;
}
void lookAt(Vec3f eye, Vec3f center, Vec3f up) 
{
    Vec3f z = (eye-center).normalize();
    Vec3f x = (up^z).normalize();
    Vec3f y = (z^x).normalize();
    ModelView = Matrix::identity(4);
    for (int i=0; i<3; i++) {
        ModelView[0][i] = x[i];
        ModelView[1][i] = y[i];
        ModelView[2][i] = z[i];
        ModelView[i][3] = -center[i];
    }
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
    Vec3f u = (s[0]^s[1]);
    if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec4f *pts, Shader &shader, TGAImage &image, TGAImage &zbuffer) 
{
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) 
    {
        for (int j=0; j<2; j++) 
        {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]/pts[i][3]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]/pts[i][3]));
        }
    }
    Vec3f P;
    TGAColor color;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) 
    {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) 
        {
            Vec3f bc_screen = barycentric(Vec3f(pts[0][0]/pts[0][3],pts[0][1]/pts[0][3],pts[0][2]/pts[0][3]),
                                Vec3f(pts[1][0]/pts[1][3],pts[1][1]/pts[1][3],pts[1][2]/pts[1][3]),
                                Vec3f(pts[2][0]/pts[2][3],pts[2][1]/pts[2][3],pts[2][2]/pts[2][3]), 
                                P);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;

            float z = pts[0][2]*bc_screen.x + pts[1][2]*bc_screen.y + pts[2][2]*bc_screen.z;
            float w = pts[0][3]*bc_screen.x + pts[1][3]*bc_screen.y + pts[2][3]*bc_screen.z;
            int frag_depth = std::max(0, std::min(255, int(z/w+.5)));
            if (zbuffer.get(P.x, P.y).r>frag_depth) continue;
            bool discard = shader.fragment(bc_screen, color);
            if (!discard) 
            {
                zbuffer.set(P.x, P.y, TGAColor(frag_depth,0,0,255));
                image.set(P.x, P.y, color);
            }
        }
    }
}
