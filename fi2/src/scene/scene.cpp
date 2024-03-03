#include "system.h"
#include "propertytree.h"
#include "renderer/renderer.h"
#include <unordered_map>
#include "scene.h"


#define PERFECT_EMPLACEMENT(a,...)  emplace(std::piecewise_construct, std::forward_as_tuple(a), std::forward_as_tuple(__VA_ARGS__))

namespace nugget::scene {
    using namespace identifier;
    using namespace properties;

    namespace {
        const IDType baseSceneNodeID = ID("scene");

        struct SceneData {
            Matrix4f viewMatrix;
        };

        SceneData sceneData;


        struct InstanceData {
            APPLY_RULE_OF_MINUS_5(InstanceData);

           InstanceData(IDType node, const Transform& transform) : transform(transform), node(node) {
           }

            IDType node = {};
            Transform transform;
        };

        std::unordered_map<IDType, InstanceData> instances;

        void Init() {
            std::vector<IDType> children;
            gProps.GetChildrenWithNodeExisting(baseSceneNodeID, ID("model"), children /*fill*/);
            for (auto&& x : children) {
                IDType model_nid = IDR(x,"model");
                IDType model_refnid = gProps.GetID(model_nid);
                renderer::ConfigureRenderModel(model_refnid);
                
//                IDType modelMatrixNode = IDR(x, "modelMatrix");
//                const Matrix4f modelMatrix = gProps.GetMatrix4f(modelMatrixNode);

                Transform t;
                IDType transformNode = IDR(x, ID("transform"));
                t.Pos(gProps.GetVector3f(IDR(transformNode, ID("pos"))));
                t.Rot(gProps.GetMatrix3f(IDR(transformNode, ID("rot"))));
                t.Scale(gProps.GetVector3f(IDR(transformNode, ID("scale"))));

                instances.PERFECT_EMPLACEMENT(x,x,t);
            }

            sceneData.viewMatrix = gProps.GetMatrix4f(IDR(baseSceneNodeID, ID("viewMatrix")));
        }

        float xxxx = -9;
        float xxxy = 0.1f;
        void SetCamera(IDType node) {
            Vector3f pos = gProps.GetVector3f(IDR(node,ID("pos")));
            const Vector3f& lookAt = gProps.GetVector3f(IDR(node, ID("lookAt")));;
            const Vector3f& lookUp = gProps.GetVector3f(IDR(node, ID("lookUp")));;
            pos.x += xxxx;
            if (abs(xxxx) < 10) {
                xxxx += xxxy;
            } else {
                xxxy *= -1;
                xxxx += xxxy;
            }
            
            Matrix4f::LookAt(pos, lookAt, lookUp, sceneData.viewMatrix);
        }

        void Update() {
            std::vector<IDType> children;

//            sceneData.viewMatrix = gProps.GetMatrix4f(IDR(baseSceneNodeID, ID("viewMatrix")));
            SetCamera(ID("camera"));

            gProps.GetChildrenWithNodeExisting(baseSceneNodeID, ID("model"), children /*fill*/);
            for (auto&& x : children) {
                IDType model_node = IDR(x, "model");
                IDType model_refnid = gProps.GetID(model_node);
                InstanceData &instance = instances.at(x);
               
                renderer::RenderModel(model_refnid, instance.transform, sceneData.viewMatrix);

                // move this to scene
                // TODO get rid of this hardwired thingy
                Vector3f RV = gProps.GetVector3f(IDR(x,ID("rotation")));
                Matrix3f R;
                Matrix3f::SetFromEulers(RV.x, RV.y, RV.z, R);
                instance.transform.Rot(R * instance.transform.Rot());
                instance.transform.OrthoNormalize();
            }
        }

        size_t init_dummy[] =
        {
            {
                nugget::system::RegisterModule([]() {
                if (!Notice::gBoard.KeyExists(ID("commandLine.assignments.runHeadless"))) {
                    Init();
                }
                return 0;
               }, 200,identifier::ID("init"),__FILE__)
            },
            {
                nugget::system::RegisterModule([]() {
                if (!Notice::gBoard.KeyExists(ID("commandLine.assignments.runHeadless"))) {
                    Update();
                }
                return 0;
                }, 200, ID("update"))
            }
        };
    }
}