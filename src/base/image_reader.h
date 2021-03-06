//
// Created by tri on 21/09/2017.
//

#ifndef BKMAP_IMAGE_READER_H
#define BKMAP_IMAGE_READER_H

#include "base/database.h"
#include "util/bitmap.h"
#include "util/threading.h"

namespace bkmap {

// Recursively iterate over the images in a directory. Skips an image if it
// already exists in the database. Extracts the camera intrinsics from EXIF and
// writes the camera information to the database.
    class ImageReader {
    public:
        struct Options {
            // Path to database in which to store the extracted data.
            std::string database_path = "";

            // Root path to folder which contains the image.
            std::string image_path = "";

            // Optional list of images to read. The list must contain the relative path
            // of the images with respect to the image_path.
            std::vector<std::string> image_list;

            // Name of the camera model.
            std::string camera_model = "SIMPLE_RADIAL";

            // Whether to use the same camera for all images.
            bool single_camera = false;

            // Specification of manual camera parameters. If empty, camera parameters
            // will be extracted from EXIF, i.e. principal point and focal length.
            std::string camera_params = "";

            // If camera parameters are not specified manually and the image does not
            // have focal length EXIF information, the focal length is set to the
            // value `default_focal_length_factor * max(width, height)`.
            double default_focal_length_factor = 1.2;

            bool Check() const;
        };

        enum class Status {
            FAILURE,
            SUCCESS,
            IMAGE_EXISTS,
            BITMAP_ERROR,
            CAMERA_SINGLE_ERROR,
            CAMERA_DIM_ERROR,
            CAMERA_PARAM_ERROR
        };

        explicit ImageReader(const Options& options, Database* database);

        Status Next(Camera* camera, Image* image, Bitmap* bitmap);
        size_t NextIndex() const;
        size_t NumImages() const;

    private:
        // Image reader options.
        Options options_;
        Database* database_;
        // Index of previously processed image.
        size_t image_index_;
        // Previously processed camera.
        Camera prev_camera_;
    };

}

#endif //BKMAP_IMAGE_READER_H
