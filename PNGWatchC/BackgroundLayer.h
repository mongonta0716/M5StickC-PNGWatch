#include "utility/In_eSPI.h"

class TFT_bgLayer : public TFT_eSPI {
    public:
        TFT_bgLayer(TFT_eSPI *tft);
        
        void*       createLayer(int16_t width, int16_t height);
        
        void        deleteLayer(void);
        
        void*       setColorDepth(int8_t b);
        int8_t      getColorDepth(void);
        
        void        pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h, uint16_t *data);
        void        pushImage(int32_t x0, int32_t y0, int32_t w, int32_t h, const uint16_t *data);
        
        void        drawPixel(int32_t x, int32_t y);
        void        drawPixel(int32_t x, int32_t y, uint32_t color);
        uint16_t    readPixel(int32_t x, int32_t y);
        
        void        drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
        void        drawFastVLine(int32_t x, int32_t y, int32_t h);
        void        drawFastHLine(int32_t x, int32_t y, int32_t w);
        
        void        pushLayer(int32_t x, int32_t y);
        
    private:
        TFT_eSPI *_tft;
        
        // Reserve memory for the Layer and return a pointer
        void*   callocLayer(int16_t width, int16_t height);
    
    protected:
        uint8_t     _bpp;       // bpp(1, 8 or 16)
        uint16_t    *_img;      // pointer to 16 bit sprite
        uint8_t     *_img8;     // pointer to  8 bit sprite
        uint8_t     *_img8_1;   // pointer to frame 1
        uint8_t     *_img8_2;   // pointer to frame 2
        
        bool        _created;

        boolean     _iswapBytes;
        
        int32_t     _iwidth, _iheight;  // Layer memory image bit width and height
};
