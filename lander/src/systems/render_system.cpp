

#include <render_system.h>

auto Render_system::collect_renderables(const std::vector<std::unique_ptr<Game_object>>& objects)
    -> void {
    for (const auto& obj : objects) {
        const C_transform* transform{obj->get_component<C_transform>()};
        // const C_renderable* renderable = obj->get_component<C_renderable>();
        const C_mesh* mesh{obj->get_component<C_mesh>()};
        const C_render* render{obj->get_component<C_render>()};

        if (transform && mesh && render && render->visible) {
            const Render_mesh_command cmd{
                .pipeline_id = render->pipeline_id,
                .mesh_id = mesh->mesh_id,
                .model_matrix = transform->get_matrix(),
                // .color = render->color,
                .depth = render->depth,
            };

            render_queue.opaque_commands.push_back(cmd);
        }
    }
}

