#include "renderer.h"
#include "nuggetgl/glinternal.h"
#include <unordered_map>
#include "propertytree.h"
#include "graphicsapi.h"
#include "system.h"

namespace nugget::renderer {
    using namespace nugget::gl;
    using namespace nugget::properties;


    void ConfigureRenderModel(IDType nodeID) {
        std::vector<IDType> sections;
        gProps.GetChildrenWithNodeExisting(nodeID, ID("function"), sections);
        ConfigureModel(nodeID, sections);
    }

#if 0

	void Update() {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
		glEnable(GL_FRAMEBUFFER_SRGB);

		for (auto&& x : renderables) {
			glBindVertexArray(x.VAO);
			for (auto&& y : x.sections) {
				y.Render();
			}
		}
	}
    struct Renderable {
        APPLY_RULE_OF_MINUS_4(Renderable);

        Renderable() {
        }

        GLuint VAO = 0;
        StableVector<RenderableSection, 100> sections;

        bool AddSections(IDType id) {
            check(VAO, "Ya didn't init, innit?");
            // id is a section in propertytree to populate the instance
            check(gProps.KeyExists(id), "Node does not exist: {}\n", IDToString(id));
            std::vector<IDType> children;
            gProps.GetChildrenOfType(id, ValueAny::Type::parent_, children);
            glBindVertexArray(VAO);
            GLuint index = 0;
            for (auto&& x : children) {
                sections.emplace_back();
                auto& section = sections.back();
                section.SetupFromPropertyTree(x, index);
                index++;
            }
            glBindVertexArray(0);
            return true;
        }
    };

    static StableVector<Renderable, 100> renderables;

    static Renderable& NewRenderable() {
        renderables.emplace_back();
        return renderables.back();
    }

    static void InitRenderables() {
        // TODO pull the object from property tree with all sections
        Renderable& renderable = NewRenderable();
        renderable.Init();
        renderable.AddSections(ID("indexedMesh"));  // TODO get rid of hard wiring
    }

    }
#endif
}