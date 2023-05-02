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
static cptrsecptr_function secname;

static int* diam_changed;
static vsecptrd_function nrn_length_change;
static nptrsecptrd_function node_exact;
static vv_function simpleconnectsection;
static vsecptr_function nrn_pushsec;
static secptrv_function nrn_sec_pop;
static dptrsecptrsptrd_function nrn_rangepointer;
static vd_function hoc_pushx;
static optrsptri_function hoc_newobj1;
static hoc_Item** section_list;
static vsecptri_function mech_insert1;
static ppoptr_function ob2pntproc_0;
static optrptr_function hoc_objpop; 
static vobjptrptr_function hoc_tobj_unref;
static voptrsptri_function call_ob_proc;
static vdptr_function hoc_pushpx;
static voptr_function hoc_obj_unref;

Object* call_object_method_that_returns_object(Object* obj, char const * const name, int narg)  {
    auto sym = hoc_table_lookup(name, obj->ctemplate->symtable);
    assert(sym);
    call_ob_proc(obj, sym, narg);
    Object** obptr = hoc_objpop();
    Object* new_ob_ptr = *obptr;
    new_ob_ptr->refcount++;
    hoc_tobj_unref(obptr);
    return new_ob_ptr;
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


void finitialize(double v0) {
    hoc_pushx(v0);
    hoc_call_func(hoc_lookup("finitialize"), 1);   
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

    section_list = (hoc_Item**) dlsym(handle, "section_list");
    assert(section_list);

    secname = (cptrsecptr_function) dlsym(handle, "_Z7secnameP7Section");
    assert(secname);

    simpleconnectsection = (vv_function) dlsym(handle, "_Z20simpleconnectsectionv");
    assert(simpleconnectsection);

    nrn_pushsec = (vsecptr_function) dlsym(handle, "nrn_pushsec");
    assert(nrn_pushsec);

    nrn_sec_pop = (secptrv_function) dlsym(handle, "_Z11nrn_sec_popv");
    assert(nrn_sec_pop);

    auto nrn_change_nseg = (vsecptri_function) dlsym(handle, "_Z15nrn_change_nsegP7Sectioni");
    assert(nrn_change_nseg);

    diam_changed = (int*) dlsym(handle, "diam_changed");
    assert(diam_changed);

    nrn_length_change = (vsecptrd_function) dlsym(handle, "_Z17nrn_length_changeP7Sectiond");
    assert(nrn_length_change);

    node_exact = (nptrsecptrd_function) dlsym(handle, "node_exact");
    assert(node_exact);

    nrn_rangepointer = (dptrsecptrsptrd_function) dlsym(handle, "_Z16nrn_rangepointerP7SectionP6Symbold");
    assert(nrn_rangepointer);

    hoc_pushx = (vd_function) dlsym(handle, "hoc_pushx");
    assert(hoc_pushx);

    hoc_newobj1 = (optrsptri_function) dlsym(handle, "_Z11hoc_newobj1P6Symboli");
    assert(hoc_newobj1);

    call_ob_proc = (voptrsptri_function) dlsym(handle, "hoc_call_ob_proc");
    assert(call_ob_proc);

    auto hoc_pushstr = (vcptrptr_function) dlsym(handle, "hoc_pushstr");
    assert(hoc_pushstr);

    auto hoc_push_object = (voptr_function) dlsym(handle, "hoc_push_object");
    assert(hoc_push_object);

    ob2pntproc_0 = (ppoptr_function) dlsym(handle, "ob2pntproc_0");
    if (!ob2pntproc_0) {
        ob2pntproc_0 = (ppoptr_function) dlsym(handle, "_Z12ob2pntproc_0P6Object");
    }
    assert(ob2pntproc_0);

    hoc_objpop = (optrptr_function) dlsym(handle, "_Z10hoc_objpopv");
    if (!hoc_objpop) {
        hoc_objpop = (optrptr_function) dlsym(handle, "hoc_objpop");
    }
    assert(hoc_objpop);

    hoc_tobj_unref = (vobjptrptr_function) dlsym(handle, "_Z14hoc_tobj_unrefPP6Object");
    if (!hoc_tobj_unref) {
        hoc_tobj_unref = (vobjptrptr_function) dlsym(handle, "hoc_tobj_unref");
    }
    assert(hoc_tobj_unref);

    hoc_obj_unref = (voptr_function) dlsym(handle, "hoc_obj_unref");
    if (!hoc_obj_unref) {
        hoc_obj_unref = (voptr_function) dlsym(handle, "_Z13hoc_obj_unrefP6Object");
    }
    assert(hoc_obj_unref);

    hoc_pushpx = (vdptr_function) dlsym(handle, "hoc_pushpx");
    if (!hoc_pushpx) {
        hoc_pushpx = (vdptr_function) dlsym(handle, "_Z10hoc_pushpxPd");
    }
    assert(hoc_pushpx);

    auto hoc_xpop = (dv_function) dlsym(handle, "hoc_xpop");
    if (!hoc_xpop) {
        hoc_xpop = (dv_function) dlsym(handle, "_Z8hoc_xpopv");
    }
    assert(hoc_xpop);  

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

    /***************************
     * creating axon Section
     **************************/
    auto axon = new_section("axon");

    // record time and axon(0.5).v
    sym = hoc_lookup("Vector");
    auto t_vec = hoc_newobj1(sym, 0);
    hoc_pushpx(hoc_lookup("t")->u.pval);
    Object* tempobj = call_object_method_that_returns_object(t_vec, "record", 1);
    // note: Vector.record returns an object, so we must unref it
    hoc_obj_unref(tempobj);


    /***************************
     * finitialize(-65)
     **************************/
    finitialize(-65);

    /***************************
     * run until t = 1 ms
     * fun fact: round-off error leads to 1 extra step
     **************************/
    auto t_ptr = hoc_lookup("t")->u.pval;
    // we can look up fadvance once instead of at each time
    auto fadvance = hoc_lookup("fadvance");
    while (*t_ptr < 1) {
        hoc_call_func(fadvance, 0);
    }

    // t2 = t_vec.c()
    Object* t2 = call_object_method_that_returns_object(t_vec, "c", 0);

    // printf returns a double
    sym = hoc_table_lookup("printf", t2->ctemplate->symtable);
    assert(sym);
    call_ob_proc(t2, sym, 0);

    cout << "Printed " << hoc_xpop() << " items" << endl;
}
