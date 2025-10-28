#pragma once
#include <cstdint>

class PPU {
public:
    PPU();
    void Reset();

    // Render the full screen to the framebuffer (background + sprites)
    void RenderFrame();

    // Return framebuffer for renderer
    uint8_t* GetFramebuffer();

    // --- NEW: VRAM / OAM access for MMU ---
    uint8_t ReadVRAM(uint16_t addr);
    void WriteVRAM(uint16_t addr, uint8_t value);

    uint8_t ReadOAM(uint16_t addr);
    void WriteOAM(uint16_t addr, uint8_t value);

private:
    // GameBoy framebuffer: 160x144 RGB
    uint8_t framebuffer[160 * 144 * 3];

    // VRAM (tiles + tile maps)
    uint8_t vram[0x2000]; // 8 KB

    // OAM (sprites)
    uint8_t oam[0xA0]; // 40 sprites × 4 bytes

    // LCDC registers
    uint8_t lcdc; // LCD control
    uint8_t scx;  // Scroll X
    uint8_t scy;  // Scroll Y

    // Palette (4 shades for background)
    uint8_t palette[4][3]; // RGB for each color

    // Sprite palettes (OBP0 and OBP1)
    uint8_t spritePalette[2][4][3];

    // Helpers
    void RenderBackground();
    void RenderTileLine(int tileX, int tileY, int line);
    void RenderSprites();
    void RenderSprite(int index);

    // Internal helper, not exposed publicly
    inline uint8_t GetTilePixel(const uint8_t* tileData, int x, int y);
};
