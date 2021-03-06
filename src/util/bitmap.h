//
// Created by tri on 21/09/2017.
//

#ifndef BKMAP_BITMAP_H
#define BKMAP_BITMAP_H

#include <algorithm>
#include <cmath>
#include <ios>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif
#include <FreeImage.h>

//#include "util/string.h"
namespace bkmap{

    // Templated bitmap color class.
    template <typename T>
    struct BitmapColor {
        BitmapColor();
        BitmapColor(const T r, const T g, const T b);

        template <typename D>
        BitmapColor<D> Cast() const;

        bool operator==(const BitmapColor<T>& rhs) const;
        bool operator!=(const BitmapColor<T>& rhs) const;

        template <typename D>
        friend std::ostream& operator<<(std::ostream& output,
                                        const BitmapColor<D>& color);

        T r;
        T g;
        T b;
    };

    // Wrapper class around FreeImage bitmaps.
    class Bitmap {
    public:
        Bitmap();

        // Copy constructor.
        Bitmap(const Bitmap& other);
        // Move constructor.
        Bitmap(Bitmap&& other);

        // Create bitmap object from existing FreeImage bitmap object. Note that
        // this class takes ownership of the object.
        explicit Bitmap(FIBITMAP* data);

        // Copy assignment.
        Bitmap& operator=(const Bitmap& other);
        // Move assignment.
        Bitmap& operator=(Bitmap&& other);

        // Allocate bitmap by overwriting the existing data.
        bool Allocate(const int width, const int height, const bool as_rgb);

        // Deallocate the bitmap by releasing the existing data.
        void Deallocate();

        // Get pointer to underlying FreeImage object.
        inline const FIBITMAP* Data() const;
        inline FIBITMAP* Data();

        // Dimensions of bitmap.
        inline int Width() const;
        inline int Height() const;
        inline int Channels() const;

        // Number of bits per pixel. This is 8 for grey and 24 for RGB image.
        inline unsigned int BitsPerPixel() const;

        // Scan width of bitmap which differs from the actual image width to achieve
        // 32 bit aligned memory. Also known as pitch or stride.
        inline unsigned int ScanWidth() const;

        // Check whether image is grey- or colorscale.
        inline bool IsRGB() const;
        inline bool IsGrey() const;

        // Number of bytes required to store image.
        size_t NumBytes() const;

        // Copy raw image data to array.
        std::vector<uint8_t> ConvertToRawBits() const;
        std::vector<uint8_t> ConvertToRowMajorArray() const;
        std::vector<uint8_t> ConvertToColMajorArray() const;

        // Manipulate individual pixels. For grayscale images, only the red element
        // of the RGB color is used.
        bool GetPixel(const int x, const int y, BitmapColor<uint8_t>* color) const;
        bool SetPixel(const int x, const int y, const BitmapColor<uint8_t>& color);

        // Get pointer to y-th scanline, where the 0-th scanline is at the top.
        const uint8_t* GetScanline(const int y) const;

        // Fill entire bitmap with uniform color. For grayscale images, the first
        // element of the vector is used.
        void Fill(const BitmapColor<uint8_t>& color);

        // Interpolate color at given floating point position.
        bool InterpolateNearestNeighbor(const double x, const double y,
                                        BitmapColor<uint8_t>* color) const;
        bool InterpolateBilinear(const double x, const double y,
                                 BitmapColor<float>* color) const;

        // Extract EXIF information from bitmap. Returns false if no EXIF information
        // is embedded in the bitmap.
        bool ExifFocalLength(double* focal_length);
        bool ExifLatitude(double* latitude);
        bool ExifLongitude(double* longitude);
        bool ExifAltitude(double* altitude);

        // Read bitmap at given path and convert to grey- or colorscale.
        bool Read(const std::string& path, const bool as_rgb = true);

        // Write image to file. Flags can be used to set e.g. the JPEG quality.
        // Consult the FreeImage documentation for all available flags.
        bool Write(const std::string& path,
                   const FREE_IMAGE_FORMAT format = FIF_UNKNOWN,
                   const int flags = 0) const;

