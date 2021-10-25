// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file hierarchical_collection.hpp
 * @brief Collection by hierarchical gossip.
 */

#ifndef FCPP_HIERARCHICAL_COLLECTION_H_
#define FCPP_HIERARCHICAL_COLLECTION_H_

#include <vector>

#include "lib/fcpp.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Minimum number whose square is at least n.
constexpr size_t discrete_sqrt(size_t n) {
    size_t lo = 0, hi = n, mid = 0;
    while (lo < hi) {
        mid = (lo + hi)/2;
        if (mid*mid < n) lo = mid+1;
        else hi = mid;
    }
    return lo;
}

//! @brief Minimum power of b exceeding n.
constexpr size_t discrete_log(size_t b, size_t n) {
    size_t e = 0;
    for (size_t r = 1; r <= n; r *= b) ++e;
    return e;
}

//! @brief Number of devices.
constexpr size_t devices = 100;

//! @brief Hierarchy growth base.
constexpr hops_t hierarchy_base = 2;

//! @brief Hierarchy length.
constexpr hops_t max_level = discrete_log(hierarchy_base, devices);

//! @brief Communication radius.
constexpr size_t comm = 100;

//! @brief Whether to put devices in a square or a line.
constexpr bool squared = false;

//! @brief X side of the deployment area.
constexpr size_t xside = squared ? discrete_sqrt(devices * 3000) : devices * 10;

//! @brief Y side of the deployment area.
constexpr size_t yside = squared ? xside : comm;

//! @brief Height of the deployment area.
constexpr size_t height = 0;

//! @brief Dimensionality of the space.
constexpr size_t dim = 3;

//! @brief Color hue scale.
constexpr float hue_scale = 360.0f/devices;

//! @brief The end of simulated time.
constexpr size_t end_time = 500;


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


namespace tags {
    //! @brief Whether the simulation is synchronous.
    struct synchrony {};

    //! @brief The overall leader of the network.
    struct main_leader {};

    //! @brief The distance estimates.
    struct dist {};

    //! @brief The device movement speed.
    struct speed {};

    //! @brief The ideal result.
    struct ideal {};

    //! @brief The bottom-up hierarchical election algorithm with hysteresis.
    struct buh {};

    //! @brief The simple bottom-up hierarchical election algorithm.
    struct bus {};

    //! @brief The top-down hierarchical election algorithm with hysteresis.
    struct tdh {};

    //! @brief The simple top-down hierarchical election algorithm.
    struct tds {};

    //! @brief The single-path collection algorithm.
    struct sp {};

    //! @brief The weighted multi-path collection algorithm.
    struct wmp {};

    //! @brief The computed total device count.
    template <typename T>
    struct count {};

    //! @brief The leader level of the node.
    struct level {};

    //! @brief The counted information for every level.
    struct count_chain {};

    //! @brief The leader information for every level.
    struct leader_chain {};

    //! @brief Distance from the chosen leader.
    struct leader_dist {};

    //! @brief The leader chosen for a node.
    struct leader {};

    //! @brief Color representing the leader chosen for a node.
    struct leader_col {};

    //! @brief Color representing the node.
    struct personal_col {};

    //! @brief Size of the current node.
    struct node_size {};

//! @brief Shape of the current node.
    struct node_shape {};
}

//! @brief Idempotent collection in isolated partitions of a network.
GEN(T, F, BOUND(F, T(T,T)))
T partitioned_idempotent_collection(ARGS, tuple<device_t, hops_t> const& ld, T const& value, T const& null, F&& accumulate) { CODE
    return nbr(CALL, null, [&](field<T> x){
        field<tuple<device_t, hops_t>> nbrld = nbr(CALL, ld);
        return fold_hood(CALL, accumulate, mux(get<1>(nbrld) > get<1>(ld) and get<0>(nbrld) == get<0>(ld), x, null), value);
    });
}
GEN_EXPORT(T) partitioned_idempotent_collection_t = common::export_list<tuple<device_t, hops_t>, T>;

