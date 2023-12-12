#include "../identifier.h"
#include <SFML/Graphics.hpp>
#include "../notice.h"
#include <unordered_map>
#include <string>
#include <numeric>
#include <algorithm>

#include "../ui_imp.h"
#include "../debug.h"

#include "../../utils/StableVector.h"


namespace nugget {
    using namespace identifier;
    namespace ui_imp {
        namespace container {
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

                    auto col = sf::ToPlatform(Notice::GetColor(IDR(id, "color")));

                    for (int i = 0; i < 8; i++) {
                        lines[i].color = col;
                    }
                   
                    mainWindow->draw(lines);
                }

                void SpaceEvenly(int axis) {
                    std::vector<IDType> children;
                    if (auto r = Notice::GetChildrenWithNodeExisting(IDR(id, "sub"), IDR("geom"), children)) {
                        float childTotalDims = std::accumulate(
                            children.begin(),
                            children.end(),
                            0.0f, [&](float acc, IDType nodeId) {
                                return acc + GetSizeCoordPropertyComputed(nodeId, axis);
                            });

                        float parentDim = GetSizeCoordPropertyComputed(id, axis);
                        float parentPos = GetPosCoordPropertyComputed(id, axis);
                        float gap = (parentDim - childTotalDims) / (children.size() + 1);

                        float pos = 0;

                        std::ranges::sort(children, [](const IDType& a, const IDType& b) {
                            IDType as = IDR(a, IDR("_seq"));
                            IDType bs = IDR(b, IDR("_seq"));
                            bool ae = Notice::KeyExists(as);
                            bool be = Notice::KeyExists(bs);

                            if (ae) {
                                return be && Notice::GetInt64(as) < Notice::GetInt64(bs);
                            } else {
                                return be;
                            }
                            });

                        for (auto& x : children) {
                            auto width = GetSizeCoordPropertyComputed(x, axis);
                            pos += gap + width;
                            SetPosCoordPropertyComputed(x, axis, pos + parentPos - width);
                        }
                    }
                }

                void Centre(int axis) {
                    std::vector<IDType> children;
                    if (auto r = Notice::GetChildrenWithNodeExisting(IDR(id, "sub"), ID("geom"),children)) {

                        float parentDim = GetSizeCoordPropertyComputed(id, axis);
                        float parentPos = GetPosCoordPropertyComputed(id, axis);
                        float centre = parentPos + parentDim / 2;

                        for (auto& x : children) {
                            auto width = GetSizeCoordPropertyComputed(x, axis);
                            SetPosCoordPropertyComputed(x, axis, centre - width / 2);
                        }
                    }
                }

                bool IsNodeVoid(IDType parentId) {
                    if (Notice::KeyExists(parentId)) {
                        Notice::ValueAny value = Notice::GetValueAny(parentId);
                        assert(value.IsVoid());
                        return true;
                    } else {
                        // if parent node does not exist, it is not void
                        return false;
                    }
                }


                static void ManageGeometrySelfAll() {
                    for (auto& x : list.GetArray()) {
                        if (!x.deleted) {
                            x.ConfigureSelfGeom();
                        }
                    }
                }

                void ManageGeometryChildren() {
                    // check we have not been deleted
                    assert(Notice::KeyExists(id));

                    IDType hpolicy = Notice::GetID(IDR(id, "hpolicy"));
                    IDType vpolicy = Notice::GetID(IDR(id, "vpolicy"));

                    switch (hpolicy) {
                        case ID("space"): {
                            SpaceEvenly(0);
                        } break;
                        case ID("centre"): {
                            Centre(0);
                        } break;
                        default:
                            assert(0);
                    }

                    switch (vpolicy) {
                        case ID("space"): {
                            SpaceEvenly(1);
                        } break;
                        case ID("centre"): {
                            Centre(1); 
                        } break;
                        default:
                            assert(0);
                    }
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

            private:
                static std::unordered_map<IDType, size_t> index;
                static StableVector<Imp, maxEntities> list;
            };

            std::unordered_map<IDType, size_t> Imp::index;
            StableVector<Imp, maxEntities> Imp::list;


            void DrawAll() {
                Imp::DrawAll();
            }
        }
    }
    namespace ui::container {

        void Create(IDType id) {
            ui_imp::container::Imp::Create(id);
        }
        
        void ManageGeometrySelf() {
            ui_imp::container::Imp::ManageGeometrySelfAll();
        }

        void ManageGeometryChildren(IDType node) {
            ui_imp::container::Imp* inst = ui_imp::GetInstance(node);

            std::vector<IDType> children;
            if (auto r = Notice::GetChildrenWithNodeExisting(IDR(node, "sub"), ID("geom"), children)) {
                for (auto& x : children) {
                    // check if container node
                    if (ui_imp::CheckInstanceType(typeid(ui_imp::container::Imp*), node)) {
                        ManageGeometryChildren(x);
                    }
                }
            }
            inst->ManageGeometryChildren();
        }

        size_t init_dummy = nugget::ui::entity::RegisterEntityInit([]() {
            auto impNode = IDR("ui::Container");
            nugget::ui::entity::RegisterSelfGeomFunction(impNode,
                nugget::ui::container::ManageGeometrySelf);
            nugget::ui::entity::RegisterEntityCreator(impNode,
                nugget::ui::container::Create);
            nugget::ui::entity::RegisterPostSelfGeomFunction(impNode,
                nugget::ui::container::ManageGeometryChildren);
            nugget::ui::entity::RegisterEntityDraw(impNode,
                nugget::ui_imp::container::DrawAll);

            });

    }
}
