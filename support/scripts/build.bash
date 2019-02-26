#!/bin/bash

## build.bash : build cm-llvm binaries
## ===================================
##
## ``build.bash -h [-v] [-l] [-x] [-c] [-s vs2010|vs2012|vs2013|vs2015|vs2017] [-m|-M path] [-t path] [-b rpath] [-d|-r] [--32|--64] <path_to_llvm>``
##
## As a cm-llvm developer, the most common way of invoking build.bash from a
## Cygwin shell is to cd to your cm-llvm root and use
##
## ``support/scripts/build.bash -d``
##
## to build a debug version of cmc and some other binaries such as llvm-dis, and
## put them in a directory called something like ``build.32.vs2012``
##
## Options
## -------
##
##   =============== ==============================================================
##   -h              help
##   -v              visual studio mode (default windows)
##   -l              cygwin/gcc linux/gcc mode (default linux)
##   -c              run cmake (not normally needed unless the build directory is
##                   in an inconsistent state)
##   -s              vis studio version (vs2013, vs2015(default), vs2017)
##   -p path         path to cmake (default C:/Program Files (x86)/CMake 2.8/bin for
##                   VS/win, /usr/bin or /Applications/CMake.app/Contents/bin on
##                   Mac, otherwise /opt/bin
##   -b rpath        relative path for the build (default name will be auto
##                   generated e.g. build.gcc or build.32.vs2015)
##   -d              build Debug
##   -r              build Release (default)
##   -i path         absolute path to copy cmc and support/include files.
##                   this option is added for creating a linux release package
##   --32            32 bit build - (default for 32 bit OS and all windows systems)
##   --64            64 bit build - (default for 64 bit OS)
##   --nocolor       Don't use color in the output (useful for automated runs and
##                   output to logs)
##   --nocolour      Don't use colour in the output (useful for automated runs and
##                   output to logs)
##   -m              VS mode use msbuild and not devenv to drive the build - it
##                   will look in default locations for the exe
##   -M              In VS mode use msbuild and not devenv. Use the provided path
##                   to locate the executable
##   -x              Use Xcode mode for darwin builds
##   -V              Add version information to built cmc - used for compiler
##                   release
##   <path_to_llvm>  optional path to llvm source (defaults to $PWD/llvm)
##   =============== ==============================================================
##

cmver=CM3.6

