// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file simulation_setup.hpp
 * @brief Setup of the basic simulation details.
 */

#ifndef FCPP_SIMULATION_SETUP_H_
#define FCPP_SIMULATION_SETUP_H_

#include "lib/hierarchical_collection.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

using namespace component::tags;
using namespace coordination::tags;

//! @brief Sequences and distributions.
//! @{
using spawn_s = sequence::multiple_n<devices, 0>;
using log_s = sequence::periodic_n<1, 0, 1, end_time>;
using sync_round_s = sequence::periodic_n<1, 1, 1, end_time+2>;
using async_round_s = sequence::periodic<
    distribution::interval_n<times_t, 0, 1>,
    distribution::weibull_n<times_t, 10, 1, 10>,
    distribution::constant_n<times_t, end_time+2>
>;
using rectangle_d = distribution::rect_n<1, 0, 0, 0, side, side, height>;
//! @}

using aggregator_t = aggregators<
    count<ideal>,   aggregator::mean<double>,
    count<sp>,      aggregator::mean<double>,
    count<wmp>,     aggregator::mean<double>,
    count<bottomup>,aggregator::mean<double>,
    count<topdown>, aggregator::mean<double>
>;

using plotter_t = plot::split<speed, plot::split<synchrony, plot::plotter<aggregator_t, plot::time, count>>>;

//! @brief General options.
template <bool sync>
DECLARE_OPTIONS(opt,
    tuple_store<
        speed,          real_t,
        count<ideal>,   int,
        count<sp>,      int,
        count<wmp>,     real_t,
        count<bottomup>,int,
        count<topdown>, int,
        level,          int,
        count_chain,    std::vector<std::vector<tuple<device_t, int>>>,
        leader_chain,   std::vector<tuple<device_t, hops_t>>,
        leader_dist,    hops_t,
        leader,         device_t,
        leader_col,     color,
        personal_col,   color,
        node_size,      double,
        node_shape,     shape
    >,
    aggregator_t,
    extra_info<synchrony, bool, speed, real_t>,
    plot_type<plotter_t>,
    synchronised<sync>,
    program<coordination::main>,
    exports<coordination::main_t>,
    spawn_schedule<spawn_s>,
    log_schedule<log_s>,
    round_schedule<std::conditional_t<sync, sync_round_s, async_round_s>>,
    retain<metric::retain<2>>,
    init<
        x,              rectangle_d,
        count<ideal>,   distribution::constant_n<int, devices>,
        speed,          distribution::constant_i<real_t, speed>
    >,
    dimension<dim>,
    connector<connect::fixed<comm, 1, dim>>,
    size_tag<node_size>,
    shape_tag<node_shape>,
    color_tag<personal_col, leader_col>
);

}


#endif // FCPP_SIMULATION_SETUP_H_
