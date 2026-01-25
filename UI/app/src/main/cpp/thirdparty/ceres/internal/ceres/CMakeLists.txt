# Ceres Solver - A fast non-linear least squares minimizer
# Minimal CMakeLists.txt for embedded build in Android

# Collect all source files
file(GLOB CERES_SOURCES
    "*.cc"
)

# Remove template instantiation file as it's included by other files
list(REMOVE_ITEM CERES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/schur_templates.cc")
list(REMOVE_ITEM CERES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/generate_template_specializations.py")
list(REMOVE_ITEM CERES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/partitioned_matrix_view_template.py")
list(REMOVE_ITEM CERES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/schur_eliminator_template.py")

# Create Ceres library
add_library(ceres_internal OBJECT ${CERES_SOURCES})

# Get Ceres config path from parent CMakeLists.txt
set(CERES_CONFIG_HEADER_DIR "${Ceres_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/ceres/internal")

# Set include directories
target_include_directories(ceres_internal
    PUBLIC
    ${Ceres_SOURCE_DIR}/include
    ${Ceres_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}
    PRIVATE
    ${Ceres_SOURCE_DIR}/internal/ceres
    ${EIGEN3_INCLUDE_DIRS}
)

# Set compile options
target_compile_options(ceres_internal PRIVATE -fPIC)

# Propagate to parent
set(CERES_INTERNAL_OBJECTS $<TARGET_OBJECTS:ceres_internal> PARENT_SCOPE)
set(CERES_INTERNAL_INCLUDE_DIRS
    ${Ceres_SOURCE_DIR}/include
    ${Ceres_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}
    PARENT_SCOPE
)
