#include "neuron_api.h"
#include <cstring>
#include <iostream>
#include <fstream>

using std::cout;
using std::endl;
using std::ofstream;

static const char* argv[] = {"netcon", "-nogui", "-nopython", nullptr};

int main(void) {
    Object* v;
    Object* t;
    char* temp_str;

    setup_neuron_api();
    nrn_init(3, argv);

    // load the stdrun library
    temp_str = new char[11];
    strcpy(temp_str, "stdrun.hoc");
    nrn_push_str(&temp_str);
    nrn_call_function(nrn_get_symbol("load_file"), 1);
    nrn_pop_double();
    delete[] temp_str;


    // topology
    auto soma = nrn_new_section("soma");

    // ion channels
    nrn_insert_mechanism(soma, nrn_get_symbol("hh"));

    // NetStim
    auto ns = nrn_new_object(nrn_get_symbol("NetStim"), 0);
    *nrn_get_pp_property_ptr(ns, "start") = 5;
    *nrn_get_pp_property_ptr(ns, "noise") = 1;
    *nrn_get_pp_property_ptr(ns, "interval") = 5;
    *nrn_get_pp_property_ptr(ns, "number") = 10;

    // syn = h.ExpSyn(soma(0.5))
    nrn_push_double(0.5);
    auto syn = nrn_new_object(nrn_get_symbol("ExpSyn"), 1);
    *nrn_get_pp_property_ptr(syn, "tau")  = 3;  // 3 ms timeconstant
    *nrn_get_pp_property_ptr(syn, "e") = 0;    // 0 mV reversal potential (excitatory synapse)

    // nc = h.NetCon(ns, syn)
    nrn_push_object(ns);
    nrn_push_object(syn);
    auto nc = nrn_new_object(nrn_get_symbol("NetCon"), 2);
    nrn_get_steered_property_ptr(nc, "weight")[0] = 0.5;
    *nrn_get_steered_property_ptr(nc, "delay") = 0;
    
    auto vec = nrn_new_object(nrn_get_symbol("Vector"), 0);

    // nc.record(vec)
    nrn_push_object(vec);
    nrn_call_method(nc, nrn_get_method_symbol(nc, "record"), 1);
    // TODO: record probably put something on the stack that should be removed

    // setup recording
    v = nrn_new_object(nrn_get_symbol("Vector"), 0);
    nrn_push_double_ptr(nrn_get_rangevar_ptr(soma, nrn_get_symbol("v"), 0.5));
    nrn_call_method(v, nrn_get_method_symbol(v, "record"), 1);
    nrn_unref_object(nrn_pop_object());  // record returns the vector
    t = nrn_new_object(nrn_get_symbol("Vector"), 0);
    nrn_push_double_ptr(nrn_get_symbol_ptr(nrn_get_symbol("t")));
    nrn_call_method(t, nrn_get_method_symbol(t, "record"), 1);
    nrn_unref_object(nrn_pop_object());  // record returns the vector

    // finitialize(-65)
    nrn_push_double(-65);
    nrn_call_function(nrn_get_symbol("finitialize"), 1);
    nrn_pop_double();

    // continuerun(100)
    nrn_push_double(100);
    nrn_call_function(nrn_get_symbol("continuerun"), 1);
    nrn_pop_double();

    double* tvec = nrn_vector_data_ptr(t);
    double* vvec = nrn_vector_data_ptr(v);
    ofstream out_file;
    out_file.open("netcon.csv");
    for (auto i = 0; i < nrn_vector_capacity(t); i++) {
        out_file << tvec[i] << "," << vvec[i] << endl;
    }
    out_file.close();
    cout << "Results saved to netcon.csv" << endl;
}