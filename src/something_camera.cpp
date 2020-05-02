struct Camera
{
    Vec2f pos;
    int window_width;
    int window_height;

    Vec2f to_screen(Vec2f world_pos)
    {
        return world_pos - (pos - vec2((float) window_width * 0.5f, (float) window_height * 0.5f));
    }

    Rectf to_screen(Rectf world_rect)
    {
        return world_rect - (pos - vec2((float) window_width * 0.5f, (float) window_height * 0.5f));
    }

    Vec2f to_world(Vec2f screen_pos)
    {
        return screen_pos + (pos - vec2((float) window_width * 0.5f, (float) window_height * 0.5f));
    }

    Rectf to_world(Rectf screen_rect)
    {
        return screen_rect + (pos - vec2((float) window_width * 0.5f, (float) window_height * 0.5f));
    }
};
