#include "cm_rt.h"
#include <sys/types.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <jansson.h>
#include <vector>
#include <iostream>

#include "point.h"

// Includes cm_rt_helpers.h to convert the integer return code returned from
// the CM runtime to a meaningful string message.
#include "../../common/cm_rt_helpers.h"
#include "../../common/isa_helpers.h"
#include "kmeans.h"

typedef bool boolean;

#define NUM_ITERATIONS 400

using namespace std;

#ifdef CMRT_EMU

void cmk_kmeans(
		SurfaceIndex pts_si,    // binding table index of points
		SurfaceIndex cen_si,    // binding table index of centroids
		SurfaceIndex acc_si,    // binding table index of accum for computing new centroids
		unsigned num_points,    // number of points 
		bool final_iteration);  // if this is the final iteration of kmeans

void cmk_compute_centroid_position(
		SurfaceIndex cen_si,      // binding table index of centroids
		SurfaceIndex acc_si);     // binding table index of accum for computing new centroids

void cmk_accum_reduction(SurfaceIndex acc_si);

#endif


inline float dist(Point p, Centroid c)
{
	float dx = p.x - c.x;
	float dy = p.y - c.y;
	return dx * dx + dy * dy;
}

void clustering(
		Point* pts,          // points
		unsigned num_pts,    // number of points
		Centroid* ctrds,     // centroids
		unsigned num_ctrds)  // number of centroids
{

	for (auto i = 0; i < num_pts; i++)
	{
		float min_dis = -1;
		auto cluster_idx = 0;		
		// for each point, compute the min distance to centroids
		for (auto j = 0; j < num_ctrds; j++)
		{
			float dis = dist(pts[i], ctrds[j]);

			if (dis < min_dis || min_dis == -1)
			{
				min_dis = dis;
				cluster_idx = j;
			}
		}
		pts[i].cluster = cluster_idx;
	}
	// compute new positions of centroids
	Accum* accum = (Accum*)malloc(num_ctrds * sizeof(Accum));
	memset(accum, 0, num_ctrds * sizeof(Accum));
	for (auto i = 0; i < num_pts; i++)
	{
		auto c = pts[i].cluster;
		accum[c].x_sum += pts[i].x;
		accum[c].y_sum += pts[i].y;
		accum[c].num_points++;
	}
	for (auto j = 0; j < num_ctrds; j++)
	{
		ctrds[j].x = accum[j].x_sum / accum[j].num_points;
		ctrds[j].y = accum[j].y_sum / accum[j].num_points;
		ctrds[j].num_points = accum[j].num_points;
	}
	delete accum;
}
Centroid baseCent[NUM_CENTROIDS] = 
{
	{ 2.477653, 2.272590, 5451 },
	{ 2.659882, 1.673272, 3890 },
	{ 1.193639, 1.349012, 2892 },
	{ 1.752660, 1.635006, 5420 },
	{ 0.751243, 2.021375, 5171 },
	{ 2.197934, 1.908455, 6461 },
	{ 1.638335, 0.065415, 5331 },
	{ 2.217693, 1.454349, 5778 },
	{ 2.517634, 2.900675, 3176 },
	{ 1.296864, 0.269517, 4845 },
	{ 1.340640, 0.724762, 5705 },
	{ 1.732499, 0.477841, 7046 },
	{ 1.748009, 0.934762, 5068 },
	{ 2.012499, 2.874845, 3274 },
	{ 0.670576, 1.497643, 3007 },
	{ 1.836300, 2.231616, 5387 },
	{ 2.094395, 0.152124, 5743 },
	{ 2.120531, 0.657693, 5445 },
	{ 1.069757, 2.535155, 3957 },
	{ 1.283161, 1.974405, 5257 } };

