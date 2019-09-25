#include "BackgroundLayer.h"

TFT_bgLayer::TFT_bgLayer(TFT_eSPI *tft)
{
    _tft = tft;
    
    _iwidth  = 0;
    _iheight = 0;
    _bpp     = 16;
    _iswapBytes = false;
    
    _created = false;
    
}

void* TFT_bgLayer::createLayer(int16_t w, int16_t h)
{
    if ( _created ) return _img8_1;
    if (w < 1 || h < 1) return NULL;
    
    _iwidth  = w; // = _dwidth = _bitwidth = w;
    _iheight = h; // = _dheight = h;
    
    _img8   = (uint8_t*)callocLayer(w, h);
    _img    = (uint16_t*)_img8;
    
    if (_bpp == 1)
    {
        w = (w+7) & 0xFFF8;
        _img8_2 = _img8 + ( (w>>3) * h + 1 );
    }
    
    if (_img8)
    {
        _created = true;
        return _img8;
    }
    return NULL;
}

void* TFT_bgLayer::callocLayer(int16_t w, int16_t h)
{
    uint8_t* ptr8 = NULL;
    
    if (_bpp == 16)
    {
#if defined (ESP32) && defined (CONFIG_SPIRAM_SUPPORT)
        if ( psramFound() ) ptr8 = (uint8_t*)ps_calloc(w * h + 1, sizeof(uint16_t));
        else
#endif
        ptr8 = (uint8_t*)calloc(w * h + 1, sizeof(uint16_t));
    }
    
    else if (_bpp == 8)
    {
#if defined (ESP32) && defined (CONFIG_SPIRAM_SUPPORT)
        if (paramFound()) ptr8 = (uint8_t*)ps_calloc(w * h + 1, sizeof(uint8_t));
        else
#endif
        ptr8 = (uint8_t*)calloc(w * h + 1, sizeof(uint8_t));
    }

    else // Must be 1 bpp
    {
        w = ( w + 7 ) & 0xFFF8;
        _iwidth = w;
//        _bitwidth = w;
        
#if defined (ESP32) && defined (CONFIG_SPIRAM_SUPPORT)
        if (psramFound()) ptr8 = (uint8_t*)ps_calloc(framed * (w >> 3) * h, sizeof(uint8_t));
        else
#endif
        ptr8 = (uint8_t*)calloc((w >> 3) * h, sizeof(uint8_t));
    }
    
    return ptr8;
}

void* TFT_bgLayer::setColorDepth(int8_t b)
{
    // Can't change an existing sprite's color depth so delete it
    if (_created) free(_img8_1);
    
    // Now define the new color depth
    if (b > 8) _bpp = 16;
    else if (b > 1) _bpp = 8;
    else _bpp = 1;
    
    // If it existed, re-create the sprite with the new color depth
    if (_created)
    {
        _created = false;
        return createLayer(_iwidth, _iheight);
    }
    return NULL;
}

int8_t TFT_bgLayer::getColorDepth(void)
{
    if (_created) return _bpp;
    else return 0;
}

void TFT_bgLayer::deleteLayer(void)
{
    if (!_created) return;
    free(_img8_1);
    _created = false;
}

void TFT_bgLayer::drawPixel(int32_t x, int32_t y)
{
    if ((x < 0) || (y < 0) || (x >= _iwidth) || (y >= _iheight)) return;
    
    uint16_t color = readPixel(x, y);
    
    drawPixel(x, y, color);
}

void TFT_bgLayer::drawPixel(int32_t x, int32_t y, uint32_t color)
{
    if ((x < 0) || (y < 0) || (x >= _iwidth) || (y >= _iheight)) return;
    _tft-> drawPixel(x, y, color);
}

uint16_t TFT_bgLayer::readPixel(int32_t x, int32_t y)
{
  if ((x < 0) || (x >= _iwidth) || (y < 0) || (y >= _iheight) || !_created) return 0;

  if (_bpp == 16)
  {
    uint16_t color = _img[x + y * _iwidth];
    return (color >> 8) | (color << 8);
  }
  
  if (_bpp == 8)
  {
    uint16_t color = _img8[x + y * _iwidth];
    if (color != 0)
    {
    uint8_t  blue[] = {0, 11, 21, 31};
      color =   (color & 0xE0)<<8 | (color & 0xC0)<<5
              | (color & 0x1C)<<6 | (color & 0x1C)<<3
              | blue[color & 0x03];
    }
    return color;
  }

}