# -h or -help option : output documentation as text through a pager
help() {
  if tty -s <&1; then
    pager=less
  else
    pager=cat
  fi
  # sed script to extract the documentation (the ## lines).
  # 1. Allow a line consisting of just spaces then "##": convert to blank line.
  # 2. Allow a line starting with spaces then "## ": remove the spaces and "## ".
  # 3. Delete any other line.
  # Having extracted the ## lines:
  # 4. Delete an rst .. command.
  # 5. For an rst .. comment (not a command), remove the comment prefix.
  # 6. Remove \ at start of line.
  # 7. Remove \ that is preceded by a non-\ character.
  # 8. Remove `` (rst code markup).
  # 9. Replace :doc:`name` with name.
  # The backslash removal is so we can use \ to escape special characters in
  # the rst markup.
  # Note that BSD sed (on Mac) demands that we have a newline after a label
  # name, rather than a semicolon and more commands.
  sed 's/^ *##$//; t gotline
      s/^ *## //; t gotline
      d; :gotline
      /^ *\.\. .*::/d
      s/^ *\.\. //
      s/^\\//
      s/\([^\]\)\\/\1/g
      s/``//g
      s/:doc:`\([^`]*\)`/\1/g
      ' "$0" | "$pager"
}

usage() {
  echo "Try ${0##*/} -h" >&2
  exit 1
}

function version_check() {
    ver=$(echo -ne "$1\n$2" | sort -Vr | head -n1)
    if [ "$2" == "$1" ]; then
        return 1
    elif [ "$2" == "$ver" ]; then
        return 0
    else
        return 1
    fi
}

function build_item() {
    if [ "$USE_VS" != "" ]; then
        if [ "$USE_MSBUILD" != "" ]; then
            if [ "$#" -gt 1 ]; then
                dirpath=$2
            else
                dirpath=tools/$1
            fi
            if ! "$MSBUILD_PATH" $dirpath/$1.vcxproj $BUILD_STRING ; then
                echo "$1 VS build step failed - exiting"
                return 1
            fi
        else
            if ! "$VS_PATH" LLVM.sln /build $BUILD_STRING /project $1 ; then
                echo "$1 VS build step failed - exiting"
                return 1
            fi
        fi
    elif [ "$USE_XCODE" != "" ]; then
        if [ "$USE_XCPRETTY" != "" ]; then
            if ! xcodebuild -target $1 $BUILD_STRING | xcpretty -c ; then
                echo "$1 xcode build step failed - exiting"
                return 1
            fi
        else
            if ! xcodebuild -target $1 $BUILD_STRING ; then
                echo "$1 xcode build step failed - exiting"
                return 1
            fi
        fi
    else
        if ! make -j 8 $1 ; then
            echo "$1 gcc build step failed - exiting"
            return 1
        fi
    fi
    return 0
}

function define_colours_on() {
    # Reset
    Colour_Off='\033[0m'      # Text Reset

    # Regular Colours
    Black='\033[0;30m'        # Black
    Red='\033[0;31m'          # Red
    Green='\033[0;32m'        # Green
    Yellow='\033[0;33m'       # Yellow
    Blue='\033[0;34m'         # Blue
    Purple='\033[0;35m'       # Purple
    Cyan='\033[0;36m'         # Cyan
    White='\033[0;37m'        # White

    # Bold
    BBlack='\033[1;30m'       # Black
    BRed='\033[1;31m'         # Red
    BGreen='\033[1;32m'       # Green
    BYellow='\033[1;33m'      # Yellow
    BBlue='\033[1;34m'        # Blue
    BPurple='\033[1;35m'      # Purple
    BCyan='\033[1;36m'        # Cyan
    BWhite='\033[1;37m'       # White
}

function define_colours_off() {
    # Reset
    Colour_Off=''      # Text Reset

    # Regular Colours
    Black=''        # Black
    Red=''          # Red
    Green=''        # Green
    Yellow=''       # Yellow
    Blue=''         # Blue
    Purple=''       # Purple
    Cyan=''         # Cyan
    White=''        # White

    # Bold
    BBlack=''       # Black
    BRed=''         # Red
    BGreen=''       # Green
    BYellow=''      # Yellow
    BBlue=''        # Blue
    BPurple=''      # Purple
    BCyan=''        # Cyan
    BWhite=''       # White
}

IN_LINUX=
IN_CYGWIN=
IN_MINGW=
IN_DARWIN=
if [ "$(uname)" == "Darwin" ]; then
    IN_DARWIN=1
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    IN_LINUX=1
elif [ "$(expr substr $(uname -s) 1 9)" == "CYGWIN_NT" ]; then
    IN_CYGWIN=1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    IN_MINGW=1
fi

BUILD=1
USE_VS=
USE_GCC=
USE_XCODE=
RUN_CMAKE=
VS_VERSION=vs2015
BUILD_DEBUG=
BUILD_RELEASE=
CMAKE_PATH=
USE_MSBUILD=
MSBUILD_PATH=
ADD_VERSION=
INSTALL_PATH=

if [ "$IN_LINUX" != "" -o "$IN_CYGWIN" != "" -o "$IN_DARWIN" != "" ]; then
    # Set up build size for non-windows
    MACHINE_TYPE=`uname -m`
    if [ ${MACHINE_TYPE} == 'x86_64' ]; then
        BUILD_SIZE=64
        OS_SIZE=64
    else
        BUILD_SIZE=32
        OS_SIZE=32
    fi
else
    # Default to 32 for all other variants
    BUILD_SIZE=32
fi

# Default to colours on
define_colours_on

while getopts ":hvlxcdrs:mM::p:b:i:V-:" opt; do
    case $opt in
        h)
            help
            exit
            ;;
        v)
            USE_VS=1
            ;;
        l)
            USE_GCC=1
            ;;
        x)
            USE_XCODE=1
            ;;
        c)
            RUN_CMAKE=1
            ;;
        s)
            VS_VERSION=$OPTARG
            ;;
        p)
            CMAKE_PATH=$OPTARG
            ;;
        b)
            BUILD_PATH=$OPTARG
            ;;
        d)
            BUILD_DEBUG=1
            ;;
        r)
            BUILD_RELEASE=1
            ;;
        i)
            INSTALL_PATH=$OPTARG
            ;;
        m)

            if [ "$IN_MINGW" != "" -o "$IN_CYGWIN" != "" ]; then
                if [ "$USE_MSBUILD" != "" ]; then
                    usage
                    exit 1
                fi
                MSBUILD_PATH="/cygdrive/c/Windows/Microsoft.NET/Framework/v4.0.30319/MSBuild.exe"
                if [ ! -f $MSBUILD_PATH ]; then
                    echo "Couldn't find MSBuild.exe in default location - please run again with -M and provide path"
                    exit 1
                fi
                USE_MSBUILD=1
             fi
            ;;
        M)
            if [ "$IN_MINGW" != "" -o "$IN_CYGWIN" != "" ]; then
                MSBUILD_PATH=$OPTARG/MSBuild.exe
                if [ ! -f $MSBUILD_PATH ]; then
                    echo "Couldn't find $MSBUILD_PATH"
                    exit 1
                fi
                USE_MSBUILD=1
            fi
            ;;
        V)
            ADD_VERSION=-DHAVE_SVN_VERSION_INC
            ;;
        -)
            case "${OPTARG}" in
                32)
                    BUILD_32=1
                    ;;
                64)
                    BUILD_64=1
                    ;;
                nocolor)
                    define_colours_off
                    ;;
                nocolour)
                    define_colours_off
                    ;;
                nobuild)
                    BUILD=0
                    ;;
            esac;;
        :)
            echo "Required parameter argument missing for -$OPTARG" >&2
            usage
            exit 1
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            usage
            exit 1
            ;;
    esac
done

shift $(($OPTIND - 1))
if [ "$1" = "" ]; then
    PATH_TO_LLVM=$PWD/llvm
elif [ "$1" != "" ]; then
    PATH_TO_LLVM=$1
fi

# Check the arguments
if [ "$VS_VERSION" != "vs2010" -a "$VS_VERSION" != "vs2012" -a "$VS_VERSION" != "vs2013" -a "$VS_VERSION" != "vs2015" -a "$VS_VERSION" != "vs2017" ]; then
    echo "Illegal vs version $VS_VERSION"
    usage
    exit 1
fi

if [ "$USE_VS" != "" -a "$USE_GCC" != "" ]; then
    echo "Specify either Visual Studio or gcc (-v or -l) not both"
    usage
    exit 1
fi

if [ "$BUILD_DEBUG" != "" -a "$BUILD_RELEASE" != "" ]; then
    echo "Only one of -d (debug) and -r (release)"
    usage
    exit 1
fi

# Set default if neither are set
if [ "$BUILD_DEBUG" = "" -a "$BUILD_RELEASE" = "" ]; then
    BUILD_RELEASE=1
fi

if [ "$USE_VS" = "" -a "$USE_XCODE" = "" -a "$USE_GCC" = "" ]; then
    if [ "$IN_CYGWIN" != "" ]; then
        USE_VS=1
    elif [ "$IN_MINGW" != "" ]; then
        USE_VS=1
    else
        USE_GCC=1
    fi
fi

if [ "$BUILD_32" != ""  -a "$BUILD_64" != "" ]; then
    echo "Can't specify 32 *and* 64 bit at the same time"
    usage
    exit 1
fi

if [ "$INSTALL_PATH" != "" ]; then
   if [[ "$INSTALL_PATH" != /* ]]; then
       echo "install-path must be an absolute path"
       exit 1
   fi
fi

case $BUILD_SIZE in
    32)
        if [ "$BUILD_64" != "" ]; then
            CROSS_BUILD=1
            BUILD_SIZE=64
            GCC_CMAKE_EXTRA_STATE="export CFLAGS=-m64 CXXFLAGS=-m64"
            if [ "$IN_CYGWIN" != "" -o "$IN_MINGW" ]; then
                echo "Can't cross compile cygwin or mingw gcc 64 bit"
                exit 1
            fi
        fi
        ;;
    64)
        if [[ $BUILD_32 -eq 1 ]]; then
            CROSS_BUILD=1
            BUILD_SIZE=32
            GCC_CMAKE_EXTRA_STATE="export CFLAGS=-m32 CXXFLAGS=-m32"
        fi
        ;;
esac

if [ "$BUILD_PATH" = "" ]; then
    if [ "$USE_VS" != "" ]; then
        BUILD_PATH="build.$BUILD_SIZE.$VS_VERSION"
    elif [ "$IN_LINUX" != "" ]; then
        if [ "$BUILD_DEBUG" != "" ]; then
            BUILD_PATH="build.$BUILD_SIZE.debug.linux"
        else
            BUILD_PATH="build.$BUILD_SIZE.linux"
        fi
    elif [ "$IN_DARWIN" != "" ]; then
        if [ "$USE_XCODE" != "" ]; then
            BUILD_PATH="build.xcode.$BUILD_SIZE"
        else
            if [ "$BUILD_DEBUG" != "" ]; then
                BUILD_PATH="build.$BUILD_SIZE.debug.darwin"
            else
                BUILD_PATH="build.$BUILD_SIZE.darwin"
            fi
        fi
    else
        if [ "$BUILD_DEBUG" != "" ]; then
            BUILD_PATH="build.$BUILD_SIZE.debug.gcc"
        else
            BUILD_PATH="build.$BUILD_SIZE.gcc"
        fi
    fi
fi

if [ "$CMAKE_PATH" = "" ]; then
    if [ "$USE_VS" != "" ]; then
        if [ "$IN_CYGWIN" != "" ]; then
            if [ -d "/cygdrive/c/Program Files/CMake/bin" ]; then
                CMAKE_PATH="/cygdrive/c/Program Files/CMake/bin"
            elif [ -d "/cygdrive/c/Program Files (x86)/CMake/bin" ]; then
                CMAKE_PATH="/cygdrive/c/Program Files (x86)/CMake/bin"
            elif [ -d "/cygdrive/c/Program Files (x86)/CMake 2.8/bin" ]; then
                CMAKE_PATH="/cygdrive/c/Program Files (x86)/CMake 2.8/bin"
            elif [ -d "/cygdrive/c/Program Files (x86)/CMake 2.8.12/bin" ]; then
                CMAKE_PATH="/cygdrive/c/Program Files (x86)/CMake 2.8.12/bin"
            elif [ -d "/cygdrive/c/Program Files (x86)/CMake 3.1.0/bin" ]; then
                CMAKE_PATH="/cygdrive/c/Program Files (x86)/CMake 3.1.0/bin"
            elif [ -d "/cygdrive/c/Program Files (x86)/CMake 3.2.1/bin" ]; then
                CMAKE_PATH="/cygdrive/c/Program Files (x86)/CMake 3.2.1/bin"
            else
                echo "Can't find cmake (and I've looked in quite a few places)"
                exit 1
            fi
        elif [ "$IN_MINGW" != "" ]; then
            CMAKE_PATH="/c/Program Files (x86)/CMake 2.8/bin"
        else
            echo "Can't make VS solutions from this shell - exiting"
            exit 1
        fi
    else
        if [ "$IN_DARWIN" != "" ]; then
            CMAKE_PATH=/opt/local/bin
            if [ ! -f "$CMAKE_PATH/cmake" ]; then
                CMAKE_PATH=/Applications/CMake.app/Contents/bin
            fi
        else
            CMAKE_PATH=/usr/bin
        fi
    fi
fi

if [ ! -f "$CMAKE_PATH/cmake" ]; then
    echo "cmake does not exist at location $CMAKE_PATH" >&2
    usage
    exit 1
fi

CMAKE_STRING=
VS_WIN64=
if [ "$BUILD_64" != "" ]; then
    VS_WIN64=" Win64"
fi
if [ "$USE_VS" != "" ]; then
    case "$VS_VERSION" in
        vs2010)
            CMAKE_STRING="Visual Studio 10$VS_WIN64"
            ;;
        vs2012)
            CMAKE_STRING="Visual Studio 11$VS_WIN64"
            ;;
        vs2013)
            CMAKE_STRING="Visual Studio 12$VS_WIN64"
            ;;
        vs2015)
            CMAKE_STRING="Visual Studio 14$VS_WIN64"
            ;;
        vs2017)
            CMAKE_STRING="Visual Studio 15$VS_WIN64"
            ;;
    esac
elif [ "$USE_XCODE" != "" ]; then
    CMAKE_STRING="Xcode"
else
    CMAKE_STRING="Unix Makefiles"
fi

# Check for combinations which are known not to work
if [ "$OS_SIZE" = "32" -a "$IN_CYGWIN" != "" -a "$BUILD_DEBUG" != "" -a "$USE_GCC" != "" ]; then
    echo "Can't build debug version using 32 bit cygwin (memory issues for clang build). Use 64 bit cygwin instead"
    exit 1
fi

if [ ! -d $PWD/llvm ] ; then
    echo "By default invoke the script from the parent directory of the llvm/doc/test repos"
    echo "e.g. test/build.bash <your arguments here>"
    echo "Or specify the path to llvm as an argument to the script\n"
    usage
    exit 1
fi

printf "Running script with the following properties:\n"
if [ "$USE_VS" != "" ]; then
    printf "${Green}Visual Studio${Colour_Off} mode for ${BGreen}$VS_VERSION${Colour_Off}\n"
elif [ "$USE_XCODE" != "" ]; then
    printf "${Green}XCode${Colour_Off} mode\n"
else
    printf "${Green}Linux / cygwin gcc / mingw gcc mode${Colour_Off}\n"
fi

if [ "$RUN_CMAKE" != "" ]; then
    printf "${BGreen}Running cmake${Colour_Off}\n"
fi
if [ "$BUILD_DEBUG" != "" ]; then
    printf "${Green}Configuration${Colour_Off} ${BGreen}Debug${Colour_Off}\n"
else
    printf "${Green}Configuration${Colour_Off} ${BGreen}Release${Colour_Off}\n"
fi

printf "${Green}Cmake${Colour_Off} being used : ${BGreen}$CMAKE_PATH/cmake${Colour_Off} in out of source dir ${BGreen}$BUILD_PATH${Colour_Off}\n"

if [ ! -d "$BUILD_PATH" ]; then
    mkdir $BUILD_PATH
    RUN_CMAKE=1
fi

# because an earlier use of build.bash used -V, act as if -V is on anyway.
# Otherwise we could get misleading version information.
if [ "$IN_DARWIN" != "" ]; then
    incfile=$(python -c 'import os,sys;print os.path.realpath(sys.argv[1])' "$BUILD_PATH/tools/clang/include/SVNVersion.inc")
else
    incfile="`realpath "$BUILD_PATH"`/tools/clang/include/SVNVersion.inc"
fi
[ -f "$incfile" ] || ADD_VERSION=-DHAVE_SVN_VERSION_INC
if [ "$ADD_VERSION" ]; then
    # Create SVNVersion.inc
    (
        cd "$PATH_TO_LLVM/tools/clang"
        clangver=local
        if [ `git status -uno --porcelain | wc -l` = 0 ]; then
            # No local changes. We can get the git version.
            clangver=`git rev-parse --short HEAD`
        fi
        cd "$PATH_TO_LLVM"
        llvmver=local
        if [ `git status -uno --porcelain | wc -l` = 0 ]; then
            # No local changes. We can get the git version.
            llvmver=`git rev-parse --short HEAD`
        fi
        mkdir -p "${incfile%/*}" || exit 1
        echo "// autogenerated by ${0##*/}
#define SVN_REVISION \"$cmver.$clangver\"
#define LLVM_REVISION \"$cmver.$llvmver\"" >"$incfile"
    ) || exit 1
fi


# Run cmake if required
if [ "$RUN_CMAKE" != "" ]; then
    OLDDIR=$PWD
    cd $BUILD_PATH

    EXTRA_OPTIONS=
    if [ "$USE_VS" != "" ]; then
        if [ "$IN_CYGWIN" != "" ]; then
            PATH_TO_LLVM="$(cygpath -w "$PATH_TO_LLVM")"
        fi
    elif [ "$USE_GCC" != "" ]; then
        if [ "$BUILD_DEBUG" != "" ]; then
            CMAKE_EXTRA_OPTIONS+=" -DCMAKE_BUILD_TYPE=Debug"
        else
            CMAKE_EXTRA_OPTIONS+=" -DCMAKE_BUILD_TYPE=Release"
        fi
        if [ "$IN_CYGWIN" != "" ]; then
            EXTRA_CXX_FLAGS="-Wno-error=unused-local-typedefs -Xassembler -mbig-obj"
        elif [ "$IN_DARWIN" ]; then
            # I want to turn on -Werror, but I get a base class uninitialized
            # error in a couple of places that -Wno-error=uninitialized does not
            # seem to disable.
            EXTRA_CXX_FLAGS="-Wall"
        else
            # Find out what version of g++ we have available
            GCC_VERSION=$(g++ -v 2>&1 | tail -1 | awk '{print $3}')
            if version_check "4.8.0" $GCC_VERSION; then
                EXTRA_CXX_FLAGS="-Wno-error=unused-local-typedefs"
            else
                EXTRA_CXX_FLAGS=""
            fi
        fi
    else
        # Xcode mode
        echo "Nothing to see here"
    fi
    EXTRA_CXX_FLAGS+=" $ADD_VERSION"

    (
        if [ "$CROSS_BUILD" != "" ]; then
            if [ "$IN_LINUX" != "" -o "$IN_CYGWIN" != "" -o "$IN_MINGW" != "" ]; then
                $GCC_CMAKE_EXTRA_STATE
            fi
        fi
        echo "$CMAKE_PATH/cmake" -G "$CMAKE_STRING" $CMAKE_EXTRA_OPTIONS -DCMAKE_CXX_FLAGS="$EXTRA_CXX_FLAGS" -DLLVM_TARGETS_TO_BUILD=GenX\;X86 "$PATH_TO_LLVM"
        if ! "$CMAKE_PATH/cmake" -G "$CMAKE_STRING" $CMAKE_EXTRA_OPTIONS -DCMAKE_CXX_FLAGS="$EXTRA_CXX_FLAGS" -DLLVM_TARGETS_TO_BUILD=GenX\;X86 "$PATH_TO_LLVM" ; then
            echo "Cmake didn't work - exiting"
            exit 1
        fi
    )

    cd $OLDDIR
fi

if [ ! -d $BUILD_PATH ]; then
    echo "Could not find the build path specified ($BUILD_PATH) - do you need to run cmake first? (option -c)"
    usage
    exit 1
fi
if [ ! -f $BUILD_PATH/CMakeCache.txt ]; then
    echo "Could not find the CMakeCache.txt in the build path specified - do you need to run cmake first (option -c)"
    usage
    exit 1
fi

# Next to build the required tools
if [ "$USE_VS" != "" ]; then
    if [ "$USE_MSBUILD" != "" ]; then
        # Build for Visual Studio with msbuild
        case "$VS_VERSION" in
            vs2010) export VisualStudioVersion=10.0;;
            vs2012) export VisualStudioVersion=11.0;;
            vs2013) export VisualStudioVersion=12.0;;
            vs2015) export VisualStudioVersion=14.0;;
            vs2017) export VisualStudioVersion=15.0;;
            *) echo "Unknown VS version $VS_VERSION"; exit 1;;
        esac
        if [ "$BUILD_DEBUG" != "" ]; then
            BUILD_STRING="/p:Configuration=Debug /verbosity:minimal /nologo"
        else
            BUILD_STRING="/p:Configuration=Release /verbosity:minimal /nologo"
        fi

        cd $BUILD_PATH
    else
        # Build for Visual Studio with devenv
        # First we need to find devenv - cmake should have done the work for us already
        # Extract from CMakeCache.txt
        VS_PATH=$(grep CMAKE_MAKE_PROGRAM:FILEPATH $BUILD_PATH/CMakeCache.txt | awk 'BEGIN {FS="="} {print $2}')
        if [ "$BUILD_DEBUG" != "" ]; then
            BUILD_STRING="Debug"
        else
            BUILD_STRING="Release"
        fi

        cd $BUILD_PATH
    fi
elif [ "$USE_XCODE" != "" ]; then
    cd $BUILD_PATH
    if ! hash xcpretty 2>/dev/null; then
        echo "Consider installing and using ruby gem xcpretty - it makes the output a lot better"
    else
        USE_XCPRETTY=1
    fi
    if [ "$BUILD_DEBUG" != "" ]; then
        BUILD_STRING="-configuration Debug"
    else
        BUILD_STRING="-configuration Release"
    fi
elif [ "$USE_GCC" != "" ]; then
    # Build for gcc (cygwin or Linux)
    cd $BUILD_PATH
fi

if [ "$BUILD" != "0" ]; then
    # Now to actually build the individual projects we require
    build_item llvm-as || exit 1
    build_item llvm-dis || exit 1
    build_item opt || exit 1
    build_item llc || exit 1
    build_item clang tools/clang/tools/driver || exit 1
    build_item FileCheck utils/Filecheck || exit 1

    if [ "$INSTALL_PATH" != "" ]; then
        if [ ! -d "$INSTALL_PATH" ]; then
            mkdir $INSTALL_PATH
        fi
        if [ ! -d "$INSTALL_PATH/include" ]; then
            mkdir $INSTALL_PATH/include
        fi
        if [ ! -d "$INSTALL_PATH/bin" ]; then
            mkdir $INSTALL_PATH/bin
        fi
        cp -Lf bin/cmc $INSTALL_PATH/bin/cmc
        cp -rf ../support/include/cm $INSTALL_PATH/include/
    fi
fi
