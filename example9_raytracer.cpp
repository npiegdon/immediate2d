#include "drawing.h"
#include <cmath>

//
// Example 9 - Ray tracing
//
// A raytracer that was originally jammed into 99 lines of C++
//
// NOTE: You may want to do the following to get much faster rendering:
// A. Set the "Debug" drop-down at the top of Visual Studio to "Release" mode
// B. Switch to "x64" in the drop-down next to that if your machine supports it
// C. Project properties --> C/C++ --> Language --> Open MP Support --> Yes
//
// Exercises:
// 1. Tinker with the number of samples per pixel (just below this comment block)
// 2. Tinker with the scene by changing the Spheres array on line 76 (different
//    colors, reflection types, positions, etc.)
//

const int samples = 5000;

//
// smallpt, a Path Tracer by Kevin Beason, 2008
// http://www.kevinbeason.com/smallpt/
//
// A good line-by-line description of how this works can be found here:
// https://docs.google.com/open?id=0B8g97JkuSSBwUENiWTJXeGtTOHFmSm51UC01YWtCZw
//
// Adapted to the Immediate2D framework (and expanded a bit to make things
// a little easier to read) by Nicholas Piegdon, 2017
//

struct Vec
{
    // position, also color (r,g,b)
    double x, y, z;

    Vec(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) { }

    Vec operator*(double b) const { return Vec(x*b, y*b, z*b); }
    Vec operator+(const Vec &b) const { return Vec(x + b.x, y + b.y, z + b.z); }
    Vec operator-(const Vec &b) const { return Vec(x - b.x, y - b.y, z - b.z); }
    Vec operator%(const Vec &b) const { return Vec(y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x); }

    Vec mult(const Vec &b) const { return Vec(x*b.x, y*b.y, z*b.z); }
    Vec& norm() { return *this = *this * (1 / sqrt(x*x + y*y + z*z)); }
    double dot(const Vec &b) const { return x*b.x + y*b.y + z*b.z; }
};

struct Ray { Vec o, d; };

// material types, used in radiance()
enum Refl_t { DIFF, SPEC, REFR };

struct Sphere
{
    double rad;  // radius
    Vec p, e, c; // position, emission, color
    Refl_t refl; // reflection type (DIFFuse, SPECular, REFRactive)

    Sphere(double rad, Vec p, Vec e, Vec c, Refl_t refl) : rad(rad), p(p), e(e), c(c), refl(refl) { }

    // returns distance, 0 if nohit
    double intersect(const Ray &r) const
    {
        // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
        Vec op = p - r.o;
        double t, eps = 1e-4, b = op.dot(r.d), det = b*b - op.dot(op) + rad*rad;

        if (det < 0) return 0;

        det = sqrt(det);
        return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
    }
};

Sphere spheres[] =
{
    // Left, right, back, and front walls
    Sphere(1e5, Vec(1e5 + 1,40.8,81.6), Vec(),Vec(.75,.25,.25), DIFF),
    Sphere(1e5, Vec(-1e5 + 99,40.8,81.6),Vec(),Vec(.25,0.6,.15), DIFF),
    Sphere(1e5, Vec(50,40.8, 1e5), Vec(),Vec(.75,.75,.75), DIFF),
    Sphere(1e5, Vec(50,40.8,-1e5 + 170), Vec(),Vec(), DIFF),

    // Floor and ceiling
    Sphere(1e5, Vec(50, 1e5, 81.6), Vec(),Vec(.75,.75,.75), DIFF),
    Sphere(1e5, Vec(50,-1e5 + 81.6,81.6),Vec(),Vec(.75,.75,.75), DIFF),

    // Mirror and glass spheres
    Sphere(16.5,Vec(27,16.5,47), Vec(),Vec(.3,.3,1)*.9, SPEC),
    Sphere(16.5,Vec(73,16.5,78), Vec(),Vec(1,1,1)*.9, REFR),

    // Ceiling light
    Sphere(600, Vec(50,681.6 - .27,81.6),Vec(12,12,12), Vec(), DIFF)
};

inline double clamp(double x) { return x < 0 ? 0 : x>1 ? 1 : x; }
inline int gamma(double x) { return int(pow(clamp(x), 1 / 2.2) * 255 + .5); }

