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

static const char* argv[] = {"function-callbacks", "-nogui", "-nopython", NULL};
extern "C" void modl_reg() {};

// magic number
const int FUNCTION_TYPE = 280;

scptr_function hoc_lookup;
optrsptri_function hoc_newobj1;
scptrslptr_function hoc_table_lookup;
voptrsptri_function call_ob_proc;
vd_function hoc_pushx;
vdptr_function hoc_pushpx;
dvptrint_function hoc_call_func;
vsecptri_function mech_insert1;
vsptr_function hoc_install_object_data_index;
voptrsptritemptrptri_function new_sections;
ppoptr_function ob2pntproc_0;
vv_function hoc_ret;

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


void finitialize(double v0) {
    hoc_pushx(v0);
    hoc_call_func(hoc_lookup("finitialize"), 1);   
}


void test_callback(void) {
    cout << "Hello from C++" << endl;
    // must call hoc_ret... don't really understand why
    hoc_ret();
    // return value must be pushed onto the stack
    hoc_pushx(42);
}

int main(void) {
    Symbol* sym;
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

    auto hoc_oc = (icptr_function) dlsym(handle, "hoc_oc");
    if (!hoc_oc) {
        hoc_oc = (icptr_function) dlsym(handle, "_Z6hoc_ocPKc");
    }
    assert(hoc_oc);

    hoc_pushx = (vd_function) dlsym(handle, "hoc_pushx");
    if (!hoc_pushx) {
        hoc_pushx = (vd_function) dlsym(handle, "_Z9hoc_pushxd");
    }
    assert(hoc_pushx);

    auto hoc_xpop = (dv_function) dlsym(handle, "hoc_xpop");
    if (!hoc_xpop) {
        hoc_xpop = (dv_function) dlsym(handle, "_Z8hoc_xpopv");
    }
    assert(hoc_xpop);    

    auto nrnmpi_stubs = (vv_function) dlsym(handle, "_Z12nrnmpi_stubsv");
    // no assert because may not exist if no dynamic MPI compiled in

    hoc_newobj1 = (optrsptri_function) dlsym(handle, "_Z11hoc_newobj1P6Symboli");
    assert(hoc_newobj1);

    call_ob_proc = (voptrsptri_function) dlsym(handle, "hoc_call_ob_proc");
    if (!call_ob_proc) {
        call_ob_proc = (voptrsptri_function) dlsym(handle, "_Z16hoc_call_ob_procP6ObjectP6Symboli");
    }
    assert(call_ob_proc);

    auto hoc_obj_unref = (voptr_function) dlsym(handle, "hoc_obj_unref");
    if (!hoc_obj_unref) {
        hoc_obj_unref = (voptr_function) dlsym(handle, "_Z13hoc_obj_unrefP6Object");
    }
    assert(hoc_obj_unref);

    auto hoc_objpop = (optrptr_function) dlsym(handle, "_Z10hoc_objpopv");
    assert(hoc_objpop);

    auto hoc_tobj_unref = (vobjptrptr_function) dlsym(handle, "_Z14hoc_tobj_unrefPP6Object");
    assert(hoc_tobj_unref);
    
    new_sections = (voptrsptritemptrptri_function) dlsym(handle, "_Z12new_sectionsP6ObjectP6SymbolPP8hoc_Itemi");
    assert(new_sections);

    hoc_install_object_data_index = (vsptr_function) dlsym(handle, "hoc_install_object_data_index");
    if (!hoc_install_object_data_index) {
        hoc_install_object_data_index = (vsptr_function) dlsym(handle, "_Z29hoc_install_object_data_indexP6Symbol");
    }
    assert(hoc_install_object_data_index);

    auto hoc_install = (scptridslptrptr_function) dlsym(handle, "_Z11hoc_installPKcidPP7Symlist");
    assert(hoc_install);

    hoc_ret = (vv_function)dlsym(handle, "hoc_ret");
    if (!hoc_ret) {
        hoc_ret = (vv_function) dlsym(handle, "_Z7hoc_retv");
    }
    assert(hoc_ret);


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

    // NOTE: this list isn't populated until after NEURON is initialized
    auto hoc_top_level_symlist = (Symlist**) dlsym(handle, "hoc_top_level_symlist");
    assert(hoc_top_level_symlist);


    /***************************
     * 
     * the test starts here
     * 
     **************************/

    cout << "attempting to register the function" << endl;

    // register the function test_callback with NEURON
    sym = hoc_install("test_callback", FUNCTION_TYPE, 0, hoc_top_level_symlist);
    cout << "populating the symbol data" << endl;
    sym->u.u_proc->defn.pf = test_callback;
    sym->u.u_proc->nauto = 0;  // total number of local variables; always 0 for C++ function
    sym->u.u_proc->nobjauto = 0;

    cout << "attempting to have HOC call our function" << endl;

    hoc_oc("{value=test_callback()}");
    hoc_oc("print \"received value: \", value");

    /***************************
     * creating axon Section
     **************************/
    auto axon = new_section("axon");


}
