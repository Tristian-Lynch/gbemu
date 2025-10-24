#include "PPU.h"
#include <cstring>

// Helper macro to get tile pixel value (2 bits per pixel)
#define GET_TILE_PIXEL(tileData, x, y) (((tileData[y * 2] >> (7 - x)) & 1) | (((tileData[y * 2 + 1] >> (7 - x)) & 1) << 1))

PPU::PPU() {
    Reset();
}

void PPU::Reset() {
    std::memset(framebuffer, 0xFF, sizeof(framebuffer)); // white
    std::memset(vram, 0, sizeof(vram));
    std::memset(oam, 0, sizeof(oam));

    lcdc = 0x91; // BG on, 8x8 tiles, default map
    scx = 0;
    scy = 0;

    // Background palette (0=white, 3=black)
    palette[0][0] = palette[0][1] = palette[0][2] = 255;
    palette[1][0] = palette[1][1] = palette[1][2] = 192;
    palette[2][0] = palette[2][1] = palette[2][2] = 96;
    palette[3][0] = palette[3][1] = palette[3][2] = 0;

    // Sprite palettes OBP0 / OBP1
    for (int p = 0; p < 2; p++) {
        spritePalette[p][0][0] = spritePalette[p][0][1] = spritePalette[p][0][2] = 255;
        spritePalette[p][1][0] = spritePalette[p][1][1] = spritePalette[p][1][2] = 192;
        spritePalette[p][2][0] = spritePalette[p][2][1] = spritePalette[p][2][2] = 96;
        spritePalette[p][3][0] = spritePalette[p][3][1] = spritePalette[p][3][2] = 0;
    }

    // Fill VRAM with test tiles: checkerboard
    for (int t = 0; t < 384; t++) { // 384 tiles
        for (int y = 0; y < 8; y++) {
            uint8_t row1 = (y % 2) ? 0xAA : 0x55;
            uint8_t row2 = row1;
            vram[t * 16 + y * 2] = row1;
            vram[t * 16 + y * 2 + 1] = row2;
        }
    }

    // Fill tile map 0 (0x9800) sequentially
    for (int i = 0; i < 1024; i++) {
        vram[0x1800 + i] = i % 384;
    }

    // Optional: add test sprite
    oam[0] = 50;  // Y position
    oam[1] = 50;  // X position
    oam[2] = 1;   // Tile number
    oam[3] = 0;   // Flags (palette 0)
}

// Render the full frame
void PPU::RenderFrame() {
    RenderBackground();
    RenderSprites();
}

// Render background tiles
void PPU::RenderBackground() {
    const uint8_t* tileMap = &vram[0x1800];

    for (int y = 0; y < 144; y++) {
        int tileRow = (y + scy) / 8;
        int pixelRow = (y + scy) % 8;

        for (int x = 0; x < 160; x++) {
            int tileCol = (x + scx) / 8;
            int pixelCol = (x + scx) % 8;

            uint8_t tileNum = tileMap[(tileRow % 32) * 32 + (tileCol % 32)];
            const uint8_t* tileData = &vram[tileNum * 16];

            uint8_t colorIndex = GET_TILE_PIXEL(tileData, pixelCol, pixelRow);
            uint8_t* rgb = &framebuffer[(y * 160 + x) * 3];

            rgb[0] = palette[colorIndex][0];
            rgb[1] = palette[colorIndex][1];
            rgb[2] = palette[colorIndex][2];
        }
    }
}

// Render all sprites
void PPU::RenderSprites() {
    for (int i = 0; i < 40; i++) {
        RenderSprite(i);
    }
}

// Render one sprite
void PPU::RenderSprite(int index) {
    int offset = index * 4;
    int y = oam[offset + 0] - 16;
    int x = oam[offset + 1] - 8;
    uint8_t tileNum = oam[offset + 2];
    uint8_t flags = oam[offset + 3];

    bool priority = flags & 0x80;
    bool yFlip = flags & 0x40;
    bool xFlip = flags & 0x20;
    int paletteNum = (flags & 0x10) ? 1 : 0;

    const uint8_t* tileData = &vram[tileNum * 16];

    for (int row = 0; row < 8; row++) {
        int py = y + (yFlip ? 7 - row : row);
        if (py < 0 || py >= 144) continue;

        for (int col = 0; col < 8; col++) {
            int px = x + (xFlip ? 7 - col : col);
            if (px < 0 || px >= 160) continue;

            uint8_t colorIndex = GET_TILE_PIXEL(tileData, col, row);
            if (colorIndex == 0) continue; // Transparent

            uint8_t* rgb = &framebuffer[(py * 160 + px) * 3];

            // Priority handling simplified: always draw on top for now
            rgb[0] = spritePalette[paletteNum][colorIndex][0];
            rgb[1] = spritePalette[paletteNum][colorIndex][1];
            rgb[2] = spritePalette[paletteNum][colorIndex][2];
        }
    }
}
