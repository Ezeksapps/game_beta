/* UI related function implementations and core Nuklear implementation */

#include "ui.h"

#include "stylesheet_parser.hpp"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_IMPLEMENTATION
#include "nuklear.h"

struct media { // UI Skin
    int id;
    struct nk_image menu;
    struct nk_image check;
    struct nk_image check_cursor;
    struct nk_image option;
    struct nk_image option_cursor;
    struct nk_image header;
    struct nk_image window;
    struct nk_image scrollbar_inc_button;
    struct nk_image scrollbar_inc_button_hover;
    struct nk_image scrollbar_dec_button;
    struct nk_image scrollbar_dec_button_hover;
    struct nk_image button;
    struct nk_image button_hover;
    struct nk_image button_active;
    struct nk_image slider;
    struct nk_image slider_hover;
    struct nk_image slider_active;
};

struct nk_context ctx;

int numFontTextures;
struct nk_font* font;

struct nk_buffer cmds;
struct nk_draw_null_texture texNull;

struct media media;

struct nk_font_atlas atlas;
struct FontAtlasData fontAtlasData;

struct nk_color setColor(const int* data, int* currentIndex) { // convenience function
    // check why array indices not valid? (Needs to be const expr apparently)
    struct nk_color color =  nk_color {
        .r = data[*currentIndex],
        .g = data[*currentIndex + 1],
        .b = data[*currentIndex + 2],
        .a = data[*currentIndex + 3]
    };
    currentIndex += 4;
    return color;
}

