#include "system.h"
#include "propertytree.h"
#include "renderer/renderer.h"
#include <unordered_map>


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

           InstanceData(IDType node, const Matrix4f modelMatrix) : modelMatrix(modelMatrix), node(node) {
           }

            IDType node = {};
            Matrix4f modelMatrix;
        };

        std::unordered_map<IDType, InstanceData> instances;

        void Init() {
            std::vector<IDType> children;
            gProps.GetChildrenWithNodeExisting(baseSceneNodeID, ID("model"), children /*fill*/);
            for (auto&& x : children) {
                IDType model_nid = IDR(x,"model");
                IDType model_refnid = gProps.GetID(model_nid);
                renderer::ConfigureRenderModel(model_refnid);
                
                IDType modelMatrixNode = IDR(x, "modelMatrix");
                const Matrix4f modelMatrix = gProps.GetMatrix4f(modelMatrixNode);

                instances.PERFECT_EMPLACEMENT(x,x,modelMatrix);
            }

            sceneData.viewMatrix = gProps.GetMatrix4f(IDR(baseSceneNodeID, ID("viewMatrix")));
        }

        void Update() {
            std::vector<IDType> children;

            sceneData.viewMatrix = gProps.GetMatrix4f(IDR(baseSceneNodeID, ID("viewMatrix")));

            gProps.GetChildrenWithNodeExisting(baseSceneNodeID, ID("model"), children /*fill*/);
            for (auto&& x : children) {
                IDType model_node = IDR(x, "model");
                IDType model_refnid = gProps.GetID(model_node);
                InstanceData &instance = instances.at(x);
               
                renderer::RenderModel(model_refnid, instance.modelMatrix, sceneData.viewMatrix);

                // move this to scene
                // TODO get rid of this hardwired thingy
                Vector3f RV = gProps.GetVector3f(IDR(x,ID("rotation")));
                Matrix4f R;
                Matrix4f::SetFromEulers(RV.x, RV.y, RV.z, R);
                instance.modelMatrix = R * instance.modelMatrix;
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
               }, 200)
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