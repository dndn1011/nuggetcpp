
#include "ui_imp.h"
#include "UIEntities.h"
#include <assert.h>
#include "uicontainer.h"
#include "uicircle.h"
#include "uibutton.h"
#include "uitextBox.h"
#include "notice.h"
#include "debug.h"

namespace nugget::ui_imp {
    using namespace nugget::Notice;
    using namespace nugget::ui;

    std::unique_ptr<sf::RenderWindow> mainWindow;
    sf::Font font;

    EntityPointer& GetInstance(IDType id) {
        check(entityMap.contains(id), "Requested instance is not in map\n");
        return entityMap.at(id);
    }

    bool CheckInstanceType(const type_info &type,IDType id) {
        if (entityMap.contains(id)) {
            return false; // it is not an instance
        }
        return entityMap.at(id).typeHash == type.hash_code();
    }


    void Init() {
        mainWindow = std::make_unique<sf::RenderWindow>(sf::VideoMode(1600, 960), "SFML Window");
        sf::RenderWindow& window = *mainWindow;
        sf::View view(sf::FloatRect(0, 0, 800, 480));

        window.setView(view);
        window.setPosition(sf::Vector2i(-1700, -200));

        entity::Init();

        if (!font.loadFromFile("c:\\windows\\fonts\\arial.ttf")) {
            assert(0);
        }

    }

    void Exec(const std::function<void()>& updateCallback) {
        sf::RenderWindow& window = *mainWindow;

        int count = 600;

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }

            window.clear();

            container::DrawAll();
            circle::DrawAll();
            button::DrawAll();
//            textBox::DrawAll();

            window.display();


            sf::sleep(sf::milliseconds(16));

