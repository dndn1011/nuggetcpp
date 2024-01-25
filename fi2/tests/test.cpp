#include "debug.h"
#include "system.h"

#include "../external/cgltf.h"
#include "../external/cgltf_write.h"

namespace nugget::gltf {

    // Function to write out the glTF data to a GLB file
    cgltf_result write_glb(const char* path, const cgltf_data* data) {
        cgltf_options options = { cgltf_file_type_glb };
        return cgltf_write_file(&options, path, data);
    }

    cgltf_data* gData = NULL;


    void FetchVertices(cgltf_data* data, size_t offset, size_t stride, float dataOut[]) {
        size_t outIndex = offset;

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
                        dataOut[outIndex + 0] = vertex[0];
                        dataOut[outIndex + 1] = vertex[1];
                        dataOut[outIndex + 2] = vertex[2];
                        outIndex += stride;
                    }
                    return;   // for now we only support the first set
                }
            }
        }
    }

    void FetchUVs(cgltf_data* data, size_t offset, size_t stride, float dataOut[]) {
        size_t outIndex = offset;
        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            for (cgltf_size j = 0; j < data->meshes[i].primitives_count; ++j) {
                cgltf_primitive* primitive = &data->meshes[i].primitives[j];
                for (cgltf_size k = 0; k < primitive->attributes_count; ++k) {
                    cgltf_attribute* attr = &primitive->attributes[k];
                    if (attr->type == cgltf_attribute_type_texcoord) {
                        cgltf_accessor* accessor = attr->data;
                        for (cgltf_size l = 0; l < accessor->count; ++l) {
                            float uv[2];
                            cgltf_accessor_read_float(accessor, l, uv, 2);
                            dataOut[outIndex + 0] = uv[0];
                            dataOut[outIndex + 1] = uv[1];
                            outIndex += stride;
                        }
                        break; // one set only
                    }
                }
            }
        }
    }

    void FetchNormals(cgltf_data* data, size_t offset, size_t stride, float dataOut[]) {
        size_t outIndex = offset;
        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            for (cgltf_size j = 0; j < data->meshes[i].primitives_count; ++j) {
                cgltf_primitive* primitive = &data->meshes[i].primitives[j];
                for (cgltf_size k = 0; k < primitive->attributes_count; ++k) {
                    cgltf_attribute* attr = &primitive->attributes[k];
                    if (attr->type == cgltf_attribute_type_normal) {
                        cgltf_accessor* accessor = attr->data;
                        for (cgltf_size l = 0; l < accessor->count; ++l) {
                            float normal[3];
                            cgltf_accessor_read_float(accessor, l, normal, 3);
                            dataOut[outIndex + 0] = normal[0];
                            dataOut[outIndex + 1] = normal[1];
                            dataOut[outIndex + 2] = normal[2];
                            outIndex += stride;
                        }
                    }
                }
            }
        }
    }

    void FetchColors(cgltf_data* data, size_t offset, size_t stride, float dataOut[]) {
        size_t outIndex = offset;
        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            for (cgltf_size j = 0; j < data->meshes[i].primitives_count; ++j) {
                cgltf_primitive* primitive = &data->meshes[i].primitives[j];
                for (cgltf_size k = 0; k < primitive->attributes_count; ++k) {
                    cgltf_attribute* attr = &primitive->attributes[k];
                    if (attr->type == cgltf_attribute_type_color) {
                        cgltf_accessor* accessor = attr->data;
                        for (cgltf_size l = 0; l < accessor->count; ++l) {
                            float color[4];
                            cgltf_accessor_read_float(accessor, l, color, 4);
                            dataOut[outIndex + 0] = color[0];
                            dataOut[outIndex + 1] = color[1];
                            dataOut[outIndex + 2] = color[2];
                            dataOut[outIndex + 3] = color[3];
                        }
                    }
                }
            }
        }
    }

    void FetchIndices(cgltf_data* data, uint16_t dataOut[]) {
        size_t outIndex = 0;
        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            for (cgltf_size j = 0; j < data->meshes[i].primitives_count; ++j) {
                cgltf_primitive* primitive = &data->meshes[i].primitives[j];
                if (primitive->indices) {
                    cgltf_accessor* accessor = primitive->indices;
                    for (cgltf_size l = 0; l < accessor->count; ++l) {
                        cgltf_uint index;
                        cgltf_accessor_read_uint(accessor, l, &index, 1);
                        dataOut[outIndex++] = (uint16_t)index;
                    }
                }
            }
        }
    }

    size_t FetchIndexCount(cgltf_data* data) {
        size_t outIndex = 0;
        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            for (cgltf_size j = 0; j < data->meshes[i].primitives_count; ++j) {
                cgltf_primitive* primitive = &data->meshes[i].primitives[j];
                if (primitive->indices) {
                    cgltf_accessor* accessor = primitive->indices;
                    return accessor->count;
                }
            }
        }
        return 0;
    }

    void DumpVertices(cgltf_data* data) {
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

    void DumpUVs(cgltf_data* data) {
        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            for (cgltf_size j = 0; j < data->meshes[i].primitives_count; ++j) {
                cgltf_primitive* primitive = &data->meshes[i].primitives[j];
                for (cgltf_size k = 0; k < primitive->attributes_count; ++k) {
                    cgltf_attribute* attr = &primitive->attributes[k];
                    if (attr->type == cgltf_attribute_type_texcoord) {
                        cgltf_accessor* accessor = attr->data;
                        for (cgltf_size l = 0; l < accessor->count; ++l) {
                            float uv[2];
                            cgltf_accessor_read_float(accessor, l, uv, 2);
                            output("UV {}: {}, {}\n",l,uv[0],uv[1]);
                        }
                    }
                }
            }
        }
    }

    void DumpNormals(cgltf_data* data) {
        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            for (cgltf_size j = 0; j < data->meshes[i].primitives_count; ++j) {
                cgltf_primitive* primitive = &data->meshes[i].primitives[j];
                for (cgltf_size k = 0; k < primitive->attributes_count; ++k) {
                    cgltf_attribute* attr = &primitive->attributes[k];
                    if (attr->type == cgltf_attribute_type_normal) {
                        cgltf_accessor* accessor = attr->data;
                        for (cgltf_size l = 0; l < accessor->count; ++l) {
                            float normal[3];
                            cgltf_accessor_read_float(accessor, l, normal, 3);
                            output("Normal {}: {}, {}, {}\n", l, normal[0], normal[1], normal[2]);
                        }
                    }
                }
            }
        }
    }

    void DumpColors(cgltf_data* data) {
        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            for (cgltf_size j = 0; j < data->meshes[i].primitives_count; ++j) {
                cgltf_primitive* primitive = &data->meshes[i].primitives[j];
                for (cgltf_size k = 0; k < primitive->attributes_count; ++k) {
                    cgltf_attribute* attr = &primitive->attributes[k];
                    if (attr->type == cgltf_attribute_type_color) {
                        cgltf_accessor* accessor = attr->data;
                        for (cgltf_size l = 0; l < accessor->count; ++l) {
                            float color[4];
                            cgltf_accessor_read_float(accessor, l, color, 4);
                            output("Color {}: {}, {}, {}, {}\n", l, color[0], color[1], color[2], color[3]);
                        }
                    }
                }
            }
        }
    }

    size_t FetchVertexCount(cgltf_data* data) {
        for (cgltf_size m = 0; m < data->meshes_count; ++m) {
            cgltf_mesh* mesh = &data->meshes[m];

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
                        // first only
                        return accessor->count;
                    }
                }
            }
        }
        return 0;
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

    size_t FaceCountFromPrimitve(cgltf_primitive* primitive) {
        if (primitive->type != cgltf_primitive_type_triangles) {
            check(0,"Non-triangle primitive");
            return 0;
        }
        if (primitive->indices) {
            return primitive->indices->count;
        } else {
            // If the model does not use indices, you would need to read the vertex positions directly
            // This part of the code will depend on how vertex positions are stored in your model
            check(0,"Mesh does not use indices. Faces are defined by every three vertices.\n");
            return 0;
        }
    }
    
    void DumpFaces(cgltf_data* data) {
        for (cgltf_size i = 0; i < data->nodes_count; ++i) {
            cgltf_node* node = &data->nodes[i];
            if (node->mesh) {
                for (cgltf_size j = 0; j < node->mesh->primitives_count; ++j) {
                    list_faces(&node->mesh->primitives[j]);
                }
            }
        }
    }

    void DumpFaceCount(cgltf_data* data) {
        for (cgltf_size i = 0; i < data->nodes_count; ++i) {
            cgltf_node* node = &data->nodes[i];
            if (node->mesh) {
                for (cgltf_size j = 0; j < node->mesh->primitives_count; ++j) {
                    size_t count = FaceCountFromPrimitve(&node->mesh->primitives[j]);
                    output("Face Count ({}): {}\n", j, count);
                }
            }
        }
    }

