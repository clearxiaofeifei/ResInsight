cmake_minimum_required(VERSION 3.15)

include(CheckCSourceCompiles)
include(FetchContent)

project(ResInsight)

set(VIZ_MODULES_FOLDER_NAME Fwk/VizFwk)

cmake_policy(SET CMP0020 NEW)
if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 OLD)
endif()

if(UNIX)
  option(
    RESINSIGHT_PREFER_LEGACY_OPENGL
    "Link with Legacy OpenGL libraries. This may be necessary in some virtualization environments"
    ON
  )
  if(RESINSIGHT_PREFER_LEGACY_OPENGL)
    set(OpenGL_GL_PREFERENCE LEGACY)
  endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL "ERT: Build shared libraries"
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(RESINSIGHT_BUNDLE_TESTMODELS "Copy TestModels into the installation" OFF)
mark_as_advanced(RESINSIGHT_BUNDLE_TESTMODELS)

option(RESINSIGHT_VCPKG_AUTO_INSTALL
       "Automatically download build pre-requisites with VCPKG" OFF
)
mark_as_advanced(RESINSIGHT_VCPKG_AUTO_INSTALL)

if(RESINSIGHT_VCPKG_AUTO_INSTALL)
  include(AutoVcpkg)
  set(AUTO_VCPKG_ROOT "${CMAKE_SOURCE_DIR}/ThirdParty/vcpkg")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ##############################################################################
# Setup the main platform defines
# ##############################################################################
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DCVF_LINUX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DCVF_OSX)
elseif(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  set(_HAS_STD_BYTE 0)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_CXX_FLAGS
      "-DCVF_LINUX -pipe -Wextra -Woverloaded-virtual -Wformat -Wno-unused-parameter"
  )
  set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -g3 -O0 -DDEBUG -D_DEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNO_DEBUG")
endif()

# ##############################################################################
# OpenMP
# ##############################################################################
option(RESINSIGHT_USE_OPENMP "Enable OpenMP parallellization in the code" ON)
if(RESINSIGHT_USE_OPENMP)
  find_package(OpenMP)
  if(OPENMP_FOUND)
    message(STATUS "Enabling OpenMP support")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -DUSE_OPENMP")
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}"
    )
  else()
    message(STATUS "Disabling OpenMP support")
  endif()
endif()

# ##############################################################################
# buildcache
# ##############################################################################
find_program(buildcache_program buildcache)
if(buildcache_program)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${buildcache_program}")
  message(STATUS "Found buildcache from : ${buildcache_program}")

  if(MSVC)
    # New concept for defining MSVC debug info flags introduced in 3.25.
    # Evaluate this concept to simplify the setting of CXX-flags
    # https://cmake.org/cmake/help/v3.25/prop_tgt/MSVC_DEBUG_INFORMATION_FORMAT.html#prop_tgt:MSVC_DEBUG_INFORMATION_FORMAT
    # https://cmake.org/cmake/help/v3.25/policy/CMP0141.html#policy:CMP0141

    # It is required to use compiler flag /Z7 to be able to use buildcache
    # https://github.com/mbitsnbites/buildcache/blob/master/doc/usage.md
    # https://learn.microsoft.com/en-us/cpp/build/reference/z7-zi-zi-debug-information-format?view=msvc-170

    string(REGEX REPLACE "/Zi" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REGEX REPLACE "/Zi" "" CMAKE_CXX_FLAGS_DEBUG
                         "${CMAKE_CXX_FLAGS_DEBUG}"
    )
    string(REGEX REPLACE "/Zi" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO
                         "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
    )

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Z7")
  endif()
else()
  message(STATUS "No buildcache found")
endif()

# ##############################################################################
# ABAQUS ODB
# ##############################################################################
set(RESINSIGHT_ODB_API_DIR
    ""
    CACHE
      PATH
      "Optional path to the ABAQUS ODB API from Simulia. Needed for support of geomechanical models"
)

if(MSVC)
  option(RESINSIGHT_DONWLOAD_ODB_FROM_SERVER
         "Download ODB library from inhouse build server" OFF
  )
  mark_as_advanced(FORCE RESINSIGHT_DONWLOAD_ODB_FROM_SERVER)

  if(RESINSIGHT_DONWLOAD_ODB_FROM_SERVER)
    FetchContent_Declare(
      odb-library-from-server
      URL http://10.10.0.26:8080/job/resinsight-dependencies/ws/odb-api-2020.zip
    )

    FetchContent_Populate(odb-library-from-server)

    set(RESINSIGHT_ODB_API_DIR ${odb-library-from-server_SOURCE_DIR})
  endif()
