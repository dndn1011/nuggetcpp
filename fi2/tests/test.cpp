#include "debug.h"
#include "system.h"

#include "../../external/cgltf.h"
#include "../../external/cgltf_write.h"

namespace nugget::gltf {

    // Function to write out the glTF data to a GLB file
    cgltf_result write_glb(const char* path, const cgltf_data* data) {
        cgltf_options options = { cgltf_file_type_glb };
        return cgltf_write_file(&options, path, data);
    }

    cgltf_data* data = NULL;

    void DumpVertices() {
        cgltf_mesh* mesh = &data->meshes[0];

        for (cgltf_size j = 0; j < mesh->primitives_count; j++) {
            cgltf_primitive* primitive = &mesh->primitives[j];

            for (cgltf_size k = 0; k < primitive->attributes_count; k++) {
                cgltf_attribute* attribute = &primitive->attributes[k];

                if (attribute->type == cgltf_attribute_type_position) {
                    cgltf_accessor* accessor = attribute->data;
                    cgltf_buffer_view* buffer_view = accessor->buffer_view;

                    if (!buffer_view) {
                        continue;
                    }

                    const uint8_t* buffer_data = static_cast<const uint8_t*>(cgltf_buffer_view_data(buffer_view));
                    if (!buffer_data) {
                        continue;
                    }

                    std::vector<float> vertex(3); // Assuming 3 components for position

                    for (cgltf_size index = 0; index < accessor->count; index++) {
                        cgltf_accessor_read_float(accessor, index, vertex.data(), 3);
                        output("Vertex {}: ({}, {}, {})\n", index, (float)vertex[0], (float)vertex[1], (float)vertex[2]);
                    }
                }
            }
        }
    }

    void list_faces(cgltf_primitive* primitive) {
        if (primitive->type != cgltf_primitive_type_triangles) {
            printf("Non-triangle primitive, skipping.\n");
            return;
        }

        if (primitive->indices) {
            for (cgltf_size i = 0; i < primitive->indices->count; i += 3) {
                output("Face: {} {} {}\n",
                    cgltf_accessor_read_index(primitive->indices, i),
                    cgltf_accessor_read_index(primitive->indices, i + 1),
                    cgltf_accessor_read_index(primitive->indices, i + 2));
            }
        } else {
            // If the model does not use indices, you would need to read the vertex positions directly
            // This part of the code will depend on how vertex positions are stored in your model
            printf("Mesh does not use indices. Faces are defined by every three vertices.\n");
        }
    }

    void DumpFaces() {
        for (cgltf_size i = 0; i < data->nodes_count; ++i) {
            cgltf_node* node = &data->nodes[i];
            if (node->mesh) {
                for (cgltf_size j = 0; j < node->mesh->primitives_count; ++j) {
                    list_faces(&node->mesh->primitives[j]);
                }
            }
        }
    }

    static void Init() {
        
        const char* filename = "assets/models/utah_teapot_pbr/scene.gltf";

        cgltf_options options = {};
        cgltf_result result = cgltf_parse_file(&options, filename, &data);
        if (result == cgltf_result_success)
        {
                result = cgltf_load_buffers(&options, data, filename);
                if (result != cgltf_result_success) {
                    check(0, "");
                }

            cgltf_result valid = cgltf_validate(data);
            if (result == cgltf_result_success) {
                if (result == 0) {
//                    DumpVertices();
                    DumpFaces();
                } else {
                    check(0, "Could not load file\n");
                }
            } else {
                check(0,"Failed to write GLB file.\n");
            }
            
            cgltf_free(data);
        }

    }



    static size_t init_dummy = nugget::system::RegisterModule([]() {
        Init();
        return 0;
        }, 999);

}
