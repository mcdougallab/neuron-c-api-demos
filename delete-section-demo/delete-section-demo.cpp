#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include "../neuron_api_headers.h"

using std::cout;
using std::endl;
using std::exit;
using std::ofstream;
using std::vector;

static const char* argv[] = {"nrn_test", "-nogui", "-nopython", NULL};
extern "C" void modl_reg() {};

scptr_function hoc_lookup;
vd_function hoc_pushx;
dvptrint_function hoc_call_func;
voptrsptritemptrptri_function new_sections;
vsecptr_function nrn_pushsec;
secptrv_function nrn_sec_pop;
vsptr_function hoc_install_object_data_index;
vv_function delete_section;
dv_function hoc_xpop;



void finitialize(double v0) {
    hoc_pushx(v0);
    hoc_call_func(hoc_lookup("finitialize"), 1);   
}


Section* new_section(const char* name) {
    Symbol* symbol = new Symbol;
    auto pitm = new hoc_Item*;
    char* name_ptr = new char[strlen(name)];
    strcpy(name_ptr, name);
    symbol->name = name_ptr;
    symbol->type = 1;
    symbol->u.oboff = 0;
    symbol->arayinfo = 0;    
    hoc_install_object_data_index(symbol);
    new_sections(nullptr, symbol, pitm, 1);
    return (*pitm)->element.sec;
}


void topology(void) {
    hoc_call_func(hoc_lookup("topology"), 0);
}


int main(void) {
    Symbol* sym;
    char* error;
    int oboff;
    void* handle = dlopen("libnrniv.dylib", RTLD_NOW | RTLD_LOCAL); 
    if (!handle) {
        cout << "Couldn't open dylib." << endl << dlerror() << endl;
        exit(-1);
    }

    /***************************
     * 
     * A bunch of functions in NEURON we'll want to be able to call
     * 
     **************************/

    // just using hoc_last_init alone is insufficient, which is too bad because it isn't mangled
    auto ivocmain = (initer_function) dlsym(handle, "_Z16ivocmain_sessioniPPKcS1_i");  // mangled version of: ivocmain_session
    error = dlerror();
    assert(!error);

    *((int64_t*) dlsym(handle, "nrn_main_launch")) = 0;

    hoc_lookup = (scptr_function) dlsym(handle, "hoc_lookup");
    assert(hoc_lookup);

    hoc_call_func = (dvptrint_function) dlsym(handle, "hoc_call_func");
    assert(hoc_call_func);

    hoc_pushx = (vd_function) dlsym(handle, "hoc_pushx");
    assert(hoc_pushx);

    auto nrnmpi_stubs = (vv_function) dlsym(handle, "_Z12nrnmpi_stubsv");
    // no assert because may not exist if no dynamic MPI compiled in
    hoc_install_object_data_index = (vsptr_function) dlsym(handle, "hoc_install_object_data_index");
    assert(hoc_install_object_data_index);

    new_sections = (voptrsptritemptrptri_function) dlsym(handle, "_Z12new_sectionsP6ObjectP6SymbolPP8hoc_Itemi");
    assert(new_sections);

    nrn_pushsec = (vsecptr_function) dlsym(handle, "nrn_pushsec");
    assert(nrn_pushsec);

    nrn_sec_pop = (secptrv_function) dlsym(handle, "_Z11nrn_sec_popv");
    assert(nrn_sec_pop);

    delete_section = (vv_function) dlsym(handle, "_Z14delete_sectionv");
    assert(delete_section);

    hoc_install_object_data_index = (vsptr_function) dlsym(handle, "hoc_install_object_data_index");
    assert(hoc_install_object_data_index);



    /***************************
     * 
     * initialization
     * 
     **************************/

    // commenting out the following line shows the banner
    *((int*) dlsym(handle, "nrn_nobanner_")) = 1;


    // need this to support dynamic MPI, but might not exist without that enabled
    if (nrnmpi_stubs) {
        nrnmpi_stubs();
    }

    ivocmain(3, argv, NULL, 0);

    /***************************
     * 
     * the main part of the code
     * 
     **************************/

    // creating sections
    auto main = new_section("main");
    auto branch1 = new_section("branch1");
    auto branch2 = new_section("branch2");

    cout << "Initial topology:" << endl;
    topology();

    cout << main->prop->dparam << endl;
    delete_section();

    cout << "Topology after deleting branch 1:" << endl;
    topology();
}
