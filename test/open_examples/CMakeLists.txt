cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# SetupTest_from_cm ProjName SUBDIR INC_LIST SRC_LIST CM_LIST SUPPORT_LIST MODE DX_VER CMC_OPTIONS ICL_OPTIONS TIMEOUT NO_ICL BUILDONLY HASASSETS CL_OPTIONS LINK_OPTIONS CMICL TAGS [OPTIONAL ARGS]

set(TAGS "HWSP_bdw_INCSIM_SM_FULL")

SetupTest_from_cm(nbody_SLM_release "nbody_SLM_release" "" "nbody_SLM_release.cpp" "nbody_SLM_release_genx.cpp" "color_in.dat;nbody_gold.dat;nbody_gold_128.dat;pos_in.dat;vel_in.dat" "HW" "DX9" "" "" "2000" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(CalcIntImage "CalcIntImage" "" "CalcIntImage.cpp" "CalcIntImage_genx.cpp" "bbc.y" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(linear_walker "linear_walker" "" "linear_walker.cpp" "linear_walker_genx.cpp" "linear_in.bmp;linear_gold_hw.bmp" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(ColorAdjustSIMDCF "ColorAdjustSIMDCF" "" "ColorAdjustSIMDCF.cpp" "ColorAdjustSIMDCF_genx.cpp" "*.bmp" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS} "riverbed_1.bmp")

SetupTest_from_cm(gaussian_blur_test "gaussian_blur_test" "" "gaussian_blur_test.cpp" "gaussian_blur_test_genx.cpp" "blur_gold.bmp;lena.bmp" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(histogram_64 "histogram_64" "" "histogram.cpp" "histogram_genx.cpp" "" "HW" "DX9" "" "" "" "TRUE" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(histogram_64_slm "histogram_64_slm" "" "histogram.cpp" "histogram_genx.cpp" "" "HW" "DX9" "" "" "" "TRUE" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(histogram_256 "histogram_256" "" "histogram.cpp" "histogram_genx.cpp" "" "HW" "DX9" "" "" "" "TRUE" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(histogram_256_slm "histogram_256_slm" "" "histogram.cpp" "histogram_genx.cpp" "" "HW" "DX9" "" "" "" "TRUE" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(linear_up_walker "linear_up_walker" "" "linear_up_walker.cpp" "linear_up_walker_genx.cpp" "linear_in.bmp;linear_gold.bmp;linear_gold_hw.bmp" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm (mandelbrot "mandelbrot" "" "mandelbrot.cpp" "mandelbrot_genx.cpp" "" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(matrix_transpose "matrix_transpose" "" "matrix_transpose.cpp" "matrix_transpose_genx.cpp" "" "HW" "DX9" "" "" "" "TRUE" "" "" "" "" "" ${TAGS})

#SetupTest_from_cm(fft "fft" "" "fft.cpp" "fft_genx.cpp" "" "HW" "DX9" "-ffast-math" "" "2000" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(MediaSampler16x4 "MediaSampler16x4" "" "avs_nearest_422_001_16x4.cpp" "avs_nearest_422_001_16x4_genx.cpp" "*.yuy2" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(MediaSampler8x8 "MediaSampler8x8" "" "avs_adaptive_422_8x8_002.cpp" "avs_adaptive_422_8x8_002_genx.cpp" "*.yuy2;*.uyvy" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(sepia "sepia" "" "sepia.cpp" "sepia_genx.cpp" "*.bmp" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(sharpen "sharpen" "" "sharpen.cpp" "sharpen_genx.cpp" "*.bmp" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(Sgemm "Sgemm" "" "Sgemm.cpp" "Sgemm_genx.cpp" "" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(Sampler3D "Sampler3D" "" "Sampler3D.cpp" "Sampler3D_genx.cpp" "*.bmp" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(SparseMatrixMul "SparseMatrixMul" "" "SparseMatrixMul.cpp" "SparseMatrixMul_genx.cpp" "*.dat" "HW" "DX9" "" "" "" "TRUE" "" "" "" "" "" ${TAGS} "Webbase_csr.dat" "100")

SetupTest_from_cm(vcaOpSAD "vcaOpSAD" "" "vcaOpSAD.cpp" "vcaOpSAD_genx.cpp" "" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS} "4096" "8" "-random")

SetupTest_from_cm(PrefixSum "PrefixSum" "" "Prefix.cpp" "Prefix_genx.cpp" "" "HW" "DX9" "" "" "" "TRUE" "" "" "" "" "" ${TAGS} "16" )

SetupTest_from_cm(BufferTest_EnqueueWithSync "BufferTest_EnqueueWithSync" "" "BufferTest_EnqueueWithSync.cpp" "BufferTest_EnqueueWithSync_genx.cpp" "" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(multi_kernels "multi_kernels" "" "multi_kernels.cpp" "multi_kernels_genx.cpp" "*.bmp" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(hello_world "hello_world" "" "hello_world.cpp" "hello_world_genx.cpp" "" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(BitonicSortK "BitonicSortK" "" "BitonicSortK.cpp" "BitonicSortK_genx.cpp" "" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(radix "radix" "" "radix.cpp" "radix_genx.cpp" "" "HW" "DX9" "" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(radixsort "radixsort" "" "radixsort.cpp" "radixsort_genx.cpp" "" "HW" "DX9" "/DCM_PTRSIZE=32" "" "" "" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(HoughCircle "HoughCircle" "" "HoughCircle.cpp" "HoughCircle_genx.cpp" "pen_circle_edges.bmp;hough_circle_res.dat" "HW" "DX9" "" "" "" "TRUE" "" "" "" "" "" ${TAGS})

SetupTest_from_cm(Graph-Cut "Graph-Cut" "" "Graph_Cut.cpp;Graph_Cut_C_model.cpp;Graph_Cut_host.cpp;MDF_Base.cpp;common_C_model.cpp" "Graph_Cut_genx.cpp" "*.F32" "HW" "DX9" "" "" "" "TRUE" "" "" "" "" "" ${TAGS} "-i:LT1_node.F32" "-j:LT1_hor_edge.F32" "-k:LT1_ver_edge.F32" "-w:640" "-h:360" "-r:32")
