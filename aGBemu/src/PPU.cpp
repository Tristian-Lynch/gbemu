#include "PPU.h"
#include <cstring>

PPU::PPU() {
    Reset();
}

void PPU::Reset() {
    std::memset(framebuffer, 0xFF, sizeof(framebuffer));
    std::memset(vram, 0, sizeof(vram));
    std::memset(oam, 0, sizeof(oam));

    lcdc = 0x91;
    scx = 0;
    scy = 0;

    palette[0][0] = palette[0][1] = palette[0][2] = 255;
    palette[1][0] = palette[1][1] = palette[1][2] = 192;
    palette[2][0] = palette[2][1] = palette[2][2] = 96;
    palette[3][0] = palette[3][1] = palette[3][2] = 0;

    for (int p = 0; p < 2; p++) {
        spritePalette[p][0][0] = spritePalette[p][0][1] = spritePalette[p][0][2] = 255;
        spritePalette[p][1][0] = spritePalette[p][1][1] = spritePalette[p][1][2] = 192;
        spritePalette[p][2][0] = spritePalette[p][2][1] = spritePalette[p][2][2] = 96;
        spritePalette[p][3][0] = spritePalette[p][3][1] = spritePalette[p][3][2] = 0;
    }

    for (int t = 0; t < 384; t++) {
        for (int y = 0; y < 8; y++) {
            uint8_t row1 = (y % 2) ? 0xAA : 0x55;
            uint8_t row2 = row1;
            vram[t * 16 + y * 2] = row1;
            vram[t * 16 + y * 2 + 1] = row2;
        }
    }

    for (int i = 0; i < 1024; i++) {
        vram[0x1800 + i] = i % 384;
    }

    oam[0] = 50;  oam[1] = 50;  oam[2] = 1;  oam[3] = 0;
}

inline uint8_t PPU::GetTilePixel(const uint8_t* tileData, int x, int y) {
    return ((tileData[y * 2] >> (7 - x)) & 1) | (((tileData[y * 2 + 1] >> (7 - x)) & 1) << 1);
}

void PPU::RenderFrame() {
    RenderBackground();
    RenderSprites();
}

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

            uint8_t colorIndex = GetTilePixel(tileData, pixelCol, pixelRow);
            uint8_t* rgb = &framebuffer[(y * 160 + x) * 3];

            rgb[0] = palette[colorIndex][0];
            rgb[1] = palette[colorIndex][1];
            rgb[2] = palette[colorIndex][2];
        }
    }
}

void PPU::RenderSprites() {
    for (int i = 0; i < 40; i++) {
        RenderSprite(i);
    }
}

void PPU::RenderSprite(int index) {
    int offset = index * 4;
    int y = oam[offset + 0] - 16;
    int x = oam[offset + 1] - 8;
    uint8_t tileNum = oam[offset + 2];
    uint8_t flags = oam[offset + 3];

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

            uint8_t colorIndex = GetTilePixel(tileData, col, row);
            if (colorIndex == 0) continue;

            uint8_t* rgb = &framebuffer[(py * 160 + px) * 3];
            rgb[0] = spritePalette[paletteNum][colorIndex][0];
            rgb[1] = spritePalette[paletteNum][colorIndex][1];
            rgb[2] = spritePalette[paletteNum][colorIndex][2];
        }
    }
}