inline bool intersect(const Ray &r, double &t, int &id)
{
    double n = sizeof(spheres) / sizeof(Sphere), d, inf = t = 1e20;
    for (int i = int(n); i--;) if ((d = spheres[i].intersect(r)) && d < t) { t = d; id = i; }
    return t < inf;
}

Vec radiance(const Ray &r, int depth)
{
    // distance and id of intersected object
    double t;
    int id = 0;

    // if miss, return black
    if (!intersect(r, t, id)) return Vec();

    // the hit object
    const Sphere &obj = spheres[id];
    Vec x = r.o + r.d*t, n = (x - obj.p).norm(), nl = n.dot(r.d) < 0 ? n : n*-1, f = obj.c;

    // max refl
    double p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y : f.z;

    // R.R.
    if (++depth > 5)
    {
        if (RandomDouble() < p) f = f*(1 / p);
        else return obj.e;
    }

    // Ideal DIFFUSE reflection
    if (obj.refl == DIFF)
    {
        double r1 = Tau * RandomDouble();
        double r2 = RandomDouble();
        double r2s = sqrt(r2);

        Vec w = nl, u = ((fabs(w.x) > .1 ? Vec(0, 1) : Vec(1)) % w).norm(), v = w%u;
        Vec d = (u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1 - r2)).norm();
        return obj.e + f.mult(radiance(Ray{ x, d }, depth));
    }

    // Ideal SPECULAR reflection
    if (obj.refl == SPEC) return obj.e + f.mult(radiance(Ray{ x, r.d - n * 2 * n.dot(r.d) }, depth));

    // Ideal dielectric REFRACTION
    Ray reflRay{ x, r.d - n * 2 * n.dot(r.d) };

    // Ray from outside going in?
    bool into = n.dot(nl) > 0;
    double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc;
    double ddn = r.d.dot(nl);
    double cos2t = 1 - nnt*nnt*(1 - ddn*ddn);

    // Total internal reflection
    if (cos2t < 0) return obj.e + f.mult(radiance(reflRay, depth));

    Vec tdir = (r.d*nnt - n*((into ? 1 : -1)*(ddn*nnt + sqrt(cos2t)))).norm();
    double a = nt - nc, b = nt + nc, R0 = a*a / (b*b), c = 1 - (into ? -ddn : tdir.dot(n));
    double Re = R0 + (1 - R0)*c*c*c*c*c, Tr = 1 - Re, P = .25 + .5*Re, RP = Re / P, TP = Tr / (1 - P);

    // Russian roulette
    return obj.e + f.mult(depth > 2 ? (RandomDouble() < P ?
        radiance(reflRay, depth)*RP : radiance(Ray{ x, tdir }, depth)*TP) :
        radiance(reflRay, depth)*Re + radiance(Ray{ x, tdir }, depth)*Tr);
}

void run()
{
    UseDoubleBuffering(true);

    Ray cam{ Vec(50, 50, 295.6), Vec(0, -0.04, -1).norm() }; // cam pos, dir
    Vec cx = Vec(Width*.5135 / Height);
    Vec cy = (cx % cam.d).norm()*.5135;

    for (int y = 0; y < Height; y++)
    {
        // Enable OpenMP in your Visual Studio project options to run this loop in parallel
#pragma omp parallel for schedule(dynamic, 1)

        for (int x = 0; x < Width; x++)
        {
            Vec c;

            // 2x2 subpixel rows
            for (int sy = 0, i = y*Width + x; sy < 2; sy++)
            {
                // 2x2 subpixel cols
                for (int sx = 0; sx < 2; sx++)
                {
                    Vec r{};
                    for (int s = 0; s < samples; s++)
                    {
                        const double r1 = 2 * RandomDouble();
                        const double dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);

                        const double r2 = 2 * RandomDouble();
                        const double dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);

                        Vec d = cx*(((sx + .5 + dx) / 2 + x) / Width - .5) + cy*(((sy + .5 + dy) / 2 + y) / Height - .5) + cam.d;
                        r = r + radiance(Ray{ cam.o + d * 140, d.norm() }, 0)*(1. / samples);
                    }
                    c = c + Vec(clamp(r.x), clamp(r.y), clamp(r.z))*.25;
                }
            }
            DrawPixel(x, Height - y - 1, MakeColor(gamma(c.x), gamma(c.y), gamma(c.z)));
        }
        Present();
    }
    SaveImage();
}



















//
// Copyright (c) 2006-2008 Kevin Beason (kevin.beason@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

