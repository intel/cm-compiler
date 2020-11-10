 This is a CM(C for metal) sample for converting 1080p RGBA image to RGB

 cm_cvt_color.cpp/h is the class implementation. It has two methods for
 initialization(CreateKernel, CreateOutputBuffer) and one method for the RGBA
 to RGB convertion(Cvt2Rgb). Please refer to rgb42rgb_test.cpp for the usage.

 rgb_cvt_genx.cpp is the CM kernel which will be compiled into
 rgb_cvt_genx.isa.


 Steps to run this sample

 1.Please open website https://github.com/intel/cm-compiler/releases and
 download "Release-2020-01-19: ubuntu 18.04
 (https://github.com/intel/cm-compiler/releases/download/Master/Linux_C_for_Metal_Development_Package_20200119.zip)
 2. Install the cm packages 
 unzip Linux_C_for_Metal_Development_Package_20200119.zip
 cd Linux_C_for_Metal_Development_Package_20200119
 mkdir -p /opt/intel/cm
 sudo cp -r runtime compiler /opt/intel/cm/
 3. Copy this folder to Linux_C_for_Metal_Development_Package_20200119/cm_runtime/examples/
 4. If your system wasn't installed with media-driver, please install it by
 bellow steps:
     a)Downlaod script https://github.com/intel-iot-devkit/concurrent-video-analytic-pipeline-optimization-sample-l/blob/master/msdk_pre_install.py
     b)Remove 215 to 233, 169 to 174 which download and build mediasdk. We only need
     the steps to build and install libva/media-driver and their dependencies.
     c)Modify line 129 to 130:
      cmd ="sudo apt-get -y install git libssl-dev dh-autoreconf cmake libpciaccess-dev build-essential;"
      cmd+="sudo apt-get -y install coreutils checkinstall pkg-config"
     d)Run "python msdk_pre_install.py" "python msdk_pre_install.py -b cfl"
     e)Run below command before you compile or run our sample
     export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/x86_64-linux-gnu:
     export LIBVA_DRIVERS_PATH=/usr/lib/x86_64-linux-gnu/dri
     export LIBVA_DRIVER_NAME=iHD
     f)Run vainfo to make sure the media-driver has been installed correctly
 5.cd
 Linux_C_for_Metal_Development_Package_20200119/cm_runtime/examples/rgb43rgb_cm
 make -f ../Makefile.linux
 You will see binary hw_x64.rgb42rgb_cm and kernel assemble file
 rgb_cvt_genx.isa under current folder.
 Run "./hw_x64.rgb42rgb_cm". It will run the 1080p RGBA to RGB converstion 10
 times and print the average time it takes.





