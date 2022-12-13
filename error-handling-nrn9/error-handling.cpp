#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include "../neuron_api_headers.h"

using std::cout;
using std::endl;
using std::exit;
using std::ofstream;

static const char* argv[] = {"nrn_test", "-nogui", "-nopython", NULL};
extern "C" void modl_reg() {};

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

    int* nrn_try_catch_nest_depth = (int*) dlsym(handle, "nrn_try_catch_nest_depth");
    assert(nrn_try_catch_nest_depth);


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
     * creating Vector of size 5
     **************************/
    hoc_pushx(5);
    sym = hoc_lookup("Vector");
    cout << "vec = Vector(5)" << endl;
    auto vec = hoc_newobj1(sym, 1);

    /***************************
     * call vec.indgen()... leaves vec: 0, 1, 2, 3, 4
     **************************/
    auto indgen = hoc_table_lookup("indgen", vec->ctemplate->symtable);
    cout << "vec.indgen()" << endl;
    call_ob_proc(vec, indgen, 0);

    /***************************
     * correctly call contains
     **************************/
    auto contains = hoc_table_lookup("contains", vec->ctemplate->symtable);
    for (auto i=0; i < 10; i++) {
        hoc_pushx(i);
        call_ob_proc(vec, contains, 1);
        cout << "vec.contains(" << i << ") = " << hoc_xpop() << endl;
    }

    /***************************
     * incorrectly call contains (not enough args)
     **************************/
    (*nrn_try_catch_nest_depth)++;
    cout << "vec.contains();  [nrn_try_catch_nest_depth = "<< *nrn_try_catch_nest_depth << "]" << endl;
    try {
        call_ob_proc(vec, contains, 0);
    } catch (...) {
        cout << "Uh oh. An error occurred." << endl;
    }
    (*nrn_try_catch_nest_depth)--;


    /***************************
     * incorrectly call contains (not enough args)
     **************************/
    (*nrn_try_catch_nest_depth)++;
    cout << "vec.contains();  [nrn_try_catch_nest_depth = "<< *nrn_try_catch_nest_depth << "]" << endl;
    try {
        call_ob_proc(vec, contains, 0);
    } catch (...) {
        cout << "Uh oh. An error occurred." << endl;
    }
    (*nrn_try_catch_nest_depth)--;

    /***************************
     * correctly call contains
     **************************/
    hoc_pushx(3);
    call_ob_proc(vec, contains, 1);
    cout << "vec.contains(3) = " << hoc_xpop() << endl;

    /***************************
     * create new Vector and correctly call contains
     **************************/

    sym = hoc_lookup("Vector");
    cout << "vec2 = Vector()" << endl;
    auto vec2 = hoc_newobj1(sym, 0);

    hoc_pushx(3);
    call_ob_proc(vec2, contains, 1);
    cout << "vec2.contains(3) = " << hoc_xpop() << endl;

    hoc_oc(
        "objref veclist\n"
        "veclist = new List(\"Vector\")\n"
        "print \"Number of Vectors: \", veclist.count()\n"
    );

    cout << "hoc_obj_unref(vec)" << endl;
    hoc_obj_unref(vec);

    hoc_oc(
        "print \"Number of Vectors: \", veclist.count(), \"<--- question: why is vec still around????\"\n"
    );

    cout << "hoc_obj_unref(vec)" << endl;
    hoc_obj_unref(vec);

    hoc_oc(
        "print \"Number of Vectors: \", veclist.count()\n"
    );

    //hoc_pushx(3);
    (*nrn_try_catch_nest_depth)++;
    cout << "vec2.contains();  [nrn_try_catch_nest_depth = "<< *nrn_try_catch_nest_depth << "] <--- question: this segfaults; why????" << endl;
    try {
        call_ob_proc(vec2, contains, 0);
    } catch (...) {
        cout << "Uh oh. An error occurred." << endl;
    }
    (*nrn_try_catch_nest_depth)--;
}
