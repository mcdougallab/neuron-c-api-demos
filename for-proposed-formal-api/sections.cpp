#include "neuron_api.h"
#include <iostream>

using std::cout;
using std::endl;
using std::ofstream;

static const char* argv[] = {"hh_sim", "-nogui", "-nopython", nullptr};

int main(void) {
    setup_neuron_api();
    nrn_init(3, argv);

    // topology
    auto soma = nrn_new_section("soma");
    auto dend1 = nrn_new_section("dend1");
    auto dend2 = nrn_new_section("dend2");
    auto dend3 = nrn_new_section("dend3");
    auto axon = nrn_new_section("axon");
    nrn_connect_sections(dend1, 0, soma, 1);
    nrn_connect_sections(dend2, 0, dend1, 1);
    nrn_connect_sections(dend3, 0, dend1, 1);
    nrn_connect_sections(axon, 0, soma, 0);
    nrn_set_nseg(axon, 5);
    
    // print out the morphology
    nrn_call_function(nrn_get_symbol("topology"), 0);

    // create a SectionList that is dend1 and its children
    auto seclist = nrn_new_object(nrn_get_symbol("SectionList"), 0);
    nrn_push_section(dend1);
    nrn_call_method(seclist, nrn_get_method_symbol(seclist, "subtree"), 0);
    nrn_pop_section();

    // loop over allsec, print out
    cout << "allsec:" << endl;
    auto sli = nrn_new_sectionlist_iterator(nrn_get_allsec());
    for (;!nrn_sectionlist_iterator_done(sli);) { 
        auto sec=nrn_sectionlist_iterator_next(sli);
        cout << "    " << nrn_secname(sec) << endl;
    }
    nrn_free_sectionlist_iterator(sli);

    // loop over dend1's subtree
    cout << "dend1's subtree:" << endl;
    sli = nrn_new_sectionlist_iterator(nrn_get_sectionlist_data(seclist));
    for (;!nrn_sectionlist_iterator_done(sli);) { 
        auto sec=nrn_sectionlist_iterator_next(sli);
        cout << "    " << nrn_secname(sec) << endl;
    }
    nrn_free_sectionlist_iterator(sli);

}