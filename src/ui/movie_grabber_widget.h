//
// Created by tri on 26/09/2017.
//

#ifndef BKMAP_MOVIE_GRABBER_WIDGET_H
#define BKMAP_MOVIE_GRABBER_WIDGET_H

#include <unordered_map>

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "base/reconstruction.h"

namespace bkmap {

    class OpenGLWindow;

    class MovieGrabberWidget : public QWidget {
    public:
        MovieGrabberWidget(QWidget* parent, OpenGLWindow* opengl_window);

        // List of views, used to visualize the movie grabber camera path.
        std::vector<Image> views;

        struct ViewData {
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW
            QMatrix4x4 model_view_matrix;
            float point_size = -1.0f;
            float image_size = -1.0f;
        };

    private:
        // Add, delete, clear viewpoints.
        void Add();
        void Delete();
        void Clear();

        // Assemble movie from current viewpoints.
        void Assemble();

        // Event slot for time modification.
        void TimeChanged(QTableWidgetItem* item);

        // Event slot for changed selection.
        void SelectionChanged(const QItemSelection& selected,
                              const QItemSelection& deselected);

        // Update state when viewpoints reordered.
        void UpdateViews();

        OpenGLWindow* opengl_window_;

        QPushButton* assemble_button_;
        QPushButton* add_button_;
        QPushButton* delete_button_;
        QPushButton* clear_button_;
        QTableWidget* table_;

        QSpinBox* frame_rate_sb_;
        QCheckBox* smooth_cb_;
        QDoubleSpinBox* smoothness_sb_;

        EIGEN_STL_UMAP(const QTableWidgetItem*, ViewData) view_data_;
    };

}

EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION_CUSTOM(
        bkmap::MovieGrabberWidget::ViewData)

#endif //BKMAP_MOVIE_GRABBER_WIDGET_H
