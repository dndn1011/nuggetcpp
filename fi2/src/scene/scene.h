#pragma once
#include "types.h"

namespace nugget::scene {
    struct Transform {
        const Matrix3f& Rot() const {
            return rot;
        }
        const Matrix4f& Matrix() const {
            if (!matrixValid) {
                MakeMatrix();
            }
            return matrix;
        }
        const Vector3f& Pos() {
            return pos;
        }
        const Vector3f& Scale() {
            return scale;
        }
        void Rot(const Matrix3f& rotIn) {
            matrixValid = false;
            rot = rotIn;
        }
        void Pos(const Vector3f& posIn) {
            matrixValid = false;
            pos = posIn;
        }
        void Scale(const Vector3f& scaleIn) {
            matrixValid = false;
            scale = scaleIn;
        }
        void OrthoNormalize() {
            matrixValid = false;
            rot.OrthoNormalize();
        }
    private:
        mutable Matrix4f matrix = {};
        mutable bool matrixValid = false;
        Matrix3f rot = {};
        Vector3f pos = {};
        Vector3f scale = {};
        void MakeMatrix() const {
            matrix[0 + 4 * 0] = rot[0 + 3 * 0] * scale.x;
            matrix[1 + 4 * 0] = rot[1 + 3 * 0] * scale.y;
            matrix[2 + 4 * 0] = rot[2 + 3 * 0] * scale.z;
            matrix[0 + 4 * 1] = rot[0 + 3 * 1] * scale.x;
            matrix[1 + 4 * 1] = rot[1 + 3 * 1] * scale.y;
            matrix[2 + 4 * 1] = rot[2 + 3 * 1] * scale.z;
            matrix[0 + 4 * 2] = rot[0 + 3 * 2] * scale.x;
            matrix[1 + 4 * 2] = rot[1 + 3 * 2] * scale.y;
            matrix[2 + 4 * 2] = rot[2 + 3 * 2] * scale.z;

            matrix[0 + 4 * 3] = pos.x;
            matrix[1 + 4 * 3] = pos.y;
            matrix[2 + 4 * 3] = pos.z;

            matrix[3 + 4 * 0] = 0;
            matrix[3 + 4 * 1] = 0;
            matrix[3 + 4 * 2] = 0;
            matrix[3 + 4 * 3] = 1;
            matrixValid = true;
        }
    };
}

