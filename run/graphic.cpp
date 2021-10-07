// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "lib/simulation_setup.hpp"

using namespace fcpp;

plotter_t p;

template <bool sync>
void run(std::string title, real_t v) {
    //! @brief The network object type.
    using net_t = typename component::interactive_simulator<parallel<true>, opt<sync>>::net;
    //! @brief The initialisation values.
    auto init_v = common::make_tagged_tuple<name, epsilon, plotter, speed, synchrony>(title, 0.1, &p, v, sync);
    //! @brief Construct the network object.
    net_t network{init_v};
    //! @brief Run the simulation until exit.
    network.run();
}

int main() {
    std::cout << "/*" << std::endl;
    run<true >("Hierarchical Collection (synchronous)",  0);
    run<false>("Hierarchical Collection (asynchronous)", 0);
    run<false>("Hierarchical Collection (asynchronous)", 5);
    std::cout << "*/" << std::endl;
    std::cout << plot::file("graphic", p.build());
    return 0;
}