        // Smooth the image using a Gaussian kernel.
        void Smooth(const float sigma_x, const float sigma_y);

        // Rescale image to the new dimensions.
        void Rescale(const int new_width, const int new_height,
                     const FREE_IMAGE_FILTER filter = FILTER_BILINEAR);

        // Clone the image to a new bitmap object.
        Bitmap Clone() const;
        Bitmap CloneAsGrey() const;
        Bitmap CloneAsRGB() const;

        // Clone metadata from this bitmap object to another target bitmap object.
        void CloneMetadata(Bitmap* target) const;

        // Read specific EXIF tag.
        bool ReadExifTag(const FREE_IMAGE_MDMODEL model, const std::string& tag_name,
                         std::string* result) const;

    private:
        typedef std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> FIBitmapPtr;

        void SetPtr(FIBITMAP* data);

        static bool IsPtrGrey(FIBITMAP* data);
        static bool IsPtrRGB(FIBITMAP* data);
        static bool IsPtrSupported(FIBITMAP* data);

        FIBitmapPtr data_;
        int width_;
        int height_;
        int channels_;
    };

    // Jet colormap inspired by Matlab. Grayvalues are expected in the range [0, 1]
    // and are converted to RGB values in the same range.
    class JetColormap {
    public:
        static float Red(const float gray);
        static float Green(const float gray);
        static float Blue(const float gray);

    private:
        static float Interpolate(const float val, const float y0, const float x0,
                                 const float y1, const float x1);
        static float Base(const float val);
    };

    ////////////////////////////////////////////////////////////////////////////////
    // Implementation
    ////////////////////////////////////////////////////////////////////////////////

    namespace internal {

        template <typename T1, typename T2>
        T2 BitmapColorCast(const T1 value) {
            return std::min(static_cast<T1>(std::numeric_limits<T2>::max()),
                            std::max(static_cast<T1>(std::numeric_limits<T2>::min()),
                                     std::round(value)));
        }

    }  // namespace internal


    template <typename T>
    BitmapColor<T>::BitmapColor() : r(0), g(0), b(0) {}

    template <typename T>
    BitmapColor<T>::BitmapColor(const T r, const T g, const T b)
            : r(r), g(g), b(b) {}

    template <typename T>
    template <typename D>
    BitmapColor<D> BitmapColor<T>::Cast() const {
        BitmapColor<D> color;
        color.r = internal::BitmapColorCast<T, D>(r);
        color.g = internal::BitmapColorCast<T, D>(g);
        color.b = internal::BitmapColorCast<T, D>(b);
        return color;
    }

    template <typename T>
    bool BitmapColor<T>::operator==(const BitmapColor<T>& rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b;
    }

    template <typename T>
    bool BitmapColor<T>::operator!=(const BitmapColor<T>& rhs) const {
        return r != rhs.r || g != rhs.g || b != rhs.b;
    }

//    template <typename T>
//    std::ostream& operator<<(std::ostream& output, const BitmapColor<T>& color) {
//        output << StringPrintf("RGB(%f, %f, %f)", static_cast<double>(color.r),
//                               static_cast<double>(color.g),
//                               static_cast<double>(color.b));
//        return output;
//    }

    FIBITMAP* Bitmap::Data() { return data_.get(); }
    const FIBITMAP* Bitmap::Data() const { return data_.get(); }

    int Bitmap::Width() const { return width_; }
    int Bitmap::Height() const { return height_; }
    int Bitmap::Channels() const { return channels_; }

    unsigned int Bitmap::BitsPerPixel() const {
        return FreeImage_GetBPP(data_.get());
    }

    unsigned int Bitmap::ScanWidth() const {
        return FreeImage_GetPitch(data_.get());
    }

    bool Bitmap::IsRGB() const { return channels_ == 3; }

    bool Bitmap::IsGrey() const { return channels_ == 1; }

}


#endif //BKMAP_BITMAP_H