//! @brief Leader election by diameter in isolated partitions of a network.
FUN tuple<device_t,hops_t> hysteresis_diameter_election_distance(ARGS, hops_t diameter, hops_t reduced_diameter) { CODE
    using type = tuple<device_t,hops_t>;
    type loc(node.uid, 0);
    get<1>(loc) -= 1;
    return nbr(CALL, type(node.uid, 0), [&](field<type> x){
        type r = min_hood(CALL, mux(get<1>(x) < diameter, x, loc), loc);
        get<1>(r) += 1;
        if (get<1>(r) > reduced_diameter and get<1>(self(CALL, x)) == 0) r = self(CALL, x);
        return r;
    });
}
FUN_EXPORT hysteresis_diameter_election_distance_t = common::export_list<tuple<device_t,hops_t>>;

//! @brief Leader election by diameter in isolated partitions of a network.
FUN tuple<device_t,hops_t> partitioned_diameter_election_distance(ARGS, tuple<device_t, hops_t> const& ld, hops_t diameter, hops_t reduced_diameter) { CODE
    using type = tuple<device_t,hops_t>;
    type loc = get<1>(ld) <= diameter ? ld : type(node.uid, 0);
    get<1>(loc) -= 1;
    return nbr(CALL, loc, [&](field<type> x){
        type r = min_hood(CALL, mux(nbr(CALL, get<0>(ld)) == get<0>(ld) and get<1>(x) < diameter, x, loc), loc);
        get<1>(r) += 1;
        if (get<1>(r) > reduced_diameter and get<1>(self(CALL, x)) == 0) r = self(CALL, x);
        return r;
    });
}
FUN_EXPORT partitioned_diameter_election_distance_t = common::export_list<tuple<device_t,hops_t>, device_t>;

//! @Brief Hierarchical collection algorithm.
GEN(T, F, BOUND(F, T(T,T)))
T hierarchical_collection(ARGS, hops_t max_level, T const& value, T const& null, F&& accumulate, bool bottomup, bool store, bool hysteresis) { CODE
    constexpr device_t offset = 1 << (sizeof(device_t)*CHAR_BIT-1);
    constexpr device_t mask = offset - 1;
    device_t uid = node.uid;

    std::vector<hops_t> rad = {1};
    for (hops_t i=1; i<=max_level; ++i) rad.push_back(rad.back()*hierarchy_base);

    std::vector<tuple<device_t, hops_t>> leaders(max_level+2);
    leaders[0] = {node.uid, 0};
    leaders[max_level+1] = {devices, rad.back()};
    auto leader_to_col = [](size_t i){
        return color::hsva(min(i, devices) * hue_scale, 1, 1, 1);
    };
    auto set_level_data = [&](size_t i, device_t leader){
        if (store) {
            node.storage(tags::level{}) = i;
            node.storage(tags::node_size{}) = 5 + 2*i;
            node.storage(tags::node_shape{}) = (shape)(i%6);
            node.storage(tags::leader_dist{}) = get<1>(leaders[i+1]);
            node.storage(tags::leader{}) = leader;
            node.storage(tags::leader_col{}) = leader_to_col(leader);
        }
    };
    if (store) {
        node.storage(tags::level{}) = -42;
        node.storage(tags::personal_col{}) = leader_to_col(node.uid);
    }
    if (bottomup) {
        for (LOOP(i, 1); i <= max_level; ++i) {
            leaders[i] = hysteresis_diameter_election_distance(CALL, rad[i]-1, hysteresis ? (rad[i]+1)/3 : rad[i]-1);
            if (get<0>(leaders[i]) != node.uid and uid == node.uid) {
                uid |= offset;
                set_level_data(i-1, get<0>(leaders[i]));
            }
        }
        if (get<0>(leaders[max_level]) == node.uid) set_level_data(max_level, devices);
    } else {
        for (LOOP(i, max_level); i > 0; --i) {
            leaders[i] = partitioned_diameter_election_distance(CALL, leaders[i+1], rad[i]-1, hysteresis ? (rad[i]+1)/3 : rad[i]-1);
            if (get<0>(leaders[i]) == uid) {
                uid |= offset;
                set_level_data(i, get<0>(leaders[i+1]));
            }
        }
        if (uid == node.uid) set_level_data(0, get<0>(leaders[1]));
    }
    assert(not store or node.storage(tags::level{}) >= 0);
    if (store) node.storage(tags::leader_chain{}) = leaders;

    using type = std::vector<tuple<device_t, T>>;
    auto sorted_merge = [](type x, type y){
        type z;
        size_t i = 0, j = 0;
        while (i < x.size() and j < y.size()) {
            if (get<0>(x[i]) < get<0>(y[j])) z.push_back(x[i++]);
            else if (get<0>(x[i]) > get<0>(y[j])) z.push_back(y[j++]);
            else z.push_back(min(x[i++], y[j++]));
        }
        for (; i < x.size(); ++i) z.push_back(x[i]);
        for (; j < y.size(); ++j) z.push_back(y[j]);
        return z;
    };
    type res = {{node.uid, value}};
    std::vector<type> counts;
    counts.emplace_back(res);
    for (LOOP(i, 1); i <= max_level; ++i) {
        res = partitioned_idempotent_collection(CALL, leaders[i], res, type{}, sorted_merge);
        counts.push_back(res);
        if (get<0>(leaders[i]) == node.uid and res.size() > 0) {
            get<0>(res[0]) = node.uid;
            for (size_t j=1; j<res.size(); ++j) get<1>(res[0]) = accumulate(get<1>(res[0]), get<1>(res[j]));
            res.resize(1);
        } else res.clear();
    }
    if (store) node.storage(tags::count_chain{}) = counts;
    return res.empty() ? null : get<1>(res[0]);
}
GEN_EXPORT(T) hierarchical_collection_t = common::export_list<partitioned_idempotent_collection_t<std::vector<tuple<device_t, T>>>, hysteresis_diameter_election_distance_t, partitioned_diameter_election_distance_t>;