            updateCallback();
        }
    }

    float CalcSelfSize(identifier::IDType id, int axis) {
        IDType parent = GetParent(GetParent(id));
        Dimension selfDimension = GetSizeCoordProperty(id, axis);
        float parentSize = GetSizeCoordPropertyComputed(parent, axis);

        switch (selfDimension.unit) {
            case Dimension::Units::none: {
                // absolute size
                return selfDimension.f;
            }break;
            case Dimension::Units::percent: {
                // relative to parent
                return selfDimension.f * parentSize / 100.0f;
            }break;
            default:
                assert(0);
        }
        return 0;
    }

    float CalcSelfPos(identifier::IDType id, int axis) {
        Dimension selfDimension = GetPosCoordProperty(id, axis);
        return selfDimension.f;
    }

    Dimension GetPosCoordProperty(identifier::IDType id, int axis) {
        IDType gid = IDR(id, ID("geom"));
        if (axis == 0) {
            auto vid = IDR(gid, "x");
            assert(Notice::KeyExists(vid));
            return Notice::GetDimension(vid);
        } else {
            auto vid = IDR(gid, "y");
            assert(KeyExists(vid));
            return GetDimension(vid);
        }
    }
    Dimension GetSizeCoordProperty(identifier::IDType id, int axis) {
        IDType gid = IDR(id, ID("geom"));
        if (axis == 0) {
            auto vid = IDR(gid, "w");
            assert(KeyExists(vid));
            return GetDimension(vid);
        } else {
            auto vid = IDR(gid, "h");
            assert(KeyExists(vid));
            return GetDimension(vid);
        }
    }
    float GetSizeCoordPropertyComputed(identifier::IDType id, int axis) {
        IDType gid = IDR(id, ID("geom"));
        if (axis == 0) {
            auto vid = IDR(gid, "cw");
            assert(KeyExists(vid));
            return GetFloat(vid);
        } else {
            auto vid = IDR(gid, "ch");
            assert(KeyExists(vid));
            return GetFloat(vid);
        }
    }
    float GetPosCoordPropertyComputed(identifier::IDType id, int axis) {
        IDType gid = IDR(id, ID("geom"));
        if (axis == 0) {
            auto vid = IDR(gid, "cx");
            assert(KeyExists(vid));
            return GetFloat(vid);
        } else {
            auto vid = IDR(gid, "cy");
            assert(KeyExists(vid));
            return GetFloat(vid);
        }
    }
    void SetPosCoordProperty(identifier::IDType id, int axis, const Dimension &val) {
        IDType gid = IDR(id, ID("geom"));
        if (axis == 0) {
            auto vid = IDR(gid, "x");
            Set(vid, val);
        } else {
            auto vid = IDR(gid, "y");
            Set(vid, val);
        }
    }
    void SetSizeCoordProperty(identifier::IDType id, int axis, float val) {
        IDType gid = IDR(id, ID("geom"));
        if (axis == 0) {
            auto vid = IDR(gid, "w");
            Set(vid, val);
        } else {
            auto vid = IDR(gid, "h");
            Set(vid, val);
        }
    }
    void SetSizeCoordPropertyComputed(identifier::IDType id, int axis, float val) {
        IDType gid = IDR(id, ID("geom"));
        if (axis == 0) {
            auto vid = IDR(gid, "cw");
            Set(vid, val);
        } else {
            auto vid = IDR(gid, "ch");
            Set(vid, val);
        }
    }
    void SetPosCoordPropertyComputed(identifier::IDType id, int axis, float val) {
        IDType gid = IDR(id, ID("geom"));
        if (axis == 0) {
            auto vid = IDR(gid, "cx");
            Set(vid, val);
        } else {
            auto vid = IDR(gid, "cy");
            Set(vid, val);
        }
    }


    UiEntityBaseImp::~UiEntityBaseImp() {
        //assert(allowDeletion);
    }

    void UiEntityBaseImp::AddHandler(identifier::IDType nodeId, Notice::HandlerFunc func) {
        auto h = Notice::Handler{ nodeId,nodeId, func };
        handlers.push_back(h);
        Notice::RegisterHandler(h);
    }

    void UiEntityBaseImp::AddHandlerOnChildren(identifier::IDType nodeId, Notice::HandlerFunc func) {
        auto h = Notice::Handler{ nodeId,0, func };
        Notice::RegisterHandlerOnChildren(h, handlers /*fill*/);
    }

    void UiEntityBaseImp::MarkDeleted() {
        for (auto& x : handlers) { 
            Notice::UnregisterHandler(x);
        }
        deleted = true;
    }

    void UiEntityBaseImp::ConfigureSelfGeom() {
        auto gid = IDR(id, "geom");
        Notice::Set(IDR(gid, "cx"), CalcSelfPos(id, 0));
        Notice::Set(IDR(gid, "cy"), CalcSelfPos(id, 1));
        Notice::Set(IDR(gid, "cw"), CalcSelfSize(id, 0));
        Notice::Set(IDR(gid, "ch"), CalcSelfSize(id, 1));
    }

    void UiEntityBaseImp::UpdateGeomProperties() {
        auto gid = IDR(id, "geom");
        if (Notice::KeyExists(gid)) {
            geom = {
                Notice::GetDimension(IDR(gid,"x")),
                Notice::GetDimension(IDR(gid,"y")),
                Notice::GetDimension(IDR(gid,"w")),
                Notice::GetDimension(IDR(gid,"h")),
                Notice::GetFloat(IDR(gid,"cx")),
                Notice::GetFloat(IDR(gid,"cy")),
                Notice::GetFloat(IDR(gid,"cw")),
                Notice::GetFloat(IDR(gid,"ch")),
            };
        } else {
            // it has been deleted
            MarkDeleted();
        }
    }

    float UiEntityBaseImp::GetPosCoord(int i) {
        return (i == 0) ? (float)(geom.x) : (float)(geom.y);
    }
    float UiEntityBaseImp::GetSizeCoord(int i) {
        return (i == 0) ? geom.w : geom.h;
    }

    bool UiEntityBaseImp::Validate() {
        if (!Notice::KeyExists(IDR(id, "geom"))) {
            assert(("entity is missing geom node", 0));
            return false;
        }
        return true;
    }

    std::unordered_map<IDType, EntityPointer> entityMap;

    namespace nugget::ui_imp{
        namespace circle {
            class Imp;
        }
    }

    void h(nugget::ui_imp::circle::Imp* ptr) {
    }

    void SetInstance(identifier::IDType id, EntityPointer& ep) {
        entityMap.emplace(id, ep);
    }

//    template void SetInstance<nugget::ui_imp::circle::Imp>(identifier::IDType, const nugget::ui_imp::circle::Imp*);

    void EntityMapEmplace(IDType id, const void* ptr) {
        entityMap.emplace(id, ptr);
    }

//    using namespace circle;
//    class Imp;

        //template void __cdecl nugget::ui_imp::SetInstance<struct nugget::ui_imp::circle::Imp>(unsigned __int64, struct nugget::ui_imp::circle::Imp const*);


}
