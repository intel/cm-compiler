#!/bin/bash

# validate installation of SDK of SDK by running all compile/run
# commands that are documented in README.txt
OUT=tmp.$(date +%y%m%d_%H%M)
mkdir -p ${OUT}
OUT=`realpath ${OUT}`
PLATFORM=SKL

function help()
{
    echo "Usage:"
    echo "  ./validate.sh [-h|-?|--help]"
    echo "  ./validate.sh [SKL|TGLLP|ICLLP]"
    echo " Where SKL stands for Skylake, TGLLP stands for Tiger Lake LP, ICLLP stands for Ice Lake LP"
}

if [[ "$1" == "-h" || "$1" == "--help" || "$1" == "-?" ]]
then
    help
    exit 0
fi
if [[ "$1" == "" || "$1" == "SKL" ]]
then
    PLATFORM=SKL
    PLATFORM_EXTENSION=skl
elif [[ "$1" == "TGLLP" ]]
then
    PLATFORM=TGLLP
    PLATFORM_EXTENSION=tgllp
elif [[ "$1" == "ICLLP" ]]
then
    PLATFORM=ICLLP
    PLATFORM_EXTENSION=icllp
else
    echo "Unknown option"
    help
    exit 0
fi

echo "Verifying environment variables"
if [ -z "${CSDK_DIR}" ];
then
    echo "CSDK_DIR environment variable is not set - did you run 'source setupenv.sh' ?"
    exit 255
fi
if [ -z "${OPENCL_VENDOR_PATH}" ];
then
    echo "OPENCL_VENDOR_PATH environment variable is not set - did you run 'source setupenv.sh' ?"
    exit 255
fi

echo "Verifying installed packages"
for package in g++ clinfo ocl-icd-opencl-dev ocl-icd-libopencl1
do
    dpkg -l $package >/dev/null 2>&1
    if [ $? -ne 0 ];
    then
        echo "PACKAGE: ${package} is not installed"
        exit 250
    fi
done

HAVE_GPU=1
clinfo | grep 'Device Name' > ${OUT}/clinfo.out 2>&1
grep -E 'Intel\(R\).*Graphics' ${OUT}/clinfo.out > /dev/null
if [ $? -ne 0 ];
then
    echo "OpenCL (setup) - did not detect Intel GPU. Running CMEMU examples only"
    HAVE_GPU=0
fi

if [ ${HAVE_GPU} -ne 0 ];
then
    echo "Checking running on HW(OpenCL) using ocloc"
    $CSDK_DIR/usr/local/bin/ocloc -device $PLATFORM_EXTENSION \
    -output_no_suffix -options "-cmc -mcpu=$PLATFORM -m64" -file kernel.cpp \
    -output kernel.$PLATFORM_EXTENSION > ${OUT}/hw.ocloc.ker 2>&1
    if [ $? -ne 0 ];
    then
        echo "Compile ($PLATFORM/ocloc) of kernel failed"
        cat ${OUT}/hw.ocloc.ker
        exit 230
    fi

    g++ -m64 -DKERNEL=\"kernel.$PLATFORM_EXTENSION\" \
        -DCL_TARGET_OPENCL_VERSION=220 \
        -L${CSDK_DIR}/usr/local/lib/ \
        -Wl,-rpath -Wl,${CSDK_DIR}/usr/local/lib/ \
        host.cpp -lOpenCL -o vector.$PLATFORM_EXTENSION
    if [ $? -ne 0 ];
    then
        echo "Compile ($PLATFORM) of host failed"
        exit 228
    fi

    ./vector.$PLATFORM_EXTENSION > ${OUT}/hw.run 2>&1
    if [ $? -ne 0 ];
    then
        echo "Run ($PLATFORM) failed"
        cat ${OUT}/hw.run
        exit 227
    fi
    grep PASSED ${OUT}/hw.run > /dev/null
    if [ $? -ne 0 ];
    then
        echo "Run ($PLATFORM) did not contain expected output"
        cat ${OUT}/hw.run
        exit 226
    fi
    echo "PASSED"

    echo "Checking running on HW(L0) using ocloc"
    g++ -m64 -DKERNEL=\"kernel.$PLATFORM_EXTENSION.spv\" \
        -I${CSDK_DIR}/usr/local/include \
        -L${CSDK_DIR}/usr/local/lib -Wl,-rpath \
        -Wl,${CSDK_DIR}/usr/local/lib  \
        host_l0.cpp -lze_loader -o vector.l0.$PLATFORM_EXTENSION
    if [ $? -ne 0 ];
    then
        echo "Compile ($PLATFORM/L0) of host failed"
        exit 200
    fi

    ./vector.l0.$PLATFORM_EXTENSION > ${OUT}/hw.l0.run 2>&1
    if [ $? -ne 0 ];
    then
        echo "Run ($PLATFORM/L0) failed"
        cat ${OUT}/hw.l0.run
        exit 199
    fi
    grep PASSED ${OUT}/hw.l0.run > /dev/null
    if [ $? -ne 0 ];
    then
        echo "Run ($PLATFORM_EXTENSION/L0) did not contain expected output"
        cat ${OUT}/hw.l0.run
        exit 198
    fi
    echo "PASSED"

    echo "Checking running on HW(OpenCL) using cmc"
    ${CSDK_DIR}/usr/bin/cmc -fcmocl -march=$PLATFORM -m64 \
    -o kernel.cmc.$PLATFORM_EXTENSION -- kernel.cpp > ${OUT}/hw.cmc.ker 2>&1
    if [ $? -ne 0 ];
    then
        echo "Compile ($PLATFORM/cmc) of kernel failed"
        cat ${OUT}/hw.cmc.ker
        exit 197
    fi

    g++ -m64 -DKERNEL=\"kernel.cmc.$PLATFORM_EXTENSION\" \
        -DCL_TARGET_OPENCL_VERSION=220 \
        -L${CSDK_DIR}/usr/local/lib/ \
        -Wl,-rpath -Wl,${CSDK_DIR}/usr/local/lib/ \
        host.cpp -lOpenCL -o vector.cmc.$PLATFORM_EXTENSION
    if [ $? -ne 0 ];
    then
        echo "Compile ($PLATFORM) of host failed"
        exit 196
    fi

    ./vector.cmc.$PLATFORM_EXTENSION > ${OUT}/hw.cmc.run 2>&1
    if [ $? -ne 0 ];
    then
        echo "Run ($PLATFORM) failed"
        cat ${OUT}/hw.cmc.run
        exit 195
    fi
    grep PASSED ${OUT}/hw.cmc.run > /dev/null
    if [ $? -ne 0 ];
    then
        echo "Run ($PLATFORM) did not contain expected output"
        cat ${OUT}/hw.cmc.run
        exit 194
    fi
    echo "PASSED"