endif()

if(NOT ${RESINSIGHT_ODB_API_DIR} EQUAL "")
  add_definitions(-DUSE_ODB_API)
  set(RESINSIGHT_USE_ODB_API 1)
  message(STATUS "Using ODB-Api from : ${RESINSIGHT_ODB_API_DIR}")
endif()

# ##############################################################################
# Version number
# ##############################################################################
include(ResInsightVersion.cmake)

# ##############################################################################
# Octave
# ##############################################################################
find_package(Octave)

# ##############################################################################
# Grpc
# ##############################################################################
option(RESINSIGHT_ENABLE_GRPC "Enable the gRPC scripting framework" OFF)
if(RESINSIGHT_ENABLE_GRPC)
  option(
    RESINSIGHT_GRPC_DOWNLOAD_PYTHON_MODULE
    "Download the gRPC python modules to enable generation of Python interface"
    ON
  )
  option(RESINSIGHT_GRPC_BUNDLE_PYTHON_MODULE
         "Bundle the gRPC python modules into the install folder" OFF
  )
  add_definitions(-DENABLE_GRPC)
endif()

# ##############################################################################
# Unity Build
# ##############################################################################

# CMAKE_UNITY_BUILD was introduced in CMake 3.16.2
option(RESINSIGHT_ENABLE_UNITY_BUILD
       "Experimental speedup of compilation using CMake Unity Build" OFF
)
mark_as_advanced(FORCE RESINSIGHT_ENABLE_UNITY_BUILD)
if(RESINSIGHT_ENABLE_UNITY_BUILD)
  set(CAF_ENABLE_UNITY_BUILD true)
  set(CVF_ENABLE_UNITY_BUILD true)
endif()

# ##############################################################################
# ERT
# ##############################################################################

# Use of CMAKE_CXX_COMPILER_LAUNCHER is not working with libecl. Disable by
# temporarily setting compiler launcher to nothing
set(TEMP_CMAKE_CXX_COMPILER_LAUNCHER ${CMAKE_CXX_COMPILER_LAUNCHER})
set(CMAKE_CXX_COMPILER_LAUNCHER)

if(NOT MSVC)
  # Linux: Optional configuration of externally installed ERT, requires path to
  # libraries and includes
  set(RESINSIGHT_ERT_EXTERNAL_LIB_ROOT
      ""
      CACHE PATH "Path to installed ERT libraries"
  )
  set(RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT
      ""
      CACHE PATH "Path to installed ERT includes"
  )
endif()

if(RESINSIGHT_ERT_EXTERNAL_LIB_ROOT OR RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT)
  if(NOT (RESINSIGHT_ERT_EXTERNAL_LIB_ROOT
          AND RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT)
  )
    message(
      FATAL_ERROR
        "Both RESINSIGHT_ERT_EXTERNAL_LIB_ROOT and RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT must be defined"
    )
  endif()

  message(
    FATAL_ERROR
      "TODO: Building using and external system installed ERT is broken."
  )

  list(APPEND ERT_INCLUDE_DIRS ${RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT})

  list(APPEND ERT_LIBRARIES ${RESINSIGHT_ERT_EXTERNAL_LIB_ROOT}/libecl.so)

