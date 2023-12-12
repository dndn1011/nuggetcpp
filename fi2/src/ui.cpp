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

namespace nugget::ui {
    void Init() {
        ui_imp::Init();

    }

    void Exec(const std::function<void()>& updateCallback) {
        ui_imp::Exec(updateCallback);
    }
}