fi

echo "Checking SHIM Layer"

g++ -std=gnu++17 -DCMRT_EMU -DSHIM -shared -fpic \
    -I${CSDK_DIR}/usr/include/libcm -I${CSDK_DIR}/usr/include/shim \
    -L${CSDK_DIR}/usr/lib -Wl,-rpath -Wl,${CSDK_DIR}/usr/lib \
    kernel.cpp -o kernel.shim -lcm
if [ $? -ne 0 ];
then
    echo "Compile (SHIM kernel) failed"
    exit 240
fi

g++ -std=gnu++17 -DKERNEL=\"kernel.shim\" \
    -DCL_TARGET_OPENCL_VERSION=220 \
    -I${CSDK_DIR}/usr/include/libcm -I${CSDK_DIR}/usr/include/shim \
    -L${CSDK_DIR}/usr/lib -Wl,-rpath -Wl,${CSDK_DIR}/usr/lib \
    host.cpp -lshim -ligfxcmrt_emu -lcm -Wl,--disable-new-dtags -o vector.shim
if [ $? -ne 0 ];
then
    echo "Compile (SHIM host) failed"
    exit 239
fi

CM_RT_PLATFORM=$PLATFORM_EXTENSION ./vector.shim > ${OUT}/shim.run 2>&1
if [ $? -ne 0 ];
then
    echo "Run (SHIM) failed"
    cat ${OUT}/shim.run
    exit 238
fi
grep PASSED ${OUT}/shim.run > /dev/null
if [ $? -ne 0 ];
then
    echo "Run (SHIM) did not contain expected output"
    cat ${OUT}/shim.run
    exit 237
fi
echo "PASSED"

echo "Checking SHIM(L0) Layer"
g++ -std=gnu++17 -DCMRT_EMU -DSHIM -shared -fpic \
    -I${CSDK_DIR}/usr/include/libcm -I${CSDK_DIR}/usr/include/shim \
    -L${CSDK_DIR}/usr/lib -Wl,-rpath -Wl,${CSDK_DIR}/usr/lib \
    kernel.cpp -o kernel.l0.shim -lcm
if [ $? -ne 0 ];
then
    echo "Compile (SHIM/L0 kernel) failed"
    exit 140
fi

g++ -std=gnu++17 -DKERNEL=\"kernel.l0.shim\" \
    -I${CSDK_DIR}/usr/include/libcm -I${CSDK_DIR}/usr/local/include \
    -L${CSDK_DIR}/usr/lib -Wl,-rpath -Wl,${CSDK_DIR}/usr/lib \
    host_l0.cpp -lshim_l0 -ligfxcmrt_emu -lcm -Wl,--disable-new-dtags -o vector.l0.shim
if [ $? -ne 0 ];
then
    echo "Compile (SHIM/L0 host) failed"
    exit 139
fi

CM_RT_PLATFORM=$PLATFORM_EXTENSION ./vector.l0.shim > ${OUT}/shim.l0.run 2>&1
if [ $? -ne 0 ];
then
    echo "Run (SHIM/L0) failed"
    cat ${OUT}/shim.l0.run
    exit 138
fi
grep PASSED ${OUT}/shim.l0.run > /dev/null
if [ $? -ne 0 ];
then
    echo "Run (SHIM/L0) did not contain expected output"
    cat ${OUT}/shim.l0.run
    exit 137
fi
echo "PASSED"

#############################

rm -rf ${OUT}
exit 0