#include "identifier.h"
#include <SFML/Graphics.hpp>
#include "notice.h"
#include <unordered_map>
#include <string>

#include "ui_imp.h"
#include "UIContainer.h"
#include "debug.h"
#include "uibutton.h"
#include "uientities.h"

namespace nugget::ui_imp::button {
    using namespace identifier;
    struct Imp : UiEntityBaseImp {
        APPLY_RULE_OF_MINUS_5(Imp);

        explicit Imp(IDType idIn) : UiEntityBaseImp(idIn) {
            Validate();

            ConfigureSelfGeom();
            UpdateGeomProperties();

            Notice::Set(IDR(id, "pressEventCount"), 0);
            AddHandlerOnChildren(IDR(id, "geom"), [&](IDType changeId) {
                UpdateGeomProperties();
                });

            AddHandler(id, [&](IDType changeId) {
                MarkDeleted();
                list.erase(id);
                });
        }
            
        class InputTracker {
            bool previousState;
            bool justPressed;
            sf::Vector2f wherePressed;

            sf::Vector2f GetMouseCoords() {
                return mainWindow->mapPixelToCoords(sf::Vector2i(sf::Mouse::getPosition(*mainWindow)));
            }

            // call every frame
        public:
            void Update() {
                bool nState = sf::Mouse::isButtonPressed(sf::Mouse::Left);
                bool pState = previousState;
                previousState = nState;
                if (nState && !pState) {
                    sf::Vector2f mousePos = GetMouseCoords();
                    wherePressed = mousePos;
                    justPressed = true;
                } else {
                    justPressed = false;
                }
            }
            bool WasJustPressed() {
                return justPressed;
            }
            sf::Vector2f GetMouseEventPosition() {
                return wherePressed;
            }
            sf::Vector2f GetMouseCurrentPosition() {
                return  GetMouseCoords();
            }

        };

        InputTracker inputTracker = {};

        bool IsPositionHovering(sf::Vector2f v) {
            return v.x >= geom.cx && v.x < geom.cx + geom.cw &&
                v.y >= geom.cy && v.y < geom.cy + geom.ch;
        }

        void Draw() {
            inputTracker.Update();

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

            auto mousePosition = inputTracker.GetMouseCurrentPosition();
            bool hovering = IsPositionHovering(mousePosition);

            sf::Color col;
            if (hovering) {
                col = sf::ToPlatform(Notice::GetColor(IDR(id, "highlightColor")));
            } else {
                col = sf::ToPlatform(Notice::GetColor(IDR(id, "color")));
            }

            for (int i = 0; i < 8; i++) {
                lines[i].color = col;
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


            if (inputTracker.WasJustPressed()) {
                auto mousepos = inputTracker.GetMouseEventPosition();
                if (IsPositionHovering(mousepos)) {
                    IDType idEvent = IDR(id, "pressEventCount");
                    Notice::Set(idEvent, Notice::GetInt32(idEvent) + 1);

                    IDType onClick = IDR(id, ID("onClick"));
                    if (Notice::KeyExists(onClick)) {
                        auto function = IDR(onClick, ID("function"));
                        check(Notice::KeyExists(function), "onClick needs a function node");
                        ui::entity::CallFunction(Notice::GetID(function), id);
                    }
                }
            }
        }

        static void Create(IDType id) {
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
        sf::Text text;
    };

    void Create(IDType id) {
        return Imp::Create(id);
    }
    /*static*/
    void DrawAll() {
        Imp::DrawAll();
    }

}

namespace nugget::ui::button {
    void Create(IDType id) {
        return ui_imp::button::Imp::Create(id);
    }
}