else()

  # Disable install of ERT libs and headers, as Ert code is compiled and linked
  # directly
  set(INSTALL_ERT
      OFF
      CACHE BOOL "ERT: Install library"
  )
  set(ERT_USE_OPENMP
      ${OPENMP_FOUND}
      CACHE BOOL "ERT: Compile using OpenMP"
  )

  # Remember original state
  set(ORIGINAL_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

  if(MSVC)
    # Force static linking on Windows
    set(BUILD_SHARED_LIBS OFF)
    set(ERT_HAVE_UNISTD OFF) # If anyone has mingw installed
  else()
    set(RESINSIGHT_ERT_EXTERNAL_SOURCE_ROOT
        ""
        CACHE STRING "Path to ERT CMakeList.txt (source path)"
    )

    # Force dynamic linking on other platforms Copy of libraries into install
    # folder of ResInsight is done a bit further down in this file
    set(BUILD_SHARED_LIBS ON)
  endif()

  if(RESINSIGHT_ERT_EXTERNAL_SOURCE_ROOT)
    add_subdirectory(
      ${RESINSIGHT_ERT_EXTERNAL_SOURCE_ROOT} ${CMAKE_BINARY_DIR}/ThirdParty/Ert
    )
  else()
    add_subdirectory(ThirdParty/Ert)
  endif()

  if(MSVC)
    # libecl : Disable some warnings
    set_target_properties(
      ecl
      PROPERTIES
        COMPILE_FLAGS
        "/wd4244 /wd4267 /wd4013 /wd4190 /wd4018 /wd4477 /wd4098 /wd4293 /wd4305 /wd4020 /wd4028 /wd4715 /wd4245 /wd4804 /wd4100 /wd4456 /wd4458 /wd4090 /wd4297 /wd4701 /wd4101 /wd4702 /wd4457"
    )
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_target_properties(
      ecl
      PROPERTIES
        COMPILE_FLAGS
        "-Wno-deprecated -Wno-deprecated-declarations -Wno-sign-compare"
    )
  else()
    set_target_properties(
      ecl
      PROPERTIES
        COMPILE_FLAGS
        "-Wno-deprecated -Wno-deprecated-declarations -Wno-clobbered -Wno-int-in-bool-context"
    )

    target_compile_options(
      ecl PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wno-class-memaccess>
    )

  endif()

  list(APPEND THIRD_PARTY_LIBRARIES ecl)

  set_property(TARGET catch2 PROPERTY FOLDER "Thirdparty")

  # Restore original state
  set(BUILD_SHARED_LIBS ${ORIGINAL_BUILD_SHARED_LIBS})

endif(RESINSIGHT_ERT_EXTERNAL_LIB_ROOT OR RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT)

# restore buildcache when libecl has been configured
set(CMAKE_CXX_COMPILER_LAUNCHER ${TEMP_CMAKE_CXX_COMPILER_LAUNCHER})

# ##############################################################################
# Init GIT submodules if they haven't already #
# ##############################################################################
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  # Update submodules as needed
  option(RESINSIGHT_UPDATE_SUBMODULES "Check submodules during build" ON)
  mark_as_advanced(RESINSIGHT_UPDATE_SUBMODULES)

  if(RESINSIGHT_UPDATE_SUBMODULES)
    set(SUBDIR "${PROJECT_SOURCE_DIR}/ThirdParty")
    message(STATUS "Initializing GIT submodules")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${SUBDIR}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE GIT_SUBMOD_RESULT
    )
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(
        FATAL_ERROR
          "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules"
      )
    endif()
  endif()
endif()

# ##############################################################################
# OpenVDS seismic file access
# ##############################################################################

if(MSVC)
  FetchContent_Declare(
    openvds
    URL https://bluware.jfrog.io/artifactory/Releases-OpenVDSPlus/3.2/openvds+-3.2.3-win.zip
  )
else()
  FetchContent_Declare(
    openvds
    URL https://bluware.jfrog.io/artifactory/Releases-OpenVDSPlus/3.2/openvds+-3.2.3-manylinux_2014.tar.gz
  )
endif()

FetchContent_Populate(openvds)

set(RESINSIGHT_OPENVDS_API_DIR ${openvds_SOURCE_DIR})
message(STATUS "Using OpenVDS api from : ${RESINSIGHT_OPENVDS_API_DIR}")

if(MSVC)
  list(APPEND EXTERNAL_LINK_LIBRARIES
       ${RESINSIGHT_OPENVDS_API_DIR}/lib/msvc_141/openvds.lib
  )
else()
  list(APPEND EXTERNAL_LINK_LIBRARIES
       ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libopenvds.so
  )
endif()

# ##############################################################################
# HDF5
# ##############################################################################

