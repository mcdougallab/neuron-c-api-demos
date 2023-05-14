#include "neuron_api.h"
#include <cstring>
#include <iostream>
#include <fstream>

using std::cout;
using std::endl;
using std::ofstream;

static const char* argv[] = {"hh_sim", "-nogui", "-nopython", nullptr};

int main(void) {
    Section* soma;
    Object* vclamp;
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
    soma = nrn_new_section("soma");
    
    // define soma morphology with two 3d points
    nrn_push_section(soma);
    nrn_set_section_length(soma, 10);
    nrn_set_segment_diam(soma, 0.5, 3);

    // voltage clamp at soma(0.5)
    nrn_push_double(0.5);
    vclamp = nrn_new_object(nrn_get_symbol("VClamp"), 1);
    auto vclamp_amp = nrn_get_pp_property_ptr(vclamp, "amp");
    auto vclamp_dur = nrn_get_pp_property_ptr(vclamp, "dur");

    // 0 mV for 1 ms; 10 mV for the next 2 ms; 5 mV for the next 3 ms

    vclamp_amp[0] = 0;
    vclamp_amp[1] = 10;
    vclamp_amp[2] = 5;

    vclamp_dur[0] = 1;
    vclamp_dur[1] = 2;
    vclamp_dur[2] = 3;

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

    // continuerun(6)
    nrn_push_double(6);
    nrn_call_function(nrn_get_symbol("continuerun"), 1);
    nrn_pop_double();

    double* tvec = nrn_vector_data_ptr(t);
    double* vvec = nrn_vector_data_ptr(v);
    ofstream out_file;
    out_file.open("vclamp.csv");
    for (auto i = 0; i < nrn_vector_capacity(t); i++) {
        out_file << tvec[i] << "," << vvec[i] << endl;
    }
    out_file.close();
    cout << "Results saved to vclamp.csv" << endl;

}