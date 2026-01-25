cmake_minimum_required(VERSION 3.22.1)
project(vulkanCore LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(ANDROID)
    add_definitions(-DANDROID)
endif()

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fopenmp")

add_definitions(
        -DVK_USE_PLATFORM_ANDROID_KHR #required
)

# -------------------- POSELIB (STATIC, EMBEDDED) --------------------

add_library(poselib STATIC
        thirdparty/poselib/robust.cc
        thirdparty/poselib/robust/ransac.cc
        thirdparty/poselib/robust/bundle.cc
        thirdparty/poselib/robust/utils.cc
        thirdparty/poselib/robust/sampling.cc

        thirdparty/poselib/solvers/gp3p.cc
        thirdparty/poselib/solvers/gp4ps.cc
        thirdparty/poselib/solvers/p1p2ll.cc
        thirdparty/poselib/solvers/p2p1ll.cc
        thirdparty/poselib/solvers/p2p2pl.cc
        thirdparty/poselib/solvers/p3ll.cc
        thirdparty/poselib/solvers/p3p.cc
        thirdparty/poselib/solvers/p3p_ding.cc
        thirdparty/poselib/solvers/p4pf.cc
        thirdparty/poselib/solvers/p5lp_radial.cc
        thirdparty/poselib/solvers/p6lp.cc
        thirdparty/poselib/solvers/ugp2p.cc
        thirdparty/poselib/solvers/ugp3ps.cc
        thirdparty/poselib/solvers/up1p2pl.cc
        thirdparty/poselib/solvers/up2p.cc
        thirdparty/poselib/solvers/up1p1ll.cc
        thirdparty/poselib/solvers/up4pl.cc
        thirdparty/poselib/solvers/ugp4pl.cc
        thirdparty/poselib/solvers/relpose_upright_3pt.cc
        thirdparty/poselib/solvers/relpose_upright_planar_2pt.cc
        thirdparty/poselib/solvers/relpose_upright_planar_3pt.cc
        thirdparty/poselib/solvers/relpose_8pt.cc
        thirdparty/poselib/solvers/relpose_5pt.cc
        thirdparty/poselib/solvers/relpose_7pt.cc
        thirdparty/poselib/solvers/relpose_6pt_focal.cc
        thirdparty/poselib/solvers/gen_relpose_upright_4pt.cc
        thirdparty/poselib/solvers/gen_relpose_5p1pt.cc
        thirdparty/poselib/solvers/gen_relpose_6pt.cc
        thirdparty/poselib/solvers/homography_4pt.cc

        thirdparty/poselib/misc/qep.cc
        thirdparty/poselib/misc/univariate.cc
        thirdparty/poselib/misc/essential.cc
        thirdparty/poselib/misc/re3q3.cc
        thirdparty/poselib/misc/colmap_models.cc
        thirdparty/poselib/misc/decompositions.cc
)

target_compile_options(poselib PRIVATE
        -g0
        -O0
        -fno-strict-aliasing
        -fno-vectorize
        -fno-slp-vectorize
)

target_compile_definitions(poselib PRIVATE
        EIGEN_DONT_VECTORIZE
        EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT
)

set_target_properties(poselib PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_include_directories(poselib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/poselib
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Eigen
)

# -------------------- SQLITE (STATIC, EMBEDDED) --------------------

add_library(sqlite3 STATIC
        thirdparty/sqlite/sqlite3.c
)

# Android requires PIC for anything linked into a .so
set_target_properties(sqlite3 PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_include_directories(sqlite3 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/sqlite
)

# Optional performance & safety flags
target_compile_definitions(sqlite3 PRIVATE
        SQLITE_THREADSAFE=1
        SQLITE_OMIT_LOAD_EXTENSION
        SQLITE_ENABLE_FTS5
        SQLITE_ENABLE_JSON1
        SQLITE_DEFAULT_MEMSTATUS=0
)

# -------------------- VLFeat (STATIC, EMBEDDED) --------------------

add_library(vlfeat STATIC
        thirdparty/VLFeat/aib.c
        thirdparty/VLFeat/array.c
        thirdparty/VLFeat/covdet.c
        thirdparty/VLFeat/dsift.c
        thirdparty/VLFeat/fisher.c
        thirdparty/VLFeat/generic.c
        thirdparty/VLFeat/getopt_long.c
        thirdparty/VLFeat/gmm.c
        thirdparty/VLFeat/hikmeans.c
        thirdparty/VLFeat/hog.c
        thirdparty/VLFeat/homkermap.c
        thirdparty/VLFeat/host.c
        thirdparty/VLFeat/ikmeans.c
        thirdparty/VLFeat/imopv.c
        thirdparty/VLFeat/kdtree.c
        thirdparty/VLFeat/kmeans.c
        thirdparty/VLFeat/lbp.c
        thirdparty/VLFeat/liop.c
        thirdparty/VLFeat/mathop.c
        thirdparty/VLFeat/mser.c
        thirdparty/VLFeat/pgm.c
        thirdparty/VLFeat/quickshift.c
        thirdparty/VLFeat/random.c
        thirdparty/VLFeat/rodrigues.c
        thirdparty/VLFeat/scalespace.c
        thirdparty/VLFeat/sift.c
        thirdparty/VLFeat/slic.c
        thirdparty/VLFeat/stringop.c
        thirdparty/VLFeat/svm.c
        thirdparty/VLFeat/svmdataset.c
        thirdparty/VLFeat/vlad.c
)

set_target_properties(vlfeat PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_include_directories(vlfeat PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VLFeat
)

add_compile_definitions(VL_DISABLE_AVX)
add_compile_definitions(VL_DISABLE_SSE2)
add_compile_definitions(VL_DISABLE_OPENMP)

# -------------------- Eigen (HEADER-ONLY, EMBEDDED) --------------------

add_library(Eigen3 INTERFACE)

target_include_directories(Eigen3 INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Eigen
)

add_subdirectory(thirdparty/Eigen)

# -------------------- ceres (STATIC, EMBEDDED) --------------------

file(GLOB CERES_INTERNAL_SOURCES
        "thirdparty/ceres/internal/ceres/*.cc"
)

list(FILTER CERES_INTERNAL_SOURCES EXCLUDE REGEX "test|bench|gmock|gtest")

add_library(ceres STATIC ${CERES_INTERNAL_SOURCES})

target_include_directories(ceres PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ceres
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ceres/include
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ceres/internal
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Eigen
)

target_compile_definitions(ceres PRIVATE
        CERES_NO_LAPACK
        CERES_NO_SUITESPARSE
        CERES_NO_CXSPARSE
        CERES_NO_CUDA
        CERES_USE_EIGEN_SPARSE
        CERES_NO_ACCELERATE_SPARSE
        # Since we are skipping the config.h generation, define the threading model
        CERES_THREADING_MODEL_OPENMP
)

set_target_properties(ceres PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        OUTPUT_NAME "ceres"  # Forces the file to be libceres.a
)

target_link_libraries(ceres PRIVATE Eigen3)

# -------------------- FREEIMAGE (STATIC, MINIMAL) --------------------

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

# Add FreeImage
add_subdirectory(thirdparty/FreeImage)

# -------------------- Android-FAISS (STATIC, EMBEDDED) --------------------

#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s -O3 -DSkip_f2c_Undefs -DNO_LONG_LONG -DNO_BLAS_WRAP")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -O3 -DSkip_f2c_Undefs -DNO_LONG_LONG -DNO_BLAS_WRAP")

if(ANDROID)
    if((${ANDROID_ABI} STREQUAL "arm64-v8a") OR (${ANDROID_ABI} STREQUAL "x86_64"))
        add_compile_definitions(ANDROID_BIT32)
    else()
        add_compile_definitions(ANDROID_BIT64)
        error("No support for other build architectures than arm64-v8a and x86_64")
    endif()
endif()

file(GLOB FAISS_ROOT_SOURCES
        thirdparty/faiss/*.cpp
)

file(GLOB FAISS_IMPL_SOURCES
        thirdparty/faiss/impl/*.cpp
        thirdparty/faiss/impl/code_distance/*.cpp
)

file(GLOB FAISS_INVLISTS_SOURCES
        thirdparty/faiss/invlists/*.cpp
)

file(GLOB FAISS_UTILS_SOURCES
        thirdparty/faiss/utils/*.cpp
        thirdparty/faiss/utils/distances_fused/*.cpp
)

set(FAISS_SOURCES
        ${FAISS_ROOT_SOURCES}
        ${FAISS_IMPL_SOURCES}
        ${FAISS_INVLISTS_SOURCES}
        ${FAISS_UTILS_SOURCES}
)

file(GLOB CLAPACK_SOURCES
        thirdparty/clapack/SRC/*.c
        thirdparty/clapack/SRC/*.cpp
)
list(APPEND FAISS_SOURCES ${CLAPACK_SOURCES})

add_library(faiss STATIC ${FAISS_SOURCES})

target_include_directories(faiss PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
)

target_compile_definitions(faiss PRIVATE
        FAISS_DISABLE_GPU
        FAISS_DISABLE_PYTHON
        FAISS_DISABLE_CUDA
        FAISS_DISABLE_SSE
        FAISS_DISABLE_AVX
        FAISS_DISABLE_NEON
        FAISS_DISABLE_FAST_SCAN
)

target_compile_options(faiss PRIVATE
        -O2
        -fno-strict-aliasing
        -fno-vectorize
        -fno-slp-vectorize
#        -fopenmp
)

set_target_properties(faiss PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)
# -------------------- minmap-core (STATIC, EMBEDDED) --------------------

add_library(minmap-core STATIC
        # controllers
        minmap-core/controllers/feature_extraction.cc
        minmap-core/controllers/feature_matching.cc
        minmap-core/controllers/feature_matching_utils.cc
        minmap-core/controllers/image_reader.cc
        minmap-core/controllers/incremental_pipeline.cc
        minmap-core/controllers/option_manager.cc

        # estimators
        minmap-core/estimators/absolute_pose.cc
        minmap-core/estimators/alignment.cc
        minmap-core/estimators/bundle_adjustment.cc
        minmap-core/estimators/coordinate_frame.cc
        minmap-core/estimators/essential_matrix.cc
        minmap-core/estimators/fundamental_matrix.cc
        minmap-core/estimators/generalized_absolute_pose.cc
        minmap-core/estimators/generalized_pose.cc
        minmap-core/estimators/generalized_relative_pose.cc
        minmap-core/estimators/homography_matrix.cc
        minmap-core/estimators/similarity_transform.cc
        minmap-core/estimators/triangulation.cc
        minmap-core/estimators/two_view_geometry.cc
        minmap-core/estimators/utils.cc

        # feature
        minmap-core/feature/index.cc
        minmap-core/feature/matcher.cc
        minmap-core/feature/pairing.cc
        minmap-core/feature/sift.cc
        minmap-core/feature/types.cc
        minmap-core/feature/utils.cc

        # geometry
        minmap-core/geometry/essential_matrix.cc
        minmap-core/geometry/gps.cc
        minmap-core/geometry/homography_matrix.cc
        minmap-core/geometry/normalization.cc
        minmap-core/geometry/pose.cc
        minmap-core/geometry/pose_prior.cc
        minmap-core/geometry/rigid3.cc
        minmap-core/geometry/sim3.cc
        minmap-core/geometry/triangulation.cc

        # math
        minmap-core/math/math.cc
        minmap-core/math/polynomial.cc
        minmap-core/math/random.cc

        # optim
        minmap-core/optim/combination_sampler.cc
        minmap-core/optim/random_sampler.cc
        minmap-core/optim/support_measurement.cc

        # scene
        minmap-core/scene/camera.cc
        minmap-core/scene/correspondence_graph.cc
        minmap-core/scene/database.cc
        minmap-core/scene/database_cache.cc
        minmap-core/scene/frame.cc
        minmap-core/scene/image.cc
        minmap-core/scene/point2d.cc
        minmap-core/scene/point3d.cc
        minmap-core/scene/projection.cc
        minmap-core/scene/reconstruction.cc
        minmap-core/scene/reconstruction_io.cc
        minmap-core/scene/reconstruction_io_binary.cc
        minmap-core/scene/reconstruction_io_text.cc
        minmap-core/scene/reconstruction_io_utils.cc
        minmap-core/scene/reconstruction_manager.cc
        minmap-core/scene/track.cc
        minmap-core/scene/two_view_geometry.cc
        minmap-core/scene/visibility_pyramid.cc

        # sensor
        minmap-core/sensor/bitmap.cc
        minmap-core/sensor/database.cc
        minmap-core/sensor/models.cc
        minmap-core/sensor/rig.cc
        minmap-core/sensor/specs.cc

        # sfm
        minmap-core/sfm/incremental_mapper.cc
        minmap-core/sfm/incremental_mapper_impl.cc
        minmap-core/sfm/incremental_triangulator.cc
        minmap-core/sfm/observation_manager.cc

        # util
        minmap-core/util/base_controller.cc
        minmap-core/util/endian.cc
        minmap-core/util/file.cc
        minmap-core/util/logging.cc
        minmap-core/util/misc.cc
        minmap-core/util/ply.cc
        minmap-core/util/string.cc
        minmap-core/util/threading.cc
        minmap-core/util/timer.cc
)

set_target_properties(minmap-core PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_include_directories(minmap-core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/minmap-core
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Eigen
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ceres/include
)

target_link_libraries(minmap-core
        PRIVATE
        sqlite3
        vlfeat
        poselib
        faiss
        FreeImage
        PUBLIC
        ceres
)

# -------------------- minmap (STATIC, EMBEDDED) --------------------

add_library(minmap STATIC
        minmap/Feature.cpp
        minmap/MapModel.cpp
        minmap/SfM.cpp
)

set_target_properties(minmap PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_include_directories(minmap PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/minmap-core
)

target_link_libraries(minmap
        PRIVATE
        minmap-core
)

# ------------------------ GLM -------------------------

add_library(glm INTERFACE)

target_include_directories(glm INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm
)

# ------------------- tiny OBJ loader --------------------

add_library(tinyobjloader INTERFACE)

target_include_directories( tinyobjloader INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyobjloader
)


# ------------------------ ASSIMP ------------------------
set(ASSIMP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/assimp)
set(ASSIMP_INCLUDE_DIR ${ASSIMP_ROOT})
set(ASSIMP_LIB_DIR ${ASSIMP_ROOT}/lib/${ANDROID_ABI})  # ABI-specific

# Include headers
#include_directories(${ASSIMP_INCLUDE_DIR})

add_library(assimp SHARED IMPORTED)
set_target_properties(assimp PROPERTIES
        IMPORTED_LOCATION ${ASSIMP_LIB_DIR}/libassimp.so
)

# If you used c++_shared, link libc++_shared too
find_library(cpp_shared c++_shared)

# --------------------- Engine Backend ---------------------

add_library(engineBackend STATIC
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/AndroidWindow.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Buffer.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Camera.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Descriptors.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Device.cpp
#    thirdparty/VulkanLiteEngine/Engine/EngineBackend/HID.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Model.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Object.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/Pipeline.cpp
    thirdparty/VulkanLiteEngine/Engine/EngineBackend/SwapChain.cpp
)

set_target_properties(engineBackend PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(engineBackend PUBLIC assimp glm tinyobjloader)

target_include_directories(engineBackend PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VulkanLiteEngine/Engine/EngineUtils
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
)

target_compile_definitions(engineBackend PRIVATE
        USE_VULKAN
        VLE_ECONOMIC_SWAP_CHAIN_MODE
        NDEBUG
)

# --------------------- Engine Utils ---------------------

add_library(engineUtils STATIC
        thirdparty/VulkanLiteEngine/Engine/EngineUtils/Aligned.cpp
        thirdparty/VulkanLiteEngine/Engine/EngineUtils/Hash.cpp
        thirdparty/VulkanLiteEngine/Engine/EngineUtils/VkHelpers.cpp
        thirdparty/VulkanLiteEngine/Engine/EngineUtils/Threading.cpp
        thirdparty/VulkanLiteEngine/Engine/EngineUtils/Timer.cpp
)

set_target_properties(engineUtils PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(engineUtils   PUBLIC glm)

target_include_directories(engineUtils PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VulkanLiteEngine/Engine
)

target_compile_definitions(engineUtils PRIVATE
        USE_VULKAN
)

# ------------------- Engine Systems --------------------

add_library(engineSystem STATIC
        thirdparty/VulkanLiteEngine/Engine/Systems/CameraSystem.cpp
        thirdparty/VulkanLiteEngine/Engine/Systems/ObjectRenderSystem.cpp
        thirdparty/VulkanLiteEngine/Engine/Systems/PointCloudRenderSystem.cpp
        thirdparty/VulkanLiteEngine/Engine/Systems/PointLightSystem.cpp
        thirdparty/VulkanLiteEngine/Engine/Systems/Renderer.cpp
)

set_target_properties(engineSystem PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(engineSystem  PUBLIC glm)

target_include_directories(engineSystem PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VulkanLiteEngine/Engine/EngineBackend
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VulkanLiteEngine/Engine/Systems
)

# ----------------------- RayTracing ------------------------

add_library(rayTracing STATIC
        RayTracing/MarkerManager.cpp
        RayTracing/PickingFramebuffer.cpp
        RayTracing/PickingRenderSystem.cpp
)

set_target_properties(rayTracing PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(rayTracing PUBLIC
        glm
        engineBackend
        engineUtils
        engineSystem
)

target_include_directories(rayTracing PUBLIC
        RayTracing
)

# -------------------- ENGINE BACKEND BRIDGE --------------------

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thridparty/utils)

include_directories(${ANDROID_NDK}//sources/android/native_app_glue)

add_library(renderEngine SHARED
        AndroidEngine.cpp
        RenderLoopProcess.cpp
        Engine_bridge.cpp
)

#if(ANDROID)
#    target_link_libraries(renderEngine PRIVATE omp)
#endif()

target_include_directories(renderEngine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VulkanLiteEngine/Engine
        ${CMAKE_CURRENT_SOURCE_DIR}/minmap-core
        ${CMAKE_CURRENT_SOURCE_DIR}/minmap
        ${CMAKE_CURRENT_SOURCE_DIR}/RayTracing
)

# -------------------- LINK EVERYTHING --------------------
find_library(vulkan-lib vulkan)

target_link_libraries(renderEngine PRIVATE
        engineBackend
        engineUtils
        engineSystem
        minmap
        rayTracing
        android
        ${vulkan-lib}
        log
        ${cpp_shared}
)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpermissive")