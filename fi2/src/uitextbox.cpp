#include "identifier.h"
#include <SFML/Graphics.hpp>
#include "notice.h"
#include <unordered_map>
#include <string>
#include <span>

#include "ui_imp.h"
#include "UIContainer.h"
#include "debug.h"
#include "uitextbox.h"
#include "uientities.h"
#include <array>
#include "../utils/stablevector.h"

static const size_t maxEntities = 100;

namespace nugget::ui_imp::textBox {
    using namespace identifier;
    struct Imp : UiEntityBaseImp {
        APPLY_RULE_OF_MINUS_4(Imp);

        Imp() {
        }

        Imp(IDType idIn) : UiEntityBaseImp(idIn) {
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

            sf::VertexArray lines(sf::Lines, 8);

            sf::Vector2f V0(sf::Vector2f(geom.cx, geom.cy));
            sf::Vector2f V1(V0 + sf::Vector2f(geom.cw, 0));
            sf::Vector2f V2(V0 + sf::Vector2f(geom.cw, geom.ch));
            sf::Vector2f V3(V0 + sf::Vector2f(0, geom.ch));

            lines[0].position = V0;
            lines[1].position = V1;
            lines[2].position = V1;
            lines[3].position = V2;
            lines[4].position = V2;
            lines[5].position = V3;
            lines[6].position = V3;
            lines[7].position = V0;


            sf::Color frameColor;
            frameColor = sf::ToPlatform(Notice::GetColor(IDR(id, "frameColor")));

            for (int i = 0; i < 8; i++) {
                lines[i].color = frameColor;
            }

            std::string textString = Notice::GetString(IDR(id, "text"));

            text.setFont(font);
            text.setString(textString);
            text.setCharacterSize((unsigned int)Notice::GetInt64(IDR(id, "textSize")));
            text.setFillColor(sf::Color::White);

            sf::FloatRect textBounds = text.getLocalBounds();

            text.setPosition(
                geom.cx + (geom.cw - textBounds.width) / 2,
                geom.cy + (geom.ch - textBounds.height) / 2 - textBounds.top
            );

            mainWindow->draw(lines);
            mainWindow->draw(text);

        }

        static void Create(IDType id) {
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

        void ManageGeometrySelf() {
            ConfigureSelfGeom();
        }

        
        static void ManageGeometrySelfAll() {
            for (auto& x : list.GetArray()) {
                if (!x.deleted) {
                    x.ConfigureSelfGeom();
                }
            }
        }

    private:
        sf::Text text;

        static std::unordered_map<IDType, size_t> index;
        static StableVector<Imp, maxEntities> list;
    };

    std::unordered_map<IDType, size_t> Imp::index;

    StableVector<Imp, maxEntities> Imp::list;

    void Create(IDType id) {
        return Imp::Create(id);
    }
    /*static*/
    void DrawAll() {
        Imp::DrawAll();
    }

  
}

namespace nugget::ui::textBox {
    void Create(IDType id) {
        //Output("---> %s\n", IDToString(id).c_str());
        ui_imp::textBox::Imp::Create(id);
    }
    void ManageGeometrySelf() {
        ui_imp::textBox::Imp::ManageGeometrySelfAll();
    }

    void Draw() {
        
    }

    size_t init_dummy = nugget::ui::entity::RegisterEntityInit([]() {
        auto impNode = IDR("ui::TextBox");
        nugget::ui::entity::RegisterSelfGeomFunction(impNode,
            nugget::ui::textBox::ManageGeometrySelf);
        nugget::ui::entity::RegisterEntityCreator(impNode,
            nugget::ui::textBox::Create);
        });
}