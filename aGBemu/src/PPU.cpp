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
    bgpReg = 0xE4;
    obp0Reg = 0xE4;
    obp1Reg = 0xE4;

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
    // LCDC bits:
    // 0: BG enable, 1: OBJ enable, 2: OBJ size, 3: BG tile map (0=9800,1=9C00)
    // 4: BG tile data (0=8800 signed,1=8000 unsigned)
    // 5: Window enable, 6: Window tile map, 7: LCD enable
    if (!(lcdc & 0x80))
        return; // LCD off: keep previous frame

    if (lcdc & 0x01) {
        RenderBackground();
    }

    // Window: simple overlay after BG
    if ((lcdc & 0x20) && (lcdc & 0x01)) {
        // Render window as a second BG using window tilemap and WX/WY
        const bool tileDataUnsigned = (lcdc & 0x10) != 0;
        const uint16_t tileDataBase = tileDataUnsigned ? 0x0000 : 0x0800; // 0x8000 vs 0x8800
        const bool windowMapHigh = (lcdc & 0x40) != 0; // 0x9C00
        const uint16_t windowMapBase = windowMapHigh ? 0x1C00 : 0x1800; // in our VRAM array

        for (int yPix = 0; yPix < 144; yPix++) {
            int wyLine = yPix - wy;
            if (wyLine < 0) continue;
            for (int xPix = 0; xPix < 160; xPix++) {
                int wxCol = xPix - (wx - 7);
                if (wxCol < 0) continue;

                int tileRow = wyLine / 8;
                int tileCol = wxCol / 8;
                int pixelRow = wyLine % 8;
                int pixelCol = wxCol % 8;

                uint8_t tileIndex = vram[windowMapBase + (tileRow % 32) * 32 + (tileCol % 32)];
                const uint8_t* tileData;
                if (tileDataUnsigned) {
                    tileData = &vram[tileDataBase + tileIndex * 16];
                } else {
                    int8_t signedIndex = static_cast<int8_t>(tileIndex);
                    tileData = &vram[tileDataBase + (signedIndex + 128) * 16];
                }

                uint8_t colorIndex = GetTilePixel(tileData, pixelCol, pixelRow);
                uint8_t shade = MapShade(bgpReg, colorIndex);
                uint8_t* rgb = &framebuffer[(yPix * 160 + xPix) * 3];
                rgb[0] = palette[shade][0];
                rgb[1] = palette[shade][1];
                rgb[2] = palette[shade][2];
            }
        }
    }

    if (lcdc & 0x02) {
        RenderSprites();
    }
}

void PPU::RenderBackground() {
    const bool tileMapHigh = (lcdc & 0x08) != 0; // BG tile map
    const uint16_t tileMapBase = tileMapHigh ? 0x1C00 : 0x1800;
    const bool tileDataUnsigned = (lcdc & 0x10) != 0;
    const uint16_t tileDataBase = tileDataUnsigned ? 0x0000 : 0x0800;

    for (int yPix = 0; yPix < 144; yPix++) {
        int tileRow = ((yPix + scy) & 0xFF) / 8;
        int pixelRow = (yPix + scy) & 0x07;

        for (int xPix = 0; xPix < 160; xPix++) {
            int tileCol = ((xPix + scx) & 0xFF) / 8;
            int pixelCol = (xPix + scx) & 0x07;

            uint8_t tileIndex = vram[tileMapBase + (tileRow % 32) * 32 + (tileCol % 32)];
            const uint8_t* tileData;
            if (tileDataUnsigned) {
                tileData = &vram[tileDataBase + tileIndex * 16];
            } else {
                int8_t signedIndex = static_cast<int8_t>(tileIndex);
                tileData = &vram[tileDataBase + (signedIndex + 128) * 16];
            }

            uint8_t colorIndex = GetTilePixel(tileData, pixelCol, pixelRow);
            uint8_t shade = MapShade(bgpReg, colorIndex);
            uint8_t* rgb = &framebuffer[(yPix * 160 + xPix) * 3];

            rgb[0] = palette[shade][0];
            rgb[1] = palette[shade][1];
            rgb[2] = palette[shade][2];
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

            uint8_t reg = paletteNum ? obp1Reg : obp0Reg;
            uint8_t shade = MapShade(reg, colorIndex);
            uint8_t* rgb = &framebuffer[(py * 160 + px) * 3];
            rgb[0] = spritePalette[paletteNum][shade][0];
            rgb[1] = spritePalette[paletteNum][shade][1];
            rgb[2] = spritePalette[paletteNum][shade][2];
        }
    }
}

// --- NEW HELPER FUNCTIONS FOR MMU ACCESS ---
uint8_t PPU::ReadVRAM(uint16_t addr) { return vram[addr]; }
void PPU::WriteVRAM(uint16_t addr, uint8_t value) { vram[addr] = value; }

uint8_t PPU::ReadOAM(uint16_t addr) { return oam[addr]; }
void PPU::WriteOAM(uint16_t addr, uint8_t value) { oam[addr] = value; }

uint8_t* PPU::GetFramebuffer() { return framebuffer; }

// Decode palette registers (DMG): bits pair per shade
static inline void DecodeDMGPalette(uint8_t reg, uint8_t out[4][3])
{
    // Map DMG shade indices 0..3 to RGB rows from light to dark
    uint8_t order[4];
    order[0] = (reg & 0x03);
    order[1] = (reg >> 2) & 0x03;
    order[2] = (reg >> 4) & 0x03;
    order[3] = (reg >> 6) & 0x03;
    // We keep palette rows fixed as 0..3 light->dark; reg selects which shade index maps to which color
    // Here we just reorder by writing the 4 shades into out according to mapping
    // For simplicity we assume base palette already set in Reset().
    // No-op here; we interpret reg by indexing when drawing instead (simpler).
}

void PPU::SetBGP(uint8_t value) { bgpReg = value; }

void PPU::SetOBP0(uint8_t value) { obp0Reg = value; }

void PPU::SetOBP1(uint8_t value) { obp1Reg = value; }
