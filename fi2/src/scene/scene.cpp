#include "system.h"
#include "propertytree.h"

namespace nugget::scene {
    using namespace identifier;
    using namespace properties;

    namespace {
        void Init() {
            IDType baseSceneNodeID = ID("scene");
            std::vector<IDType> children;
            gProps.GetChildrenWithNodeExisting(baseSceneNodeID, ID("model"), children /*fill*/);
            for (auto&& x : children) {
                IDType model_nid = IDR(x,"model");
                IDType model_refnid = gProps.GetID(model_nid);
                std::vector<IDType> sections;
                gProps.GetChildrenWithNodeExisting(model_refnid,ID("function"),sections);
                for (auto&& y : sections) {
                    IDType funcID = gProps.GetID(IDR(y, ID("function")));
                    system::CallFunctionByID(funcID);
                }
            }
        }

        void Update() {
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