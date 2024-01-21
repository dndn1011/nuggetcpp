#include "system.h"
#include "debug.h"
#include "identifier.h"
#include "notice.h"
#include "propertytree.h"

namespace nugget::tests {
    using namespace identifier;
    using namespace properties;

    static void Init() {
        std::vector<IDType> list;
        gNotice.GetChildrenWithNodeExisting(ID("tests"), ID("mat"), list);
        int errors = 0;
        int cases = 0;
        for (auto&& x : list) {
            Matrix4f mat = gNotice.GetMatrix4f(IDR(x, ID("mat")));
            Vector4f vec = gNotice.GetVector4f(IDR(x, ID("vec")));
            Vector4f expected = gNotice.GetVector4f(IDR(x, ID("result")));
            Vector4f value = mat * vec;
            if (value != expected) {
                output("ERROR {} {} {}\n", IDToString(x), expected.to_string(), value.to_string());
                errors++;
            }
            cases++;
        }
        output("TEST COMPLETED: {} cases, {} errors\n", cases, errors);
        if (errors > 0) {
            exit(1);
        }
    }

    static size_t init_dummy = nugget::system::RegisterModule([]() {
        Init();
        return 0;
        }, 999);

}