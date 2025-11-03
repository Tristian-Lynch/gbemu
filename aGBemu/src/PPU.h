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

    // IO register accessors
    void SetLCDC(uint8_t value) { lcdc = value; }
    uint8_t GetLCDC() const { return lcdc; }
    void SetSCY(uint8_t value) { scy = value; }
    void SetSCX(uint8_t value) { scx = value; }
    void SetBGP(uint8_t value);
    void SetOBP0(uint8_t value);
    void SetOBP1(uint8_t value);
    void SetWY(uint8_t value) { wy = value; }
    void SetWX(uint8_t value) { wx = value; }

private:
    // GameBoy framebuffer: 160x144 RGB
    uint8_t framebuffer[160 * 144 * 3];

    // VRAM (tiles + tile maps)
    uint8_t vram[0x2000]; // 8 KB

    // OAM (sprites)
    uint8_t oam[0xA0]; // 40 sprites � 4 bytes

    // LCDC registers
    uint8_t lcdc; // LCD control
    uint8_t scx;  // Scroll X
    uint8_t scy;  // Scroll Y
    uint8_t wy = 0; // Window Y
    uint8_t wx = 0; // Window X (minus 7 when drawing)

    // DMG palette registers (BGP/OBP0/OBP1)
    uint8_t bgpReg = 0xE4;  // default: 11 10 01 00
    uint8_t obp0Reg = 0xE4;
    uint8_t obp1Reg = 0xE4;

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
    inline uint8_t MapShade(uint8_t paletteReg, uint8_t colorIndex) const { return (paletteReg >> (colorIndex * 2)) & 0x03; }
};
