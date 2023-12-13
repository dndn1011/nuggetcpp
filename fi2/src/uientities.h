#pragma once
#include "identifier.h"
#include <functional>
#include "utils/utils.h"

namespace nugget::ui::entity {
    struct EntityPointer {

        EntityPointer() = default;
        EntityPointer(const EntityPointer& other) = default;
        EntityPointer(EntityPointer&& other) = delete;
        EntityPointer& operator=(const EntityPointer& other) = delete;
        EntityPointer& operator=(EntityPointer&& other) = delete;

        void* ptr;
        size_t typeHash;

        template <typename T>
        EntityPointer(T p) : ptr((void*)p), typeHash(typeid(T).hash_code()) {
        }

        // convert safely to required type
        template <typename T>
        operator T () const {
            auto thisHash = typeid(T).hash_code();
            check(typeHash == thisHash, ("Invalid retrieval: type mismatch\n"));
            T retr = static_cast<T>(ptr);
            return retr;
        }
    };

    using namespace identifier;

    using CreateLambda = std::function<void(IDType id)>;
    using SimpleLambda = std::function<void()>;

    void CallFunction(IDType functionId, IDType entityId);
    void CreateEntity(IDType id);
    void UpdateEntity(IDType to, IDType from);
    void Init();
    void ManageGeometry(IDType nodeId);
    size_t RegisterEntityInit(std::function<void()> func);
    
    void RegisterPostSelfGeomFunction(IDType node, std::function<void(IDType)> func);
    void RegisterEntityCreator(IDType node, CreateLambda func);
    void RegisterEntityDraw(IDType typeID, SimpleLambda func);
    void RegisterSelfGeomFunction(IDType node, std::function<void()> func);

    void DrawAll();
}
