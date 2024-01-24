
#include <unordered_map>
#include "propertytree.h"
#include "glinternal.h"


namespace nugget::gl {
    using namespace identifier;
    using namespace properties;

    Matrix4f projectionMatrix;

    struct PerspectiveCameraData {
        float fov;
        float nearClip;
        float farClip;
    };

    static PerspectiveCameraData perspectiveCameraData;

#define PRIMDEF(a) { ID(#a),a },
    static std::unordered_map<IDType, GLenum> primitiveMap = {
        PRIMDEF(GL_TRIANGLES_ADJACENCY)
        PRIMDEF(GL_TRIANGLES)
    };

    GLenum GLPrimitiveFromIdentifier(IDType ID) {
        check(primitiveMap.contains(ID), "Dunno what {} is\n", IDToString(ID));
        return primitiveMap.at(ID);
    }

    void GLCameraSetProjectionFromProperties(IDType nodeID) {
        auto &pcd = perspectiveCameraData;
        
        pcd.fov = gProps.GetFloat(IDR(nodeID, ID("fov")));
        pcd.nearClip = gProps.GetFloat(IDR(nodeID, ID("nearClip")));
        pcd.farClip = gProps.GetFloat(IDR(nodeID, ID("farClip")));

        Matrix4f::CreateProjectionMatrix(
            (float)gProps.GetInt64(ID("app.window.w")),
            (float)gProps.GetInt64(ID("app.window.h")),
            pcd.fov, pcd.nearClip, pcd.farClip, projectionMatrix);

    }

    const GLfloat* GLCameraProjectionMatrix() {
        return static_cast<const GLfloat*>(projectionMatrix.GetArray());
    }


}