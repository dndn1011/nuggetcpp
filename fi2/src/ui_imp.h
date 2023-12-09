#pragma once

#include <functional>
#include <SFML/Graphics.hpp>
#include "identifier.h"
#include "notice.h"
#include "dimensions.h"

#define APPLY_RULE_OF_MINUS_5(Imp) \
            Imp() = delete;\
            Imp(const Imp& other) = delete;\
            Imp(Imp&& other) = delete;\
            Imp& operator=(const Imp& other) = delete;\
            Imp& operator=(Imp&& other) = delete

namespace nugget::ui_imp {
    ///////////////////////////////////////
    extern std::unique_ptr<sf::RenderWindow> mainWindow;
    extern sf::Font font;
    ///////////////////////////////////////

	void Init();
    void Exec(const std::function<void()>& updateCallback);

    ui::Dimension GetPosCoordProperty(identifier::IDType id,int i);
    ui::Dimension GetSizeCoordProperty(identifier::IDType id, int i);
    void SetPosCoordProperty(identifier::IDType id, int axis, const ui::Dimension &val);
    void SetSizeCoordProperty(identifier::IDType id, int axis, const ui::Dimension& val);
    float GetSizeCoordPropertyComputed(identifier::IDType id, int axis);
    float CalcSelfSize(identifier::IDType id, int axis);
    void SetSizeCoordPropertyComputed(identifier::IDType id, int axis, float val);
    void SetPosCoordPropertyComputed(identifier::IDType id, int axis, float val);
    float CalcSelfPos(identifier::IDType id, int axis);
    float GetPosCoordPropertyComputed(identifier::IDType id, int axis);

    struct Geom {
        ui::Dimension x, y, w, h;
        float cx, cy, cw, ch;
    };

    struct UiEntityBaseImp {
        ~UiEntityBaseImp();

        void AddHandler(identifier::IDType nodeId, Notice::HandlerFunc func);
        void AddHandlerOnChildren(identifier::IDType nodeId, Notice::HandlerFunc func);
        void MarkDeleted();
        void UpdateGeomProperties();
        void ConfigureSelfGeom();

        float GetPosCoord(int i);
        float GetSizeCoord(int i);
        
        std::vector<Notice::Handler> handlers;

        bool Validate();

    protected:
        bool deleted = false;
        UiEntityBaseImp(identifier::IDType idIn) : id(idIn) {}
        bool allowDeletion = false;
        identifier::IDType id = 0;

        Geom geom = {};
    };
}