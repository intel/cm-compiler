#include <cm/cm.h>
#include <cm/cmtl.h>

// each threads handle N pixels in a row
#define N (16)

_GENX_MAIN_ void
mandelbrot(SurfaceIndex output_index, int crunch,
           float xOff, float yOff, float scale)
{
    int ix_start = get_thread_origin_x() * N;
    int iy_start = get_thread_origin_y();

    cm_vector(vn, int, N, 0, 1);
    vector<float, N> ix = ix_start + vn;
    vector<float, N> iy = iy_start;
    vector<int, N>   m = crunch - 1;

    vector<float, N> xPos = ix * scale + xOff;
    vector<float, N> yPos = iy * scale + yOff;
    vector<float, N> x = xPos;
    vector<float, N> y = yPos;
    vector<float, N> xx = x * x;
    vector<float, N> yy = y * y;

    SIMD_IF_BEGIN ((m != 0) & (xx + yy < 4.0f)) {
        SIMD_DO_WHILE_BEGIN {
            y  = x * y * 2.0f + yPos;
            x  = xx - yy + xPos;
            yy = y * y;
            xx = x * x;
            m -= 1;
        } SIMD_DO_WHILE_END ((m != 0) & (xx + yy < 4.0f));
    } SIMD_IF_END;

    m.merge(crunch - m, 0, m > 0);
    vector<int, N> color =
        ((((m << 4) - m) & 0xff)      ) +
        ((((m << 3) - m) & 0xff) << 8 ) +
        ((((m << 2) - m) & 0xff) << 16);

#if 0
    if (0/*iy_start == 0*/) {
        for (int i = 0; i < N; ++i)
            printf("%d : %d\n", ix_start+i, color(i));
    }
#endif

    // because the output is a y-tile 2D surface
    // we can only write 32-byte wide 
#pragma unroll
    for (int i = 0; i < N; i += 8) {
         write(output_index,
               (ix_start+i) * sizeof(int), iy_start,
               color.select<8, 1>(i));
    }
}