void initUi() {

    {
        // load font(s) and initialise font atlas image data

        const char* font_path = "assets/ui/font.ttf"; // PMD font, TODO: make sure to fill in missing glyphs (could probably just dump off EUR rom)
        nk_font_atlas_init_default(&atlas);
        nk_font_atlas_begin(&atlas);
        font = nk_font_atlas_add_from_file(&atlas, font_path, 13.0f, NULL);

        int w, h;
        const void* image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
        fontAtlasData = FontAtlasData {
            .image = image,
            .w = w, .h = h
        };

        // do not end atlas yet, nk_font_atlas_end will be called via endFontAtlas() in UI renderer
        // NOTE: That function MUST be called for this to work properly
    }

    nk_init_default(&ctx, &font->handle); // init context with font
    {

        const int* stylesheetData = loadStylesheet();
        // TODO: Implement proper skin in addition to custom colour theme
        // media.id = //image_load("assets/ui/skin.png"); << SKIN WIP
        //  media.check = nk_subimage_id(media.id, 512,512, nk_rect(464,32,15,15));

        // missing property, chart, combo button, contextual button, progress bar, slider, selectable and tree (Not currently needed in UI)

        // TODO: Create some mechanism that prevents the need to hard-code indices (convenience func)

        /* default text colour */
        ctx.style.text.color = nk_rgba();

        /* window */
        ctx.style.window.background = nk_rgba(stylesheetData[0], stylesheetData[1], stylesheetData[2], stylesheetData[3]);
        //ctx.style.window.fixed_background = nk_style_item_image(media.window);
        ctx.style.window.border_color = nk_rgb(67,67,67);
        ctx.style.window.border_color = nk_rgba(stylesheetData[4], stylesheetData[5], stylesheetData[6], stylesheetData[7]);
        //ctx.style.window.combo_border_color = nk_rgb(67,67,67);
        ctx.style.window.background = nk_rgba(stylesheetData[0], stylesheetData[1], stylesheetData[2], stylesheetData[3]);
        ctx.style.window.background = nk_rgba(stylesheetData[0], stylesheetData[1], stylesheetData[2], stylesheetData[3]);

        ctx.style.window.contextual_border_color = nk_rgb(67,67,67);
        ctx.style.window.menu_border_color = nk_rgb(67,67,67);
        ctx.style.window.group_border_color = nk_rgb(67,67,67);
        ctx.style.window.tooltip_border_color = nk_rgb(67,67,67);
        ctx.style.window.scrollbar_size = nk_vec2(16,16);
        ctx.style.window.border_color = nk_rgba(0,0,0,0);
        ctx.style.window.padding = nk_vec2(8,4);
        ctx.style.window.border = 3;

        /* window header */
        ctx.style.window.header.normal = nk_style_item_image(media.header);
        ctx.style.window.header.hover = nk_style_item_image(media.header);
        ctx.style.window.header.active = nk_style_item_image(media.header);
        ctx.style.window.header.label_normal = nk_rgb(95,95,95);
        ctx.style.window.header.label_hover = nk_rgb(95,95,95);
        ctx.style.window.header.label_active = nk_rgb(95,95,95);

        /* checkbox toggle */
        {
            struct nk_style_toggle *toggle;
            toggle = &ctx.style.checkbox;
            toggle->normal          = nk_style_item_image(media.check);
            toggle->hover           = nk_style_item_image(media.check);
            toggle->active          = nk_style_item_image(media.check);
            toggle->cursor_normal   = nk_style_item_image(media.check_cursor);
            toggle->cursor_hover    = nk_style_item_image(media.check_cursor);
            toggle->text_normal     = nk_rgb(95,95,95);
            toggle->text_hover      = nk_rgb(95,95,95);
            toggle->text_active     = nk_rgb(95,95,95);

        }

        /* option toggle */
        {
            struct nk_style_toggle *toggle;
            toggle = &ctx.style.option;
            toggle->normal          = nk_style_item_image(media.option);
            toggle->hover           = nk_style_item_image(media.option);
            toggle->active          = nk_style_item_image(media.option);
            toggle->cursor_normal   = nk_style_item_image(media.option_cursor);
            toggle->cursor_hover    = nk_style_item_image(media.option_cursor);
            toggle->text_normal     = nk_rgb(95,95,95);
            toggle->text_hover      = nk_rgb(95,95,95);
            toggle->text_active     = nk_rgb(95,95,95);

        }

        /* default button */
        ctx.style.button.normal = nk_style_item_image(media.button);
        ctx.style.button.hover = nk_style_item_image(media.button_hover);
        ctx.style.button.active = nk_style_item_image(media.button_active);
        ctx.style.button.border_color = nk_rgba(0,0,0,0);
        ctx.style.button.text_background = nk_rgba(0,0,0,0);
        ctx.style.button.text_normal = nk_rgb(95,95,95);
        ctx.style.button.text_hover = nk_rgb(95,95,95);
        ctx.style.button.text_active = nk_rgb(95,95,95);



        /* menu button */
        ctx.style.menu_button.normal = nk_style_item_color(nk_rgb(206,206,206));
        ctx.style.menu_button.hover = nk_style_item_color(nk_rgb(229,229,229));
        ctx.style.menu_button.active = nk_style_item_color(nk_rgb(99,202,255));
        ctx.style.menu_button.border_color = nk_rgba(0,0,0,0);
        ctx.style.menu_button.text_background = nk_rgba(0,0,0,0);
        ctx.style.menu_button.text_normal = nk_rgb(95,95,95);
        ctx.style.menu_button.text_hover = nk_rgb(95,95,95);
        ctx.style.menu_button.text_active = nk_rgb(95,95,95);

        /* edit */
        ctx.style.edit.normal = nk_style_item_color(nk_rgb(240,240,240));
        ctx.style.edit.hover = nk_style_item_color(nk_rgb(240,240,240));
        ctx.style.edit.active = nk_style_item_color(nk_rgb(240,240,240));
        ctx.style.edit.border_color = nk_rgb(62,62,62);
        ctx.style.edit.cursor_normal = nk_rgb(99,202,255);
        ctx.style.edit.cursor_hover = nk_rgb(99,202,255);
        ctx.style.edit.cursor_text_normal = nk_rgb(95,95,95);
        ctx.style.edit.cursor_text_hover = nk_rgb(95,95,95);
        ctx.style.edit.text_normal = nk_rgb(95,95,95);
        ctx.style.edit.text_hover = nk_rgb(95,95,95);
        ctx.style.edit.text_active = nk_rgb(95,95,95);
        ctx.style.edit.selected_normal = nk_rgb(99,202,255);
        ctx.style.edit.selected_hover = nk_rgb(99,202,255);
        ctx.style.edit.selected_text_normal = nk_rgb(95,95,95);
        ctx.style.edit.selected_text_hover = nk_rgb(95,95,95);
        ctx.style.edit.border = 1;
        ctx.style.edit.rounding = 0; // round corners are a disease

    }
}

