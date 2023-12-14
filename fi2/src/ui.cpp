#include <iostream>
#include <filesystem>
#include <assert.h>
#include <functional>
#include <memory>

#include "identifier.h"
#include "UI.h"
#include "notice.h"
#include "UIEntities.h"
#include "ui_imp.h"
#include "nuggetgl/gl.h"

namespace nugget::ui {
    void Init() {
        nugget::gl::OpenWindow();
    }

    void Exec(const std::function<void()>& updateCallback) {
//        ui_imp::Exec(updateCallback);
        nugget::gl::MainLoop(updateCallback);
    }
}

