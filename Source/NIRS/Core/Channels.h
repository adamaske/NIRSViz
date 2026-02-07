#pragma once
#include <vector>

#include <Eigen/Dense>

namespace NIRS {
    namespace Channels {

        using ChannelValue = double;
        using ChannelDataVector = std::vector<ChannelValue>;

        /// Raw time-series matrix parsed from data1/dataTimeSeries.
        /// Rows = channels, Cols = time-points (transposed from HDF5 layout).
        struct ChannelDataStore {
            Eigen::Matrix<double,
                Eigen::Dynamic,
                Eigen::Dynamic,
                Eigen::RowMajor> matrix;  // [channels x timepoints]
        };
    }
}
