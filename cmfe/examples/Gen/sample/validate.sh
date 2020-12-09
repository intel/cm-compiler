#!/bin/bash

# validate installation of SDK by running all compile/run
# commands that are documented in README.txt

echo "Checking environment variables"
if [ -z "${CSDK_IGC}" ];
then
    echo "CSDK_IGC environment variable is not set - did you run 'source setupenv.sh' ?"
    exit 255
fi
if [ -z "${OPENCL_VENDOR_PATH}" ];
then
    echo "OPENCL_VENDOR_PATH environment variable is not set - did you run 'source setupenv.sh' ?"
    exit 255
fi
SIM_DIR=${CSDK_WORKSPACE}/opt/intel/sim

echo "Checking installed packages"
for package in build-essential make g++ gcc-multilib clinfo ocl-icd-opencl-dev ocl-icd-libopencl1
do
    dpkg -l $package >/dev/null 2>&1
    if [ $? -ne 0 ];
    then
        echo "PACKAGE: ${package} is not installed"
        exit 250
    fi
done

LIBCOUNT=`ls -1 ${CSDK_IGC}/usr/lib/x86_64-linux-gnu/libze_intel_gpu.so* | sed -e 's!.*/libze_intel_gpu.so.!!' | sed -e 's/\.0$//' | sed -e 's/\.0$//' | uniq | wc -l`
if [ "${LIBCOUNT}" -ne 1 ]
then
    echo "Multiple versions of libze_intel_gpu* detected."
    echo "...delete all but the latest version."
    exit 253
fi

OUT=`mktemp -d`
if [ $? -ne 0 ]; then
    OUT=tmp.$(date +%y%m%d_%H%M)
    mkdir -p ${OUT}
    OUT=`realpath ${OUT}`
fi
echo "Will create output files in ${OUT} directory"

echo "Checking OpenCL Runtime configuration"
# remove simulator file, if present
rm -f igdrcl.config

ldd `which clinfo` | grep OpenCL > ${OUT}/clinfo.ocl 2>&1
grep /usr/lib/x86_64-linux-gnu ${OUT}/clinfo.ocl > /dev/null
if [ $? -ne 0 ];
then
    echo "OpenCL (setup) - using wrong verison of libOpenCL"
    echo "    export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu"
    exit 250
fi

HAVE_GPU=1

clinfo | grep 'Device Name' > ${OUT}/clinfo.out 2>&1
grep -E 'Intel\(R\).*Graphics' ${OUT}/clinfo.out > /dev/null
if [ $? -ne 0 ];
then
    echo "OpenCL (setup) - did not detect Intel GEN9 GPU"
    HAVE_GPU=0
fi


if [ ${HAVE_GPU} -ne 0 ];
then
    echo "Checking running on GEN9 HW"
    ${CSDK_IGC}/usr/bin/cmc -emit-spirv -fcmocl -mcpu=SKL -m64 \
        -isystem ${CSDK_IGC}/usr/include kernel.cpp -o kernel.spv.skl > /dev/null 2>&1
    if [ $? -ne 0 ];
    then
        echo "Compile (SKL/cmc) of kernel failed"
        exit 230
    fi

    ${CSDK_IGC}/usr/bin/ocloc -device skl -output_no_suffix -options "-cmc" \
        -spirv_input -file kernel.spv.skl -output kernel.skl > /dev/null 2>&1
    if [ $? -ne 0 ];
    then
        echo "Compile (SKL/ocloc) of kernel failed"
        exit 229
    fi

    g++ -m64 -DKERNEL=\"kernel.skl\" \
        -I${CSDK_IGC}/opt/intel/opencl/include \
        -L${CSDK_IGC}/usr/lib/x86_64-linux-gnu/ -L/usr/lib/x86_64-linux-gnu/ \
        -Wl,-rpath -Wl,${CSDK_IGC}/usr/lib/x86_64-linux-gnu/ \
        host.cpp -lOpenCL -o vector.skl
    if [ $? -ne 0 ];
    then
        echo "Compile (SKL) of host failed"
        exit 228
    fi

    ./vector.skl > ${OUT}/hw.run 2>&1
    if [ $? -ne 0 ];
    then
        echo "Run (SKL) failed"
        exit 227
    fi
    grep PASSED ${OUT}/hw.run > /dev/null
    if [ $? -ne 0 ];
    then
        echo "Run (SKL) did not contain expected output"
        exit 226
    fi
fi





if [ ${HAVE_GPU} -ne 0 ];
then
    echo "Checking running on GEN9 HW(L0)"
    g++ -m64 -DKERNEL=\"kernel.spv.skl\" -I${CSDK_IGC}/usr/local/include -L${CSDK_IGC}/usr/local/lib -Wl,-rpath -Wl,${CSDK_IGC}/usr/local/lib:${CSDK_IGC}/usr/lib/x86_64-linux-gnu host_l0.cpp -lze_loader -o vector.l0.skl
    if [ $? -ne 0 ];
    then
        echo "Compile (SKL/L0) of host failed"
        exit 200
    fi

    rm -rf igdrcl.config
    ./vector.l0.skl > ${OUT}/hw.l0.run 2>&1
    if [ $? -ne 0 ];
    then
        echo "Run (SKL/L0) failed"
        cat ${OUT}/hw.l0.run
        exit 199
    fi
    grep PASSED ${OUT}/hw.l0.run > /dev/null
    if [ $? -ne 0 ];
    then
        echo "Run (SKL/L0) did not contain expected output"
        cat ${OUT}/hw.l0.run
        exit 198
    fi
fi




echo "SUCCESS"
rm -rf ${OUT}
exit 0
