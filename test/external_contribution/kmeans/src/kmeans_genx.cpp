#include "cm/cm.h"
#include "kmeans.h"
#include "point.h"

const uchar init8[8] = { 0, 1, 2, 3, 4, 5, 6, 7};
const uchar init16[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
const ushort initmask[8] = { 1, 1, 1, 0, 0, 0, 0, 0};


// each HW thread process POINTS_PER_THREAD points. If the total number of 
// points is not divisible by POINTS_PER_THREAD. The following code then needs
// to handle out-of-bound reads/write. For now, we ignore the scenario by 
// assuming totoal number of points can be divided by POINTS_PER_THREAD. 
// kmeans is invoked multiple of iterations for centroids to converge. Only the 
// final clustering results of points of the later iteration are recorded 
// 

_GENX_MAIN_ void cmk_kmeans(
		SurfaceIndex pts_si,    // binding table index of points
		SurfaceIndex cen_si,    // binding table index of centroids
		SurfaceIndex acc_si,    // binding table index of accum for computing new centroids
		unsigned num_points,    // number of points 
		bool final_iteration)   // if this is the final iteration of kmeans
{
	// read in all centroids
	// this version we can only handle number of centroids no more than 64
	// We don't need cluster field so we read only the first two field
	matrix<float, 2, ROUND_TO_16_NUM_CENTROIDS> centroids;
	vector<unsigned int, 16> offsets(init16);
	offsets = offsets*DWORD_PER_POINT;

#pragma unroll
	for (unsigned i = 0; i < ROUND_TO_16_NUM_CENTROIDS; i+=16) // round up to next 16
	{
		// untyped read will pack R, G into SOA
		// matrix.row(0): x x x x . . . . // 16 position x
		// matrix.row(1): y y y y . . . . // 16 position y
		read_untyped(cen_si, CM_GR_ENABLE, centroids.select<2,1,16,1>(0,i), offsets + i*DWORD_PER_CENTROID);
	}

	matrix<float, 3, ROUND_TO_16_NUM_CENTROIDS> accum = 0;
	matrix_ref<unsigned, 1, ROUND_TO_16_NUM_CENTROIDS> num = accum.row(2).format<unsigned, 1, ROUND_TO_16_NUM_CENTROIDS>();


	uint linear_tid = cm_linear_group_id();
	// each thread handles 128 points
	unsigned start = linear_tid * POINTS_PER_THREAD * DWORD_PER_POINT;  // each point has 3 DWORD


	// use untyped read to read in points. 
	// Point is x, y, c
	// the returned result will be shuffled. x, y, c will be packed nicely
	for (unsigned i = 0; i < POINTS_PER_THREAD; i += 16)
	{
		matrix<float, 2, 16> pos;
		;
		vector<unsigned, 16> cluster = 0;
		read_untyped(pts_si, CM_GR_ENABLE, pos, start+ offsets+i*DWORD_PER_POINT);
		vector<float, 16> dx = pos.row(0) - centroids(0,0);
		vector<float, 16> dy = pos.row(1) - centroids(1,0);
		vector<float, 16> min_dist = dx * dx + dy * dy;
#pragma unroll
		for (unsigned j = 1; j < NUM_CENTROIDS; j++)
		{
			// compute distance
			dx = pos.row(0) - centroids(0, j);
			dy = pos.row(1) - centroids(1, j);
			vector<float, 16> dist = dx * dx + dy * dy;
			cluster.merge(j, dist < min_dist);
			min_dist.merge(dist, dist < min_dist);	
		}
		// if this is the final invocation of kmeans, write back clustering 
		// result
		if (final_iteration)
		{
			// point: x, y, cluster
			// i * DWORD_PER_POINT + 2 to write to cluster field 
			write(pts_si, start, offsets + i * DWORD_PER_POINT + 2, cluster);
		}
		// go over each point and according to their classified cluster update accum

#pragma unroll
		for (unsigned k = 0; k < 16; k++)
		{
			unsigned c = cluster(k);
			accum(0, c) += pos(0, k);
		}
#pragma unroll
		for (unsigned k = 0; k < 16; k++)
		{
			unsigned c = cluster(k);
			accum(1, c) += pos(1, k);

		}			
#pragma unroll
		for (unsigned k = 0; k < 16; k++)
		{
			unsigned c = cluster(k);			
			num(0,c)++;
		}

	}

	unsigned startoff = linear_tid * DWORD_PER_ACCUM * NUM_CENTROIDS; 
#pragma unroll
	for (unsigned i = 0; i < ROUND_TO_16_NUM_CENTROIDS; i += 16) // round up to next 16
	{
		matrix<float, 3, 16> a16 = accum.select<3, 1, 16, 1>(0, i);
		SIMD_IF_BEGIN(offsets + i * DWORD_PER_ACCUM < NUM_CENTROIDS * DWORD_PER_ACCUM) {
			write_untyped(acc_si, CM_BGR_ENABLE, a16, startoff + offsets + i * DWORD_PER_ACCUM);
		} SIMD_IF_END;
	}

}

// each HW thread sum up 8xNUM_CENTROIDS accum
_GENX_MAIN_ void cmk_accum_reduction(SurfaceIndex acc_si) {    

	// each thread computes one single centriod
	uint linear_tid = cm_linear_group_id();
	matrix<float, 3, ROUND_TO_16_NUM_CENTROIDS> accum;
	matrix<float, 3, ROUND_TO_16_NUM_CENTROIDS> sum = 0;
	matrix_ref<unsigned, 1, ROUND_TO_16_NUM_CENTROIDS> num = sum.row(2).format<unsigned, 1, ROUND_TO_16_NUM_CENTROIDS>();
	vector<unsigned int, 16> offsets(init16);
	offsets = offsets * DWORD_PER_POINT;
	unsigned start = linear_tid * ACCUM_REDUCTION_RATIO*NUM_CENTROIDS*DWORD_PER_ACCUM;
#pragma unroll
	for (unsigned i = 0; i < ACCUM_REDUCTION_RATIO; i++)
	{
		unsigned next = start + i * NUM_CENTROIDS*DWORD_PER_ACCUM;
#pragma unroll
		for (unsigned j = 0; j < ROUND_TO_16_NUM_CENTROIDS; j += 16) // round up to next 16
		{
			read_untyped(acc_si, CM_BGR_ENABLE, accum.select<3, 1, 16, 1>(0, j), next + offsets + j * DWORD_PER_ACCUM);
		}
		sum.row(0) += accum.row(0);
		sum.row(1) += accum.row(1);
		num += accum.row(2).format<unsigned, 1, ROUND_TO_16_NUM_CENTROIDS>();
	}


#pragma unroll
	for (unsigned i = 0; i < ROUND_TO_16_NUM_CENTROIDS; i += 16) // round up to next 16
	{
		matrix<float, 3, 16> a16 = sum.select<3, 1, 16, 1>(0, i);
		SIMD_IF_BEGIN(offsets + i * DWORD_PER_ACCUM < NUM_CENTROIDS * DWORD_PER_ACCUM) {
			write_untyped(acc_si, CM_BGR_ENABLE, a16, start + offsets + i * DWORD_PER_ACCUM);
		} SIMD_IF_END;
	}
}

_GENX_MAIN_ void cmk_compute_centroid_position(
		SurfaceIndex cen_si,      // binding table index of centroids
		SurfaceIndex acc_si) {    // binding table index of accum for computing new centroids

	vector<unsigned int, 16> offsets(init16);
	const unsigned stride = NUM_CENTROIDS * ACCUM_REDUCTION_RATIO;	
	offsets = offsets * DWORD_PER_ACCUM * stride;

	// each thread computes one single centriod
	uint linear_tid = cm_linear_group_id();

	vector<float, 16> X = 0;
	vector<float, 16> Y= 0;
	vector<unsigned, 16> N = 0;

	unsigned num_accum_record = (NUM_POINTS / (POINTS_PER_THREAD*ACCUM_REDUCTION_RATIO));

	// process 4 reads per iterations to hide latency
#pragma unroll
	for (unsigned i = 0; i < (num_accum_record >> 6) << 6; i += 64)
	{
		// untyped read will pack R, G, B into SOA
		// matrix.row(0): x x x x . . . . // 16 position x
		// matrix.row(1): y y y y . . . . // 16 position y
		// matrix.row(1): n n n n . . . . // 16 position num of points
		matrix<float, 3, 16> accum0;
		read_untyped(acc_si, CM_BGR_ENABLE, accum0, offsets + i* DWORD_PER_ACCUM * stride + linear_tid * DWORD_PER_ACCUM);
		matrix<float, 3, 16> accum1;
		read_untyped(acc_si, CM_BGR_ENABLE, accum1, offsets + (i+16) * DWORD_PER_ACCUM * stride + linear_tid * DWORD_PER_ACCUM);
		matrix<float, 3, 16> accum2;
		read_untyped(acc_si, CM_BGR_ENABLE, accum2, offsets + (i+32) * DWORD_PER_ACCUM * stride + linear_tid * DWORD_PER_ACCUM);
		matrix<float, 3, 16> accum3;
		read_untyped(acc_si, CM_BGR_ENABLE, accum3, offsets + (i+48) * DWORD_PER_ACCUM * stride + linear_tid * DWORD_PER_ACCUM);
		X += accum0.row(0) + accum1.row(0) + accum2.row(0) + accum3.row(0);
		Y += accum0.row(1) + accum1.row(1) + accum2.row(1) + accum3.row(1);
		N += accum0.row(2).format<unsigned, 1, 16>() + accum1.row(2).format<unsigned, 1, 16>() +
			accum2.row(2).format<unsigned, 1, 16>() + accum3.row(2).format<unsigned, 1, 16>();
	}
	// process remaining loop iterations
#pragma unroll
	for (unsigned i = (num_accum_record >> 6) << 6; i < num_accum_record; i += 16)
	{
		matrix<float, 3, 16> accum0;
		read_untyped(acc_si, CM_BGR_ENABLE, accum0, offsets + i * DWORD_PER_ACCUM * stride + linear_tid * DWORD_PER_ACCUM);
		X += accum0.row(0);
		Y += accum0.row(1);
		N += accum0.row(2).format<unsigned, 1, 16>();
	}

	vector<float, 8> centroid = 0;
	unsigned num = cm_sum<unsigned>(N);
	centroid(0) = cm_sum<float>(X)/num;
	centroid(1) = cm_sum<float>(Y)/num;
	centroid(2) = *((float*)&num);

	// update centroid(linear_tid)

	vector<ushort, 8> mask(initmask);	
	vector<uint, 8> offs(init8);
	SIMD_IF_BEGIN(mask) {
		write(cen_si, linear_tid * DWORD_PER_CENTROID, offs, centroid);
	} SIMD_IF_END;

}