void TFT_bgLayer::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    if (!_created) return;
    
    boolean steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        swap_coord(x0, y0);
        swap_coord(x1, y1);
    }
    
    if (x0 > x1) {
        swap_coord(x0, x1);
        swap_coord(y0, y1);
    }
    
    int32_t dx = x1 - x0, dy = abs(y1 - y0);
    
    int32_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;
    
    if (y0 < y1) ystep = 1;
    
    // Split into steep and not steep for FastH/V separation
    if (steep) {
        for(; x0 <= x1; x0++) {
            dlen++;
            err -= dy;
            if (err < 0) {
                err += dx;
                if (dlen == 1) drawPixel(y0, xs);
                else drawFastVLine(y0, xs, dlen);
                dlen = 0; y0 += ystep; xs = x0 + 1;
            }
        }
        if (dlen) drawFastVLine(y0, xs, dlen);
    }
    else
    {
        for (; x0 <= x1; x0++) {
            dlen++;
            err -= dy;
            if (err < 0) {
                err += dx;
                if (dlen == 1) drawPixel(xs, y0);
                else drawFastHLine(xs, y0, dlen);
                dlen = 0; y0 += ystep; xs = x0 + 1;
            }
        }
        if (dlen) drawFastHLine(xs, y0, dlen);
    }
}

void TFT_bgLayer::drawFastVLine(int32_t x, int32_t y, int32_t h)
{
    if ((x < 0) || (x >= _iwidth) || (y >= _iheight) || !_created) return;
    
    if (y < 0) { h += y; y = 0; }
    
    if ((y + h) > _iheight) h = _iheight - y;
    
    if (h < 1) return;
    
    while (h--)
    {
        drawPixel(x, y);
        y++;
    }
}

void TFT_bgLayer::drawFastHLine(int32_t x, int32_t y, int32_t w)
{
    if ((y < 0) || (x >= _iwidth) || (y >= _iheight) || !_created) return;
    
    if (x < 0) { w += x; x = 0; }
    
    if ((x + w) > _iwidth) w = _iwidth -x;
    
    if (w < 1) return;
    
    while (w--)
    {
        drawPixel(x, y);
        x++;
    }
}

void TFT_bgLayer::pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data)
{
    if ((x >= _iwidth) || (y >= _iheight) || (w == 0) || (h == 0) || !_created) return;
    if ((x + w < 0) || (y + h < 0)) return;
    
    int32_t xo = 0;
    int32_t yo = 0;
    
    int32_t xs = x;
    int32_t ys = y;
    
    uint32_t ws = w;
    uint32_t hs = h;
    
    if (x < 0) { xo = -x; xs = 0; }
    if (y < 0) { yo = -y; ys = 0; }
    
    if (xs + w >= _iwidth)  ws = _iwidth  - xs;
    if (ys + h >= _iheight) hs = _iheight - ys;
    
    if (_bpp == 16)
    {
        for (uint32_t yp = yo; yp < yo + hs; yp++)
        {
            x = xs;
            for (uint32_t xp = xo; xp < xo + ws; xp++)
            {
                uint16_t color = data[xp + yp * w];
                if (!_iswapBytes) color = color<<8 | color>>8;
                _img[x + ys * _iwidth] = color;
                drawPixel(xp, yp);
                x++;
            }
            ys++;
        }
    }
    else if (_bpp == 8)
    {
        for (uint32_t yp = yo; yp < yo + hs; yp++)
        {
            x = xs;
            for (uint32_t xp = xo; xp < xo + ws; xp++)
            {
                uint16_t color = data[xp + yp * w];
                if(_iswapBytes) color = color<<8 | color>>8;
                _img8[x + ys * _iwidth] = (uint8_t)((color & 0xE000)>>8 | (color & 0x0700)>>6 | (color & 0x0018)>>3);
                drawPixel(xp, yp);
                x++;
            }
            ys++;
        }
    }
}

void TFT_bgLayer::pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data)
{
  pushImage(x, y, w, h, (uint16_t*) data);
}

void TFT_bgLayer::pushLayer(int32_t x, int32_t y)
{
  if (!_created) return;

  if (_bpp == 16) _tft->pushImage(x, y, _iwidth, _iheight, _img);
  else _tft->pushImage(x, y, _iwidth, _iheight, _img8, (bool)(_bpp == 8));
}
