#include "identifier.h"
#include <SFML/Graphics.hpp>
#include "notice.h"
#include <unordered_map>
#include <string>

#include "ui_imp.h"
#include "UIContainer.h"
#include "debug.h"
#include "uientities.h"

namespace nugget::ui_imp::circle {
    using namespace identifier;
    struct Imp : UiEntityBaseImp {
        APPLY_RULE_OF_MINUS_5(Imp);

        explicit Imp(IDType idIn) : UiEntityBaseImp(idIn) {
            Validate();

            UpdateGeomProperties();

            AddHandlerOnChildren(IDR(id, "geom"), [&](IDType changeId) {
                UpdateGeomProperties();
                });

            AddHandler(id, [&](IDType changeId) {
                MarkDeleted();
                list.erase(id);
                });
        }

        void Draw() {
            sf::CircleShape circle(geom.w/2); // Diameter of 50 pixels
            circle.setPosition(geom.x, geom.y); // Position the circle in the center

            Color col = Notice::GetColor(IDR(id, "color"));

            circle.setFillColor(sf::ToPlatform(col));

            mainWindow->draw(circle);
        }

        static void Create(IDType id) {
            // The emplace uses an implicit construction  to call the single
            // valid constructor and emplace the result in the container
            // To not try to explcitly convert the hashid (IDType) to Imp (implementation)
            // It will not compile, because of rule of -5
            if (!list.contains(id)) {
                auto r = list.emplace(id, (id)/*->Imp */);
                assert(r.second);
            }
        }

        static void DrawAll() {
            for (auto& x : list) {
                if (!x.second.deleted) {
                    x.second.Draw();
                }
            }
        }

    private:
        static inline std::unordered_map<IDType, Imp> list;
    };
        
    void Create(IDType id) {
        //output("---> %s\n", IDToString(id).c_str());
        return Imp::Create(id);
    }
    /*static*/
    void DrawAll() {
        Imp::DrawAll();
    }

}
 
namespace nugget::ui::circle {
    void Create(IDType id) {
        return ui_imp::circle::Imp::Create(id);
    }
}