#define max(a,b) (((a) > (b))? (a) : (b))
boolean verify_result(
		Point* gpu_pts,      // gpu points result
		Point* cpu_pts,      // cpu points result
		Centroid* gpu_ctrds, // gpu centroids result
		Centroid* cpu_ctrds, // cpu centroids result
		unsigned num_pts,    // number of points	
		unsigned num_ctrds)  // number of centroids
{

	for (auto i = 0; i < num_ctrds; i++)
	{
		float errX = fabs(gpu_ctrds[i].x - cpu_ctrds[i].x) / max(fabs(gpu_ctrds[i].x), fabs(cpu_ctrds[i].x));
		float errY = fabs(gpu_ctrds[i].y - cpu_ctrds[i].y) / max(fabs(gpu_ctrds[i].y), fabs(cpu_ctrds[i].y));
		float errSize = abs(gpu_ctrds[i].num_points - cpu_ctrds[i].num_points) / max(abs(gpu_ctrds[i].num_points), abs(cpu_ctrds[i].num_points));
		std::cout <<  i << ": Wanted (" << cpu_ctrds[i].x << "," << cpu_ctrds[i].y << ", " << cpu_ctrds[i].num_points << ")\n";
		std::cout << "Got (" << gpu_ctrds[i].x << "," << gpu_ctrds[i].y << ", " << gpu_ctrds[i].num_points << ")\n";
		if (errX >= 0.001f || errY >= 0.001f || errSize >= 0.001f) {
			std::cout << "Error, index " << i << ": Wanted (" << cpu_ctrds[i].x << "," << cpu_ctrds[i].y << ", " << cpu_ctrds[i].num_points << ")\n";
			std::cout << "Got (" << gpu_ctrds[i].x << "," << gpu_ctrds[i].y << ", " << gpu_ctrds[i].num_points << ")\n";
			return false;
		}
	}

	return true;
}

void dump_accum(Accum* accum, unsigned total_accum_record)
{
	std::cout << "           \tx_sum\t\ty_sum\t num_points" << std::endl;
	for (unsigned i = 0; i < total_accum_record; i++)
	{
		if (i%NUM_CENTROIDS == 0)
			std::cout << std::endl;
		std::cout << "Accum " << i << " \t" << accum[i].x_sum << "\t\t" << accum[i].y_sum << " \t" << accum[i].num_points << std::endl;
	}
}
// take initial points and run k mean clustering number of iterations
void cpu_kmeans(Point* pts,         // points
		unsigned num_pts,    // number of points
		Centroid* ctrds,     // centroids
		unsigned num_ctrds,  // number of centroids
		unsigned iterations) // run clustering number of iterations
{

	for (auto i = 0; i < iterations; i++)
	{
		clustering(pts, num_pts, ctrds, num_ctrds);
	}

}

// GPU has already aggregrated x_sum, y_sum and num_points for each cluster
// this function computes new centroid position
void cpu_compute_centroid_position(
		Accum* accum,
		Centroid* centroids,
		unsigned num_centroids)
{
	for (auto i = 0; i < num_centroids; i++)
	{
		centroids[i].x = accum[i].x_sum / accum[i].num_points;
		centroids[i].y = accum[i].y_sum / accum[i].num_points;
		centroids[i].num_points = accum[i].num_points;
	}
}