#if 0
    void DumpVertices() {
        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            cgltf_mesh* mesh = &data->meshes[i];
            for (cgltf_size j = 0; j < mesh->primitives_count; ++j) {
                cgltf_primitive* primitive = &mesh->primitives[j];
                for (cgltf_size k = 0; k < primitive->attributes_count; ++k) {
                    cgltf_attribute* attr = &primitive->attributes[k];
                    if (attr->type == cgltf_attribute_type_position) {
                        cgltf_accessor* accessor = attr->data;
                        cgltf_buffer_view* buffer_view = accessor->buffer_view;
                        cgltf_buffer* buffer = buffer_view->buffer;
                        float* positions = (float*)(buffer->data + buffer_view->offset + accessor->offset);

                        for (cgltf_size v = 0; v < accessor->count; ++v) {
                            float x = positions[v * 3 + 0];
                            float y = positions[v * 3 + 1];
                            float z = positions[v * 3 + 2];
                            // Process vertex position (x, y, z)
                        }
                    }
                }
            }
        }
    }
#endif

#if 0
    std::vector<GLfloat> buffer();

    void ExposeData() {
        for (cgltf_size i = 0; i < data->nodes_count; ++i) {
            cgltf_node* node = &data->nodes[i];
            if (node->mesh) {
                size_t numVertices = node->mesh->
                size_t numIndices = node->mesh->primitives_count;

                buffer.resize();
                for (cgltf_size j = 0; j < node->mesh->primitives_count; ++j) {
                    list_faces(&node->mesh->primitives[j]);
                }
            }
            // one only for now
            break;
        }
        buffer.push_back();
    }
