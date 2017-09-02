//
//  PixelIterator.h
//  RayTracerXcode
//
//  Created by Peter Zhang on 9/1/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//

#ifndef PixelIterator_h
#define PixelIterator_h

class PixelIterator
{
private:
    std::atomic_int ix{0};

public:
    PixelIterator() {}
    PixelIterator(const PixelIterator& p)
    {
        ix.store(p.ix.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
    }
    ~PixelIterator() {}
    
    bool GetPixelLocation(int& x, int& y)
    {
        int i = ix++;
        int imageHeight = renderImage.GetHeight();
        int imageWidth = renderImage.GetWidth();
        
        if (i > imageHeight*imageWidth) {
            return false;
        }
        
        x = i % imageWidth;
        y = i / imageWidth;
        return true;
    }
    
    bool IterationComplete() {
        int imageHeight = renderImage.GetHeight();
        int imageWidth = renderImage.GetWidth();
        if (ix > imageHeight*imageWidth) {
            return true;
        }
        else {
            return false;
        }
    }
};


#endif /* PixelIterator_h */
