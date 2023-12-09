#pragma once
namespace std {
    template<typename> class function;
}

namespace nugget::ui {
    using namespace identifier;
    struct Geometry {
        int x = 0, y = 0, w = 100, h = 100;
    };

    void Exec(const std::function<void()>& updateCallback);
    void Button(IDType id, IDType classid, const Geometry& geom, const std::string& text);
    void Circle(IDType id, IDType classid, const Geometry& geom);
    void Init();
    void Exec(const std::function<void()>& updateCallback);
}