#endif

    std::vector<float> loadBuffer;
    std::vector<uint16_t> indexBuffer;

    static void Init() {
        
        const char* filename = "assets/models/utah_teapot_pbr/scene.gltf";

        cgltf_options options = {};
        cgltf_result result = cgltf_parse_file(&options, filename, &gData);
        if (result == cgltf_result_success)
        {
                result = cgltf_load_buffers(&options, gData, filename);
                if (result != cgltf_result_success) {
                    check(0, "");
                }

            cgltf_result valid = cgltf_validate(gData);
            if (result == cgltf_result_success) {
                if (result == 0) {
#if 0
                    DumpVertices();
                    DumpFaceCount();
                    DumpVertexCount();
                    DumpUVs();
                    DumpNormals();
                    DumpColors();
#endif
                    size_t vertCount = FetchVertexCount(gData);
                    size_t indexCount = FetchIndexCount(gData);
                    loadBuffer.resize(vertCount*12);
                    indexBuffer.resize(indexCount);
                    FetchVertices(gData, 0, 12, loadBuffer.data());
                    FetchUVs(gData, 3, 12, loadBuffer.data());
                    FetchColors(gData, 5, 12, loadBuffer.data());
                    FetchNormals(gData, 9, 12, loadBuffer.data());
                    FetchIndices(gData, indexBuffer.data());
                } else {
                    check(0, "Could not load file\n");
                }
            } else {
                check(0,"Failed to write GLB file.\n");
            }
            
            cgltf_free(gData);
        }

    }



    static size_t init_dummy = nugget::system::RegisterModule([]() {
        Init();
        return 0;
        }, 150);

}