if(MSVC)
  # this option is disabled in the daily build defined in
  # ResInsightWithCache.yml, as the text input to ninja becomes too long
  option(RESINSIGHT_ENABLE_HDF5 "Use external HDF5 library" ON)

  if(RESINSIGHT_ENABLE_HDF5)
    FetchContent_Declare(
      hdf-external-lib
      URL https://github.com/CeetronSolutions/resinsight-dependencies/releases/download/2023.04/HDF_Group.zip
    )
    FetchContent_Populate(hdf-external-lib)
    set(RESINSIGHT_HDF5_DIR ${hdf-external-lib_SOURCE_DIR}/HDF5/1.8.18)
  endif()

  if(NOT ${RESINSIGHT_HDF5_DIR} EQUAL "")
    list(APPEND EXTERNAL_LINK_LIBRARIES ${RESINSIGHT_HDF5_DIR}/lib/hdf5.lib
         ${RESINSIGHT_HDF5_DIR}/lib/hdf5_cpp.lib
    )

    set(RESINSIGHT_FOUND_HDF5 1)
    message(STATUS "Using HDF5 from : ${RESINSIGHT_HDF5_DIR}")
  else()
    message(
      WARNING
        "Use of HDF5 is enabled, but RESINSIGHT_HDF5_DIR is empty. Specify RESINSIGHT_HDF5_DIR to be able to use HDF5"
    )
  endif()
else()
  find_package(HDF5 COMPONENTS CXX)
  if(HDF5_FOUND)
    list(APPEND EXTERNAL_LINK_LIBRARIES ${HDF5_LIBRARIES})

    set(RESINSIGHT_FOUND_HDF5 1)
    message(STATUS "Using HDF5 libraries : ${HDF5_LIBRARIES}")

    option(RESINSIGHT_HDF5_BUNDLE_LIBRARIES "Bundle HDF5 libraries" OFF)
    mark_as_advanced(FORCE RESINSIGHT_HDF5_BUNDLE_LIBRARIES)
    if(RESINSIGHT_HDF5_BUNDLE_LIBRARIES)
      message(STATUS "Bundling of HDF5 libraries is enabled")
    endif() # RESINSIGHT_HDF5_BUNDLE_LIBRARIES

  else()
    message(WARNING "Use of HDF5 is enabled, but no HDF5 is found.")
  endif() # HDF5_FOUND
endif() # MSVC

# ##############################################################################
# Opm
# ##############################################################################

if(RESINSIGHT_VCPKG_AUTO_INSTALL)
  vcpkg_install(boost-filesystem)
  vcpkg_install(boost-spirit)
  include(${CMAKE_TOOLCHAIN_FILE})
endif()

add_subdirectory(ThirdParty/custom-opm-flowdiagnostics)
add_subdirectory(ThirdParty/custom-opm-flowdiag-app)

# Option used to build opm-common from source or use precompiled binaries
option(RESINSIGHT_BUILD_LIBS_FROM_SOURCE "Build opm-common from source" ON)
mark_as_advanced(RESINSIGHT_BUILD_LIBS_FROM_SOURCE)

if((NOT RESINSIGHT_BUILD_LIBS_FROM_SOURCE) AND MSVC)
  FetchContent_Declare(
    ri-dependencies
    URL https://github.com/CeetronSolutions/resinsight-dependencies/releases/download/2023.04/custom-opm-common.zip
  )
  FetchContent_Populate(ri-dependencies)
  list(APPEND EXTERNAL_LINK_LIBRARIES
       ${ri-dependencies_SOURCE_DIR}/custom-opm-common.lib
  )
  message(STATUS "opm-common: Enabled use of precompiled library")
else()
  add_subdirectory(ThirdParty/custom-opm-common)
  add_subdirectory(ThirdParty/custom-opm-common/custom-opm-parser-tests)
  list(APPEND OPM_LIBRARIES custom-opm-common)
  set_property(TARGET opm-parser-tests PROPERTY FOLDER "Thirdparty/OPM")
endif()

list(APPEND OPM_LIBRARIES custom-opm-flowdiagnostics custom-opm-flowdiag-app)
set_property(TARGET ${OPM_LIBRARIES} PROPERTY FOLDER "Thirdparty/OPM")

# ##############################################################################
# NRLib
# ##############################################################################

add_subdirectory(ThirdParty/NRLib)

list(APPEND THIRD_PARTY_LIBRARIES NRLib)

# ##############################################################################
# openzgy
# ##############################################################################

add_definitions(-DOPENZGY_STATIC)
add_subdirectory(ThirdParty/openzgy)