void cleanupUi() {
    nk_font_atlas_clear(&atlas);
    nk_buffer_free(&cmds);
    nk_free(&ctx);
}

FontAtlasData* getFontAtlasData() { return &fontAtlasData; }

void endFontAtlas(void* texView) {
    nk_font_atlas_end(&atlas, nk_handle_ptr(texView), &texNull);
   // if (nk_dlg_ctx->atlas.default_font) nk_style_set_font(&nk_dlg_ctx->ctx, &nk_dlg_ctx->atlas.default_font->handle);
}

// nk_begin creates window w/ no header (title is only for in-code ID), nk_begin_titles created window with header

// adapted from DiligentSamples Nuklear Demo (NkDiligent.cpp)
int convertVertices(void* vertexBufferMem, void* indexBufferMem) {

    struct nk_convert_config config;

    NK_STORAGE const struct nk_draw_vertex_layout_element vertexLayout[] =
    {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT,    NK_OFFSETOF(struct UiVertex, pos)},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT,    NK_OFFSETOF(struct UiVertex, uv)},
        {NK_VERTEX_COLOR,    NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct UiVertex, col)},
        {NK_VERTEX_LAYOUT_END}
    };

    memset(&config, 0, sizeof(config));
    config.vertex_layout        = vertexLayout;
    config.vertex_size          = sizeof(struct UiVertex);
    config.vertex_alignment     = NK_ALIGNOF(struct UiVertex);
    config.global_alpha         = 1.0f;
    config.shape_AA             = NK_ANTI_ALIASING_ON;
    config.line_AA              = NK_ANTI_ALIASING_ON;
    config.circle_segment_count = 11;
    config.curve_segment_count  = 11;
    config.arc_segment_count    = 11;
    config.tex_null             = texNull;

    // setup buffers to load vertices and elements
    struct nk_buffer vertexBuffer, indexBuffer;
    nk_buffer_init_fixed(&vertexBuffer, vertexBufferMem, (size_t)512 * 1024);
    nk_buffer_init_fixed(&indexBuffer, indexBufferMem, (size_t)128 * 1024);
    return nk_convert(&ctx, &cmds, &vertexBuffer, &indexBuffer, &config);
}

// adapted from DiligentSamples Nuklear Demo (NkDiligent.cpp)
void drawCmdsForEach(void* _this, void (*execDrawCmd)(void* _this, struct nk_rect clipRect, void* texPtr, unsigned int elemCount)) {

    const struct nk_draw_command* cmd = nullptr;

    nk_draw_foreach(cmd, &ctx, &cmds) {
        if (!cmd->elem_count) continue;
        execDrawCmd(_this, cmd->clip_rect, cmd->texture.ptr, cmd->elem_count);
    }
    nk_clear(&ctx);
}

/* TODO: IMPLEMENT ALL NECESSARY UI MENUS/VIEWS */
/* TODO: Create screen 'stack' for screen history, so pressing ESC (equiv B) will properly return to previous screen */


void bagView() {}
void profileView() {}
void optionsMenu() {}
void multiplayerView() {}
void saveMenu() {}
void mapView() {}


// in-game pause menu
void pauseMenu() {
    if (nk_begin(&ctx, "Pause Menu", nk_rect(0, 0, 100, 240),
        NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {

        // Nuklear only has rows as a UI container, each row has a specified number of colums
        // a column container with specified rows doesn't exist

        /* Layout of pause menu
         * -=-=-=-=-=-=-=-=-=-=-=-
         *
         * Pokemon
         * Bag
         * Profile
         * Options
         * Multiplayer
         * Save
         * Map
         */

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Pokemon")) {}

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Bag")) {/*handle event*/}

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Profile")) {/*handle event*/}

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Options")) {/*handle event*/}

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Multiplyr")) {/*handle event*/}

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Save")) {/*handle event*/}

        nk_layout_row_static(&ctx, 20, 100, 1);
        if (nk_button_label(&ctx, "Map")) {/*handle event*/}

        // Press ESC to exit

    }
}

