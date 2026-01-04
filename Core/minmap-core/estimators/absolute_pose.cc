// Copyright (c), ETH Zurich and UNC Chapel Hill.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of ETH Zurich and UNC Chapel Hill nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "absolute_pose.h"

#include "utils.h"
#include "../math/polynomial.h"
#include "../util/logging.h"
#include "../util/poselib_sturm.h"

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <stdexcept>

/*
This is a part taken from PoseLib, u8nder BSD 3 - Clause.

BSD 3-Clause License

Copyright (c) 2020, Viktor Larsson
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
namespace poselib {

    struct Camera {
        int model_id;
        int width;
        int height;
        std::vector<double> params;

        Camera();
        Camera(const std::string& model_name, const std::vector<double>& params, int width, int height);
        Camera(int model_id, const std::vector<double>& params, int width, int height);

        // Projection and distortion
        void project(const Eigen::Vector2d& x, Eigen::Vector2d* xp) const;
        void project_with_jac(const Eigen::Vector2d& x, Eigen::Vector2d* xp, Eigen::Matrix2d* jac) const;
        void unproject(const Eigen::Vector2d& xp, Eigen::Vector2d* x) const;

        // vector wrappers for the project/unprojection
        void project(const std::vector<Eigen::Vector2d>& x, std::vector<Eigen::Vector2d>* xp) const;
        void project_with_jac(const std::vector<Eigen::Vector2d>& x, std::vector<Eigen::Vector2d>* xp,
            std::vector<Eigen::Matrix<double, 2, 2>>* jac) const;
        void unproject(const std::vector<Eigen::Vector2d>& xp, std::vector<Eigen::Vector2d>* x) const;

        // Update the camera parameters such that the projections are rescaled
        void rescale(double scale);
        // Return camera model as string
        std::string model_name() const;
        // Returns focal length (average in case of non-unit aspect ratio)
        double focal() const;
        double focal_x() const;
        double focal_y() const;
        Eigen::Vector2d principal_point() const;

        // Parses a camera from a line from cameras.txt, returns the camera_id
        int initialize_from_txt(const std::string& line);
        // Creates line for cameras.txt (inverse of initialize_from_txt)
        // If camera_id == -1 it is ommited
        std::string to_cameras_txt(int camera_id = -1) const;

        // helpers for camera model ids
        static int id_from_string(const std::string& model_name);
        static std::string name_from_id(int id);
    };

#define SETUP_CAMERA_SHARED_DEFS(ClassName, ModelName, ModelId)                                                        \
        class ClassName {                                                                                                  \
          public:                                                                                                          \
            static void project(const std::vector<double> &params, const Eigen::Vector2d &x, Eigen::Vector2d *xp);         \
            static void project_with_jac(const std::vector<double> &params, const Eigen::Vector2d &x, Eigen::Vector2d *xp, \
                                         Eigen::Matrix2d *jac);                                                            \
            static void unproject(const std::vector<double> &params, const Eigen::Vector2d &xp, Eigen::Vector2d *x);       \
            static const std::vector<size_t> focal_idx;                                                                    \
            static const std::vector<size_t> principal_point_idx;                                                          \
            static const int model_id = ModelId;                                                                           \
            static const std::string to_string() { return ModelName; }                                                     \
        };

    SETUP_CAMERA_SHARED_DEFS(NullCameraModel, "NULL", -1);
    SETUP_CAMERA_SHARED_DEFS(SimplePinholeCameraModel, "SIMPLE_PINHOLE", 0);
    SETUP_CAMERA_SHARED_DEFS(PinholeCameraModel, "PINHOLE", 1);
    SETUP_CAMERA_SHARED_DEFS(SimpleRadialCameraModel, "SIMPLE_RADIAL", 2);
    SETUP_CAMERA_SHARED_DEFS(RadialCameraModel, "RADIAL", 3);
    SETUP_CAMERA_SHARED_DEFS(OpenCVCameraModel, "OPENCV", 4);
    SETUP_CAMERA_SHARED_DEFS(OpenCVFisheyeCameraModel, "OPENCV_FISHEYE", 5);
    SETUP_CAMERA_SHARED_DEFS(FullOpenCVCameraModel, "FULL_OPENCV", 6);

#define SWITCH_CAMERA_MODELS                                                                                           \
        SWITCH_CAMERA_MODEL_CASE(NullCameraModel)                                                                          \
        SWITCH_CAMERA_MODEL_CASE(SimplePinholeCameraModel)                                                                 \
        SWITCH_CAMERA_MODEL_CASE(PinholeCameraModel)                                                                       \
        SWITCH_CAMERA_MODEL_CASE(SimpleRadialCameraModel)                                                                  \
        SWITCH_CAMERA_MODEL_CASE(RadialCameraModel)                                                                        \
        SWITCH_CAMERA_MODEL_CASE(OpenCVCameraModel)                                                                        \
        SWITCH_CAMERA_MODEL_CASE(OpenCVFisheyeCameraModel)                                                                 \
        SWITCH_CAMERA_MODEL_CASE(FullOpenCVCameraModel)

#undef SETUP_CAMERA_SHARED_DEFS

    static const double UNDIST_TOL = 1e-10;
    static const size_t UNDIST_MAX_ITER = 25;

    ///////////////////////////////////////////////////////////////////
    // Camera - base class storing ID

    Camera::Camera() : model_id(-1), width(-1), height(-1), params() {}
    Camera::Camera(const std::string& model_name, const std::vector<double>& p, int w, int h) {
        model_id = id_from_string(model_name);
        params = p;
        width = w;
        height = h;
    }
    Camera::Camera(int id, const std::vector<double>& p, int w, int h) {
        model_id = id;
        params = p;
        width = w;
        height = h;
    }

    int Camera::id_from_string(const std::string& model_name) {
#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    if (model_name == Model::to_string()) {                                                                            \
        return Model::model_id;                                                                                        \
    }

        SWITCH_CAMERA_MODELS

#undef SWITCH_CAMERA_MODEL_CASE

            return -1;
    }

    std::string Camera::name_from_id(int model_id) {
#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    case Model::model_id:                                                                                              \
        return Model::to_string();

        switch (model_id) {
            SWITCH_CAMERA_MODELS

        default:
            return "INVALID_MODEL";
        }
#undef SWITCH_CAMERA_MODEL_CASE
    }

    // Projection and distortion
    void Camera::project(const Eigen::Vector2d& x, Eigen::Vector2d* xp) const {
#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    case Model::model_id:                                                                                              \
        Model::project(params, x, xp);                                                                                 \
        break;

        switch (model_id) {
            SWITCH_CAMERA_MODELS

        default:
            throw std::runtime_error("PoseLib: CAMERA MODEL NYI");
        }
#undef SWITCH_CAMERA_MODEL_CASE
    }
    void Camera::project_with_jac(const Eigen::Vector2d& x, Eigen::Vector2d* xp, Eigen::Matrix2d* jac) const {
#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    case Model::model_id:                                                                                              \
        Model::project_with_jac(params, x, xp, jac);                                                                   \
        break;

        switch (model_id) {
            SWITCH_CAMERA_MODELS

        default:
            throw std::runtime_error("PoseLib: CAMERA MODEL NYI");
        }
#undef SWITCH_CAMERA_MODEL_CASE
    }
    void Camera::unproject(const Eigen::Vector2d& xp, Eigen::Vector2d* x) const {
#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    case Model::model_id:                                                                                              \
        Model::unproject(params, xp, x);                                                                               \
        break;

        switch (model_id) {
            SWITCH_CAMERA_MODELS

        default:
            throw std::runtime_error("PoseLib: CAMERA MODEL NYI");
        }
#undef SWITCH_CAMERA_MODEL_CASE
    }

    void Camera::project(const std::vector<Eigen::Vector2d>& x, std::vector<Eigen::Vector2d>* xp) const {
        xp->resize(x.size());
#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    case Model::model_id:                                                                                              \
        for (size_t i = 0; i < x.size(); ++i) {                                                                        \
            Model::project(params, x[i], &((*xp)[i]));                                                                 \
        }                                                                                                              \
        break;

        switch (model_id) {
            SWITCH_CAMERA_MODELS

        default:
            throw std::runtime_error("PoseLib: CAMERA MODEL NYI");
        }
#undef SWITCH_CAMERA_MODEL_CASE
    }
    void Camera::project_with_jac(const std::vector<Eigen::Vector2d>& x, std::vector<Eigen::Vector2d>* xp,
        std::vector<Eigen::Matrix<double, 2, 2>>* jac) const {
        xp->resize(x.size());
        jac->resize(x.size());
#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    case Model::model_id:                                                                                              \
        for (size_t i = 0; i < x.size(); ++i) {                                                                        \
            Model::project_with_jac(params, x[i], &((*xp)[i]), &((*jac)[i]));                                          \
        }                                                                                                              \
        break;

        switch (model_id) {
            SWITCH_CAMERA_MODELS

        default:
            throw std::runtime_error("PoseLib: CAMERA MODEL NYI");
        }
#undef SWITCH_CAMERA_MODEL_CASE
    }

    void Camera::unproject(const std::vector<Eigen::Vector2d>& xp, std::vector<Eigen::Vector2d>* x) const {
        x->resize(xp.size());
#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    case Model::model_id:                                                                                              \
        for (size_t i = 0; i < xp.size(); ++i) {                                                                       \
            Model::unproject(params, xp[i], &((*x)[i]));                                                               \
        }                                                                                                              \
        break;

        switch (model_id) {
            SWITCH_CAMERA_MODELS

        default:
            throw std::runtime_error("PoseLib: CAMERA MODEL NYI");
        }
#undef SWITCH_CAMERA_MODEL_CASE
    }

    std::string Camera::model_name() const { return name_from_id(model_id); }

    double Camera::focal() const {
        if (params.empty()) {
            return 1.0; // empty camera assumed to be identity
        }

        double focal = 0.0;
        switch (model_id) {
#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    case Model::model_id:                                                                                              \
        for (size_t idx : Model::focal_idx)                                                                            \
            focal += params.at(idx) / Model::focal_idx.size();                                                         \
        break;

            SWITCH_CAMERA_MODELS
        }
#undef SWITCH_CAMERA_MODEL_CASE
        return focal;
    }

    double Camera::focal_x() const {
        if (params.empty()) {
            return 1.0; // empty camera assumed to be identity
        }

        switch (model_id) {
#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    case Model::model_id:                                                                                              \
        return params.at(Model::focal_idx[0]);

            SWITCH_CAMERA_MODELS
        }
#undef SWITCH_CAMERA_MODEL_CASE
        return -1.0;
    }
    double Camera::focal_y() const {
        if (params.empty()) {
            return 1.0; // empty camera assumed to be identity
        }

        switch (model_id) {
#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    case Model::model_id:                                                                                              \
        if (Model::focal_idx.size() > 1) {                                                                             \
            return params.at(Model::focal_idx[1]);                                                                     \
        } else {                                                                                                       \
            return params.at(Model::focal_idx[0]);                                                                     \
        }

            SWITCH_CAMERA_MODELS
        }
#undef SWITCH_CAMERA_MODEL_CASE
        return -1.0;
    }

    Eigen::Vector2d Camera::principal_point() const {
        if (params.empty()) {
            return Eigen::Vector2d(0.0, 0.0);
        }
        switch (model_id) {
#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    case Model::model_id:                                                                                              \
        return Eigen::Vector2d(params.at(Model::principal_point_idx[0]), params.at(Model::principal_point_idx[1]));

            SWITCH_CAMERA_MODELS
        }
#undef SWITCH_CAMERA_MODEL_CASE
        return Eigen::Vector2d(-1.0, -1.0);
    }

    // Update the camera parameters such that the projections are rescaled
    void Camera::rescale(double scale) {
        if (params.empty()) {
            return;
        }

#define SWITCH_CAMERA_MODEL_CASE(Model)                                                                                \
    case Model::model_id:                                                                                              \
        for (size_t idx : Model::focal_idx)                                                                            \
            params.at(idx) *= scale;                                                                                   \
        for (size_t idx : Model::principal_point_idx)                                                                  \
            params.at(idx) *= scale;                                                                                   \
        break;

        switch (model_id) { SWITCH_CAMERA_MODELS }
#undef SWITCH_CAMERA_MODEL_CASE
    }

    int Camera::initialize_from_txt(const std::string& line) {
        std::stringstream ss(line);
        int camera_id;
        ss >> camera_id;

        // Read the model
        std::string model_name;
        ss >> model_name;
        model_id = id_from_string(model_name);
        if (model_id == -1) {
            return -1;
        }

        // Read sizes
        double d;
        ss >> d;
        width = d;
        ss >> d;
        height = d;

        // Read parameters
        params.clear();
        double param;
        while (ss >> param) {
            params.push_back(param);
        }

        return camera_id;
    }
    std::string Camera::to_cameras_txt(int camera_id) const {
        std::stringstream ss;
        if (camera_id != -1) {
            ss << camera_id << " ";
        }
        ss << model_name();
        ss << " " << width;
        ss << " " << height;
        ss << std::setprecision(16);
        for (double d : params) {
            ss << " " << d;
        }
        return ss.str();
    }

    //  xp = f * d(r) * x
    //  J = f * d'(r) * Jr + f * d(r)
    // r = |x|, Jr = x / |x|

    // Solves
    //   rd = (1+k1 * r*r) * r
    double undistort_poly1(double k1, double rd) {
        // f  = k1 * r^3 + r + 1 - rd = 0
        // fp = 3 * k1 * r^2 + 1
        double r = rd;
        for (size_t iter = 0; iter < UNDIST_MAX_ITER; ++iter) {
            double r2 = r * r;
            double f = k1 * r2 * r + r - rd;
            if (std::abs(f) < UNDIST_TOL) {
                break;
            }
            double fp = 3.0 * k1 * r2 + 1.0;
            r = r - f / fp;
        }
        return r;
    }

    // Solves
    //   rd = (1+ k1 * r^2 + k2 * r^4) * r
    double undistort_poly2(double k1, double k2, double rd) {
        // f  = k2 * r^5 + k1 * r^3 + r + 1 - rd = 0
        // fp = 5 * k2 * r^4 + 3 * k1 * r^2 + 1
        double r = rd;
        for (size_t iter = 0; iter < UNDIST_MAX_ITER; ++iter) {
            double r2 = r * r;
            double f = k2 * r2 * r2 * r + k1 * r2 * r + r - rd;
            if (std::abs(f) < UNDIST_TOL) {
                break;
            }
            double fp = 5.0 * k2 * r2 * r2 + 3.0 * k1 * r2 + 1.0;
            r = r - f / fp;
        }
        return r;
    }

    ///////////////////////////////////////////////////////////////////
    // Pinhole camera
    // params = fx, fy, cx, cy

    void PinholeCameraModel::project(const std::vector<double>& params, const Eigen::Vector2d& x, Eigen::Vector2d* xp) {
        (*xp)(0) = params[0] * x(0) + params[2];
        (*xp)(1) = params[1] * x(1) + params[3];
    }
    void PinholeCameraModel::project_with_jac(const std::vector<double>& params, const Eigen::Vector2d& x,
        Eigen::Vector2d* xp, Eigen::Matrix2d* jac) {
        (*xp)(0) = params[0] * x(0) + params[2];
        (*xp)(1) = params[1] * x(1) + params[3];
        (*jac)(0, 0) = params[0];
        (*jac)(0, 1) = 0.0;
        (*jac)(1, 0) = 0.0;
        (*jac)(1, 1) = params[1];
    }
    void PinholeCameraModel::unproject(const std::vector<double>& params, const Eigen::Vector2d& xp, Eigen::Vector2d* x) {
        (*x)(0) = (xp(0) - params[2]) / params[0];
        (*x)(1) = (xp(1) - params[3]) / params[1];
    }
    const std::vector<size_t> PinholeCameraModel::focal_idx = { 0, 1 };
    const std::vector<size_t> PinholeCameraModel::principal_point_idx = { 2, 3 };

    ///////////////////////////////////////////////////////////////////
    // Simple Pinhole camera
    // params = f, cx, cy

    void SimplePinholeCameraModel::project(const std::vector<double>& params, const Eigen::Vector2d& x,
        Eigen::Vector2d* xp) {
        (*xp)(0) = params[0] * x(0) + params[1];
        (*xp)(1) = params[0] * x(1) + params[2];
    }
    void SimplePinholeCameraModel::project_with_jac(const std::vector<double>& params, const Eigen::Vector2d& x,
        Eigen::Vector2d* xp, Eigen::Matrix2d* jac) {
        (*xp)(0) = params[0] * x(0) + params[1];
        (*xp)(1) = params[0] * x(1) + params[2];
        (*jac)(0, 0) = params[0];
        (*jac)(0, 1) = 0.0;
        (*jac)(1, 0) = 0.0;
        (*jac)(1, 1) = params[0];
    }
    void SimplePinholeCameraModel::unproject(const std::vector<double>& params, const Eigen::Vector2d& xp,
        Eigen::Vector2d* x) {
        (*x)(0) = (xp(0) - params[1]) / params[0];
        (*x)(1) = (xp(1) - params[2]) / params[0];
    }
    const std::vector<size_t> SimplePinholeCameraModel::focal_idx = { 0 };
    const std::vector<size_t> SimplePinholeCameraModel::principal_point_idx = { 1, 2 };

    ///////////////////////////////////////////////////////////////////
    // Radial camera
    // params = f, cx, cy, k1, k2

    void RadialCameraModel::project(const std::vector<double>& params, const Eigen::Vector2d& x, Eigen::Vector2d* xp) {
        const double r2 = x.squaredNorm();
        const double alpha = (1.0 + params[3] * r2 + params[4] * r2 * r2);
        (*xp)(0) = params[0] * alpha * x(0) + params[1];
        (*xp)(1) = params[0] * alpha * x(1) + params[2];
    }
    void RadialCameraModel::project_with_jac(const std::vector<double>& params, const Eigen::Vector2d& x,
        Eigen::Vector2d* xp, Eigen::Matrix2d* jac) {
        const double r2 = x.squaredNorm();
        const double alpha = (1.0 + params[3] * r2 + params[4] * r2 * r2);
        const double alphap = (2.0 * params[3] + 4.0 * params[4] * r2);
        *jac = alphap * (x * x.transpose());
        (*jac)(0, 0) += alpha;
        (*jac)(1, 1) += alpha;
        (*jac)(0, 0) *= params[0];
        (*jac)(0, 1) *= params[0];
        (*jac)(1, 0) *= params[0];
        (*jac)(1, 1) *= params[0];
        (*xp)(0) = params[0] * alpha * x(0) + params[1];
        (*xp)(1) = params[0] * alpha * x(1) + params[2];
    }
    void RadialCameraModel::unproject(const std::vector<double>& params, const Eigen::Vector2d& xp, Eigen::Vector2d* x) {
        (*x)(0) = (xp(0) - params[1]) / params[0];
        (*x)(1) = (xp(1) - params[2]) / params[0];
        double r0 = x->norm();
        double r = undistort_poly2(params[3], params[4], r0);
        (*x) *= r / r0;
    }
    const std::vector<size_t> RadialCameraModel::focal_idx = { 0 };
    const std::vector<size_t> RadialCameraModel::principal_point_idx = { 1, 2 };

    ///////////////////////////////////////////////////////////////////
    // Simple Radial camera
    // params = f, cx, cy, k1

    void SimpleRadialCameraModel::project(const std::vector<double>& params, const Eigen::Vector2d& x,
        Eigen::Vector2d* xp) {
        const double r2 = x.squaredNorm();
        const double alpha = (1.0 + params[3] * r2);
        (*xp)(0) = params[0] * alpha * x(0) + params[1];
        (*xp)(1) = params[0] * alpha * x(1) + params[2];
    }
    void SimpleRadialCameraModel::project_with_jac(const std::vector<double>& params, const Eigen::Vector2d& x,
        Eigen::Vector2d* xp, Eigen::Matrix2d* jac) {
        const double r2 = x.squaredNorm();
        const double alpha = (1.0 + params[3] * r2);
        *jac = 2.0 * params[3] * (x * x.transpose());
        (*jac)(0, 0) += alpha;
        (*jac)(1, 1) += alpha;
        *jac *= params[0];
        (*xp)(0) = params[0] * alpha * x(0) + params[1];
        (*xp)(1) = params[0] * alpha * x(1) + params[2];
    }
    void SimpleRadialCameraModel::unproject(const std::vector<double>& params, const Eigen::Vector2d& xp,
        Eigen::Vector2d* x) {
        (*x)(0) = (xp(0) - params[1]) / params[0];
        (*x)(1) = (xp(1) - params[2]) / params[0];
        double r0 = x->norm();
        double r = undistort_poly1(params[3], r0);
        (*x) *= r / r0;
    }
    const std::vector<size_t> SimpleRadialCameraModel::focal_idx = { 0 };
    const std::vector<size_t> SimpleRadialCameraModel::principal_point_idx = { 1, 2 };

    ///////////////////////////////////////////////////////////////////
    // OpenCV camera
    //   params = fx, fy, cx, cy, k1, k2, p1, p2

    void compute_opencv_distortion(double k1, double k2, double p1, double p2, const Eigen::Vector2d& x,
        Eigen::Vector2d& xp) {
        const double u = x(0);
        const double v = x(1);
        const double u2 = u * u;
        const double uv = u * v;
        const double v2 = v * v;
        const double r2 = u * u + v * v;
        const double alpha = 1.0 + k1 * r2 + k2 * r2 * r2;
        xp(0) = alpha * u + 2.0 * p1 * uv + p2 * (r2 + 2.0 * u2);
        xp(1) = alpha * v + 2.0 * p2 * uv + p1 * (r2 + 2.0 * v2);
    }

    void compute_opencv_distortion_jac(double k1, double k2, double p1, double p2, const Eigen::Vector2d& x,
        Eigen::Vector2d& xp, Eigen::Matrix2d& jac) {
        const double u = x(0);
        const double v = x(1);
        const double u2 = u * u;
        const double uv = u * v;
        const double v2 = v * v;
        const double r2 = u * u + v * v;
        jac(0, 0) = k2 * r2 * r2 + 6 * p2 * u + 2 * p1 * v + u * (2 * k1 * u + 4 * k2 * u * r2) + k1 * r2 + 1.0;
        jac(0, 1) = 2 * p1 * u + 2 * p2 * v + v * (2 * k1 * u + 4 * k2 * u * r2);
        jac(1, 0) = 2 * p1 * u + 2 * p2 * v + u * (2 * k1 * v + 4 * k2 * v * r2);
        jac(1, 1) = k2 * r2 * r2 + 2 * p2 * u + 6 * p1 * v + v * (2 * k1 * v + 4 * k2 * v * r2) + k1 * r2 + 1.0;

        const double alpha = 1.0 + k1 * r2 + k2 * r2 * r2;
        xp(0) = alpha * u + 2.0 * p1 * uv + p2 * (r2 + 2.0 * u2);
        xp(1) = alpha * v + 2.0 * p2 * uv + p1 * (r2 + 2.0 * v2);
    }

    void OpenCVCameraModel::project(const std::vector<double>& params, const Eigen::Vector2d& x, Eigen::Vector2d* xp) {
        compute_opencv_distortion(params[4], params[5], params[6], params[7], x, *xp);
        (*xp)(0) = params[0] * (*xp)(0) + params[2];
        (*xp)(1) = params[1] * (*xp)(1) + params[3];
    }

    Eigen::Vector2d undistort_opencv(double k1, double k2, double p1, double p2, const Eigen::Vector2d& xp) {
        Eigen::Vector2d x = xp;
        Eigen::Vector2d xd;
        Eigen::Matrix2d jac;
        static const double lambda = 1e-8;
        for (size_t iter = 0; iter < UNDIST_MAX_ITER; ++iter) {
            compute_opencv_distortion_jac(k1, k2, p1, p2, x, xd, jac);
            jac(0, 0) += lambda;
            jac(1, 1) += lambda;
            Eigen::Vector2d res = xd - xp;

            if (res.norm() < UNDIST_TOL) {
                break;
            }

            x = x - jac.inverse() * res;
        }
        return x;
    }

    void OpenCVCameraModel::project_with_jac(const std::vector<double>& params, const Eigen::Vector2d& x,
        Eigen::Vector2d* xp, Eigen::Matrix2d* jac) {
        compute_opencv_distortion_jac(params[4], params[5], params[6], params[7], x, *xp, *jac);
        jac->row(0) *= params[0];
        jac->row(1) *= params[1];
        (*xp)(0) = params[0] * (*xp)(0) + params[2];
        (*xp)(1) = params[1] * (*xp)(1) + params[3];
    }
    void OpenCVCameraModel::unproject(const std::vector<double>& params, const Eigen::Vector2d& xp, Eigen::Vector2d* x) {
        (*x)(0) = (xp(0) - params[2]) / params[0];
        (*x)(1) = (xp(1) - params[3]) / params[1];

        *x = undistort_opencv(params[4], params[5], params[6], params[7], *x);
    }
    const std::vector<size_t> OpenCVCameraModel::focal_idx = { 0, 1 };
    const std::vector<size_t> OpenCVCameraModel::principal_point_idx = { 2, 3 };

    ///////////////////////////////////////////////////////////////////
    // Full OpenCV camera
    //   params = fx, fy, cx, cy, k1, k2, p1, p2, k3, k4, k5, k6

    void compute_full_opencv_distortion(double k1, double k2, double p1, double p2, double k3, double k4, double k5,
        double k6, const Eigen::Vector2d& x, Eigen::Vector2d& xp) {
        const double u = x(0);
        const double v = x(1);
        const double u2 = u * u;
        const double uv = u * v;
        const double v2 = v * v;
        const double r2 = u * u + v * v;
        const double r4 = r2 * r2;
        const double r6 = r2 * r4;
        const double alpha = (1.0 + k1 * r2 + k2 * r4 + k3 * r6) / (1.0 + k4 * r2 + k5 * r4 + k6 * r6);
        xp(0) = alpha * u + 2.0 * p1 * uv + p2 * (r2 + 2.0 * u2);
        xp(1) = alpha * v + 2.0 * p2 * uv + p1 * (r2 + 2.0 * v2);
    }

    void compute_full_opencv_distortion_jac(double k1, double k2, double p1, double p2, double k3, double k4, double k5,
        double k6, const Eigen::Vector2d& x, Eigen::Vector2d& xp,
        Eigen::Matrix2d& jac) {
        const double u = x(0);
        const double v = x(1);
        const double u2 = u * u;
        const double uv = u * v;
        const double v2 = v * v;
        const double r2 = u * u + v * v;
        const double r4 = r2 * r2;
        const double r6 = r2 * r4;

        const double nn = 1.0 + k1 * r2 + k2 * r4 + k3 * r6;
        const double dd = 1.0 + k4 * r2 + k5 * r4 + k6 * r6;
        const double nn_r = 2.0 * k1 + 4.0 * k2 * r2 + 6.0 * k3 * r4;
        const double dd_r = 2.0 * k4 + 4.0 * k5 * r2 + 6.0 * k6 * r4;
        const double dd2 = dd * dd;

        jac(0, 0) = 6 * p2 * u + 2 * p1 * v + nn / dd + (u2 * nn_r) / dd - (nn * u2 * dd_r) / dd2;
        jac(0, 1) = 2 * p1 * u + 2 * p2 * v + (uv * nn_r) / dd - (nn * uv * dd_r) / dd2;
        jac(1, 0) = jac(0, 1);
        // jac(1,0) = 2*p1*u + 2*p2*v + (uv*nn_r)/dd - (nn*uv*dd_r)/dd^2;
        jac(1, 1) = 2 * p2 * u + 6 * p1 * v + nn / dd + (v2 * nn_r) / dd - (nn * v2 * dd_r) / dd2;

        const double alpha = nn / dd;
        xp(0) = alpha * u + 2.0 * p1 * uv + p2 * (r2 + 2.0 * u2);
        xp(1) = alpha * v + 2.0 * p2 * uv + p1 * (r2 + 2.0 * v2);
    }

    void FullOpenCVCameraModel::project(const std::vector<double>& params, const Eigen::Vector2d& x, Eigen::Vector2d* xp) {
        compute_full_opencv_distortion(params[4], params[5], params[6], params[7], params[8], params[9], params[10],
            params[11], x, *xp);
        (*xp)(0) = params[0] * (*xp)(0) + params[2];
        (*xp)(1) = params[1] * (*xp)(1) + params[3];
    }

    Eigen::Vector2d undistort_full_opencv(double k1, double k2, double p1, double p2, double k3, double k4, double k5,
        double k6, const Eigen::Vector2d& xp) {
        Eigen::Vector2d x = xp;
        Eigen::Vector2d xd;
        Eigen::Matrix2d jac;
        static const double lambda = 1e-8;
        for (size_t iter = 0; iter < UNDIST_MAX_ITER; ++iter) {
            compute_full_opencv_distortion_jac(k1, k2, p1, p2, k3, k4, k5, k6, x, xd, jac);
            jac(0, 0) += lambda;
            jac(1, 1) += lambda;
            Eigen::Vector2d res = xd - xp;

            if (res.norm() < UNDIST_TOL) {
                break;
            }

            x = x - jac.inverse() * res;
        }
        return x;
    }

    void FullOpenCVCameraModel::project_with_jac(const std::vector<double>& params, const Eigen::Vector2d& x,
        Eigen::Vector2d* xp, Eigen::Matrix2d* jac) {
        compute_full_opencv_distortion_jac(params[4], params[5], params[6], params[7], params[8], params[9], params[10],
            params[11], x, *xp, *jac);
        if (jac) {
            jac->row(0) *= params[0];
            jac->row(1) *= params[1];
        }
        (*xp)(0) = params[0] * (*xp)(0) + params[2];
        (*xp)(1) = params[1] * (*xp)(1) + params[3];
    }

    void FullOpenCVCameraModel::unproject(const std::vector<double>& params, const Eigen::Vector2d& xp,
        Eigen::Vector2d* x) {
        Eigen::Vector2d xp0;
        xp0 << (xp(0) - params[2]) / params[0], (xp(1) - params[3]) / params[1];
        *x = undistort_full_opencv(params[4], params[5], params[6], params[7], params[8], params[9], params[10], params[11],
            xp0);
    }

    const std::vector<size_t> FullOpenCVCameraModel::focal_idx = { 0, 1 };
    const std::vector<size_t> FullOpenCVCameraModel::principal_point_idx = { 2, 3 };

    ///////////////////////////////////////////////////////////////////
    // OpenCV Fisheye camera
    //   params = fx, fy, cx, cy, k1, k2, k3, k4

    void OpenCVFisheyeCameraModel::project(const std::vector<double>& params, const Eigen::Vector2d& x,
        Eigen::Vector2d* xp) {
        double rho = x.norm();

        if (rho > 1e-8) {
            double theta = std::atan2(rho, 1.0);
            double theta2 = theta * theta;
            double theta4 = theta2 * theta2;
            double theta6 = theta2 * theta4;
            double theta8 = theta2 * theta6;

            double rd = theta * (1.0 + theta2 * params[4] + theta4 * params[5] + theta6 * params[6] + theta8 * params[7]);
            const double inv_r = 1.0 / rho;
            (*xp)(0) = params[0] * x(0) * inv_r * rd + params[2];
            (*xp)(1) = params[1] * x(1) * inv_r * rd + params[3];
        }
        else {
            // Very close to the principal axis - ignore distortion
            (*xp)(0) = params[0] * x(0) + params[2];
            (*xp)(1) = params[1] * x(1) + params[3];
        }
    }
    void OpenCVFisheyeCameraModel::project_with_jac(const std::vector<double>& params, const Eigen::Vector2d& x,
        Eigen::Vector2d* xp, Eigen::Matrix2d* jac) {
        double rho = x.norm();

        if (rho > 1e-8) {
            double theta = std::atan2(rho, 1.0);
            double theta2 = theta * theta;
            double theta4 = theta2 * theta2;
            double theta6 = theta2 * theta4;
            double theta8 = theta2 * theta6;

            double rd = theta * (1.0 + theta2 * params[4] + theta4 * params[5] + theta6 * params[6] + theta8 * params[7]);
            const double inv_r = 1.0 / rho;

            double drho_dx = x(0) / rho;
            double drho_dy = x(1) / rho;

            double rho_z2 = rho * rho + 1.0;
            double dtheta_drho = 1.0 / rho_z2;

            double drd_dtheta = (1.0 + 3.0 * theta2 * params[4] + 5.0 * theta4 * params[5] + 7.0 * theta6 * params[6] +
                9.0 * theta8 * params[7]);
            double drd_dx = drd_dtheta * dtheta_drho * drho_dx;
            double drd_dy = drd_dtheta * dtheta_drho * drho_dy;

            double dinv_r_drho = -1.0 / (rho * rho);
            double dinv_r_dx = dinv_r_drho * drho_dx;
            double dinv_r_dy = dinv_r_drho * drho_dy;

            (*xp)(0) = params[0] * x(0) * inv_r * rd + params[2];
            (*jac)(0, 0) = params[0] * (inv_r * rd + x(0) * dinv_r_dx * rd + x(0) * inv_r * drd_dx);
            (*jac)(0, 1) = params[0] * x(0) * (dinv_r_dy * rd + inv_r * drd_dy);

            (*xp)(1) = params[1] * x(1) * inv_r * rd + params[3];
            (*jac)(1, 0) = params[1] * x(1) * (dinv_r_dx * rd + inv_r * drd_dx);
            (*jac)(1, 1) = params[1] * (inv_r * rd + x(1) * dinv_r_dy * rd + x(1) * inv_r * drd_dy);
        }
        else {
            // Very close to the principal axis - ignore distortion
            (*xp)(0) = params[0] * x(0) + params[2];
            (*xp)(1) = params[1] * x(1) + params[3];
            (*jac)(0, 0) = params[0];
            (*jac)(0, 1) = 0.0;
            (*jac)(1, 0) = 0.0;
            (*jac)(1, 1) = params[1];
        }
    }

    double opencv_fisheye_newton(const std::vector<double>& params, double rd, double& theta) {
        double f;
        for (size_t iter = 0; iter < UNDIST_MAX_ITER; iter++) {
            const double theta2 = theta * theta;
            const double theta4 = theta2 * theta2;
            const double theta6 = theta2 * theta4;
            const double theta8 = theta2 * theta6;
            f = theta * (1.0 + theta2 * params[4] + theta4 * params[5] + theta6 * params[6] + theta8 * params[7]) - rd;
            if (std::abs(f) < UNDIST_TOL) {
                return std::abs(f);
            }
            double fp = (1.0 + 3.0 * theta2 * params[4] + 5.0 * theta4 * params[5] + 7.0 * theta6 * params[6] +
                9.0 * theta8 * params[7]);
            fp += std::copysign(1e-10, fp);
            theta = theta - f / fp;
        }
        return std::abs(f);
    }

    void OpenCVFisheyeCameraModel::unproject(const std::vector<double>& params, const Eigen::Vector2d& xp,
        Eigen::Vector2d* x) {
        const double px = (xp(0) - params[2]) / params[0];
        const double py = (xp(1) - params[3]) / params[1];
        const double rd = std::sqrt(px * px + py * py);
        double theta = 0;

        if (rd > 1e-8) {
            // try zero-init first
            double res = opencv_fisheye_newton(params, rd, theta);
            if (res > UNDIST_TOL || theta < 0) {
                // If this fails try to initialize with rho (first order approx.)
                theta = rd;
                res = opencv_fisheye_newton(params, rd, theta);

                if (res > UNDIST_TOL || theta < 0) {
                    // try once more
                    theta = 0.5 * rd;
                    res = opencv_fisheye_newton(params, rd, theta);

                    if (res > UNDIST_TOL || theta < 0) {
                        // try once more
                        theta = 1.5 * rd;
                        res = opencv_fisheye_newton(params, rd, theta);
                        // if this does not work, just fail silently... yay
                    }
                }
            }

            const double inv_z = std::tan(theta);
            (*x)(0) = px / rd * inv_z;
            (*x)(1) = py / rd * inv_z;

        }
        else {
            (*x)(0) = px;
            (*x)(1) = py;
        }
    }
    const std::vector<size_t> OpenCVFisheyeCameraModel::focal_idx = { 0, 1 };
    const std::vector<size_t> OpenCVFisheyeCameraModel::principal_point_idx = { 2, 3 };

    ///////////////////////////////////////////////////////////////////
    // Null camera - this is used as a dummy value in various places
    // params = {}

    void NullCameraModel::project(const std::vector<double>& params, const Eigen::Vector2d& x, Eigen::Vector2d* xp) {}
    void NullCameraModel::project_with_jac(const std::vector<double>& params, const Eigen::Vector2d& x, Eigen::Vector2d* xp,
        Eigen::Matrix2d* jac) {
    }
    void NullCameraModel::unproject(const std::vector<double>& params, const Eigen::Vector2d& xp, Eigen::Vector2d* x) {}
    const std::vector<size_t> NullCameraModel::focal_idx = {};
    const std::vector<size_t> NullCameraModel::principal_point_idx = {};


    inline Eigen::Matrix3d quat_to_rotmat(const Eigen::Vector4d& q) {
        return Eigen::Quaterniond(q(0), q(1), q(2), q(3)).toRotationMatrix();
    }
    inline Eigen::Matrix<double, 9, 1> quat_to_rotmatvec(const Eigen::Vector4d& q) {
        Eigen::Matrix3d R = quat_to_rotmat(q);
        Eigen::Matrix<double, 9, 1> r = Eigen::Map<Eigen::Matrix<double, 9, 1>>(R.data());
        return r;
    }

    inline Eigen::Vector4d rotmat_to_quat(const Eigen::Matrix3d& R) {
        Eigen::Quaterniond q_flip(R);
        Eigen::Vector4d q;
        q << q_flip.w(), q_flip.x(), q_flip.y(), q_flip.z();
        q.normalize();
        return q;
    }
    inline Eigen::Vector4d quat_multiply(const Eigen::Vector4d& qa, const Eigen::Vector4d& qb) {
        const double qa1 = qa(0), qa2 = qa(1), qa3 = qa(2), qa4 = qa(3);
        const double qb1 = qb(0), qb2 = qb(1), qb3 = qb(2), qb4 = qb(3);

        return Eigen::Vector4d(qa1 * qb1 - qa2 * qb2 - qa3 * qb3 - qa4 * qb4, qa1 * qb2 + qa2 * qb1 + qa3 * qb4 - qa4 * qb3,
            qa1 * qb3 + qa3 * qb1 - qa2 * qb4 + qa4 * qb2,
            qa1 * qb4 + qa2 * qb3 - qa3 * qb2 + qa4 * qb1);
    }

    inline Eigen::Vector3d quat_rotate(const Eigen::Vector4d& q, const Eigen::Vector3d& p) {
        const double q1 = q(0), q2 = q(1), q3 = q(2), q4 = q(3);
        const double p1 = p(0), p2 = p(1), p3 = p(2);
        const double px1 = -p1 * q2 - p2 * q3 - p3 * q4;
        const double px2 = p1 * q1 - p2 * q4 + p3 * q3;
        const double px3 = p2 * q1 + p1 * q4 - p3 * q2;
        const double px4 = p2 * q2 - p1 * q3 + p3 * q1;
        return Eigen::Vector3d(px2 * q1 - px1 * q2 - px3 * q4 + px4 * q3, px3 * q1 - px1 * q3 + px2 * q4 - px4 * q2,
            px3 * q2 - px2 * q3 - px1 * q4 + px4 * q1);
    }
    inline Eigen::Vector4d quat_conj(const Eigen::Vector4d& q) { return Eigen::Vector4d(q(0), -q(1), -q(2), -q(3)); }

    inline Eigen::Vector4d quat_exp(const Eigen::Vector3d& w) {
        const double theta2 = w.squaredNorm();
        const double theta = std::sqrt(theta2);
        const double theta_half = 0.5 * theta;

        double re, im;
        if (theta > 1e-6) {
            re = std::cos(theta_half);
            im = std::sin(theta_half) / theta;
        }
        else {
            // we are close to zero, use taylor expansion to avoid problems
            // with zero divisors in sin(theta/2)/theta
            const double theta4 = theta2 * theta2;
            re = 1.0 - (1.0 / 8.0) * theta2 + (1.0 / 384.0) * theta4;
            im = 0.5 - (1.0 / 48.0) * theta2 + (1.0 / 3840.0) * theta4;

            // for the linearized part we re-normalize to ensure unit length
            // here s should be roughly 1.0 anyways, so no problem with zero div
            const double s = std::sqrt(re * re + im * im * theta2);
            re /= s;
            im /= s;
        }
        return Eigen::Vector4d(re, im * w(0), im * w(1), im * w(2));
    }

    inline Eigen::Vector4d quat_step_pre(const Eigen::Vector4d& q, const Eigen::Vector3d& w_delta) {
        return quat_multiply(quat_exp(w_delta), q);
    }
    inline Eigen::Vector4d quat_step_post(const Eigen::Vector4d& q, const Eigen::Vector3d& w_delta) {
        return quat_multiply(q, quat_exp(w_delta));
    }

    struct alignas(32) CameraPose {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

            // Rotation is represented as a unit quaternion
            // with real part first, i.e. QW, QX, QY, QZ
            Eigen::Vector4d q;
        Eigen::Vector3d t;

        // Constructors (Defaults to identity camera)
        CameraPose() : q(1.0, 0.0, 0.0, 0.0), t(0.0, 0.0, 0.0) {}
        CameraPose(const Eigen::Vector4d& qq, const Eigen::Vector3d& tt) : q(qq), t(tt) {}
        CameraPose(const Eigen::Matrix3d& R, const Eigen::Vector3d& tt) : q(rotmat_to_quat(R)), t(tt) {}

        // Helper functions
        inline Eigen::Matrix3d R() const { return quat_to_rotmat(q); }
        inline Eigen::Matrix<double, 3, 4> Rt() const {
            Eigen::Matrix<double, 3, 4> tmp;
            tmp.block<3, 3>(0, 0) = quat_to_rotmat(q);
            tmp.col(3) = t;
            return tmp;
        }
        inline Eigen::Vector3d rotate(const Eigen::Vector3d& p) const { return quat_rotate(q, p); }
        inline Eigen::Vector3d derotate(const Eigen::Vector3d& p) const { return quat_rotate(quat_conj(q), p); }
        inline Eigen::Vector3d apply(const Eigen::Vector3d& p) const { return rotate(p) + t; }

        inline Eigen::Vector3d center() const { return -derotate(t); }
    };

    typedef std::vector<CameraPose> CameraPoseVector;

    // The new subclass with extra parameters
    struct alignas(32) MonoDepthTwoViewGeometry {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

            CameraPose pose;

        // Extra members
        double scale;
        double shift1;
        double shift2;

        MonoDepthTwoViewGeometry() : pose(), scale(1.0), shift1(0.0), shift2(0.0) {}

        MonoDepthTwoViewGeometry(const Eigen::Vector4d& qq, const Eigen::Vector3d& tt, double scale, double shift1,
            double shift2)
            : pose(qq, tt), scale(scale), shift1(shift1), shift2(shift2) {
        }

        MonoDepthTwoViewGeometry(const Eigen::Vector4d& qq, const Eigen::Vector3d& tt, double scale)
            : pose(qq, tt), scale(scale), shift1(0.0), shift2(0.0) {
        }

        MonoDepthTwoViewGeometry(const Eigen::Matrix3d& R, const Eigen::Vector3d& tt, double scale, double shift1,
            double shift2)
            : pose(R, tt), scale(scale), shift1(shift1), shift2(shift2) {
        }

        MonoDepthTwoViewGeometry(const Eigen::Matrix3d& R, const Eigen::Vector3d& tt, double scale)
            : pose(R, tt), scale(scale), shift1(0.0), shift2(0.0) {
        }

        explicit MonoDepthTwoViewGeometry(CameraPose pose) : pose(std::move(pose)), scale(1.0), shift1(0.0), shift2(0.0) {}

        MonoDepthTwoViewGeometry(CameraPose pose, double scale)
            : pose(std::move(pose)), scale(scale), shift1(0.0), shift2(0.0) {
        }

        MonoDepthTwoViewGeometry(const CameraPose& pose, double scale, double s1, double s2)
            : pose(pose), scale(scale), shift1(s1), shift2(s2) {
        }
    };

    struct alignas(32) Image {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
            // Struct simply holds information about camera and its pose
            CameraPose pose;
        Camera camera;

        // Constructors (Defaults to identity camera and pose)
        Image() : pose(CameraPose()), camera(Camera()) {}
        Image(CameraPose pose, Camera camera) : pose(std::move(pose)), camera(std::move(camera)) {}
    };

    typedef std::vector<Image> ImageVector;

    struct alignas(32) ImagePair {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
            // Struct simply holds information about two cameras and their relative pose
            CameraPose pose;
        Camera camera1;
        Camera camera2;

        // Constructors (Defaults to identity camera and poses)
        ImagePair() : pose(CameraPose()), camera1(Camera()), camera2(Camera()) {}
        ImagePair(CameraPose pose, Camera camera1, Camera camera2)
            : pose(std::move(pose)), camera1(std::move(camera1)), camera2(std::move(camera2)) {
        }
    };

    struct alignas(32) MonoDepthImagePair {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
            // Struct simply holds information about two cameras and their relative pose
            MonoDepthTwoViewGeometry geometry;
        Camera camera1;
        Camera camera2;

        // Constructors (Defaults to identity camera and poses)
        MonoDepthImagePair() : geometry(MonoDepthTwoViewGeometry()), camera1(Camera()), camera2(Camera()) {}
        MonoDepthImagePair(MonoDepthTwoViewGeometry geometry, Camera camera1, Camera camera2)
            : geometry(std::move(geometry)), camera1(std::move(camera1)), camera2(std::move(camera2)) {
        }
    };

    typedef std::vector<ImagePair> ImagePairVector;

    namespace univariate {
        /* Solves the quadratic equation a*x^2 + b*x + c = 0 */
        void solve_quadratic(double a, double b, double c, std::complex<double> roots[2]) {

            std::complex<double> b2m4ac = b * b - 4 * a * c;
            std::complex<double> sq = std::sqrt(b2m4ac);

            // Choose sign to avoid cancellations
            roots[0] = (b > 0) ? (2 * c) / (-b - sq) : (2 * c) / (-b + sq);
            roots[1] = c / (a * roots[0]);
        }

        /* Solves the quadratic equation a*x^2 + b*x + c = 0 */
        int solve_quadratic_real(double a, double b, double c, double roots[2]) {

            double b2m4ac = b * b - 4 * a * c;
            if (b2m4ac < 0)
                return 0;

            double sq = std::sqrt(b2m4ac);

            // Choose sign to avoid cancellations
            roots[0] = (b > 0) ? (2 * c) / (-b - sq) : (2 * c) / (-b + sq);
            roots[1] = c / (a * roots[0]);

            return 2;
        }

        /* Sign of component with largest magnitude */
        inline double sign2(const std::complex<double> z) {
            if (std::abs(z.real()) > std::abs(z.imag()))
                return z.real() < 0 ? -1.0 : 1.0;
            else
                return z.imag() < 0 ? -1.0 : 1.0;
        }

        /* Sign of component with largest magnitude */
        inline double sign(const double z) { return z < 0 ? -1.0 : 1.0; }

        bool solve_cubic_single_real(double c2, double c1, double c0, double& root) {
            double a = c1 - c2 * c2 / 3.0;
            double b = (2.0 * c2 * c2 * c2 - 9.0 * c2 * c1) / 27.0 + c0;
            double c = b * b / 4.0 + a * a * a / 27.0;
            if (c != 0) {
                if (c > 0) {
                    c = std::sqrt(c);
                    b *= -0.5;
                    root = std::cbrt(b + c) + std::cbrt(b - c) - c2 / 3.0;
                    return true;
                }
                else {
                    c = 3.0 * b / (2.0 * a) * std::sqrt(-3.0 / a);
                    root = 2.0 * std::sqrt(-a / 3.0) * std::cos(std::acos(c) / 3.0) - c2 / 3.0;
                }
            }
            else {
                root = -c2 / 3.0 + (a != 0 ? (3.0 * b / a) : 0);
            }
            return false;
        }

        int solve_cubic_real(double c2, double c1, double c0, double roots[3]) {
            double a = c1 - c2 * c2 / 3.0;
            double b = (2.0 * c2 * c2 * c2 - 9.0 * c2 * c1) / 27.0 + c0;
            double c = b * b / 4.0 + a * a * a / 27.0;
            int n_roots;
            if (c > 0) {
                c = std::sqrt(c);
                b *= -0.5;
                roots[0] = std::cbrt(b + c) + std::cbrt(b - c) - c2 / 3.0;
                n_roots = 1;
            }
            else {
                c = 3.0 * b / (2.0 * a) * std::sqrt(-3.0 / a);
                double d = 2.0 * std::sqrt(-a / 3.0);
                roots[0] = d * std::cos(std::acos(c) / 3.0) - c2 / 3.0;
                roots[1] = d * std::cos(std::acos(c) / 3.0 - 2.09439510239319526263557236234192) - c2 / 3.0; // 2*pi/3
                roots[2] = d * std::cos(std::acos(c) / 3.0 - 4.18879020478639052527114472468384) - c2 / 3.0; // 4*pi/3
                n_roots = 3;
            }

            // single newton iteration
            for (int i = 0; i < n_roots; ++i) {
                double x = roots[i];
                double x2 = x * x;
                double x3 = x * x2;
                double dx = -(x3 + c2 * x2 + c1 * x + c0) / (3 * x2 + 2 * c2 * x + c1);
                roots[i] += dx;
            }
            return n_roots;
        }

        /* Solves the quartic equation x^4 + b*x^3 + c*x^2 + d*x + e = 0 */
        void solve_quartic(double b, double c, double d, double e, std::complex<double> roots[4]) {

            // Find depressed quartic
            std::complex<double> p = c - 3.0 * b * b / 8.0;
            std::complex<double> q = b * b * b / 8.0 - 0.5 * b * c + d;
            std::complex<double> r = (-3.0 * b * b * b * b + 256.0 * e - 64.0 * b * d + 16.0 * b * b * c) / 256.0;

            // Resolvent cubic is now
            // U^3 + 2*p U^2 + (p^2 - 4*r) * U - q^2
            std::complex<double> bb = 2.0 * p;
            std::complex<double> cc = p * p - 4.0 * r;
            std::complex<double> dd = -q * q;

            // Solve resolvent cubic
            std::complex<double> d0 = bb * bb - 3.0 * cc;
            std::complex<double> d1 = 2.0 * bb * bb * bb - 9.0 * bb * cc + 27.0 * dd;

            std::complex<double> C3 = (d1.real() < 0) ? (d1 - sqrt(d1 * d1 - 4.0 * d0 * d0 * d0)) / 2.0
                : (d1 + sqrt(d1 * d1 - 4.0 * d0 * d0 * d0)) / 2.0;

            std::complex<double> C;
            if (C3.real() < 0)
                C = -std::pow(-C3, 1.0 / 3);
            else
                C = std::pow(C3, 1.0 / 3);

            std::complex<double> u2 = (bb + C + d0 / C) / -3.0;

            // std::complex<double> db = u2 * u2 * u2 + bb * u2 * u2 + cc * u2 + dd;

            std::complex<double> u = sqrt(u2);

            std::complex<double> s = -u;
            std::complex<double> t = (p + u * u + q / u) / 2.0;
            std::complex<double> v = (p + u * u - q / u) / 2.0;

            roots[0] = (-u - sign2(u) * sqrt(u * u - 4.0 * v)) / 2.0;
            roots[1] = v / roots[0];
            roots[2] = (-s - sign2(s) * sqrt(s * s - 4.0 * t)) / 2.0;
            roots[3] = t / roots[2];

            for (int i = 0; i < 4; i++) {
                roots[i] = roots[i] - b / 4.0;

                // do one step of newton refinement
                std::complex<double> x = roots[i];
                std::complex<double> x2 = x * x;
                std::complex<double> x3 = x * x2;
                std::complex<double> dx =
                    -(x2 * x2 + b * x3 + c * x2 + d * x + e) / (4.0 * x3 + 3.0 * b * x2 + 2.0 * c * x + d);
                roots[i] = x + dx;
            }
        }

        /* Solves the quartic equation x^4 + b*x^3 + c*x^2 + d*x + e = 0 */
        int solve_quartic_real(double b, double c, double d, double e, double roots[4]) {

            // Find depressed quartic
            double p = c - 3.0 * b * b / 8.0;
            double q = b * b * b / 8.0 - 0.5 * b * c + d;
            double r = (-3.0 * b * b * b * b + 256.0 * e - 64.0 * b * d + 16.0 * b * b * c) / 256.0;

            // Resolvent cubic is now
            // U^3 + 2*p U^2 + (p^2 - 4*r) * U - q^2
            double bb = 2.0 * p;
            double cc = p * p - 4.0 * r;
            double dd = -q * q;

            // Solve resolvent cubic
            double u2;
            solve_cubic_single_real(bb, cc, dd, u2);

            if (u2 < 0)
                return 0;

            double u = sqrt(u2);

            double s = -u;
            double t = (p + u * u + q / u) / 2.0;
            double v = (p + u * u - q / u) / 2.0;

            int sols = 0;
            double disc = u * u - 4.0 * v;
            if (disc > 0) {
                roots[0] = (-u - sign(u) * std::sqrt(disc)) / 2.0;
                roots[1] = v / roots[0];
                sols += 2;
            }
            disc = s * s - 4.0 * t;
            if (disc > 0) {
                roots[sols] = (-s - sign(s) * std::sqrt(disc)) / 2.0;
                roots[sols + 1] = t / roots[sols];
                sols += 2;
            }

            for (int i = 0; i < sols; i++) {
                roots[i] = roots[i] - b / 4.0;

                // do one step of newton refinement
                double x = roots[i];
                double x2 = x * x;
                double x3 = x * x2;
                double dx = -(x2 * x2 + b * x3 + c * x2 + d * x + e) / (4.0 * x3 + 3.0 * b * x2 + 2.0 * c * x + d);
                roots[i] = x + dx;
            }
            return sols;
        }
    } // univariate

    namespace re3q3 {

        /* Homogeneous linear constraints on rotation matrix
             Rcoeffs*R(:) = 0
          converted into 3q3 problem. */
        void rotation_to_3q3(const Eigen::Matrix<double, 3, 9>& Rcoeffs, Eigen::Matrix<double, 3, 10>* coeffs) {
            for (int k = 0; k < 3; k++) {
                (*coeffs)(k, 0) = Rcoeffs(k, 0) - Rcoeffs(k, 4) - Rcoeffs(k, 8);
                (*coeffs)(k, 1) = 2 * Rcoeffs(k, 1) + 2 * Rcoeffs(k, 3);
                (*coeffs)(k, 2) = 2 * Rcoeffs(k, 2) + 2 * Rcoeffs(k, 6);
                (*coeffs)(k, 3) = Rcoeffs(k, 4) - Rcoeffs(k, 0) - Rcoeffs(k, 8);
                (*coeffs)(k, 4) = 2 * Rcoeffs(k, 5) + 2 * Rcoeffs(k, 7);
                (*coeffs)(k, 5) = Rcoeffs(k, 8) - Rcoeffs(k, 4) - Rcoeffs(k, 0);
                (*coeffs)(k, 6) = 2 * Rcoeffs(k, 5) - 2 * Rcoeffs(k, 7);
                (*coeffs)(k, 7) = 2 * Rcoeffs(k, 6) - 2 * Rcoeffs(k, 2);
                (*coeffs)(k, 8) = 2 * Rcoeffs(k, 1) - 2 * Rcoeffs(k, 3);
                (*coeffs)(k, 9) = Rcoeffs(k, 0) + Rcoeffs(k, 4) + Rcoeffs(k, 8);
            }
        }

        /* Inhomogeneous linear constraints on rotation matrix
             Rcoeffs*[R(:);1] = 0
          converted into 3q3 problem. */
        void rotation_to_3q3(const Eigen::Matrix<double, 3, 10>& Rcoeffs, Eigen::Matrix<double, 3, 10>* coeffs) {
            for (int k = 0; k < 3; k++) {
                (*coeffs)(k, 0) = Rcoeffs(k, 0) - Rcoeffs(k, 4) - Rcoeffs(k, 8) + Rcoeffs(k, 9);
                (*coeffs)(k, 1) = 2 * Rcoeffs(k, 1) + 2 * Rcoeffs(k, 3);
                (*coeffs)(k, 2) = 2 * Rcoeffs(k, 2) + 2 * Rcoeffs(k, 6);
                (*coeffs)(k, 3) = Rcoeffs(k, 4) - Rcoeffs(k, 0) - Rcoeffs(k, 8) + Rcoeffs(k, 9);
                (*coeffs)(k, 4) = 2 * Rcoeffs(k, 5) + 2 * Rcoeffs(k, 7);
                (*coeffs)(k, 5) = Rcoeffs(k, 8) - Rcoeffs(k, 4) - Rcoeffs(k, 0) + Rcoeffs(k, 9);
                (*coeffs)(k, 6) = 2 * Rcoeffs(k, 5) - 2 * Rcoeffs(k, 7);
                (*coeffs)(k, 7) = 2 * Rcoeffs(k, 6) - 2 * Rcoeffs(k, 2);
                (*coeffs)(k, 8) = 2 * Rcoeffs(k, 1) - 2 * Rcoeffs(k, 3);
                (*coeffs)(k, 9) = Rcoeffs(k, 0) + Rcoeffs(k, 4) + Rcoeffs(k, 8) + Rcoeffs(k, 9);
            }
        }

        void cayley_param(const Eigen::Matrix<double, 3, 1>& c, Eigen::Matrix<double, 3, 3>* R) {
            *R << c(0) * c(0) - c(1) * c(1) - c(2) * c(2) + 1, 2 * c(0) * c(1) - 2 * c(2), 2 * c(1) + 2 * c(0) * c(2),
                2 * c(2) + 2 * c(0) * c(1), c(1)* c(1) - c(0) * c(0) - c(2) * c(2) + 1, 2 * c(1) * c(2) - 2 * c(0),
                2 * c(0) * c(2) - 2 * c(1), 2 * c(0) + 2 * c(1) * c(2), c(2)* c(2) - c(1) * c(1) - c(0) * c(0) + 1;
            *R /= 1 + c(0) * c(0) + c(1) * c(1) + c(2) * c(2);
        }

        inline void refine_3q3(const Eigen::Matrix<double, 3, 10>& coeffs, Eigen::Matrix<double, 3, 8>* solutions, int n_sols) {
            Eigen::Matrix3d J;
            Eigen::Vector3d r;
            Eigen::Vector3d dx;
            double x, y, z;

            for (int i = 0; i < n_sols; ++i) {
                x = (*solutions)(0, i);
                y = (*solutions)(1, i);
                z = (*solutions)(2, i);

                // [x^2, x*y, x*z, y^2, y*z, z^2, x, y, z, 1.0]
                for (int iter = 0; iter < 5; ++iter) {
                    r = coeffs.col(0) * x * x + coeffs.col(1) * x * y + coeffs.col(2) * x * z + coeffs.col(3) * y * y +
                        coeffs.col(4) * y * z + coeffs.col(5) * z * z + coeffs.col(6) * x + coeffs.col(7) * y +
                        coeffs.col(8) * z + coeffs.col(9);

                    if (r.cwiseAbs().maxCoeff() < 1e-8)
                        break;

                    J.col(0) = 2.0 * coeffs.col(0) * x + coeffs.col(1) * y + coeffs.col(2) * z + coeffs.col(6);
                    J.col(1) = coeffs.col(1) * x + 2.0 * coeffs.col(3) * y + coeffs.col(4) * z + coeffs.col(7);
                    J.col(2) = coeffs.col(2) * x + coeffs.col(4) * y + 2.0 * coeffs.col(5) * z + coeffs.col(8);

                    dx = J.inverse() * r;

                    x -= dx(0);
                    y -= dx(1);
                    z -= dx(2);
                }

                (*solutions)(0, i) = x;
                (*solutions)(1, i) = y;
                (*solutions)(2, i) = z;
            }
        }

        /*
 * Order of coefficients is:  x^2, xy, xz, y^2, yz, z^2, x, y, z, 1.0;
 *
 */
        int re3q3(const Eigen::Matrix<double, 3, 10>& coeffs, Eigen::Matrix<double, 3, 8>* solutions,
            bool try_random_var_change) {
            try_random_var_change = true;

            Eigen::Matrix<double, 3, 3> Ax, Ay, Az;
            Ax << coeffs.col(3), coeffs.col(5), coeffs.col(4); // y^2, z^2, yz
            Ay << coeffs.col(0), coeffs.col(5), coeffs.col(2); // x^2, z^2, xz
            Az << coeffs.col(3), coeffs.col(0), coeffs.col(1); // y^2, x^2, yx

            // We check det(A) as a cheaper proxy for condition number
            int elim_var = 0;
            double detx = std::abs(Ax.determinant());
            double dety = std::abs(Ay.determinant());
            double detz = std::abs(Az.determinant());
            double det = detx;
            if (det < dety) {
                det = dety;
                elim_var = 1;
            }
            if (det < detz) {
                det = detz;
                elim_var = 2;
            }

            if (try_random_var_change && det < 1e-10) {
                Eigen::Matrix<double, 3, 4> A;
                A.block<3, 3>(0, 0) = Eigen::Quaternion<double>::UnitRandom().toRotationMatrix();
                A.block<3, 1>(0, 3).setRandom().normalize();

                Eigen::Matrix<double, 10, 10> B;
                B << A(0, 0) * A(0, 0), 2 * A(0, 0) * A(0, 1), 2 * A(0, 0) * A(0, 2), A(0, 1)* A(0, 1), 2 * A(0, 1) * A(0, 2),
                    A(0, 2)* A(0, 2), 2 * A(0, 0) * A(0, 3), 2 * A(0, 1) * A(0, 3), 2 * A(0, 2) * A(0, 3), A(0, 3)* A(0, 3),
                    A(0, 0)* A(1, 0), A(0, 0)* A(1, 1) + A(0, 1) * A(1, 0), A(0, 0)* A(1, 2) + A(0, 2) * A(1, 0),
                    A(0, 1)* A(1, 1), A(0, 1)* A(1, 2) + A(0, 2) * A(1, 1), A(0, 2)* A(1, 2),
                    A(0, 0)* A(1, 3) + A(0, 3) * A(1, 0), A(0, 1)* A(1, 3) + A(0, 3) * A(1, 1),
                    A(0, 2)* A(1, 3) + A(0, 3) * A(1, 2), A(0, 3)* A(1, 3), A(0, 0)* A(2, 0),
                    A(0, 0)* A(2, 1) + A(0, 1) * A(2, 0), A(0, 0)* A(2, 2) + A(0, 2) * A(2, 0), A(0, 1)* A(2, 1),
                    A(0, 1)* A(2, 2) + A(0, 2) * A(2, 1), A(0, 2)* A(2, 2), A(0, 0)* A(2, 3) + A(0, 3) * A(2, 0),
                    A(0, 1)* A(2, 3) + A(0, 3) * A(2, 1), A(0, 2)* A(2, 3) + A(0, 3) * A(2, 2), A(0, 3)* A(2, 3),
                    A(1, 0)* A(1, 0), 2 * A(1, 0) * A(1, 1), 2 * A(1, 0) * A(1, 2), A(1, 1)* A(1, 1), 2 * A(1, 1) * A(1, 2),
                    A(1, 2)* A(1, 2), 2 * A(1, 0) * A(1, 3), 2 * A(1, 1) * A(1, 3), 2 * A(1, 2) * A(1, 3), A(1, 3)* A(1, 3),
                    A(1, 0)* A(2, 0), A(1, 0)* A(2, 1) + A(1, 1) * A(2, 0), A(1, 0)* A(2, 2) + A(1, 2) * A(2, 0),
                    A(1, 1)* A(2, 1), A(1, 1)* A(2, 2) + A(1, 2) * A(2, 1), A(1, 2)* A(2, 2),
                    A(1, 0)* A(2, 3) + A(1, 3) * A(2, 0), A(1, 1)* A(2, 3) + A(1, 3) * A(2, 1),
                    A(1, 2)* A(2, 3) + A(1, 3) * A(2, 2), A(1, 3)* A(2, 3), A(2, 0)* A(2, 0), 2 * A(2, 0) * A(2, 1),
                    2 * A(2, 0) * A(2, 2), A(2, 1)* A(2, 1), 2 * A(2, 1) * A(2, 2), A(2, 2)* A(2, 2), 2 * A(2, 0) * A(2, 3),
                    2 * A(2, 1) * A(2, 3), 2 * A(2, 2) * A(2, 3), A(2, 3)* A(2, 3), 0, 0, 0, 0, 0, 0, A(0, 0), A(0, 1),
                    A(0, 2), A(0, 3), 0, 0, 0, 0, 0, 0, A(1, 0), A(1, 1), A(1, 2), A(1, 3), 0, 0, 0, 0, 0, 0, A(2, 0), A(2, 1),
                    A(2, 2), A(2, 3), 0, 0, 0, 0, 0, 0, 0, 0, 0, 1;
                Eigen::Matrix<double, 3, 10> coeffsB = coeffs * B;

                int n_sols = poselib::re3q3::re3q3(coeffsB, solutions, false);

                // Revert change of variables
                for (int k = 0; k < n_sols; k++) {
                    solutions->col(k) = A.block<3, 3>(0, 0) * solutions->col(k) + A.col(3);
                }

                // In some cases the numerics are quite poor after the change of variables, so we do some newton steps with the
                // original coefficients.
                refine_3q3(coeffs, solutions, n_sols);

                return n_sols;
            }

            Eigen::Matrix<double, 3, 7> P;

            if (elim_var == 0) {
                // re-order columns to eliminate x (target: y^2 z^2 yz x^2 xy xz x y z 1)
                P << coeffs.col(0), coeffs.col(1), coeffs.col(2), coeffs.col(6), coeffs.col(7), coeffs.col(8), coeffs.col(9);
                P = -Ax.inverse() * P;
            }
            else if (elim_var == 1) {
                // re-order columns to eliminate y (target: x^2 z^2 xz y^2 xy yz y x z 1)
                P << coeffs.col(3), coeffs.col(1), coeffs.col(4), coeffs.col(7), coeffs.col(6), coeffs.col(8), coeffs.col(9);
                P = -Ay.inverse() * P;
            }
            else if (elim_var == 2) {
                // re-order columns to eliminate z (target: y^2 x^2 yx z^2 zy z y x 1)
                P << coeffs.col(5), coeffs.col(4), coeffs.col(2), coeffs.col(8), coeffs.col(7), coeffs.col(6), coeffs.col(9);
                P = -Az.inverse() * P;
            }

            double a11 = P(0, 1) * P(2, 1) + P(0, 2) * P(1, 1) - P(2, 1) * P(0, 1) - P(2, 2) * P(2, 1) - P(2, 0);
            double a12 = P(0, 1) * P(2, 4) + P(0, 4) * P(2, 1) + P(0, 2) * P(1, 4) + P(0, 5) * P(1, 1) - P(2, 1) * P(0, 4) -
                P(2, 4) * P(0, 1) - P(2, 2) * P(2, 4) - P(2, 5) * P(2, 1) - P(2, 3);
            double a13 = P(0, 4) * P(2, 4) + P(0, 5) * P(1, 4) - P(2, 4) * P(0, 4) - P(2, 5) * P(2, 4) - P(2, 6);
            double a14 = P(0, 1) * P(2, 2) + P(0, 2) * P(1, 2) - P(2, 1) * P(0, 2) - P(2, 2) * P(2, 2) + P(0, 0);
            double a15 = P(0, 1) * P(2, 5) + P(0, 4) * P(2, 2) + P(0, 2) * P(1, 5) + P(0, 5) * P(1, 2) - P(2, 1) * P(0, 5) -
                P(2, 4) * P(0, 2) - P(2, 2) * P(2, 5) - P(2, 5) * P(2, 2) + P(0, 3);
            double a16 = P(0, 4) * P(2, 5) + P(0, 5) * P(1, 5) - P(2, 4) * P(0, 5) - P(2, 5) * P(2, 5) + P(0, 6);
            double a17 = P(0, 1) * P(2, 0) + P(0, 2) * P(1, 0) - P(2, 1) * P(0, 0) - P(2, 2) * P(2, 0);
            double a18 = P(0, 1) * P(2, 3) + P(0, 4) * P(2, 0) + P(0, 2) * P(1, 3) + P(0, 5) * P(1, 0) - P(2, 1) * P(0, 3) -
                P(2, 4) * P(0, 0) - P(2, 2) * P(2, 3) - P(2, 5) * P(2, 0);
            double a19 = P(0, 1) * P(2, 6) + P(0, 4) * P(2, 3) + P(0, 2) * P(1, 6) + P(0, 5) * P(1, 3) - P(2, 1) * P(0, 6) -
                P(2, 4) * P(0, 3) - P(2, 2) * P(2, 6) - P(2, 5) * P(2, 3);
            double a110 = P(0, 4) * P(2, 6) + P(0, 5) * P(1, 6) - P(2, 4) * P(0, 6) - P(2, 5) * P(2, 6);

            double a21 = P(2, 1) * P(2, 1) + P(2, 2) * P(1, 1) - P(1, 1) * P(0, 1) - P(1, 2) * P(2, 1) - P(1, 0);
            double a22 = P(2, 1) * P(2, 4) + P(2, 4) * P(2, 1) + P(2, 2) * P(1, 4) + P(2, 5) * P(1, 1) - P(1, 1) * P(0, 4) -
                P(1, 4) * P(0, 1) - P(1, 2) * P(2, 4) - P(1, 5) * P(2, 1) - P(1, 3);
            double a23 = P(2, 4) * P(2, 4) + P(2, 5) * P(1, 4) - P(1, 4) * P(0, 4) - P(1, 5) * P(2, 4) - P(1, 6);
            double a24 = P(2, 1) * P(2, 2) + P(2, 2) * P(1, 2) - P(1, 1) * P(0, 2) - P(1, 2) * P(2, 2) + P(2, 0);
            double a25 = P(2, 1) * P(2, 5) + P(2, 4) * P(2, 2) + P(2, 2) * P(1, 5) + P(2, 5) * P(1, 2) - P(1, 1) * P(0, 5) -
                P(1, 4) * P(0, 2) - P(1, 2) * P(2, 5) - P(1, 5) * P(2, 2) + P(2, 3);
            double a26 = P(2, 4) * P(2, 5) + P(2, 5) * P(1, 5) - P(1, 4) * P(0, 5) - P(1, 5) * P(2, 5) + P(2, 6);
            double a27 = P(2, 1) * P(2, 0) + P(2, 2) * P(1, 0) - P(1, 1) * P(0, 0) - P(1, 2) * P(2, 0);
            double a28 = P(2, 1) * P(2, 3) + P(2, 4) * P(2, 0) + P(2, 2) * P(1, 3) + P(2, 5) * P(1, 0) - P(1, 1) * P(0, 3) -
                P(1, 4) * P(0, 0) - P(1, 2) * P(2, 3) - P(1, 5) * P(2, 0);
            double a29 = P(2, 1) * P(2, 6) + P(2, 4) * P(2, 3) + P(2, 2) * P(1, 6) + P(2, 5) * P(1, 3) - P(1, 1) * P(0, 6) -
                P(1, 4) * P(0, 3) - P(1, 2) * P(2, 6) - P(1, 5) * P(2, 3);
            double a210 = P(2, 4) * P(2, 6) + P(2, 5) * P(1, 6) - P(1, 4) * P(0, 6) - P(1, 5) * P(2, 6);

            double t2 = P(2, 1) * P(2, 1);
            double t3 = P(2, 2) * P(2, 2);
            double t4 = P(0, 1) * P(1, 4);
            double t5 = P(0, 4) * P(1, 1);
            double t6 = t4 + t5;
            double t7 = P(0, 2) * P(1, 5);
            double t8 = P(0, 5) * P(1, 2);
            double t9 = t7 + t8;
            double t10 = P(0, 1) * P(1, 5);
            double t11 = P(0, 4) * P(1, 2);
            double t12 = t10 + t11;
            double t13 = P(0, 2) * P(1, 4);
            double t14 = P(0, 5) * P(1, 1);
            double t15 = t13 + t14;
            double t16 = P(2, 1) * P(2, 5);
            double t17 = P(2, 2) * P(2, 4);
            double t18 = t16 + t17;
            double t19 = P(2, 4) * P(2, 4);
            double t20 = P(2, 5) * P(2, 5);
            double a31 = P(0, 0) * P(1, 1) + P(0, 1) * P(1, 0) - P(2, 0) * P(2, 1) * 2.0 - P(0, 1) * t2 - P(1, 1) * t3 -
                P(2, 2) * t2 * 2.0 + (P(0, 1) * P(0, 1)) * P(1, 1) + P(0, 2) * P(1, 1) * P(1, 2) +
                P(0, 1) * P(1, 2) * P(2, 1) + P(0, 2) * P(1, 1) * P(2, 1);
            double a32 = P(0, 0) * P(1, 4) + P(0, 1) * P(1, 3) + P(0, 3) * P(1, 1) + P(0, 4) * P(1, 0) -
                P(2, 0) * P(2, 4) * 2.0 - P(2, 1) * P(2, 3) * 2.0 - P(0, 4) * t2 + P(0, 1) * t6 - P(1, 4) * t3 +
                P(1, 1) * t9 + P(2, 1) * t12 + P(2, 1) * t15 - P(2, 1) * t18 * 2.0 + P(0, 1) * P(0, 4) * P(1, 1) +
                P(0, 2) * P(1, 2) * P(1, 4) + P(0, 1) * P(1, 2) * P(2, 4) + P(0, 2) * P(1, 1) * P(2, 4) -
                P(0, 1) * P(2, 1) * P(2, 4) * 2.0 - P(1, 1) * P(2, 2) * P(2, 5) * 2.0 -
                P(2, 1) * P(2, 2) * P(2, 4) * 2.0;
            double a33 = P(0, 1) * P(1, 6) + P(0, 3) * P(1, 4) + P(0, 4) * P(1, 3) + P(0, 6) * P(1, 1) -
                P(2, 1) * P(2, 6) * 2.0 - P(2, 3) * P(2, 4) * 2.0 + P(0, 4) * t6 - P(0, 1) * t19 + P(1, 4) * t9 -
                P(1, 1) * t20 + P(2, 4) * t12 + P(2, 4) * t15 - P(2, 4) * t18 * 2.0 + P(0, 1) * P(0, 4) * P(1, 4) +
                P(0, 5) * P(1, 1) * P(1, 5) + P(0, 4) * P(1, 5) * P(2, 1) + P(0, 5) * P(1, 4) * P(2, 1) -
                P(0, 4) * P(2, 1) * P(2, 4) * 2.0 - P(1, 4) * P(2, 2) * P(2, 5) * 2.0 -
                P(2, 1) * P(2, 4) * P(2, 5) * 2.0;
            double a34 = P(0, 4) * P(1, 6) + P(0, 6) * P(1, 4) - P(2, 4) * P(2, 6) * 2.0 - P(0, 4) * t19 - P(1, 4) * t20 -
                P(2, 5) * t19 * 2.0 + (P(0, 4) * P(0, 4)) * P(1, 4) + P(0, 5) * P(1, 4) * P(1, 5) +
                P(0, 4) * P(1, 5) * P(2, 4) + P(0, 5) * P(1, 4) * P(2, 4);
            double a35 = P(0, 0) * P(1, 2) + P(0, 2) * P(1, 0) - P(2, 0) * P(2, 2) * 2.0 - P(0, 2) * t2 - P(1, 2) * t3 -
                P(2, 1) * t3 * 2.0 + P(0, 2) * (P(1, 2) * P(1, 2)) + P(0, 1) * P(0, 2) * P(1, 1) +
                P(0, 1) * P(1, 2) * P(2, 2) + P(0, 2) * P(1, 1) * P(2, 2);
            double a36 = P(0, 0) * P(1, 5) + P(0, 2) * P(1, 3) + P(0, 3) * P(1, 2) + P(0, 5) * P(1, 0) -
                P(2, 0) * P(2, 5) * 2.0 - P(2, 2) * P(2, 3) * 2.0 - P(0, 5) * t2 + P(0, 2) * t6 - P(1, 5) * t3 +
                P(1, 2) * t9 + P(2, 2) * t12 + P(2, 2) * t15 - P(2, 2) * t18 * 2.0 + P(0, 1) * P(0, 5) * P(1, 1) +
                P(0, 2) * P(1, 2) * P(1, 5) + P(0, 1) * P(1, 2) * P(2, 5) + P(0, 2) * P(1, 1) * P(2, 5) -
                P(0, 2) * P(2, 1) * P(2, 4) * 2.0 - P(1, 2) * P(2, 2) * P(2, 5) * 2.0 -
                P(2, 1) * P(2, 2) * P(2, 5) * 2.0;
            double a37 = P(0, 2) * P(1, 6) + P(0, 3) * P(1, 5) + P(0, 5) * P(1, 3) + P(0, 6) * P(1, 2) -
                P(2, 2) * P(2, 6) * 2.0 - P(2, 3) * P(2, 5) * 2.0 + P(0, 5) * t6 - P(0, 2) * t19 + P(1, 5) * t9 -
                P(1, 2) * t20 + P(2, 5) * t12 + P(2, 5) * t15 - P(2, 5) * t18 * 2.0 + P(0, 2) * P(0, 4) * P(1, 4) +
                P(0, 5) * P(1, 2) * P(1, 5) + P(0, 4) * P(1, 5) * P(2, 2) + P(0, 5) * P(1, 4) * P(2, 2) -
                P(0, 5) * P(2, 1) * P(2, 4) * 2.0 - P(1, 5) * P(2, 2) * P(2, 5) * 2.0 -
                P(2, 2) * P(2, 4) * P(2, 5) * 2.0;
            double a38 = P(0, 5) * P(1, 6) + P(0, 6) * P(1, 5) - P(2, 5) * P(2, 6) * 2.0 - P(0, 5) * t19 - P(1, 5) * t20 -
                P(2, 4) * t20 * 2.0 + P(0, 5) * (P(1, 5) * P(1, 5)) + P(0, 4) * P(0, 5) * P(1, 4) +
                P(0, 4) * P(1, 5) * P(2, 5) + P(0, 5) * P(1, 4) * P(2, 5);
            double a39 = P(0, 0) * P(1, 0) - P(0, 0) * t2 - P(1, 0) * t3 - P(2, 0) * P(2, 0) + P(0, 0) * P(0, 1) * P(1, 1) +
                P(0, 2) * P(1, 0) * P(1, 2) + P(0, 1) * P(1, 2) * P(2, 0) + P(0, 2) * P(1, 1) * P(2, 0) -
                P(2, 0) * P(2, 1) * P(2, 2) * 2.0;
            double a310 = P(0, 0) * P(1, 3) + P(0, 3) * P(1, 0) - P(2, 0) * P(2, 3) * 2.0 - P(0, 3) * t2 + P(0, 0) * t6 -
                P(1, 3) * t3 + P(1, 0) * t9 + P(2, 0) * t12 + P(2, 0) * t15 - P(2, 0) * t18 * 2.0 +
                P(0, 1) * P(0, 3) * P(1, 1) + P(0, 2) * P(1, 2) * P(1, 3) + P(0, 1) * P(1, 2) * P(2, 3) +
                P(0, 2) * P(1, 1) * P(2, 3) - P(0, 0) * P(2, 1) * P(2, 4) * 2.0 - P(1, 0) * P(2, 2) * P(2, 5) * 2.0 -
                P(2, 1) * P(2, 2) * P(2, 3) * 2.0;
            double a311 = P(0, 0) * P(1, 6) + P(0, 3) * P(1, 3) + P(0, 6) * P(1, 0) - P(2, 0) * P(2, 6) * 2.0 - P(0, 6) * t2 +
                P(0, 3) * t6 - P(0, 0) * t19 - P(1, 6) * t3 + P(1, 3) * t9 - P(1, 0) * t20 + P(2, 3) * t12 +
                P(2, 3) * t15 - P(2, 3) * t18 * 2.0 - P(2, 3) * P(2, 3) + P(0, 0) * P(0, 4) * P(1, 4) +
                P(0, 1) * P(0, 6) * P(1, 1) + P(0, 2) * P(1, 2) * P(1, 6) + P(0, 5) * P(1, 0) * P(1, 5) +
                P(0, 1) * P(1, 2) * P(2, 6) + P(0, 2) * P(1, 1) * P(2, 6) + P(0, 4) * P(1, 5) * P(2, 0) +
                P(0, 5) * P(1, 4) * P(2, 0) - P(0, 3) * P(2, 1) * P(2, 4) * 2.0 - P(1, 3) * P(2, 2) * P(2, 5) * 2.0 -
                P(2, 0) * P(2, 4) * P(2, 5) * 2.0 - P(2, 1) * P(2, 2) * P(2, 6) * 2.0;
            double a312 = P(0, 3) * P(1, 6) + P(0, 6) * P(1, 3) - P(2, 3) * P(2, 6) * 2.0 + P(0, 6) * t6 - P(0, 3) * t19 +
                P(1, 6) * t9 - P(1, 3) * t20 + P(2, 6) * t12 + P(2, 6) * t15 - P(2, 6) * t18 * 2.0 +
                P(0, 3) * P(0, 4) * P(1, 4) + P(0, 5) * P(1, 3) * P(1, 5) + P(0, 4) * P(1, 5) * P(2, 3) +
                P(0, 5) * P(1, 4) * P(2, 3) - P(0, 6) * P(2, 1) * P(2, 4) * 2.0 - P(1, 6) * P(2, 2) * P(2, 5) * 2.0 -
                P(2, 3) * P(2, 4) * P(2, 5) * 2.0;
            double a313 = P(0, 6) * P(1, 6) - P(0, 6) * t19 - P(1, 6) * t20 - P(2, 6) * P(2, 6) + P(0, 4) * P(0, 6) * P(1, 4) +
                P(0, 5) * P(1, 5) * P(1, 6) + P(0, 4) * P(1, 5) * P(2, 6) + P(0, 5) * P(1, 4) * P(2, 6) -
                P(2, 4) * P(2, 5) * P(2, 6) * 2.0;

            // det(M(x))
            double c[9];
            c[8] = a14 * a27 * a31 - a17 * a24 * a31 - a11 * a27 * a35 + a17 * a21 * a35 + a11 * a24 * a39 - a14 * a21 * a39;
            c[7] = a14 * a27 * a32 + a14 * a28 * a31 + a15 * a27 * a31 - a17 * a24 * a32 - a17 * a25 * a31 - a18 * a24 * a31 -
                a11 * a27 * a36 - a11 * a28 * a35 - a12 * a27 * a35 + a17 * a21 * a36 + a17 * a22 * a35 + a18 * a21 * a35 +
                a11 * a25 * a39 + a12 * a24 * a39 - a14 * a22 * a39 - a15 * a21 * a39 + a11 * a24 * a310 - a14 * a21 * a310;
            c[6] = a14 * a27 * a33 + a14 * a28 * a32 + a14 * a29 * a31 + a15 * a27 * a32 + a15 * a28 * a31 + a16 * a27 * a31 -
                a17 * a24 * a33 - a17 * a25 * a32 - a17 * a26 * a31 - a18 * a24 * a32 - a18 * a25 * a31 - a19 * a24 * a31 -
                a11 * a27 * a37 - a11 * a28 * a36 - a11 * a29 * a35 - a12 * a27 * a36 - a12 * a28 * a35 - a13 * a27 * a35 +
                a17 * a21 * a37 + a17 * a22 * a36 + a17 * a23 * a35 + a18 * a21 * a36 + a18 * a22 * a35 + a19 * a21 * a35 +
                a11 * a26 * a39 + a12 * a25 * a39 + a13 * a24 * a39 - a14 * a23 * a39 - a15 * a22 * a39 - a16 * a21 * a39 +
                a11 * a24 * a311 + a11 * a25 * a310 + a12 * a24 * a310 - a14 * a21 * a311 - a14 * a22 * a310 -
                a15 * a21 * a310;
            c[5] = a14 * a27 * a34 + a14 * a28 * a33 + a14 * a29 * a32 + a15 * a27 * a33 + a15 * a28 * a32 + a15 * a29 * a31 +
                a16 * a27 * a32 + a16 * a28 * a31 - a17 * a24 * a34 - a17 * a25 * a33 - a17 * a26 * a32 - a18 * a24 * a33 -
                a18 * a25 * a32 - a18 * a26 * a31 - a19 * a24 * a32 - a19 * a25 * a31 - a11 * a27 * a38 - a11 * a28 * a37 -
                a11 * a29 * a36 - a12 * a27 * a37 - a12 * a28 * a36 - a12 * a29 * a35 - a13 * a27 * a36 - a13 * a28 * a35 +
                a17 * a21 * a38 + a17 * a22 * a37 + a17 * a23 * a36 + a18 * a21 * a37 + a18 * a22 * a36 + a18 * a23 * a35 +
                a19 * a21 * a36 + a19 * a22 * a35 + a12 * a26 * a39 + a13 * a25 * a39 - a15 * a23 * a39 - a16 * a22 * a39 -
                a24 * a31 * a110 + a21 * a35 * a110 + a14 * a31 * a210 - a11 * a35 * a210 + a11 * a24 * a312 +
                a11 * a25 * a311 + a11 * a26 * a310 + a12 * a24 * a311 + a12 * a25 * a310 + a13 * a24 * a310 -
                a14 * a21 * a312 - a14 * a22 * a311 - a14 * a23 * a310 - a15 * a21 * a311 - a15 * a22 * a310 -
                a16 * a21 * a310;
            c[4] = a14 * a28 * a34 + a14 * a29 * a33 + a15 * a27 * a34 + a15 * a28 * a33 + a15 * a29 * a32 + a16 * a27 * a33 +
                a16 * a28 * a32 + a16 * a29 * a31 - a17 * a25 * a34 - a17 * a26 * a33 - a18 * a24 * a34 - a18 * a25 * a33 -
                a18 * a26 * a32 - a19 * a24 * a33 - a19 * a25 * a32 - a19 * a26 * a31 - a11 * a28 * a38 - a11 * a29 * a37 -
                a12 * a27 * a38 - a12 * a28 * a37 - a12 * a29 * a36 - a13 * a27 * a37 - a13 * a28 * a36 - a13 * a29 * a35 +
                a17 * a22 * a38 + a17 * a23 * a37 + a18 * a21 * a38 + a18 * a22 * a37 + a18 * a23 * a36 + a19 * a21 * a37 +
                a19 * a22 * a36 + a19 * a23 * a35 + a13 * a26 * a39 - a16 * a23 * a39 - a24 * a32 * a110 - a25 * a31 * a110 +
                a21 * a36 * a110 + a22 * a35 * a110 + a14 * a32 * a210 + a15 * a31 * a210 - a11 * a36 * a210 -
                a12 * a35 * a210 + a11 * a24 * a313 + a11 * a25 * a312 + a11 * a26 * a311 + a12 * a24 * a312 +
                a12 * a25 * a311 + a12 * a26 * a310 + a13 * a24 * a311 + a13 * a25 * a310 - a14 * a21 * a313 -
                a14 * a22 * a312 - a14 * a23 * a311 - a15 * a21 * a312 - a15 * a22 * a311 - a15 * a23 * a310 -
                a16 * a21 * a311 - a16 * a22 * a310;
            c[3] = a14 * a29 * a34 + a15 * a28 * a34 + a15 * a29 * a33 + a16 * a27 * a34 + a16 * a28 * a33 + a16 * a29 * a32 -
                a17 * a26 * a34 - a18 * a25 * a34 - a18 * a26 * a33 - a19 * a24 * a34 - a19 * a25 * a33 - a19 * a26 * a32 -
                a11 * a29 * a38 - a12 * a28 * a38 - a12 * a29 * a37 - a13 * a27 * a38 - a13 * a28 * a37 - a13 * a29 * a36 +
                a17 * a23 * a38 + a18 * a22 * a38 + a18 * a23 * a37 + a19 * a21 * a38 + a19 * a22 * a37 + a19 * a23 * a36 -
                a24 * a33 * a110 - a25 * a32 * a110 - a26 * a31 * a110 + a21 * a37 * a110 + a22 * a36 * a110 +
                a23 * a35 * a110 + a14 * a33 * a210 + a15 * a32 * a210 + a16 * a31 * a210 - a11 * a37 * a210 -
                a12 * a36 * a210 - a13 * a35 * a210 + a11 * a25 * a313 + a11 * a26 * a312 + a12 * a24 * a313 +
                a12 * a25 * a312 + a12 * a26 * a311 + a13 * a24 * a312 + a13 * a25 * a311 + a13 * a26 * a310 -
                a14 * a22 * a313 - a14 * a23 * a312 - a15 * a21 * a313 - a15 * a22 * a312 - a15 * a23 * a311 -
                a16 * a21 * a312 - a16 * a22 * a311 - a16 * a23 * a310;
            c[2] = a15 * a29 * a34 + a16 * a28 * a34 + a16 * a29 * a33 - a18 * a26 * a34 - a19 * a25 * a34 - a19 * a26 * a33 -
                a12 * a29 * a38 - a13 * a28 * a38 - a13 * a29 * a37 + a18 * a23 * a38 + a19 * a22 * a38 + a19 * a23 * a37 -
                a24 * a34 * a110 - a25 * a33 * a110 - a26 * a32 * a110 + a21 * a38 * a110 + a22 * a37 * a110 +
                a23 * a36 * a110 + a14 * a34 * a210 + a15 * a33 * a210 + a16 * a32 * a210 - a11 * a38 * a210 -
                a12 * a37 * a210 - a13 * a36 * a210 + a11 * a26 * a313 + a12 * a25 * a313 + a12 * a26 * a312 +
                a13 * a24 * a313 + a13 * a25 * a312 + a13 * a26 * a311 - a14 * a23 * a313 - a15 * a22 * a313 -
                a15 * a23 * a312 - a16 * a21 * a313 - a16 * a22 * a312 - a16 * a23 * a311;
            c[1] = a16 * a29 * a34 - a19 * a26 * a34 - a13 * a29 * a38 + a19 * a23 * a38 - a25 * a34 * a110 - a26 * a33 * a110 +
                a22 * a38 * a110 + a23 * a37 * a110 + a15 * a34 * a210 + a16 * a33 * a210 - a12 * a38 * a210 -
                a13 * a37 * a210 + a12 * a26 * a313 + a13 * a25 * a313 + a13 * a26 * a312 - a15 * a23 * a313 -
                a16 * a22 * a313 - a16 * a23 * a312;
            c[0] = -a26 * a34 * a110 + a23 * a38 * a110 + a16 * a34 * a210 - a13 * a38 * a210 + a13 * a26 * a313 -
                a16 * a23 * a313;

            double roots[8];

            int n_roots = bisect_sturm<8>(c, roots);

            Eigen::Matrix<double, 3, 3> A;
            for (int i = 0; i < n_roots; ++i) {
                double xs1 = roots[i];
                double xs2 = xs1 * xs1;
                double xs3 = xs1 * xs2;
                double xs4 = xs1 * xs3;

                A << a11 * xs2 + a12 * xs1 + a13, a14* xs2 + a15 * xs1 + a16, a17* xs3 + a18 * xs2 + a19 * xs1 + a110,
                    a21* xs2 + a22 * xs1 + a23, a24* xs2 + a25 * xs1 + a26, a27* xs3 + a28 * xs2 + a29 * xs1 + a210,
                    a31* xs3 + a32 * xs2 + a33 * xs1 + a34, a35* xs3 + a36 * xs2 + a37 * xs1 + a38,
                    a39* xs4 + a310 * xs3 + a311 * xs2 + a312 * xs1 + a313;

                (*solutions)(0, i) = xs1;
                (*solutions)(1, i) = (A(1, 2) * A(0, 1) - A(0, 2) * A(1, 1)) / (A(0, 0) * A(1, 1) - A(1, 0) * A(0, 1));
                (*solutions)(2, i) = (A(1, 2) * A(0, 0) - A(0, 2) * A(1, 0)) / (A(0, 1) * A(1, 0) - A(1, 1) * A(0, 0));
            }
            if (elim_var == 1) {
                solutions->row(0).swap(solutions->row(1));
            }
            else if (elim_var == 2) {
                solutions->row(0).swap(solutions->row(2));
            }

            refine_3q3(coeffs, solutions, n_roots);

            return n_roots;
        }

        inline int re3q3_rotation_impl(Eigen::Matrix<double, 3, 10>& Rcoeffs, Eigen::Matrix<double, 4, 8>* solutions,
            bool try_random_var_change) {
            try_random_var_change = true;
            Eigen::Vector4d q0 = Eigen::Quaterniond::UnitRandom().coeffs();
            Eigen::Matrix3d R0 = quat_to_rotmat(q0);
            Rcoeffs.block<3, 3>(0, 0) = Rcoeffs.block<3, 3>(0, 0) * R0;
            Rcoeffs.block<3, 3>(0, 3) = Rcoeffs.block<3, 3>(0, 3) * R0;
            Rcoeffs.block<3, 3>(0, 6) = Rcoeffs.block<3, 3>(0, 6) * R0;

            Eigen::Matrix<double, 3, 10> coeffs;
            rotation_to_3q3(Rcoeffs, &coeffs);

            Eigen::Matrix<double, 3, 8> solutions_cayley;
            int n_sols = re3q3(coeffs, &solutions_cayley, try_random_var_change);

            for (int i = 0; i < n_sols; ++i) {
                Eigen::Vector4d q{ 1.0, solutions_cayley(0, i), solutions_cayley(1, i), solutions_cayley(2, i) };
                q.normalize();
                solutions->col(i) = quat_multiply(q0, q);
            }
            return n_sols;
        }

        int re3q3_rotation(const Eigen::Matrix<double, 3, 9>& Rcoeffs, Eigen::Matrix<double, 4, 8>* solutions,
            bool try_random_var_change) {
            try_random_var_change = true;
            Eigen::Matrix<double, 3, 10> Rcoeffs_copy;
            Rcoeffs_copy.block<3, 9>(0, 0) = Rcoeffs;
            Rcoeffs_copy.block<3, 1>(0, 9).setZero();
            return re3q3_rotation_impl(Rcoeffs_copy, solutions, try_random_var_change);
        }
        int re3q3_rotation(const Eigen::Matrix<double, 3, 10>& Rcoeffs, Eigen::Matrix<double, 4, 8>* solutions,
            bool try_random_var_change) {
            Eigen::Matrix<double, 3, 10> Rcoeffs_copy = Rcoeffs;
            return re3q3_rotation_impl(Rcoeffs_copy, solutions, try_random_var_change);
        }
    } // re3q3

    int p4pf(const std::vector<Eigen::Vector2d>& x, const std::vector<Eigen::Vector3d>& X, std::vector<CameraPose>* output,
        std::vector<double>* output_fx, std::vector<double>* output_fy, bool filter_solutions) {

        Eigen::Matrix<double, 2, 4> points2d;
        for (int i = 0; i < 4; ++i) {
            points2d.col(i) = x[i];
        }
        double f0 = points2d.colwise().norm().mean();
        points2d /= f0;

        double d[48];
        // Setup nullspace
        Eigen::Matrix<double, 8, 4> M;
        Eigen::Matrix<double, 4, 4> A;
        Eigen::Map<Eigen::Matrix<double, 8, 4>> N(d);
        Eigen::Map<Eigen::Matrix<double, 4, 4>> B(d + 32);

        for (int i = 0; i < 4; i++) {
            M(0, i) = -points2d(1, i) * X[i](0);
            M(2, i) = -points2d(1, i) * X[i](1);
            M(4, i) = -points2d(1, i) * X[i](2);
            M(6, i) = -points2d(1, i);
            M(1, i) = points2d(0, i) * X[i](0);
            M(3, i) = points2d(0, i) * X[i](1);
            M(5, i) = points2d(0, i) * X[i](2);
            M(7, i) = points2d(0, i);
        }

        // Compute nullspace using QR
        Eigen::Matrix<double, 8, 8> Q = M.householderQr().householderQ();
        N = Q.rightCols(4);

        // Setup matrices A and B (see paper for definition)
        for (int i = 0; i < 4; ++i) {
            if (std::abs(points2d(0, i)) < std::abs(points2d(1, i))) {
                A(i, 0) = points2d(1, i) * X[i](0);
                A(i, 1) = points2d(1, i) * X[i](1);
                A(i, 2) = points2d(1, i) * X[i](2);
                A(i, 3) = points2d(1, i);

                B(i, 0) = X[i](0) * N(1, 0) + X[i](1) * N(3, 0) + X[i](2) * N(5, 0) + N(7, 0); // alpha1
                B(i, 1) = X[i](0) * N(1, 1) + X[i](1) * N(3, 1) + X[i](2) * N(5, 1) + N(7, 1); // alpha2
                B(i, 2) = X[i](0) * N(1, 2) + X[i](1) * N(3, 2) + X[i](2) * N(5, 2) + N(7, 2); // alpha3
                B(i, 3) = X[i](0) * N(1, 3) + X[i](1) * N(3, 3) + X[i](2) * N(5, 3) + N(7, 3); // 1
            }
            else {
                A(i, 0) = points2d(0, i) * X[i](0);
                A(i, 1) = points2d(0, i) * X[i](1);
                A(i, 2) = points2d(0, i) * X[i](2);
                A(i, 3) = points2d(0, i);

                B(i, 0) = X[i](0) * N(0, 0) + X[i](1) * N(2, 0) + X[i](2) * N(4, 0) + N(6, 0); // alpha1
                B(i, 1) = X[i](0) * N(0, 1) + X[i](1) * N(2, 1) + X[i](2) * N(4, 1) + N(6, 1); // alpha2
                B(i, 2) = X[i](0) * N(0, 2) + X[i](1) * N(2, 2) + X[i](2) * N(4, 2) + N(6, 2); // alpha3
                B(i, 3) = X[i](0) * N(0, 3) + X[i](1) * N(2, 3) + X[i](2) * N(4, 3) + N(6, 3); // 1
            }
        }

        // [p31,p32,p33,p34] = B * [alpha; 1]
        B = A.inverse() * B;

        Eigen::Matrix<double, 3, 10> coeffs;
        Eigen::Matrix<double, 3, 8> solutions;

        // Orthogonality constraints
        coeffs.row(0) << d[0] * d[1] + d[2] * d[3] + d[4] * d[5],
            d[0] * d[9] + d[1] * d[8] + d[2] * d[11] + d[3] * d[10] + d[4] * d[13] + d[5] * d[12],
            d[0] * d[17] + d[1] * d[16] + d[2] * d[19] + d[3] * d[18] + d[4] * d[21] + d[5] * d[20],
            d[8] * d[9] + d[10] * d[11] + d[12] * d[13],
            d[8] * d[17] + d[9] * d[16] + d[10] * d[19] + d[11] * d[18] + d[12] * d[21] + d[13] * d[20],
            d[16] * d[17] + d[18] * d[19] + d[20] * d[21],
            d[0] * d[25] + d[1] * d[24] + d[2] * d[27] + d[3] * d[26] + d[4] * d[29] + d[5] * d[28],
            d[8] * d[25] + d[9] * d[24] + d[10] * d[27] + d[11] * d[26] + d[12] * d[29] + d[13] * d[28],
            d[16] * d[25] + d[17] * d[24] + d[18] * d[27] + d[19] * d[26] + d[20] * d[29] + d[21] * d[28],
            d[24] * d[25] + d[26] * d[27] + d[28] * d[29];
        coeffs.row(1) << d[0] * d[32] + d[2] * d[33] + d[4] * d[34],
            d[0] * d[36] + d[2] * d[37] + d[8] * d[32] + d[4] * d[38] + d[10] * d[33] + d[12] * d[34],
            d[0] * d[40] + d[2] * d[41] + d[4] * d[42] + d[16] * d[32] + d[18] * d[33] + d[20] * d[34],
            d[8] * d[36] + d[10] * d[37] + d[12] * d[38],
            d[8] * d[40] + d[10] * d[41] + d[16] * d[36] + d[12] * d[42] + d[18] * d[37] + d[20] * d[38],
            d[16] * d[40] + d[18] * d[41] + d[20] * d[42],
            d[0] * d[44] + d[2] * d[45] + d[4] * d[46] + d[24] * d[32] + d[26] * d[33] + d[28] * d[34],
            d[8] * d[44] + d[10] * d[45] + d[12] * d[46] + d[24] * d[36] + d[26] * d[37] + d[28] * d[38],
            d[16] * d[44] + d[18] * d[45] + d[24] * d[40] + d[20] * d[46] + d[26] * d[41] + d[28] * d[42],
            d[24] * d[44] + d[26] * d[45] + d[28] * d[46];
        coeffs.row(2) << d[1] * d[32] + d[3] * d[33] + d[5] * d[34],
            d[1] * d[36] + d[3] * d[37] + d[9] * d[32] + d[5] * d[38] + d[11] * d[33] + d[13] * d[34],
            d[1] * d[40] + d[3] * d[41] + d[5] * d[42] + d[17] * d[32] + d[19] * d[33] + d[21] * d[34],
            d[9] * d[36] + d[11] * d[37] + d[13] * d[38],
            d[9] * d[40] + d[11] * d[41] + d[17] * d[36] + d[13] * d[42] + d[19] * d[37] + d[21] * d[38],
            d[17] * d[40] + d[19] * d[41] + d[21] * d[42],
            d[1] * d[44] + d[3] * d[45] + d[5] * d[46] + d[25] * d[32] + d[27] * d[33] + d[29] * d[34],
            d[9] * d[44] + d[11] * d[45] + d[13] * d[46] + d[25] * d[36] + d[27] * d[37] + d[29] * d[38],
            d[17] * d[44] + d[19] * d[45] + d[25] * d[40] + d[21] * d[46] + d[27] * d[41] + d[29] * d[42],
            d[25] * d[44] + d[27] * d[45] + d[29] * d[46];

        // The fourth unused constraint (norms of two first rows equal)
        //	d[0] * d[0] - d[1] * d[1] + d[2] * d[2] - d[3] * d[3] + d[4] * d[4] - d[5] * d[5], 2 * d[0] * d[8] - 2 * d[1] *
        // d[9] + 2 * d[2] * d[10] - 2 * d[3] * d[11] + 2 * d[4] * d[12] - 2 * d[5] * d[13], 2 * d[0] * d[16] - 2 * d[1] *
        // d[17] + 2 * d[2] * d[18] - 2 * d[3] * d[19] + 2 * d[4] * d[20] - 2 * d[5] * d[21], d[8] * d[8] - d[9] * d[9] +
        // d[10] * d[10] - d[11] * d[11] + d[12] * d[12] - d[13] * d[13], 2 * d[8] * d[16] - 2 * d[9] * d[17] + 2 * d[10] *
        // d[18] - 2 * d[11] * d[19] + 2 * d[12] * d[20] - 2 * d[13] * d[21], d[16] * d[16] - d[17] * d[17] + d[18] * d[18]
        // - d[19] * d[19] + d[20] * d[20] - d[21] * d[21], 2 * d[0] * d[24] - 2 * d[1] * d[25] + 2 * d[2] * d[26] - 2 *
        // d[3]
        // * d[27] + 2 * d[4] * d[28] - 2 * d[5] * d[29], 2 * d[8] * d[24] - 2 * d[9] * d[25] + 2 * d[10] * d[26] - 2 *
        // d[11]
        // * d[27] + 2 * d[12] * d[28] - 2 * d[13] * d[29], 2 * d[16] * d[24] - 2 * d[17] * d[25] + 2 * d[18] * d[26] - 2 *
        // d[19] * d[27] + 2 * d[20] * d[28] - 2 * d[21] * d[29], d[24] * d[24] - d[25] * d[25] + d[26] * d[26] - d[27] *
        // d[27] + d[28] * d[28] - d[29] * d[29];

        int n_sols = poselib::re3q3::re3q3(coeffs, &solutions, true);

        CameraPose best_pose;
        output->clear();
        output->reserve(n_sols);
        output_fx->clear();
        output_fx->reserve(n_sols);
        output_fy->clear();
        output_fy->reserve(n_sols);

        for (int i = 0; i < n_sols; ++i) {
            Eigen::Matrix<double, 3, 4> P;
            Eigen::Vector4d alpha;
            alpha << solutions.col(i), 1.0;
            Eigen::Matrix<double, 8, 1> P12 = N * alpha;
            P.block<2, 4>(0, 0) = Eigen::Map<Eigen::Matrix<double, 2, 4>>(P12.data());
            P.row(2) = B * alpha;

            if (P.block<3, 3>(0, 0).determinant() < 0)
                P = -P;

            P = P / P.block<1, 3>(2, 0).norm();
            double fx = P.block<1, 3>(0, 0).norm();
            double fy = P.block<1, 3>(1, 0).norm();
            P.row(0) = P.row(0) / fx;
            P.row(1) = P.row(1) / fy;

            Eigen::Matrix3d R = P.block<3, 3>(0, 0);
            Eigen::Vector3d t = P.block<3, 1>(0, 3);
            fx *= f0;
            fy *= f0;

            CameraPose pose(R, t);

            if (filter_solutions) {
                // Check cheirality
                bool ok = true;
                for (int k = 0; k < 4; ++k) {
                    if (R.row(2) * X[k] + t(2) < 0.0) {
                        ok = false;
                        break;
                    }
                }
                if (!ok) {
                    continue;
                }
            }
            output->push_back(pose);
            output_fx->push_back(fx);
            output_fy->push_back(fy);
        }
        return output->size();
    }

    int p4pf(const std::vector<Eigen::Vector2d>& x, const std::vector<Eigen::Vector3d>& X, std::vector<CameraPose>* output,
        std::vector<double>* output_focal, bool filter_solutions) {

        std::vector<CameraPose> poses;
        std::vector<double> fx;
        std::vector<double> fy;
        int n = p4pf(x, X, &poses, &fx, &fy, filter_solutions);

        if (filter_solutions) {
            int best_ind = -1;
            double best_err = 1.0;

            for (int i = 0; i < n; ++i) {
                double a = fx[i] / fy[i];
                double err = std::max(std::abs(a - 1.0), std::abs(1 / a - 1.0));
                if (err < best_err) {
                    best_err = err;
                    best_ind = i;
                }
            }
            if (best_err < 1.0 && best_ind > -1) {
                double focal = (fx[best_ind] + fy[best_ind]) / 2.0;
                output_focal->push_back(focal);
                output->push_back(poses[best_ind]);
            }
        }
        else {
            *output = poses;
            output_focal->resize(n);
            for (int i = 0; i < n; ++i) {
                (*output_focal)[i] = (fx[i] + fy[i]) / 2.0;
            }
        }
        return output->size();
    }


    bool inline root2real(double b, double c, double& r1, double& r2) {
        double THRESHOLD = -1.0e-12;
        double v = b * b - 4.0 * c;
        if (v < THRESHOLD) {
            r1 = r2 = -0.5 * b;
            return v >= 0;
        }
        if (v > THRESHOLD && v < 0.0) {
            r1 = -0.5 * b;
            r2 = -2;
            return true;
        }

        double y = std::sqrt(v);
        if (b < 0) {
            r1 = 0.5 * (-b + y);
            r2 = 0.5 * (-b - y);
        }
        else {
            r1 = 2.0 * c / (-b + y);
            r2 = 2.0 * c / (-b - y);
        }
        return true;
    }

    inline std::array<Eigen::Vector3d, 2> compute_pq(Eigen::Matrix3d C) {
        std::array<Eigen::Vector3d, 2> pq;
        Eigen::Matrix3d C_adj;

        C_adj(0, 0) = C(1, 2) * C(2, 1) - C(1, 1) * C(2, 2);
        C_adj(1, 1) = C(0, 2) * C(2, 0) - C(0, 0) * C(2, 2);
        C_adj(2, 2) = C(0, 1) * C(1, 0) - C(0, 0) * C(1, 1);
        C_adj(0, 1) = C(0, 1) * C(2, 2) - C(0, 2) * C(2, 1);
        C_adj(0, 2) = C(0, 2) * C(1, 1) - C(0, 1) * C(1, 2);
        C_adj(1, 0) = C_adj(0, 1);
        C_adj(1, 2) = C(0, 0) * C(1, 2) - C(0, 2) * C(1, 0);
        C_adj(2, 0) = C_adj(0, 2);
        C_adj(2, 1) = C_adj(1, 2);

        Eigen::Vector3d v;
        if (C_adj(0, 0) > C_adj(1, 1)) {
            if (C_adj(0, 0) > C_adj(2, 2)) {
                v = C_adj.col(0) / std::sqrt(C_adj(0, 0));
            }
            else {
                v = C_adj.col(2) / std::sqrt(C_adj(2, 2));
            }
        }
        else if (C_adj(1, 1) > C_adj(2, 2)) {
            v = C_adj.col(1) / std::sqrt(C_adj(1, 1));
        }
        else {
            v = C_adj.col(2) / std::sqrt(C_adj(2, 2));
        }

        C(0, 1) -= v(2);
        C(0, 2) += v(1);
        C(1, 2) -= v(0);
        C(1, 0) += v(2);
        C(2, 0) -= v(1);
        C(2, 1) += v(0);

        pq[0] = C.col(0);
        pq[1] = C.row(0);

        return pq;
    }

    // Performs a few newton steps on the equations
    inline void refine_lambda(double& lambda1, double& lambda2, double& lambda3, const double a12, const double a13,
        const double a23, const double b12, const double b13, const double b23) {

        for (int iter = 0; iter < 5; ++iter) {
            double r1 = (lambda1 * lambda1 - 2.0 * lambda1 * lambda2 * b12 + lambda2 * lambda2 - a12);
            double r2 = (lambda1 * lambda1 - 2.0 * lambda1 * lambda3 * b13 + lambda3 * lambda3 - a13);
            double r3 = (lambda2 * lambda2 - 2.0 * lambda2 * lambda3 * b23 + lambda3 * lambda3 - a23);
            if (std::abs(r1) + std::abs(r2) + std::abs(r3) < 1e-10)
                return;
            double x11 = lambda1 - lambda2 * b12;
            double x12 = lambda2 - lambda1 * b12;
            double x21 = lambda1 - lambda3 * b13;
            double x23 = lambda3 - lambda1 * b13;
            double x32 = lambda2 - lambda3 * b23;
            double x33 = lambda3 - lambda2 * b23;
            double detJ = 0.5 / (x11 * x23 * x32 + x12 * x21 * x33); // half minus inverse determinant
            // This uses the closed form of the inverse for the jacobean.
            // Due to the zero elements this actually becomes quite nice.
            lambda1 += (-x23 * x32 * r1 - x12 * x33 * r2 + x12 * x23 * r3) * detJ;
            lambda2 += (-x21 * x33 * r1 + x11 * x33 * r2 - x11 * x23 * r3) * detJ;
            lambda3 += (x21 * x32 * r1 - x11 * x32 * r2 - x12 * x21 * r3) * detJ;
        }
    }

    int p3p(const std::vector<Eigen::Vector3d>& x_copy, const std::vector<Eigen::Vector3d>& X_copy,
        std::vector<CameraPose>* output) {
        if (output == nullptr) {
            return 0;
        }
        output->clear();
        output->reserve(4);

        Eigen::Vector3d X01 = X_copy[0] - X_copy[1];
        Eigen::Vector3d X02 = X_copy[0] - X_copy[2];
        Eigen::Vector3d X12 = X_copy[1] - X_copy[2];

        double a01 = X01.squaredNorm();
        double a02 = X02.squaredNorm();
        double a12 = X12.squaredNorm();

        std::array<Eigen::Vector3d, 3> X = { X_copy[0], X_copy[1], X_copy[2] };
        std::array<Eigen::Vector3d, 3> x = { x_copy[0], x_copy[1], x_copy[2] };

        // Switch X,x so that BC is the largest distance among {X01, X02, X12}
        if (a01 > a02) {
            if (a01 > a12) {
                std::swap(x[0], x[2]);
                std::swap(X[0], X[2]);
                std::swap(a01, a12);
                X01 = -X12;
                X02 = -X02;
            }
        }
        else if (a02 > a12) {
            std::swap(x[0], x[1]);
            std::swap(X[0], X[1]);
            std::swap(a02, a12);
            X01 = -X01;
            X02 = X12;
        }

        const double a12d = 1.0 / a12;
        const double a = a01 * a12d;
        const double b = a02 * a12d;

        const double m01 = x[0].dot(x[1]);
        const double m02 = x[0].dot(x[2]);
        const double m12 = x[1].dot(x[2]);

        // Ugly parameters to simplify the calculation
        const double m12sq = -m12 * m12 + 1.0;
        const double m02sq = -1.0 + m02 * m02;
        const double m01sq = -1.0 + m01 * m01;
        const double ab = a * b;
        const double bsq = b * b;
        const double asq = a * a;
        const double m013 = -2.0 + 2.0 * m01 * m02 * m12;
        const double bsqm12sq = bsq * m12sq;
        const double asqm12sq = asq * m12sq;
        const double abm12sq = 2.0 * ab * m12sq;

        const double k3_inv = 1.0 / (bsqm12sq + b * m02sq);
        const double k2 = k3_inv * ((-1.0 + a) * m02sq + abm12sq + bsqm12sq + b * m013);
        const double k1 = k3_inv * (asqm12sq + abm12sq + a * m013 + (-1.0 + b) * m01sq);
        const double k0 = k3_inv * (asqm12sq + a * m01sq);

        double s;
        bool G = univariate::solve_cubic_single_real(k2, k1, k0, s);

        Eigen::Matrix3d C;
        C(0, 0) = -a + s * (1 - b);
        C(0, 1) = -m02 * s;
        C(0, 2) = a * m12 + b * m12 * s;
        C(1, 0) = C(0, 1);
        C(1, 1) = s + 1;
        C(1, 2) = -m01;
        C(2, 0) = C(0, 2);
        C(2, 1) = C(1, 2);
        C(2, 2) = -a - b * s + 1;

        std::array<Eigen::Vector3d, 2> pq = compute_pq(C);

        double d0, d1, d2;
        CameraPose pose;
        output->clear();
        Eigen::Matrix3d XX;

        XX << X01, X02, X01.cross(X02);
        XX = XX.inverse().eval();

        Eigen::Vector3d v1, v2;
        Eigen::Matrix3d YY;

        int n_sols = 0;

        for (int i = 0; i < 2; ++i) {
            // [p0 p1 p2] * [1; x; y] = 0, or [p0 p1 p2] * [d2; d0; d1] = 0
            double p0 = pq[i](0);
            double p1 = pq[i](1);
            double p2 = pq[i](2);
            // here we run into trouble if p0 is zero,
            // so depending on which is larger, we solve for either d0 or d1
            // The case p0 = p1 = 0 is degenerate and can be ignored
            bool switch_12 = std::abs(p0) <= std::abs(p1);

            if (switch_12) {
                // eliminate d0
                double w0 = -p0 / p1;
                double w1 = -p2 / p1;
                double ca = 1.0 / (w1 * w1 - b);
                double cb = 2.0 * (b * m12 - m02 * w1 + w0 * w1) * ca;
                double cc = (w0 * w0 - 2 * m02 * w0 - b + 1.0) * ca;
                double taus[2];
                if (!root2real(cb, cc, taus[0], taus[1]))
                    continue;
                for (double tau : taus) {
                    if (tau <= 0)
                        continue;
                    // positive only
                    d2 = std::sqrt(a12 / (tau * (tau - 2.0 * m12) + 1.0));
                    d1 = tau * d2;
                    d0 = (w0 * d2 + w1 * d1);
                    if (d0 < 0)
                        continue;

                    refine_lambda(d0, d1, d2, a01, a02, a12, m01, m02, m12);
                    v1 = d0 * x[0] - d1 * x[1];
                    v2 = d0 * x[0] - d2 * x[2];
                    YY << v1, v2, v1.cross(v2);
                    Eigen::Matrix3d R = YY * XX;
                    output->emplace_back(R, d0 * x[0] - R * X[0]);
                    ++n_sols;
                }
            }
            else {
                double w0 = -p1 / p0;
                double w1 = -p2 / p0;
                double ca = 1.0 / (-a * w1 * w1 + 2 * a * m12 * w1 - a + 1);
                double cb = 2 * (a * m12 * w0 - m01 - a * w0 * w1) * ca;
                double cc = (1 - a * w0 * w0) * ca;

                double taus[2];
                if (!root2real(cb, cc, taus[0], taus[1]))
                    continue;
                for (double tau : taus) {
                    if (tau <= 0)
                        continue;
                    d0 = std::sqrt(a01 / (tau * (tau - 2.0 * m01) + 1.0));
                    d1 = tau * d0;
                    d2 = w0 * d0 + w1 * d1;

                    if (d2 < 0)
                        continue;

                    refine_lambda(d0, d1, d2, a01, a02, a12, m01, m02, m12);
                    v1 = d0 * x[0] - d1 * x[1];
                    v2 = d0 * x[0] - d2 * x[2];
                    YY << v1, v2, v1.cross(v2);
                    Eigen::Matrix3d R = YY * XX;
                    output->emplace_back(R, d0 * x[0] - R * X[0]);
                    ++n_sols;
                }
            }

            if (n_sols > 0 && G)
                break;
        }

        return output->size();
    }

}