list(APPEND THIRD_PARTY_LIBRARIES openzgy)
set_property(TARGET zfp PROPERTY FOLDER "Thirdparty")
if(TARGET openzgy-tests)
  set_property(TARGET openzgy-tests PROPERTY FOLDER "Thirdparty")
endif()

# ##############################################################################
# Qt
# ##############################################################################
set(RI_QT_MINIMUM_VERSION 5.12)

find_package(
  Qt5 ${RI_QT_MINIMUM_VERSION} COMPONENTS Core Gui OpenGL Network Widgets
)

# Open GL
find_package(OpenGL)

# ##############################################################################
# QtCharts
# ##############################################################################
option(RESINSIGHT_USE_QT_CHARTS "Enable QtCharts in the code" ON)
if(RESINSIGHT_USE_QT_CHARTS)
  find_package(Qt5 OPTIONAL_COMPONENTS Charts)

  if(Qt5Charts_FOUND)
    message(STATUS "QtCharts found: ${Qt5Charts_LIBRARIES}")
    add_definitions(-DUSE_QTCHARTS)
  else()
    message(STATUS "QtCharts not found.")
    set(RESINSIGHT_USE_QT_CHARTS OFF)
  endif()
endif()

# ##############################################################################
# Qwt
# ##############################################################################

add_subdirectory(ThirdParty/qwt)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(
    qwt PRIVATE -Wno-deprecated-copy -Wno-deprecated-enum-float-conversion
                -Wno-deprecated-enum-enum-conversion
  )
elseif(MSVC)
  target_compile_options(qwt PRIVATE /wd4996 /wd4005)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(qwt PRIVATE -Wno-deprecated)
endif()

list(APPEND THIRD_PARTY_LIBRARIES qwt)

# ##############################################################################
# Qt Advanced Docking System
# ##############################################################################
set(ADS_VERSION "4.0.3")

add_subdirectory(ThirdParty/qtadvanceddocking EXCLUDE_FROM_ALL)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(qt5advanceddocking PRIVATE -Wno-deprecated-copy)
elseif(MSVC)
  target_compile_options(qt5advanceddocking PRIVATE /wd4996 /wd4005)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(qt5advanceddocking PRIVATE -Wno-deprecated)
endif()

list(APPEND THIRD_PARTY_LIBRARIES qt5advanceddocking)

# ##############################################################################
# Nightcharts
# ##############################################################################

add_subdirectory(ThirdParty/nightcharts)
if(MSVC)
  target_compile_options(nightcharts PRIVATE /wd4996)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(nightcharts PRIVATE -Wno-deprecated)
endif()

list(APPEND THIRD_PARTY_LIBRARIES nightcharts)

# ##############################################################################
# C++ Mathematical Expression Parsing And Evaluation Library
# ##############################################################################

add_subdirectory(ThirdParty/expressionparser)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(expressionparser PUBLIC -Wno-overloaded-virtual)
endif()

list(APPEND THIRD_PARTY_LIBRARIES expressionparser)

# ##############################################################################
# clipper
# ##############################################################################

add_subdirectory(ThirdParty/clipper)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(clipper PUBLIC -Wno-deprecated-copy)
endif()

list(APPEND THIRD_PARTY_LIBRARIES clipper)

# ##############################################################################
# roffcpp
# ##############################################################################

add_subdirectory(ThirdParty/roffcpp)
list(APPEND THIRD_PARTY_LIBRARIES roffcpp)
set_property(TARGET roffcpp-tests gtest gtest_main PROPERTY FOLDER "Thirdparty")

# ##############################################################################
# regression-analysis
# ##############################################################################

set(REGRESSION_FETCH_EIGEN false)
add_subdirectory(ThirdParty/regression-analysis)
list(APPEND THIRD_PARTY_LIBRARIES regression-analysis)
set_property(
  TARGET regression-analysis-tests gtest gtest_main PROPERTY FOLDER
                                                             "Thirdparty"
)

# ##############################################################################
# Thirdparty libraries are put in ThirdParty solution folder
# ##############################################################################
set_property(TARGET ${THIRD_PARTY_LIBRARIES} PROPERTY FOLDER "Thirdparty")

