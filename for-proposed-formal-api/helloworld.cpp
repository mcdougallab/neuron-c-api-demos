#include "neuron_api.h"

static const char* argv[] = {"nrn_test", "-nogui", "-nopython", nullptr};

int main(void) {
    setup_neuron_api();

    nrn_init(3, argv);

    nrn_call_hoc("print \"hello world\"");
}