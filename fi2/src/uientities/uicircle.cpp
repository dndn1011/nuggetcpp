#include "identifier.h"
#include <SFML/Graphics.hpp>
#include "notice.h"
#include <unordered_map>
#include <string>

#include "ui_imp.h"
#include "debug.h"
#include "uientities.h"
#include "utils/StableVector.h"

namespace nugget::ui_imp::circle {
    using namespace identifier;
    using namespace utils;

    static const size_t maxEntities = 100;
    struct Imp : UiEntityBaseImp {
        APPLY_RULE_OF_MINUS_4(Imp);

        Imp() = default;

        explicit Imp(IDType idIn) : UiEntityBaseImp(idIn) {
            RegisterInstance(this);

            UpdateGeomProperties();

            AddHandlerOnChildren(IDR(id, "geom"), [&](IDType changeId) {
                UpdateGeomProperties();
                });

            AddHandler(id, [&](IDType changeId) {
                MarkDeleted();
                });
        }

        void Draw() {
            sf::CircleShape circle(geom.cw/2); // Diameter of 50 pixels
            circle.setPosition(geom.cx, geom.cy); // Position the circle in the center

            Color col = Notice::GetColor(IDR(id, "color"));

            circle.setFillColor(sf::ToPlatform(col));
             
            mainWindow->draw(circle);
        }

        static void Create(IDType id) {
            // The emplace uses an implicit construction  to call the single
            // valid constructor and emplace the result in the container
            // To not try to explcitly convert the hashid (IDType) to Imp (implementation)
            // It will not compile, because of rule of -5
            if (!index.contains(id)) {
                size_t i = list.emplace_back((id)/*->Imp */);
                index.emplace(id, i);
            }
        }

        static void DrawAll() {
            for (auto& x : list.GetArray()) {
                if (!x.deleted) {
                    x.Draw();
                }
            }
        }

        static void ManageGeometrySelfAll() {
            for (auto& x : list.GetArray()) {
                if (!x.deleted) {
                    x.ConfigureSelfGeom();
                }
            }
        }

    private:
        static std::unordered_map<IDType, size_t> index;
        static StableVector<Imp, maxEntities> list;
    };
    StableVector<Imp, maxEntities> Imp::list;
    std::unordered_map<IDType, size_t> Imp::index;

    void Create(IDType id) {
        Imp::Create(id);
    }
    /*static*/
    void DrawAll() {
        Imp::DrawAll();
    }

}

namespace nugget::ui::circle {
    using namespace nugget::identifier;

    void Create(IDType id) {
        ui_imp::circle::Imp::Create(id);
    }

    void ManageGeometrySelf() {
        ui_imp::circle::Imp::ManageGeometrySelfAll();
    }

    size_t init_dummy = nugget::ui::entity::RegisterEntityInit([]() {

        auto impNode = IDR("ui::Circle");
        nugget::ui::entity::RegisterSelfGeomFunction(impNode,
            nugget::ui::circle::ManageGeometrySelf);
        nugget::ui::entity::RegisterEntityCreator(impNode,
            nugget::ui::circle::Create);
        nugget::ui::entity::RegisterEntityDraw(impNode,
            nugget::ui_imp::circle::DrawAll);

        });

}