# ##############################################################################
# Unity Build
# ##############################################################################
if(RESINSIGHT_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : qwt")
  set_property(TARGET qwt PROPERTY UNITY_BUILD true)
  message("Cmake Unity build is enabled on : custom-opm-flowdiagnostics")
  set_property(TARGET custom-opm-flowdiagnostics PROPERTY UNITY_BUILD true)
  # message("Cmake Unity build is enabled on : custom-opm-common")
  # set_property(TARGET custom-opm-common PROPERTY UNITY_BUILD true)
  message("Cmake Unity build is enabled on : qtadvanceddocking")
  set_property(TARGET qt5advanceddocking PROPERTY UNITY_BUILD true)
endif()

# ##############################################################################
# Vizualization Framework
# ##############################################################################

# Allow use of non-threadsafe reference counter in cvf::Object on systems with
# no atomics support
if(CMAKE_COMPILER_IS_GNUCC)

  if(NOT DEFINED HAVE_GCC_SYNC_FUNCTIONS)
    check_c_source_compiles(
      "int main(int argc, char **argv) {
            int a;
            __sync_add_and_fetch(&a, 1);
            __sync_fetch_and_add(&a, 1);
            __sync_sub_and_fetch(&a, 1);
            __sync_fetch_and_sub(&a, 1); }"
      HAVE_GCC_SYNC_FUNCTIONS
    )
  endif()

  if(HAVE_GCC_SYNC_FUNCTIONS)
    message(STATUS "GCC synchronization functions detected")
  else()
    message(
      STATUS
        "GCC synchronization functions NOT detected, fallback to non threadsafe reference counting"
    )
    add_definitions(-DCVF_USE_NON_THREADSAFE_REFERENCE_COUNT)
  endif()

endif()

add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibCore)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibGeometry)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibRender)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibViewing)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibGuiQt)

list(
  APPEND
  VIZ_FWK_LIBRARIES
  LibGuiQt
  LibViewing
  LibRender
  LibGeometry
  LibCore
)

if(MSVC)
  target_compile_options(LibGuiQt PRIVATE /wd4996)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(
    LibRender PRIVATE -Wno-undefined-var-template -Wno-invalid-source-encoding
                      -Wno-null-pointer-arithmetic
  )
endif()

set_property(TARGET ${VIZ_FWK_LIBRARIES} PROPERTY FOLDER "VizFwk")

# ##############################################################################
# Application Framework
# ##############################################################################

add_subdirectory(Fwk/AppFwk/cafAnimControl)
add_subdirectory(Fwk/AppFwk/cafViewer)

add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmCore)
add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmUiCore)
add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmXml)
add_subdirectory(Fwk/AppFwk/cafProjectDataModel)

add_subdirectory(Fwk/AppFwk/cafCommand)
add_subdirectory(Fwk/AppFwk/cafUserInterface)
add_subdirectory(Fwk/AppFwk/cafPdmCvf)

add_subdirectory(Fwk/AppFwk/CommonCode)
add_subdirectory(Fwk/AppFwk/cafVizExtensions)

option(CAF_CVF_SCRIPTING "" ON)
add_subdirectory(Fwk/AppFwk/cafPdmScripting)
add_subdirectory(Fwk/AppFwk/cafCommandFeatures)

add_subdirectory(Fwk/AppFwk/cafTensor)
add_subdirectory(Fwk/AppFwk/cafHexInterpolator)

if(MSVC)
  target_compile_options(cafAnimControl PRIVATE /wd5054)
  target_compile_options(cafViewer PRIVATE /wd4996 /wd5054)
  target_compile_options(cafPdmCore PRIVATE /wd4996)
  target_compile_options(cafPdmUiCore PRIVATE /wd5054)
  target_compile_options(cafCommandFeatures PRIVATE /wd4996 /wd5054)
  target_compile_options(cafCommand PRIVATE /wd5054)
  target_compile_options(cafPdmXml PRIVATE /wd4996)
  target_compile_options(cafUserInterface PRIVATE /wd4996 /wd5054)
  target_compile_options(qwt PRIVATE /wd5055)
  target_compile_options(CommonCode PRIVATE /wd5054)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(cafCommandFeatures PRIVATE -Wno-deprecated-copy)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(
    CommonCode PRIVATE -Wno-deprecated-copy -Wno-undefined-var-template
  )
  target_compile_options(
    cafCommandFeatures PRIVATE -Wno-deprecated-copy -Wno-undefined-var-template
                               -Wno-deprecated-enum-enum-conversion
  )
  target_compile_options(
    cafVizExtensions PRIVATE -Wno-deprecated-copy -Wno-undefined-var-template
                             -Wno-invalid-source-encoding
  )
  target_compile_options(cafUserInterface PRIVATE -Wno-null-pointer-arithmetic)
