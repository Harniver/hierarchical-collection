// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "lib/simulation_setup.hpp"

using namespace fcpp;

plotter_t p;

template <bool sync>
void run() {
    //! @brief The component object type.
    using comp_t = typename component::batch_simulator<parallel<false>, opt<sync>>;
    //! @brief The list of initialisation values to be used for simulations.
    auto init_list = batch::make_tagged_tuple_sequence(
        batch::arithmetic<seed>(0, 3, 1),
        batch::arithmetic<speed>(0, 6, 2),
        batch::constant<synchrony>(sync),
        batch::stringify<output>("output/batch", "txt"),
        batch::constant<plotter>(&p)
    );
    //! @brief Runs the given simulations.
    batch::run(comp_t{}, init_list);
}

int main() {
    run<true >();
    run<false>();
    std::cout << plot::file("batch", p.build());
    return 0;
}