int main (int argc, const char** argv)
{
	json_t *json;
	json_error_t error;
	size_t index;
	json_t *value;


	//json = json_load_file("points.json", 0, &error);   
	json = json_load_file("points.big.json", 0, &error);

	// 100.000 points it's the repository default.

	// validates json
	if (!json) {
		printf("Error parsing Json file");
		fflush(stdout);
		return -1;
	}

	// allocate memory for points 
	Point* points  = (Point*)CM_ALIGNED_MALLOC(NUM_POINTS*sizeof(Point), 0x1000);
	memset(points, 0, NUM_POINTS*sizeof(Point));

#if 0
	// create a bigger input file by duplicatge points.json multiple times
	// make sure
	// 1. json = json_load_file("points.json", 0, &error);
	// 2. use #define NUM_POINTS 98304 in kmeans.h
	// 3. define MULTIPLE_TIMES you like
#define MULTIPLE_TIMES 8	
	ofstream ofile("points.big.json");
	ofile << "[";
	// load points from json
	for (unsigned i = 0; i < MULTIPLE_TIMES; i++)
	{
		json_array_foreach(json, index, value)
		{
			double x = json_number_value(json_array_get(value, 0));
			double y = json_number_value(json_array_get(value, 1));
			ofile << "[" << std::setprecision(16) << x << "," << y <<  "]";
			if (index == NUM_POINTS)			
				break;
			ofile << ",";
		}
		if (i < MULTIPLE_TIMES - 1)  // skip last iteration
			ofile << ",";
	}
	ofile << "]";
	ofile.close();
#endif

	// load points from json
	json_array_foreach(json, index, value)
	{
		float x = json_number_value(json_array_get(value, 0));
		float y = json_number_value(json_array_get(value, 1));
		points[index].x = x;
		points[index].y = y;

		if(index == NUM_POINTS)
			break;

	}
	cout << "read in points" << endl;

	// allocate memory for points and centroids
	Centroid* centroids = (Centroid*)CM_ALIGNED_MALLOC(NUM_CENTROIDS * sizeof(Centroid), 0x1000);
	// init centroids with the first num_centroids points
	for (auto i = 0; i < NUM_CENTROIDS; i++)
	{
		centroids[i].x = points[i].x;
		centroids[i].y = points[i].y;
		centroids[i].num_points = 0;
	}
	// Accum is for aggregrating (x,y) of the same cluster to compute new centroid positions
	Accum* accum = (Accum*)CM_ALIGNED_MALLOC((NUM_POINTS/ POINTS_PER_THREAD)* NUM_CENTROIDS * sizeof(Accum), 0x1000);
	memset(accum, 0, (NUM_POINTS / POINTS_PER_THREAD)*NUM_CENTROIDS * sizeof(Accum));

	// compute CPU kmean results for verifying results later
        cout << "compute reference output" << endl;
	Point* cpu_points = (Point*)malloc(NUM_POINTS * sizeof(Point));
	memcpy(cpu_points, points, NUM_POINTS * sizeof(Point));
	Centroid* cpu_centroids = (Centroid*)malloc(NUM_CENTROIDS * sizeof(Centroid));
	memcpy(cpu_centroids, centroids, NUM_CENTROIDS * sizeof(Centroid));
	cpu_kmeans(cpu_points, NUM_POINTS, cpu_centroids, NUM_CENTROIDS, NUM_ITERATIONS);

	int result;
	// Create a CM Device
	CmDevice* pCmDev = NULL;
	UINT version = 0;
	cm_result_check(::CreateCmDevice(pCmDev, version));

	// create zero copy point  and centroid buffers
	CmBufferUP*  pointsBuf = NULL;
	cm_result_check(pCmDev->CreateBufferUP(NUM_POINTS*sizeof(Point), points, pointsBuf));
	CmBufferUP*  centroidsBuf = NULL;
	cm_result_check(pCmDev->CreateBufferUP(NUM_CENTROIDS* sizeof(Centroid), centroids, centroidsBuf));
	CmBufferUP*  accumBuf = NULL;
	cm_result_check(pCmDev->CreateBufferUP((NUM_POINTS / POINTS_PER_THREAD)*NUM_CENTROIDS * sizeof(Accum), accum, accumBuf));

	FILE* pISA = fopen("src/kmeans_genx.isa", "rb");
	if (pISA == NULL) {
		perror("kmeans_genx.isa");
		return -1;
	}

	fseek(pISA, 0, SEEK_END);
	int codeSize = ftell(pISA);
	rewind(pISA);

	if (codeSize == 0)
	{
		return -1;
	}

	void *pCommonISACode = (BYTE*)malloc(codeSize);
	if (!pCommonISACode)
	{
		return -1;
	}

	if (fread(pCommonISACode, 1, codeSize, pISA) != codeSize) {
		perror("kmeans_genx.isa");
		return -1;
	}
	fclose(pISA);

	cout << "Loading program" << endl;
	// Creates a CmProgram object consisting of the kernels loaded
	// from the code buffer.
	CmProgram *program = nullptr;
	cm_result_check(pCmDev->LoadProgram(pCommonISACode, codeSize, program));

	// Creates the kmeans kernel.
	CmKernel *kmeans_kernel = nullptr;
	cm_result_check(pCmDev->CreateKernel(program,
				CM_KERNEL_FUNCTION(cmk_kmeans),
				kmeans_kernel));
	// prepare kernel arguments
	SurfaceIndex * index0 = NULL;
	pointsBuf->GetIndex(index0);
	kmeans_kernel->SetKernelArg(0, sizeof(SurfaceIndex), index0);

	SurfaceIndex * index1 = NULL;
	centroidsBuf->GetIndex(index1);
	kmeans_kernel->SetKernelArg(1, sizeof(SurfaceIndex), index1);

	SurfaceIndex * index2 = NULL;
	accumBuf->GetIndex(index2);
	kmeans_kernel->SetKernelArg(2, sizeof(SurfaceIndex), index2);

	unsigned num_points = NUM_POINTS;
	kmeans_kernel->SetKernelArg(3, sizeof(unsigned), &num_points);

	// take care of NUM_POINTS is not divided by POINTS_PER_THREAD
	unsigned int total_threads = (NUM_POINTS - 1) / POINTS_PER_THREAD + 1;
	CmThreadGroupSpace* pTGS = NULL;
	result = pCmDev->CreateThreadGroupSpace(1, 1, total_threads, 1, pTGS);
	// Create a task queue
	CmQueue* pCmQueue = NULL;
	cm_result_check(pCmDev->CreateQueue(pCmQueue));


	// create a task to execute kernels
	CmTask *pKernelArray = NULL;
	cm_result_check(pCmDev->CreateTask(pKernelArray));
	cm_result_check(pKernelArray->AddKernel(kmeans_kernel));

	// Creates the compute centroid kernel
	CmKernel *compute_centroid_kernel = nullptr;
	cm_result_check(pCmDev->CreateKernel(program,
				CM_KERNEL_FUNCTION(cmk_compute_centroid_position),
				compute_centroid_kernel));
	compute_centroid_kernel->SetKernelArg(0, sizeof(SurfaceIndex), index1);  // set centroid buffer
	compute_centroid_kernel->SetKernelArg(1, sizeof(SurfaceIndex), index2);  // set accum buffer	
	CmThreadGroupSpace* pTGS1 = NULL;
	// each HW thread compute one centroid
	result = pCmDev->CreateThreadGroupSpace(1, 1, NUM_CENTROIDS, 1, pTGS1); 
	// create a task to execute kernels
	CmTask *pKernelArray1 = NULL;
	cm_result_check(pCmDev->CreateTask(pKernelArray1));
	cm_result_check(pKernelArray1->AddKernel(compute_centroid_kernel));

	// Creates the accum reduction kernel
	CmKernel *accum_reduction_kernel = nullptr;
	cm_result_check(pCmDev->CreateKernel(program,
				CM_KERNEL_FUNCTION(cmk_accum_reduction),
				accum_reduction_kernel));
	accum_reduction_kernel->SetKernelArg(0, sizeof(SurfaceIndex), index2);  // set centroid buffer
	CmThreadGroupSpace* pTGS2 = NULL;
	// each HW thread compute accum reduction of ACCUM_REDUCTION_RATIO (NUM_CENTROIDS record)
	// kmeans kernel computes POINTS_PER_THREAD points per HW thread and writes out NUM_CENTROID of Accum.
	// We have total NUM_POINTS / POINTS_PER_THREAD of (Accum*NUM_CENTROID) records. If the number is still 
	// too big, accum_reduction is invoked to do parallel reduction with a ration of ACCUM_REDUCTION_RATIO
	unsigned total_accum_record = NUM_POINTS / POINTS_PER_THREAD;
	result = pCmDev->CreateThreadGroupSpace(1, 1, total_accum_record/ACCUM_REDUCTION_RATIO, 1, pTGS2);
	// create a task to execute kernels
	CmTask *pKernelArray2 = NULL;
	cm_result_check(pCmDev->CreateTask(pKernelArray2));
	cm_result_check(pKernelArray2->AddKernel(accum_reduction_kernel));

	cout << "enqueue kernels" << endl;
	// kmeans take multiple iterations for centroids to converge
	UINT64 kernel1_time_in_ns = 0;
	UINT64 kernel2_time_in_ns = 0;
	UINT64 kernel3_time_in_ns = 0;

	for (auto i = 0; i < NUM_ITERATIONS - 1; i++)
	{
		//cout << "iteration " << i << endl;
		// tell  kernel to write back the cluster result of the last 
		// iteration
		bool final_iteration = false;
		kmeans_kernel->SetKernelArg(4, sizeof(bool), &final_iteration);

		CmEvent* e1 = NULL, * e2 = NULL, * e3 = NULL; 
		UINT64 time_in_ns = 0;
		unsigned long time_out = -1;
		cm_result_check(pCmQueue->EnqueueWithGroup(pKernelArray, e1, pTGS));
		cm_result_check(e1->WaitForTaskFinished(time_out));
		cm_result_check(e1->GetExecutionTime(time_in_ns));
		kernel1_time_in_ns += time_in_ns;

#if ACCUM_REDUCTION_RATIO > 1
		time_in_ns = 0;
		time_out = -1;
		cm_result_check(pCmQueue->EnqueueWithGroup(pKernelArray2, e2, pTGS2));
		cm_result_check(e2->WaitForTaskFinished(time_out));
		cm_result_check(e2->GetExecutionTime(time_in_ns));
		kernel2_time_in_ns += time_in_ns;			
#endif

		time_in_ns = 0;
		time_out = -1;
		cm_result_check(pCmQueue->EnqueueWithGroup(pKernelArray1, e3, pTGS1));
		cm_result_check(e3->WaitForTaskFinished(time_out));
		cm_result_check(e3->GetExecutionTime(time_in_ns));
		kernel3_time_in_ns += time_in_ns;
	}
	// final iteration. need waitForTaskFinished
	{
		//cout << "iteration " << i << endl;
		// tell  kernel to write back the cluster result of the last 
		// iteration
		bool final_iteration = true;
		kmeans_kernel->SetKernelArg(4, sizeof(bool), &final_iteration);

		CmEvent* e1 = NULL, *e2 = NULL, *e3 = NULL;
		UINT64 time_in_ns = 0;
		unsigned long time_out = -1;
		cm_result_check(pCmQueue->EnqueueWithGroup(pKernelArray, e1, pTGS));
		cm_result_check(e1->WaitForTaskFinished(time_out));
		cm_result_check(e1->GetExecutionTime(time_in_ns));
		kernel1_time_in_ns += time_in_ns;

#if ACCUM_REDUCTION_RATIO > 1
		time_in_ns = 0;
		time_out = -1;
		cm_result_check(pCmQueue->EnqueueWithGroup(pKernelArray2, e2, pTGS2));
		cm_result_check(e2->WaitForTaskFinished(time_out));
		cm_result_check(e2->GetExecutionTime(time_in_ns));
		kernel2_time_in_ns += time_in_ns;
#endif

		time_in_ns = 0;
		time_out = -1;
		cm_result_check(pCmQueue->EnqueueWithGroup(pKernelArray1, e3, pTGS1));
		cm_result_check(e3->WaitForTaskFinished(time_out));
		cm_result_check(e3->GetExecutionTime(time_in_ns));
		kernel3_time_in_ns += time_in_ns;

	}


	auto correct = verify_result(points, cpu_points, centroids, cpu_centroids, NUM_POINTS, NUM_CENTROIDS);
	cout << ((correct)? "PASS" : "FAIL") << std::endl;

	float kernel1_time = kernel1_time_in_ns / 1000000.0f;
	float kernel2_time = kernel2_time_in_ns / 1000000.0f;
	float kernel3_time = kernel3_time_in_ns / 1000000.0f;
	float kernel_time = kernel1_time + kernel2_time + kernel3_time;

	printf("\n--- CM Kernel execution stats begin ---\n");

	printf("NUMBER_OF_POINTS: %d\n", NUM_POINTS);
	printf("NUMBER_OF_CENTROIDS: %d\n", NUM_CENTROIDS);
	printf("NUMBER_OF_ITERATIONS: %d\n", NUM_ITERATIONS);
	printf("POINTS_PER_THREAD: %d\n", POINTS_PER_THREAD);
	printf("ACCUM_REDUCTION_RATIO: %d\n\n", ACCUM_REDUCTION_RATIO);

	printf("Average kernel1 time: %f ms\n", kernel1_time / NUM_ITERATIONS);
	printf("Total kernel1 time: %f ms\n\n", kernel1_time);

	printf("Average kernel2 time: %f ms\n", kernel2_time / NUM_ITERATIONS);
	printf("Total kernel2 time: %f ms\n\n", kernel2_time);

	printf("Average kernel3 time: %f ms\n", kernel3_time / NUM_ITERATIONS);
	printf("Total kernel3 time: %f ms\n\n", kernel3_time);

	printf("Average kernel time: %f ms\n", kernel_time / NUM_ITERATIONS);
	printf("Total kernel time: %f ms\n\n", kernel_time);

	printf("--- CM Kernel execution stats end ---\n\n");

	CM_ALIGNED_FREE(points);
	CM_ALIGNED_FREE(centroids);
	CM_ALIGNED_FREE(accum);
	delete pCommonISACode;
	delete cpu_points;
	delete cpu_centroids;

	return 0;

}