endif()

list(
  APPEND
  APP_FWK_LIBRARIES
  cafPdmCore
  cafPdmUiCore
  cafPdmXml
  cafProjectDataModel
  cafUserInterface
  cafViewer
  cafAnimControl
  cafCommand
  cafPdmCvf
  cafTensor
  CommonCode
  cafVizExtensions
  cafPdmScripting
  cafCommandFeatures
)

set_property(TARGET ${APP_FWK_LIBRARIES} PROPERTY FOLDER "AppFwk")

option(RESINSIGHT_INCLUDE_APPFWK_TESTS "Enable AppFwk Tests" OFF)
mark_as_advanced(FORCE RESINSIGHT_INCLUDE_APPFWK_TESTS)

if(RESINSIGHT_INCLUDE_APPFWK_TESTS)
  # Unit Tests
  add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafProjectDataModel_UnitTests)
  add_subdirectory(
    Fwk/AppFwk/cafProjectDataModel/cafPdmCore/cafPdmCore_UnitTests
  )
  add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmXml/cafPdmXml_UnitTests)
  add_subdirectory(Fwk/AppFwk/cafPdmScripting/cafPdmScripting_UnitTests)
  add_subdirectory(Fwk/AppFwk/cafUserInterface/cafUserInterface_UnitTests)

  # Executables
  add_subdirectory(Fwk/AppFwk/cafTests/cafTestApplication)
  add_subdirectory(Fwk/AppFwk/cafTests/cafTestCvfApplication)

  list(
    APPEND
    APP_FWK_TEST_PROJECTS
    cafProjectDataModel_UnitTests
    cafPdmCore_UnitTests
    cafPdmXml_UnitTests
    cafPdmScripting_UnitTests
    cafUserInterface_UnitTests
    cafTestApplication
    cafTestCvfApplication
  )

  set_property(TARGET ${APP_FWK_TEST_PROJECTS} PROPERTY FOLDER "AppFwkTests")

endif()

# ##############################################################################
# Installation settings
# ##############################################################################

set(RESINSIGHT_INSTALL_FOLDER ".")

# override system install prefix if private installation chosen
option(RESINSIGHT_PRIVATE_INSTALL
       "Linux only: Install the libecl shared libraries along the executable"
       ON
)
mark_as_advanced(FORCE RESINSIGHT_PRIVATE_INSTALL)

if(RESINSIGHT_PRIVATE_INSTALL)

  # ############################################################################
  # ERT shared library files Install procedure will copy so-files from ERT into
  # same install folder as ResInsight
  # ############################################################################

  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(ERT_SHARED_LIB_FILES
        ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libecl.so
        ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libecl.so.2
        ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libecl.so.2.4
    )
    install(FILES ${ERT_SHARED_LIB_FILES}
            DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
    )
  endif()

  if(APPLE)
    set(ERT_SHARED_LIB_FILES
        ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libecl.dylib
        ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libecl.2.dylib
        ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libecl.2.4.dylib
    )
    install(
      FILES ${ERT_SHARED_LIB_FILES}
      DESTINATION ${RESINSIGHT_INSTALL_FOLDER}/ResInsight.app/Contents/MacOS
    )
  endif()

endif(RESINSIGHT_PRIVATE_INSTALL)

if(RESINSIGHT_HDF5_BUNDLE_LIBRARIES)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # find all file names with text hdf5 use file globbing to also copy the
    # symlinks to make sure the dependencies from ResInsight runtime is correct
    foreach(FILE_TO_COPY ${HDF5_LIBRARIES})
      string(FIND ${FILE_TO_COPY} "hdf5" POS_IN_STRING)
      if(${POS_IN_STRING} GREATER -1)
        file(GLOB FILE_AND_SYMLINKS ${FILE_TO_COPY}*)
        install(FILES ${FILE_AND_SYMLINKS}
                DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
        )
      endif()
    endforeach()
  endif()
