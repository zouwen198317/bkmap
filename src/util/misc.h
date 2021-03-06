//
// Created by tri on 21/09/2017.
//

#ifndef BKMAP_MISC_H
#define BKMAP_MISC_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include "util/endian.h"
#include "util/logging.h"
#include "util/string.h"

namespace bkmap {

#ifndef STRINGIFY
#define STRINGIFY(s) STRINGIFY_(s)
#define STRINGIFY_(s) #s
#endif  // STRINGIFY

// Append trailing slash to string if it does not yet end with a slash.
    std::string EnsureTrailingSlash(const std::string& str);

// Check whether file name has the file extension (case insensitive).
    bool HasFileExtension(const std::string& file_name, const std::string& ext);

// Split the path into its root and extension, for example,
// "dir/file.jpg" into "dir/file" and ".jpg".
    void SplitFileExtension(const std::string& path, std::string* root,
                            std::string* ext);

// Check if the path points to an existing directory.
    bool ExistsFile(const std::string& path);

// Check if the path points to an existing directory.
    bool ExistsDir(const std::string& path);

// Check if the path points to an existing file or directory.
    bool ExistsPath(const std::string& path);

// Create the directory if it does not exist.
    void CreateDirIfNotExists(const std::string& path);

// Extract the base name of a path, e.g., "image.jpg" for "/dir/image.jpg".
    std::string GetPathBaseName(const std::string& path);

// Get the path of the parent directory for the given path.
    std::string GetParentDir(const std::string& path);

// Join multiple paths into one path.
    template <typename... T>
    std::string JoinPaths(T const&... paths);

// Return list of files in directory.
    std::vector<std::string> GetFileList(const std::string& path);

// Return list of files, recursively in all sub-directories.
    std::vector<std::string> GetRecursiveFileList(const std::string& path);

// Return list of directories, recursively in all sub-directories.
    std::vector<std::string> GetDirList(const std::string& path);

// Return list of directories, recursively in all sub-directories.
    std::vector<std::string> GetRecursiveDirList(const std::string& path);

// Get the size in bytes of a file.
    size_t GetFileSize(const std::string& path);

// Print first-order heading with over- and underscores to `std::cout`.
    void PrintHeading1(const std::string& heading);

// Print second-order heading with underscores to `std::cout`.
    void PrintHeading2(const std::string& heading);

// Check if vector contains elements.
    template <typename T>
    bool VectorContainsValue(const std::vector<T>& vector, const T value);

    template <typename T>
    bool VectorContainsDuplicateValues(const std::vector<T>& vector);

// Parse CSV line to a list of values.
    template <typename T>
    std::vector<T> CSVToVector(const std::string& csv);

// Concatenate values in list to comma-separated list.
    template <typename T>
    std::string VectorToCSV(const std::vector<T>& values);

// Read contiguous binary blob from file.
    template <typename T>
    void ReadBinaryBlob(const std::string& path, std::vector<T>* data);

// Write contiguous binary blob to file.
    template <typename T>
    void WriteBinaryBlob(const std::string& path, const std::vector<T>& data);

// Read each line of a text file into a separate element. Empty lines are
// ignored and leading/trailing whitespace is removed.
    std::vector<std::string> ReadTextFileLines(const std::string& path);

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

    template <typename... T>
    std::string JoinPaths(T const&... paths) {
        boost::filesystem::path result;
        int unpack[]{0, (result = result / boost::filesystem::path(paths), 0)...};
        static_cast<void>(unpack);
        return result.string();
    }

    template <typename T>
    bool VectorContainsValue(const std::vector<T>& vector, const T value) {
        return std::find_if(vector.begin(), vector.end(), [value](const T element) {
            return element == value;
        }) != vector.end();
    }

    template <typename T>
    bool VectorContainsDuplicateValues(const std::vector<T>& vector) {
        std::vector<T> unique_vector = vector;
        return std::unique(unique_vector.begin(), unique_vector.end()) !=
               unique_vector.end();
    }

    template <typename T>
    std::string VectorToCSV(const std::vector<T>& values) {
        std::string string;
        for (const T value : values) {
            string += std::to_string(value) + ", ";
        }
        return string.substr(0, string.length() - 2);
    }

    template <typename T>
    void ReadBinaryBlob(const std::string& path, std::vector<T>* data) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
//        CHECK(file.is_open()) << path;
        file.seekg(0, std::ios::end);
        const size_t num_bytes = file.tellg();
//        CHECK_EQ(num_bytes % sizeof(T), 0);
        data->resize(num_bytes / sizeof(T));
        file.seekg(0, std::ios::beg);
        ReadBinaryLittleEndian<T>(&file, data);
    }

    template <typename T>
    void WriteBinaryBlob(const std::string& path, const std::vector<T>& data) {
        std::ofstream file(path, std::ios::binary);
//        CHECK(file.is_open()) << path;
        WriteBinaryLittleEndian<T>(&file, data);
    }

}

#endif //BKMAP_MISC_H