//! @brief Main function.
MAIN() {
    if (node.uid == 0) node.position() = 2*node.current_time() < end_time ? make_vec(0,yside/2,height/2) : make_vec(xside,yside/2,height/2);
    else rectangle_walk(CALL, make_vec(0,0,0), make_vec(xside,yside,height), node.storage(tags::speed{}), 1);
    node.storage(tags::count<tags::bus>{}) = hierarchical_collection(CALL, max_level, 1, 0, [](int x, int y){ return x+y; }, true,  false, false);
    node.storage(tags::count<tags::tds>{}) = hierarchical_collection(CALL, max_level, 1, 0, [](int x, int y){ return x+y; }, false, false, false);
    node.storage(tags::count<tags::buh>{}) = hierarchical_collection(CALL, max_level, 1, 0, [](int x, int y){ return x+y; }, true,  true,  true);
    node.storage(tags::count<tags::tdh>{}) = hierarchical_collection(CALL, max_level, 1, 0, [](int x, int y){ return x+y; }, false, false, true);
    device_t leader_id = node.storage(tags::main_leader{}) = wave_election(CALL);
    real_t leader_dist = node.storage(tags::dist{}) = bis_distance(CALL, node.uid == leader_id, 1, 0.6*comm);
    real_t wmp = wmp_collection(CALL, leader_dist, comm, real_t(1), [](real_t x, real_t y){ return x+y; }, [](real_t x, real_t f){ return x*f; });
    real_t sp = sp_collection(CALL, leader_dist, 1, 0, [](int x, int y){ return x+y; });
    node.storage(tags::count<tags::wmp>{}) = node.uid == leader_id ? wmp : 0;
    node.storage(tags::count<tags::sp>{}) = node.uid == leader_id ? sp : 0;
}
FUN_EXPORT main_t = common::export_list<rectangle_walk_t<3>, hierarchical_collection_t<int>, wave_election_t<>, sp_collection_t<real_t, int>, wmp_collection_t<real_t>, bis_distance_t>;


}


}

#endif // FCPP_HIERARCHICAL_COLLECTION_H_