endif(RESINSIGHT_HDF5_BUNDLE_LIBRARIES)

if(RESINSIGHT_BUNDLE_TESTMODELS)
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/TestModels
          DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
  )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(OPENVDS_SHARED_LIB_FILES
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libopenvds.so
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libopenvds.so.3
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libopenvds.so.3.2.3
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libopenvds-968eeb36.so.3.2.3
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libsegyutils.so
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libsegyutils.so.3
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libsegyutils.so.3.2.3
  )
  install(FILES ${OPENVDS_SHARED_LIB_FILES}
          DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
  )

  install(PROGRAMS ${RESINSIGHT_OPENVDS_API_DIR}/bin/SEGYImport
          DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
  )
endif()

# ##############################################################################
# Application
# ##############################################################################
add_subdirectory(ApplicationLibCode)
add_subdirectory(ApplicationLibCode/Commands)
add_subdirectory(ApplicationLibCode/ResultStatisticsCache)
add_subdirectory(ApplicationLibCode/GeoMech/GeoMechDataModel)
if(RESINSIGHT_USE_ODB_API)
  add_subdirectory(ApplicationLibCode/GeoMech/OdbReader)
endif()

if(RESINSIGHT_ENABLE_GRPC)
  add_subdirectory(GrpcInterface)
endif()
add_subdirectory(ApplicationExeCode)

if(OCTAVE_MKOCTFILE)
  message(STATUS "Adding OctavePlugin library")

  add_subdirectory(OctavePlugin)
else(OCTAVE_MKOCTFILE)
  message(
    STATUS "Could not find OCTAVE_MKOCTFILE, skipping OctavePlugin library"
  )
endif(OCTAVE_MKOCTFILE)

# ##############################################################################
# Visual Studio : Create the ruleset file to be used by Static Code Analysis
# https://stackoverflow.com/questions/75031903/how-to-enable-static-analysis-with-custom-ruleset-in-msvc-via-cmakelists-txt
# ##############################################################################

option(RESINSIGHT_ENABLE_STATIC_ANALYSIS "Enable static analysis" OFF)
if(MSVC AND RESINSIGHT_ENABLE_STATIC_ANALYSIS)
  set(RULESET_FILE ${CMAKE_CURRENT_BINARY_DIR}/msvc-ruleset.props)
  configure_file(
    ${CMAKE_SOURCE_DIR}/scripts/static-code-analysis/msvc-property-sheet-template.props
    ${RULESET_FILE}
    @ONLY
  )

  set(TARGETS_FOR_STATIC_ANALYSIS
      ${APP_FWK_LIBRARIES} ${APP_FWK_TEST_PROJECTS} ApplicationLibCode Commands
      ResultStatisticsCache RigGeoMechDataModel
  )

  foreach(TARGET_PROJECT ${TARGETS_FOR_STATIC_ANALYSIS})
    set_property(
      TARGET ${TARGET_PROJECT} PROPERTY VS_USER_PROPS "${RULESET_FILE}"
    )
    target_compile_options(
      ${TARGET_PROJECT} PRIVATE /external:anglebrackets /analyze:external-
    )
  endforeach()
endif(MSVC AND RESINSIGHT_ENABLE_STATIC_ANALYSIS)

# ##############################################################################
# Code documentation using Doxygen
# ##############################################################################
option(RESINSIGHT_BUILD_DOCUMENTATION
       "Use Doxygen to create the HTML based API documentation" OFF
)
if(RESINSIGHT_BUILD_DOCUMENTATION)
  find_package(Doxygen)
  if(NOT DOXYGEN_FOUND)
    message(
      FATAL_ERROR
        "Doxygen is needed to build the documentation. Please install it correctly"
    )
  endif()
  # -- Configure the Template Doxyfile for our specific project
  configure_file(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY IMMEDIATE)
  # -- Add a custom target to run Doxygen when ever the project is built
  add_custom_target(
    Docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
    SOURCES ${PROJECT_BINARY_DIR}/Doxyfile
  )
  # IF you do NOT want the documentation to be generated EVERY time you build
  # the project then leave out the 'ALL' keyword from the above command.
endif(RESINSIGHT_BUILD_DOCUMENTATION)