namespace colmap {

P3PEstimator::P3PEstimator(ImgFromCamFunc img_from_cam_func)
    : img_from_cam_func_(std::move(img_from_cam_func)) {}

void P3PEstimator::Estimate(const std::vector<X_t>& points2D,
                            const std::vector<Y_t>& points3D,
                            std::vector<M_t>* cams_from_world) const {
  THROW_CHECK_EQ(points2D.size(), 3);
  THROW_CHECK_EQ(points3D.size(), 3);
  THROW_CHECK_NOTNULL(cams_from_world);

  std::vector<Eigen::Vector3d> rays(3);
  for (int i = 0; i < 3; ++i) {
    rays[i] = points2D[i].camera_ray;
  }

  std::vector<poselib::CameraPose> poses;
  const int num_poses = poselib::p3p(rays, points3D, &poses);

  cams_from_world->resize(num_poses);
  for (int i = 0; i < num_poses; ++i) {
    (*cams_from_world)[i] = poses[i].Rt();
  }
}

void P3PEstimator::Residuals(const std::vector<X_t>& points2D,
                             const std::vector<Y_t>& points3D,
                             const M_t& cam_from_world,
                             std::vector<double>* residuals) const {
  ComputeSquaredReprojectionError(
      points2D, points3D, cam_from_world, img_from_cam_func_, residuals);
}

void P4PFEstimator::Estimate(const std::vector<X_t>& points2D,
                             const std::vector<Y_t>& points3D,
                             std::vector<M_t>* models) {
  THROW_CHECK_EQ(points2D.size(), 4);
  THROW_CHECK_EQ(points3D.size(), 4);
  THROW_CHECK_NOTNULL(models);

  std::vector<poselib::CameraPose> poses;
  std::vector<double> focals;
  const int num_poses = poselib::p4pf(
      points2D, points3D, &poses, &focals, /*filter_solutions=*/true);

  models->resize(num_poses);
  for (int i = 0; i < num_poses; ++i) {
    (*models)[i].cam_from_world = poses[i].Rt();
    (*models)[i].focal_length = focals[i];
  }
}

void P4PFEstimator::Residuals(const std::vector<X_t>& points2D,
                              const std::vector<Y_t>& points3D,
                              const M_t& model,
                              std::vector<double>* residuals) {
  const size_t num_points2D = points2D.size();
  CHECK_EQ(num_points2D, points3D.size());
  residuals->resize(num_points2D);
  for (size_t i = 0; i < num_points2D; ++i) {
    const Eigen::Vector3d point3D_in_cam =
        model.cam_from_world * points3D[i].homogeneous();
    // Check if 3D point is in front of camera.
    if (point3D_in_cam.z() > std::numeric_limits<double>::epsilon()) {
      (*residuals)[i] =
          (model.focal_length * point3D_in_cam.hnormalized() - points2D[i])
              .squaredNorm();
    } else {
      (*residuals)[i] = std::numeric_limits<double>::max();
    }
  }
}

EPNPEstimator::EPNPEstimator(ImgFromCamFunc img_from_cam_func)
    : img_from_cam_func_(std::move(img_from_cam_func)) {}

void EPNPEstimator::Estimate(const std::vector<X_t>& points2D,
                             const std::vector<Y_t>& points3D,
                             std::vector<M_t>* cams_from_world) {
  THROW_CHECK_GE(points2D.size(), 4);
  THROW_CHECK_EQ(points2D.size(), points3D.size());
  THROW_CHECK_NOTNULL(cams_from_world);

  cams_from_world->clear();

  M_t cam_from_world;
  if (!ComputePose(points2D, points3D, &cam_from_world)) {
    return;
  }

  cams_from_world->resize(1);
  (*cams_from_world)[0] = cam_from_world;
}

void EPNPEstimator::Residuals(const std::vector<X_t>& points2D,
                              const std::vector<Y_t>& points3D,
                              const M_t& cam_from_world,
                              std::vector<double>* residuals) const {
  ComputeSquaredReprojectionError(
      points2D, points3D, cam_from_world, img_from_cam_func_, residuals);
}

bool EPNPEstimator::ComputePose(const std::vector<X_t>& points2D,
                                const std::vector<Y_t>& points3D,
                                Eigen::Matrix3x4d* cam_from_world) {
  points2D_ = &points2D;
  points3D_ = &points3D;

  ChooseControlPoints();

  if (!ComputeBarycentricCoordinates()) {
    return false;
  }

  const Eigen::Matrix<double, Eigen::Dynamic, 12> M = ComputeM();
  const Eigen::Matrix<double, 12, 12> MtM = M.transpose() * M;

  Eigen::JacobiSVD<Eigen::Matrix<double, 12, 12>> svd(
      MtM, Eigen::ComputeFullV | Eigen::ComputeFullU);
  const Eigen::Matrix<double, 12, 12> Ut = svd.matrixU().transpose();

  const Eigen::Matrix<double, 6, 10> L6x10 = ComputeL6x10(Ut);
  const Eigen::Matrix<double, 6, 1> rho = ComputeRho();

  Eigen::Vector4d betas[4];
  std::array<double, 4> reproj_errors;
  std::array<Eigen::Matrix3d, 4> Rs;
  std::array<Eigen::Vector3d, 4> ts;

  FindBetasApprox1(L6x10, rho, &betas[1]);
  RunGaussNewton(L6x10, rho, &betas[1]);
  reproj_errors[1] = ComputeRT(Ut, betas[1], &Rs[1], &ts[1]);

  FindBetasApprox2(L6x10, rho, &betas[2]);
  RunGaussNewton(L6x10, rho, &betas[2]);
  reproj_errors[2] = ComputeRT(Ut, betas[2], &Rs[2], &ts[2]);

  FindBetasApprox3(L6x10, rho, &betas[3]);
  RunGaussNewton(L6x10, rho, &betas[3]);
  reproj_errors[3] = ComputeRT(Ut, betas[3], &Rs[3], &ts[3]);

  int best_idx = 1;
  if (reproj_errors[2] < reproj_errors[1]) {
    best_idx = 2;
  }
  if (reproj_errors[3] < reproj_errors[best_idx]) {
    best_idx = 3;
  }

  cam_from_world->leftCols<3>() = Rs[best_idx];
  cam_from_world->rightCols<1>() = ts[best_idx];

  return true;
}

void EPNPEstimator::ChooseControlPoints() {
  // Take C0 as the reference points centroid:
  cws_[0].setZero();
  for (size_t i = 0; i < points3D_->size(); ++i) {
    cws_[0] += (*points3D_)[i];
  }
  cws_[0] /= points3D_->size();

  Eigen::Matrix<double, Eigen::Dynamic, 3> PW0(points3D_->size(), 3);
  for (size_t i = 0; i < points3D_->size(); ++i) {
    PW0.row(i) = (*points3D_)[i] - cws_[0];
  }

  const Eigen::Matrix3d PW0tPW0 = PW0.transpose() * PW0;
  Eigen::JacobiSVD<Eigen::Matrix3d> svd(
      PW0tPW0, Eigen::ComputeFullV | Eigen::ComputeFullU);
  const Eigen::Vector3d& D = svd.singularValues();
  const Eigen::Matrix3d Ut = svd.matrixU().transpose();

  for (int i = 1; i < 4; ++i) {
    const double k = std::sqrt(D(i - 1) / points3D_->size());
    cws_[i] = cws_[0] + k * Ut.row(i - 1).transpose();
  }
}

bool EPNPEstimator::ComputeBarycentricCoordinates() {
  Eigen::Matrix3d CC;
  for (int i = 0; i < 3; ++i) {
    for (int j = 1; j < 4; ++j) {
      CC(i, j - 1) = cws_[j][i] - cws_[0][i];
    }
  }

  if (CC.colPivHouseholderQr().rank() < 3) {
    return false;
  }

  const Eigen::Matrix3d CC_inv = CC.inverse();

  alphas_.resize(points2D_->size());
  for (size_t i = 0; i < points3D_->size(); ++i) {
    for (int j = 0; j < 3; ++j) {
      alphas_[i][1 + j] = CC_inv(j, 0) * ((*points3D_)[i][0] - cws_[0][0]) +
                          CC_inv(j, 1) * ((*points3D_)[i][1] - cws_[0][1]) +
                          CC_inv(j, 2) * ((*points3D_)[i][2] - cws_[0][2]);
    }
    alphas_[i][0] = 1.0 - alphas_[i][1] - alphas_[i][2] - alphas_[i][3];
  }

  return true;
}

Eigen::Matrix<double, Eigen::Dynamic, 12> EPNPEstimator::ComputeM() {
  Eigen::Matrix<double, Eigen::Dynamic, 12> M(3 * points2D_->size(), 12);
  for (size_t i = 0; i < points3D_->size(); ++i) {
    const Eigen::Vector3d& ray = (*points2D_)[i].camera_ray;
    for (size_t j = 0; j < 4; ++j) {
      M(3 * i, 3 * j) = 0.0;
      M(3 * i, 3 * j + 1) = -alphas_[i][j] * ray.z();
      M(3 * i, 3 * j + 2) = alphas_[i][j] * ray.y();

      M(3 * i + 1, 3 * j) = alphas_[i][j] * ray.z();
      M(3 * i + 1, 3 * j + 1) = 0.0;
      M(3 * i + 1, 3 * j + 2) = -alphas_[i][j] * ray.x();

      M(3 * i + 2, 3 * j) = -alphas_[i][j] * ray.y();
      M(3 * i + 2, 3 * j + 1) = alphas_[i][j] * ray.x();
      M(3 * i + 2, 3 * j + 2) = 0;
    }
  }
  return M;
}

Eigen::Matrix<double, 6, 10> EPNPEstimator::ComputeL6x10(
    const Eigen::Matrix<double, 12, 12>& Ut) {
  Eigen::Matrix<double, 6, 10> L6x10;

  std::array<std::array<Eigen::Vector3d, 6>, 4> dv;
  for (int i = 0; i < 4; ++i) {
    int a = 0, b = 1;
    for (int j = 0; j < 6; ++j) {
      dv[i][j][0] = Ut(11 - i, 3 * a) - Ut(11 - i, 3 * b);
      dv[i][j][1] = Ut(11 - i, 3 * a + 1) - Ut(11 - i, 3 * b + 1);
      dv[i][j][2] = Ut(11 - i, 3 * a + 2) - Ut(11 - i, 3 * b + 2);

      b += 1;
      if (b > 3) {
        a += 1;
        b = a + 1;
      }
    }
  }

  for (int i = 0; i < 6; ++i) {
    L6x10(i, 0) = dv[0][i].transpose() * dv[0][i];
    L6x10(i, 1) = 2.0 * dv[0][i].transpose() * dv[1][i];
    L6x10(i, 2) = dv[1][i].transpose() * dv[1][i];
    L6x10(i, 3) = 2.0 * dv[0][i].transpose() * dv[2][i];
    L6x10(i, 4) = 2.0 * dv[1][i].transpose() * dv[2][i];
    L6x10(i, 5) = dv[2][i].transpose() * dv[2][i];
    L6x10(i, 6) = 2.0 * dv[0][i].transpose() * dv[3][i];
    L6x10(i, 7) = 2.0 * dv[1][i].transpose() * dv[3][i];
    L6x10(i, 8) = 2.0 * dv[2][i].transpose() * dv[3][i];
    L6x10(i, 9) = dv[3][i].transpose() * dv[3][i];
  }

  return L6x10;
}

Eigen::Matrix<double, 6, 1> EPNPEstimator::ComputeRho() {
  Eigen::Matrix<double, 6, 1> rho;
  rho[0] = (cws_[0] - cws_[1]).squaredNorm();
  rho[1] = (cws_[0] - cws_[2]).squaredNorm();
  rho[2] = (cws_[0] - cws_[3]).squaredNorm();
  rho[3] = (cws_[1] - cws_[2]).squaredNorm();
  rho[4] = (cws_[1] - cws_[3]).squaredNorm();
  rho[5] = (cws_[2] - cws_[3]).squaredNorm();
  return rho;
}

// betas10        = [B11 B12 B22 B13 B23 B33 B14 B24 B34 B44]
// betas_approx_1 = [B11 B12     B13         B14]

void EPNPEstimator::FindBetasApprox1(const Eigen::Matrix<double, 6, 10>& L6x10,
                                     const Eigen::Matrix<double, 6, 1>& rho,
                                     Eigen::Vector4d* betas) {
  Eigen::Matrix<double, 6, 4> L_6x4;
  for (int i = 0; i < 6; ++i) {
    L_6x4(i, 0) = L6x10(i, 0);
    L_6x4(i, 1) = L6x10(i, 1);
    L_6x4(i, 2) = L6x10(i, 3);
    L_6x4(i, 3) = L6x10(i, 6);
  }

  Eigen::JacobiSVD<Eigen::Matrix<double, 6, 4>> svd(
      L_6x4, Eigen::ComputeFullV | Eigen::ComputeFullU);
  const Eigen::Matrix<double, 4, 1> b4 = svd.solve(rho);

  if (b4[0] < 0) {
    (*betas)[0] = std::sqrt(-b4[0]);
    (*betas)[1] = -b4[1] / (*betas)[0];
    (*betas)[2] = -b4[2] / (*betas)[0];
    (*betas)[3] = -b4[3] / (*betas)[0];
  } else {
    (*betas)[0] = std::sqrt(b4[0]);
    (*betas)[1] = b4[1] / (*betas)[0];
    (*betas)[2] = b4[2] / (*betas)[0];
    (*betas)[3] = b4[3] / (*betas)[0];
  }
}

// betas10        = [B11 B12 B22 B13 B23 B33 B14 B24 B34 B44]
// betas_approx_2 = [B11 B12 B22                            ]

void EPNPEstimator::FindBetasApprox2(const Eigen::Matrix<double, 6, 10>& L6x10,
                                     const Eigen::Matrix<double, 6, 1>& rho,
                                     Eigen::Vector4d* betas) {
  Eigen::Matrix<double, 6, 3> L_6x3(6, 3);

  for (int i = 0; i < 6; ++i) {
    L_6x3(i, 0) = L6x10(i, 0);
    L_6x3(i, 1) = L6x10(i, 1);
    L_6x3(i, 2) = L6x10(i, 2);
  }

  Eigen::JacobiSVD<Eigen::Matrix<double, 6, 3>> svd(
      L_6x3, Eigen::ComputeFullV | Eigen::ComputeFullU);
  const Eigen::Matrix<double, 3, 1> b3 = svd.solve(rho);

  if (b3[0] < 0) {
    (*betas)[0] = std::sqrt(-b3[0]);
    (*betas)[1] = (b3[2] < 0) ? std::sqrt(-b3[2]) : 0.0;
  } else {
    (*betas)[0] = std::sqrt(b3[0]);
    (*betas)[1] = (b3[2] > 0) ? std::sqrt(b3[2]) : 0.0;
  }

  if (b3[1] < 0) {
    (*betas)[0] = -(*betas)[0];
  }

  (*betas)[2] = 0.0;
  (*betas)[3] = 0.0;
}

// betas10        = [B11 B12 B22 B13 B23 B33 B14 B24 B34 B44]
// betas_approx_3 = [B11 B12 B22 B13 B23                    ]

void EPNPEstimator::FindBetasApprox3(const Eigen::Matrix<double, 6, 10>& L6x10,
                                     const Eigen::Matrix<double, 6, 1>& rho,
                                     Eigen::Vector4d* betas) {
  Eigen::JacobiSVD<Eigen::Matrix<double, 6, 5>> svd(
      L6x10.leftCols<5>(), Eigen::ComputeFullV | Eigen::ComputeFullU);
  const Eigen::Matrix<double, 5, 1> b5 = svd.solve(rho);

  if (b5[0] < 0) {
    (*betas)[0] = std::sqrt(-b5[0]);
    (*betas)[1] = (b5[2] < 0) ? std::sqrt(-b5[2]) : 0.0;
  } else {
    (*betas)[0] = std::sqrt(b5[0]);
    (*betas)[1] = (b5[2] > 0) ? std::sqrt(b5[2]) : 0.0;
  }
  if (b5[1] < 0) {
    (*betas)[0] = -(*betas)[0];
  }
  (*betas)[2] = b5[3] / (*betas)[0];
  (*betas)[3] = 0.0;
}

void EPNPEstimator::RunGaussNewton(const Eigen::Matrix<double, 6, 10>& L6x10,
                                   const Eigen::Matrix<double, 6, 1>& rho,
                                   Eigen::Vector4d* betas) {
  Eigen::Matrix<double, 6, 4> A;
  Eigen::Matrix<double, 6, 1> b;

  const int kNumIterations = 5;
  for (int k = 0; k < kNumIterations; ++k) {
    for (int i = 0; i < 6; ++i) {
      A(i, 0) = 2 * L6x10(i, 0) * (*betas)[0] + L6x10(i, 1) * (*betas)[1] +
                L6x10(i, 3) * (*betas)[2] + L6x10(i, 6) * (*betas)[3];
      A(i, 1) = L6x10(i, 1) * (*betas)[0] + 2 * L6x10(i, 2) * (*betas)[1] +
                L6x10(i, 4) * (*betas)[2] + L6x10(i, 7) * (*betas)[3];
      A(i, 2) = L6x10(i, 3) * (*betas)[0] + L6x10(i, 4) * (*betas)[1] +
                2 * L6x10(i, 5) * (*betas)[2] + L6x10(i, 8) * (*betas)[3];
      A(i, 3) = L6x10(i, 6) * (*betas)[0] + L6x10(i, 7) * (*betas)[1] +
                L6x10(i, 8) * (*betas)[2] + 2 * L6x10(i, 9) * (*betas)[3];

      b(i) = rho[i] - (L6x10(i, 0) * (*betas)[0] * (*betas)[0] +
                       L6x10(i, 1) * (*betas)[0] * (*betas)[1] +
                       L6x10(i, 2) * (*betas)[1] * (*betas)[1] +
                       L6x10(i, 3) * (*betas)[0] * (*betas)[2] +
                       L6x10(i, 4) * (*betas)[1] * (*betas)[2] +
                       L6x10(i, 5) * (*betas)[2] * (*betas)[2] +
                       L6x10(i, 6) * (*betas)[0] * (*betas)[3] +
                       L6x10(i, 7) * (*betas)[1] * (*betas)[3] +
                       L6x10(i, 8) * (*betas)[2] * (*betas)[3] +
                       L6x10(i, 9) * (*betas)[3] * (*betas)[3]);
    }

    const Eigen::Vector4d x = A.colPivHouseholderQr().solve(b);

    (*betas) += x;
  }
}

double EPNPEstimator::ComputeRT(const Eigen::Matrix<double, 12, 12>& Ut,
                                const Eigen::Vector4d& betas,
                                Eigen::Matrix3d* R,
                                Eigen::Vector3d* t) {
  ComputeCcs(betas, Ut);
  ComputePcs();

  SolveForSign();

  EstimateRT(R, t);

  return ComputeTotalError(*R, *t);
}

void EPNPEstimator::ComputeCcs(const Eigen::Vector4d& betas,
                               const Eigen::Matrix<double, 12, 12>& Ut) {
  for (int i = 0; i < 4; ++i) {
    ccs_[i][0] = ccs_[i][1] = ccs_[i][2] = 0.0;
  }

  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      for (int k = 0; k < 3; ++k) {
        ccs_[j][k] += betas[i] * Ut(11 - i, 3 * j + k);
      }
    }
  }
}

