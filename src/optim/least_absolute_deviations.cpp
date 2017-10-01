//
// Created by tri on 26/09/2017.
//

#include "optim/least_absolute_deviations.h"

#include <Eigen/SparseCholesky>

namespace bkmap {
    namespace {

        Eigen::VectorXd Shrinkage(const Eigen::VectorXd& a, const double kappa) {
            const Eigen::VectorXd a_plus_kappa = a.array() + kappa;
            const Eigen::VectorXd a_minus_kappa = a.array() - kappa;
            return a_plus_kappa.cwiseMin(0) + a_minus_kappa.cwiseMax(0);
        }

    }  // namespace

    bool SolveLeastAbsoluteDeviations(const LeastAbsoluteDeviationsOptions& options,
                                      const Eigen::SparseMatrix<double>& A,
                                      const Eigen::VectorXd& b,
                                      Eigen::VectorXd* x) {
        CHECK_NOTNULL(x);
        CHECK_GT(options.rho, 0);
        CHECK_GT(options.alpha, 0);
        CHECK_GT(options.max_num_iterations, 0);
        CHECK_GE(options.absolute_tolerance, 0);
        CHECK_GE(options.relative_tolerance, 0);

        Eigen::SimplicialLLT<Eigen::SparseMatrix<double>> linear_solver;
        linear_solver.compute(A.transpose() * A);

        Eigen::VectorXd z = Eigen::VectorXd::Zero(A.rows());
        Eigen::VectorXd z_old(A.rows());
        Eigen::VectorXd u = Eigen::VectorXd::Zero(A.rows());

        Eigen::VectorXd Ax(A.rows());
        Eigen::VectorXd Ax_hat(A.rows());

        const double b_norm = b.norm();
        const double eps_pri_threshold =
                std::sqrt(A.rows()) * options.absolute_tolerance;
        const double eps_dual_threshold =
                std::sqrt(A.cols()) * options.absolute_tolerance;

        for (int i = 0; i < options.max_num_iterations; ++i) {
            *x = linear_solver.solve(A.transpose() * (b + z - u));
            if (linear_solver.info() != Eigen::Success) {
                return false;
            }

            Ax = A * *x;
            Ax_hat = options.alpha * Ax + (1 - options.alpha) * (z + b);

            z_old = z;
            z = Shrinkage(Ax_hat - b + u, 1 / options.rho);

            u += Ax_hat - z - b;

            const double r_norm = (Ax - z - b).norm();
            const double s_norm = (-options.rho * A.transpose() * (z - z_old)).norm();
            const double eps_pri =
                    eps_pri_threshold + options.relative_tolerance *
                                        std::max(b_norm, std::max(Ax.norm(), z.norm()));
            const double eps_dual =
                    eps_dual_threshold +
                    options.relative_tolerance * (options.rho * A.transpose() * u).norm();

            if (r_norm < eps_pri && s_norm < eps_dual) {
                break;
            }
        }

        return true;
    }

}