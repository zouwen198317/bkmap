//
// Created by tri on 26/09/2017.
//

#include "mvs/consistency_graph.h"

#include <fstream>
#include <numeric>

#include "util/logging.h"
#include "util/misc.h"

namespace bkmap {
    namespace mvs {

        const int ConsistencyGraph::kNoConsistentImageIds = -1;

        ConsistencyGraph::ConsistencyGraph() {}

        ConsistencyGraph::ConsistencyGraph(const size_t width, const size_t height,
                                           const std::vector<int>& data)
                : data_(data) {
            InitializeMap(width, height);
        }

        size_t ConsistencyGraph::GetNumBytes() const {
            return (data_.size() + map_.size()) * sizeof(int);
        }

        void ConsistencyGraph::GetImageIds(const int row, const int col,
                                           int* num_images,
                                           const int** image_ids) const {
            const int index = map_(row, col);
            if (index == kNoConsistentImageIds) {
                *num_images = 0;
                *image_ids = nullptr;
            } else {
                *num_images = data_.at(index);
                *image_ids = &data_.at(index + 1);
            }
        }

        void ConsistencyGraph::Read(const std::string& path) {
            std::fstream text_file(path, std::ios::in | std::ios::binary);
            CHECK(text_file.is_open()) << path;

            size_t width = 0;
            size_t height = 0;
            size_t depth = 0;
            char unused_char;

            text_file >> width >> unused_char >> height >> unused_char >> depth >>
            unused_char;
            const std::streampos pos = text_file.tellg();
            text_file.close();

            CHECK_GT(width, 0);
            CHECK_GT(height, 0);
            CHECK_GT(depth, 0);

            std::fstream binary_file(path, std::ios::in | std::ios::binary);
            CHECK(binary_file.is_open()) << path;

            binary_file.seekg(0, std::ios::end);
            const size_t num_bytes = binary_file.tellg() - pos;

            data_.resize(num_bytes / sizeof(int));

            binary_file.seekg(pos);
            ReadBinaryLittleEndian<int>(&binary_file, &data_);
            binary_file.close();

            InitializeMap(width, height);
        }

        void ConsistencyGraph::Write(const std::string& path) const {
            std::fstream text_file(path, std::ios::out);
            CHECK(text_file.is_open()) << path;
            text_file << map_.cols() << "&" << map_.rows() << "&" << 1 << "&";
            text_file.close();

            std::fstream binary_file(path,
                                     std::ios::out | std::ios::binary | std::ios::app);
            CHECK(binary_file.is_open()) << path;
            WriteBinaryLittleEndian<int>(&binary_file, data_);
            binary_file.close();
        }

        void ConsistencyGraph::InitializeMap(const size_t width, const size_t height) {
            map_.resize(height, width);
            map_.setConstant(kNoConsistentImageIds);
            for (size_t i = 0; i < data_.size();) {
                const int num_images = data_.at(i + 2);
                if (num_images > 0) {
                    const int col = data_.at(i);
                    const int row = data_.at(i + 1);
                    map_(row, col) = i + 2;
                }
                i += 3 + num_images;
            }
        }

    }  // namespace mvs
}