

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

auto Render_system::collect_text(const std::vector<defs::types::text::Text>& objects) -> void {
    for (const auto& obj : objects) {
        if (not obj.visible || not obj.draw_data)
            continue;

        const Render_text_command cmd{
            .draw_data = obj.draw_data,
            .model_matrix = obj.model_matrix,
            .depth = obj.position.y,    // can sort however
        };

        render_queue.text_commands.push_back(cmd);
    }
}
