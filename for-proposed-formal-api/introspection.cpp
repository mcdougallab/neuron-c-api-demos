#include "neuron_api.h"
#include <iostream>
#include <cstring>

using std::cout;
using std::endl;

static const char* argv[] = {"hh_sim", "-nogui", "-nopython", nullptr};

void print_symbol_table(SymbolTableIterator* st) {
    int line_length = 0;
    bool first_output = true;
    for (;!nrn_symbol_table_iterator_done(st);) { 
        auto name = nrn_symbol_table_iterator_next(st);
        if (line_length + strlen(name) + 2 < 70) {
            if (first_output) {
                cout << "    " << name;
                line_length = strlen(name);
            } else {
                cout << ", " << name;
                line_length += strlen(name) + 2;
            }
        } else {
            line_length = strlen(name);
            cout << "," << endl << "    " << name;
        }
        first_output = false;
    }
    cout << endl;
}

int main(void) {
    setup_neuron_api();
    nrn_init(3, argv);

    auto sym = nrn_get_symbol("topology");
    cout << "symbol for topology corresponds to: " << nrn_get_symbol_name(sym) << endl;

    auto vec = nrn_new_object(nrn_get_symbol("Vector"), 0);
    cout << "Vector instance has class: " << nrn_get_class_name(vec) << endl;

    // global symbol table
    cout << "global symbol table:" << endl;
    auto st = nrn_new_symbol_table_iterator(nrn_get_global_symbol_table());
    print_symbol_table(st);
    nrn_free_symbol_table_iterator(st);
    cout << endl;

    // Vector symbol table
    cout << "Vector symbol table:" << endl;
    st = nrn_new_symbol_table_iterator(nrn_get_symbol_table(nrn_get_symbol("Vector")));
    print_symbol_table(st);
    nrn_free_symbol_table_iterator(st);
    cout << endl;

    auto cvode = nrn_new_object(nrn_get_symbol("CVode"), 0);
    cout << "Symbol table from instance of " << nrn_get_class_name(cvode) << ":" << endl;
    st = nrn_new_symbol_table_iterator(nrn_get_symbol_table(nrn_get_symbol(nrn_get_class_name(cvode))));
    print_symbol_table(st);
    nrn_free_symbol_table_iterator(st);
    cout << endl;
}