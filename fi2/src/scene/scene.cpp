#include "system.h"
#include "propertytree.h"
#include "renderer/renderer.h"

namespace nugget::scene {
    using namespace identifier;
    using namespace properties;

    namespace {
        const IDType baseSceneNodeID = ID("scene");
        void Init() {
            
            std::vector<IDType> children;
            gProps.GetChildrenWithNodeExisting(baseSceneNodeID, ID("model"), children /*fill*/);
            for (auto&& x : children) {
                IDType model_nid = IDR(x,"model");
                IDType model_refnid = gProps.GetID(model_nid);
                renderer::ResModel(model_refnid);
            }
        }

        void Update() {
            std::vector<IDType> children;
            gProps.GetChildrenWithNodeExisting(baseSceneNodeID, ID("model"), children /*fill*/);
            for (auto&& x : children) {
                IDType model_nid = IDR(x, "model");
                IDType model_refnid = gProps.GetID(model_nid);
                renderer::RenderModel(model_refnid);
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