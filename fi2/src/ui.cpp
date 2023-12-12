#include <iostream>
#include <filesystem>
#include <SFML/Graphics.hpp>
#include <assert.h>
#include <functional>
#include <memory>

#include "identifier.h"
#include "UI.h"
#include "notice.h"
#include "UIEntities.h"
#include "ui_imp.h"
#include "uicircle.h"

namespace nugget::ui {
    void Init() {
        ui_imp::Init();

    }

    void Exec(const std::function<void()>& updateCallback) {
        ui_imp::Exec(updateCallback);
    }
}








#if 0
namespace nugget::ui {
    using namespace identifier;
    class ButtonImp {
    public:
//        ButtonImp() {
  //      }

        ButtonImp(IDType id,float x, float y, float width, float height, sf::Color idleColor, sf::Color hoverColor,
            const std::string& buttonText, const sf::Font& font, unsigned int characterSize,IDType statusID)
            : rect(sf::Vector2f(width, height)),
            idleColor(sf::ToPlatform(Notice::GetColor(IDCombine(id, "color")))),
            hoverColor(hoverColor), statusID(statusID) {
            rect.setPosition(x, y);
            rect.setFillColor(idleColor);

            text.setFont(font);
            text.setString(buttonText);
            text.setCharacterSize(characterSize);
            text.setFillColor(sf::Color::White);

            // Center the text both horizontally and vertically within the button
            sf::FloatRect textBounds = text.getLocalBounds();
            text.setPosition(
                x + (width - textBounds.width) / 2,
                y + (height - textBounds.height) / 2 - textBounds.top
            );
        }

        bool isMouseOver(const sf::RenderWindow& window) const {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            return rect.getGlobalBounds().contains(mousePos);
        }

        void update(const sf::RenderWindow& window) {
            if (isMouseOver(window)) {
                rect.setFillColor(hoverColor);
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    Notice::Set(statusID, 1);
                } else {
                    Notice::Set(statusID, 0);
                }
            } else {
                rect.setFillColor(idleColor);
            }
        }

        void draw(sf::RenderWindow& window) const {
            window.draw(rect);
            window.draw(text);
        }

    private:
        sf::RectangleShape rect;
        sf::Color idleColor;
        sf::Color hoverColor;
        sf::Text text;
        IDType statusID;
    };

    class CircleImp {
    public:

        CircleImp(IDType id, float x, float y, float radius) : id(id)
             {
            circle = sf::CircleShape(radius);
            sf::Color color = sf::ToPlatform(Notice::GetColor(IDCombine(id, "color")));
            circle.setPosition(x - radius, y - radius); // Set position for the center of the circle
            circle.setFillColor(color);

        }

        void Init() {
            IDType colorID = IDCombine(id, "color");
            Notice::RegisterHandler(colorID, [&](IDType id) {
                circle.setFillColor(sf::ToPlatform(Notice::GetColor(id)));
                });
        }

        void draw(sf::RenderWindow& window) const {
            window.draw(circle); 
        }

    private:
    sf::CircleShape circle;
    IDType id;
        
    };

    struct Buttons {
        struct Data {
            std::vector<std::vector<ButtonImp>> layers;
            std::unordered_map<IDType, ButtonImp*> map;
            Data() {
                layers.push_back(std::vector<ButtonImp>());
            }
        };
        static Data data;

        static void UpdateAll() {
            for (auto&& x : data.layers) {
                for (auto& y : x) {
                    y.update(*mainWindow);
                }
            }
        }
        static void DrawAll() {
            for (auto& x : data.layers) {
                for (auto& y : x) {
                    y.draw(*mainWindow);
                }
            }
        }
        static void Make(IDType id, IDType classid, const Geometry& geom, const std::string& text) {
            assert(!data.map.contains(id));
            sf::Color color = sf::ToPlatform(Notice::GetColor(IDCombine(id, "color")));
            size_t layer = 0;
            IDType key = IDCombine(id, "state");
            ButtonImp button = ButtonImp(id,(float)geom.x, (float)geom.y, (float)geom.w, (float)geom.h, color, sf::Color(64, 64, 0),
                text, font, 20, key);
            data.layers[layer].push_back(std::move(button));
            data.map[id] = &data.layers[layer].back();
        }
    };
    Buttons::Data Buttons::data;

    void Button(IDType id, IDType classid, const Geometry& geom, const std::string& text) {
        Buttons::Make(id, classid, geom, text);
    }

    struct Circles {
        struct Data {
            std::vector<std::vector<CircleImp>> layers;
            std::unordered_map<IDType, CircleImp*> map;
            Data() {
                layers.push_back(std::vector<CircleImp>());
            }
        };
        static Data data;
        void UpdateAll() {
        }
        void DrawAll() {
            for (auto& x : data.layers) {
                for (auto& y : x) {
                    y.draw(*mainWindow);
                }
            }
        }
        static void Make(IDType id, IDType classid, const Geometry& geom) {
            assert(!data.map.contains(id));
            size_t layer = 0;
            CircleImp circle = CircleImp(id,(float)geom.x, (float)geom.y, (float)geom.w);
            data.layers[layer].push_back(std::move(circle));
            auto finalCircle = data.map[id] = &data.layers[layer].back();
            finalCircle->Init();
        }
        sf::Color color;
    };
    Circles::Data Circles::data;

    void Circle(IDType id, IDType classid, const Geometry& geom) {
        Circles::Make(id, classid, geom);
    }

    ///////////////////////////////////////
    static Buttons buttons;
    static Circles circles;
    ///////////////////////////////////////

}


#endif