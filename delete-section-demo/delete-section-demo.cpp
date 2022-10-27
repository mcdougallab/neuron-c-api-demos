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
Objectdata** hoc_top_level_data;
icptr_function hoc_oc;


void finitialize(double v0) {
    hoc_pushx(v0);
    hoc_call_func(hoc_lookup("finitialize"), 1);   
}


Section* new_section(const char* name) {
    Symbol* symbol = new Symbol;
    auto pitm = (hoc_Item**) malloc(sizeof(hoc_Item*));  // new hoc_Item*;  
    char* name_ptr = new char[strlen(name)];
    strcpy(name_ptr, name);
    symbol->name = name_ptr;
    symbol->type = 308;
    symbol->arayinfo = 0;    
    hoc_install_object_data_index(symbol);
    cout << name << " oboff: " << symbol->u.oboff << endl;
    (*hoc_top_level_data)[symbol->u.oboff].psecitm = pitm;
    new_sections(nullptr, symbol, pitm, 1);
    /*
    for (auto i = 0; i < 11; i++) {
        cout << "    " << i << ": " << (*pitm)->element.sec->prop->dparam[i].val << endl;
    }
    cout << "    prop->next: " << (*pitm)->element.sec->prop->next << endl;
    */
    return (*pitm)->element.sec;
}


void topology(void) {
    hoc_call_func(hoc_lookup("topology"), 0);
}

void my_delete_section(Section* sec) {
    nrn_pushsec(sec);
    //delete_section();
    hoc_oc("delete_section()");
    nrn_sec_pop();
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

    hoc_top_level_data = (Objectdata**) dlsym(handle, "hoc_top_level_data");
    assert(hoc_top_level_data);

    hoc_install_object_data_index = (vsptr_function) dlsym(handle, "hoc_install_object_data_index");
    assert(hoc_install_object_data_index);

    hoc_oc = (icptr_function) dlsym(handle, "hoc_oc");
    assert(hoc_oc);


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

    cout << "Currently accessed section: ";
    hoc_oc("secname()");

    cout << "Initial topology:" << endl;
    topology();

    /*
    for (auto i = 0; i < 11; i++) {
        cout << "    main->prop->dparam[" << i << "]: " << main->prop->dparam[i].val << endl;
    }
    cout << "    prop->next: " << main->prop->next << endl;
    */
    //delete_section();

    cout << "Topology after deleting branch1:" << endl;
    my_delete_section(branch1);

    topology();


    cout << "Topology after deleting branch2:" << endl;
    my_delete_section(branch2);

    topology();

    // topology is apparently unhappy if the only thing that remains is a deleted section
    // so let's give it something else
    auto foo = new_section("foo");

    cout << "Topology after adding foo and deleting main:" << endl;
    
    my_delete_section(main);

    topology();
}