void EPNPEstimator::ComputePcs() {
  pcs_.resize(points2D_->size());
  for (size_t i = 0; i < points3D_->size(); ++i) {
    for (int j = 0; j < 3; ++j) {
      pcs_[i][j] = alphas_[i][0] * ccs_[0][j] + alphas_[i][1] * ccs_[1][j] +
                   alphas_[i][2] * ccs_[2][j] + alphas_[i][3] * ccs_[3][j];
    }
  }
}

void EPNPEstimator::SolveForSign() {
  if (pcs_[0][2] < 0.0) {
    for (int i = 0; i < 4; ++i) {
      ccs_[i] = -ccs_[i];
    }
    for (size_t i = 0; i < points3D_->size(); ++i) {
      pcs_[i] = -pcs_[i];
    }
  }
}

void EPNPEstimator::EstimateRT(Eigen::Matrix3d* R, Eigen::Vector3d* t) {
  Eigen::Vector3d pc0 = Eigen::Vector3d::Zero();
  Eigen::Vector3d pw0 = Eigen::Vector3d::Zero();

  for (size_t i = 0; i < points3D_->size(); ++i) {
    pc0 += pcs_[i];
    pw0 += (*points3D_)[i];
  }
  pc0 /= points3D_->size();
  pw0 /= points3D_->size();

  Eigen::Matrix3d abt = Eigen::Matrix3d::Zero();
  for (size_t i = 0; i < points3D_->size(); ++i) {
    for (int j = 0; j < 3; ++j) {
      abt(j, 0) += (pcs_[i][j] - pc0[j]) * ((*points3D_)[i][0] - pw0[0]);
      abt(j, 1) += (pcs_[i][j] - pc0[j]) * ((*points3D_)[i][1] - pw0[1]);
      abt(j, 2) += (pcs_[i][j] - pc0[j]) * ((*points3D_)[i][2] - pw0[2]);
    }
  }

  Eigen::JacobiSVD<Eigen::Matrix3d> svd(
      abt, Eigen::ComputeFullV | Eigen::ComputeFullU);
  const Eigen::Matrix3d& abt_U = svd.matrixU();
  const Eigen::Matrix3d& abt_V = svd.matrixV();

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      (*R)(i, j) = abt_U.row(i) * abt_V.row(j).transpose();
    }
  }

  if (R->determinant() < 0) {
    Eigen::Matrix3d Abt_v_prime = abt_V;
    Abt_v_prime.col(2) = -abt_V.col(2);
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        (*R)(i, j) = abt_U.row(i) * Abt_v_prime.row(j).transpose();
      }
    }
  }

  *t = pc0 - *R * pw0;
}

