#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cassert>
#include "../neuron_api_headers.h"

using std::cout;
using std::endl;
using std::exit;
using std::ofstream;

static const char* argv[] = {"nrn_test", "-nogui", "-nopython", NULL};
extern "C" void modl_reg() {};

static scptr_function hoc_lookup;
static scptrslptr_function hoc_table_lookup;
static dvptrint_function hoc_call_func;
static vsptr_function hoc_install_object_data_index;
static voptrsptritemptrptri_function new_sections;




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



int main(void) {
    Symbol* sym;
    Symbol* sym2;
    char* error;
    int oboff;
    void* handle = dlopen("libnrniv.dylib", RTLD_NOW | RTLD_LOCAL); 
    if (!handle) {
        handle = dlopen("libnrniv.so", RTLD_NOW | RTLD_LOCAL);
        if (!handle) { 
            cout << "Couldn't open NEURON library." << endl << dlerror() << endl;
            exit(-1);
        }
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
    if (!hoc_lookup) {
        hoc_lookup = (scptr_function) dlsym(handle, "_Z10hoc_lookupPKc");
    }
    assert(hoc_lookup);

    hoc_table_lookup = (scptrslptr_function) dlsym(handle, "hoc_table_lookup");
    if (!hoc_table_lookup) {
        hoc_table_lookup = (scptrslptr_function) dlsym(handle, "_Z16hoc_table_lookupPKcP7Symlist");
    }
    assert(hoc_table_lookup);

    hoc_call_func = (dvptrint_function) dlsym(handle, "hoc_call_func");
    assert(hoc_call_func);

    auto hoc_oc = (icptr_function) dlsym(handle, "hoc_oc");
    if (!hoc_oc) {
        hoc_oc = (icptr_function) dlsym(handle, "_Z6hoc_ocPKc");
    }
    assert(hoc_oc);

    auto nrnmpi_stubs = (vv_function) dlsym(handle, "_Z12nrnmpi_stubsv");
    // no assert because may not exist if no dynamic MPI compiled in

    new_sections = (voptrsptritemptrptri_function) dlsym(handle, "_Z12new_sectionsP6ObjectP6SymbolPP8hoc_Itemi");
    assert(new_sections);

    hoc_install_object_data_index = (vsptr_function) dlsym(handle, "hoc_install_object_data_index");
    if (!hoc_install_object_data_index) {
        hoc_install_object_data_index = (vsptr_function) dlsym(handle, "_Z29hoc_install_object_data_indexP6Symbol");
    }
    assert(hoc_install_object_data_index);

    auto section_list = (hoc_Item**) dlsym(handle, "section_list");
    assert(section_list);

    auto secname = (cptrsecptr_function) dlsym(handle, "_Z7secnameP7Section");
    assert(secname);

    auto section_unref = (vsecptr_function) dlsym(handle, "_Z13section_unrefP7Section");
    assert(section_unref);

    auto hoc_l_delete = (vitemptr_function) dlsym(handle, "_Z12hoc_l_deleteP8hoc_Item");
    assert(hoc_l_delete);

    /***************************
     * 
     * initialization
     * 
     **************************/

    // commenting out the following line shows the banner
    *((int64_t*) dlsym(handle, "nrn_nobanner_")) = 1;


    // need this to support dynamic MPI, but might not exist without that enabled
    if (nrnmpi_stubs) {
        nrnmpi_stubs();
    }

    ivocmain(3, argv, NULL, 0);

    /***************************
     * 
     * the test starts here
     * 
     **************************/

    // creating sections, some in HOC, some in C++ to show it doesn't matter
    hoc_oc("create soma");
    auto dend1 = new_section("dend1");
    hoc_oc("create axon");

    cout << "created three sections, which we demonstrate through topology():" << endl;
    sym = hoc_lookup("topology");
    hoc_call_func(sym, 0);  // the 0 is the number of args; returns the return of the function (1)


    cout << "Now lets loop through them in C++:" << endl;

    // note: we have a circularly linked list with a special node at start/stop
    for (auto list_ptr = (*section_list)->next; list_ptr != *section_list; list_ptr=list_ptr->next) {
        Section* sec = list_ptr->element.sec;
        // check to make sure the section has properties
        // missing properties means that the section has been invalidated
        // but not removed
        if (sec->prop) {
            // we've found a real section, now let's do something
            cout << "    sec = " << secname(sec) << endl;
        } else {
            // invalidated section; we can remove it from the section_list and unref it
            // it would be mostly harmless to skip this, but NEURON does not routinely
            // search for and remove sections
            hoc_l_delete(list_ptr);
            section_unref(sec);
        }
    }
}