double EPNPEstimator::ComputeTotalError(const Eigen::Matrix3d& R,
                                        const Eigen::Vector3d& t) {
  Eigen::Matrix3x4d cam_from_world;
  cam_from_world.leftCols<3>() = R;
  cam_from_world.rightCols<1>() = t;

  std::vector<double> residuals;
  ComputeSquaredReprojectionError(
      *points2D_, *points3D_, cam_from_world, img_from_cam_func_, &residuals);

  double error = 0.0;
  for (const double residual : residuals) {
    error += std::sqrt(residual);
  }

  return error;
}

void ComputeSquaredReprojectionError(
    const std::vector<Point2DWithRay>& points2D,
    const std::vector<Eigen::Vector3d>& points3D,
    const Eigen::Matrix3x4d& cam_from_world,
    const ImgFromCamFunc& img_from_cam_func,
    std::vector<double>* residuals) {
  const size_t num_points = points2D.size();
  THROW_CHECK_EQ(num_points, points3D.size());
  residuals->resize(num_points);
  for (size_t i = 0; i < num_points; ++i) {
    const Eigen::Vector3d point3D_in_cam =
        cam_from_world * points3D[i].homogeneous();
    const std::optional<Eigen::Vector2d> proj_image_point =
        img_from_cam_func(point3D_in_cam);
    if (proj_image_point) {
      (*residuals)[i] =
          (*proj_image_point - points2D[i].image_point).squaredNorm();
    } else {
      (*residuals)[i] = std::numeric_limits<double>::max();
    }
  }
}

}  // namespace colmap
