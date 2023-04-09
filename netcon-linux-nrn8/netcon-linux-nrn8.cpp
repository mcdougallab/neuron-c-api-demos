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
static optrsptri_function hoc_newobj1;
static scptrslptr_function hoc_table_lookup;
static voptrsptri_function call_ob_proc;
static vd_function hoc_pushx;
static vdptr_function hoc_pushpx;
static dvptrint_function hoc_call_func;
static vsecptri_function mech_insert1;
static vsptr_function hoc_install_object_data_index;
static voptrsptritemptrptri_function new_sections;
static ppoptr_function ob2pntproc_0;
static vsptr_function hoc_pushs;
static dptrv_function hoc_pxpop;



double& pp_property(Object* pp, const char* name) {
    int index = hoc_table_lookup(name, pp->ctemplate->symtable)->u.rng.index;
    return ob2pntproc_0(pp)->prop->param[index];
}

void finitialize(double v0) {
    hoc_pushx(v0);
    hoc_call_func(hoc_lookup("finitialize"), 1);   
}


double& steered_property(Object* obj, const char* name) {
    assert(obj->ctemplate->steer);
    auto sym2 = hoc_table_lookup(name, obj->ctemplate->symtable);
    assert(sym2);
    hoc_pushs(sym2);
    // put the pointer for the memory location on the stack 
    obj->ctemplate->steer(obj->u.this_pointer);
    return *hoc_pxpop();
}

void insert_mechanism(Section* sec, const char* mech_name) {
    auto sym = hoc_lookup(mech_name);
    assert(sym);
    // the type indicates that it's a mechanism; the subtype indicates which
    mech_insert1(sec, sym->subtype);
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


Object* new_vector_record(double* record) {
    // create a new Vector
    auto sym = hoc_lookup("Vector");
    auto vec = hoc_newobj1(sym, 0);

    hoc_pushpx(record);
    auto sym2 = hoc_table_lookup("record", vec->ctemplate->symtable);
    assert(sym2);
    call_ob_proc(vec, sym2, 1);
    return vec;
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

    hoc_pushx = (vd_function) dlsym(handle, "hoc_pushx");
    if (!hoc_pushx) {
        hoc_pushx = (vd_function) dlsym(handle, "_Z9hoc_pushxd");
    }
    assert(hoc_pushx);

    hoc_pushpx = (vdptr_function) dlsym(handle, "hoc_pushpx");
    if (!hoc_pushpx) {
        hoc_pushpx = (vdptr_function) dlsym(handle, "_Z10hoc_pushpxPd");
    }
    assert(hoc_pushpx);

    mech_insert1 = (vsecptri_function) dlsym(handle, "_Z12mech_insert1P7Sectioni");
    assert(mech_insert1);

    auto hoc_push_object = (voptr_function) dlsym(handle, "hoc_push_object");
    assert(hoc_push_object);

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

    new_sections = (voptrsptritemptrptri_function) dlsym(handle, "_Z12new_sectionsP6ObjectP6SymbolPP8hoc_Itemi");
    assert(new_sections);

    hoc_install_object_data_index = (vsptr_function) dlsym(handle, "hoc_install_object_data_index");
    if (!hoc_install_object_data_index) {
        hoc_install_object_data_index = (vsptr_function) dlsym(handle, "_Z29hoc_install_object_data_indexP6Symbol");
    }
    assert(hoc_install_object_data_index);

    ob2pntproc_0 = (ppoptr_function) dlsym(handle, "ob2pntproc_0");
    if (!ob2pntproc_0) {
        ob2pntproc_0 = (ppoptr_function) dlsym(handle, "_Z12ob2pntproc_0P6Object");
    }
    assert(ob2pntproc_0);

    auto vector_capacity = (ivptr_function) dlsym(handle, "vector_capacity");
    assert(vector_capacity);

    auto vector_vec = (dptrvptr_function) dlsym(handle, "vector_vec");
    assert(vector_vec);

    hoc_pushs = (vsptr_function) dlsym(handle, "hoc_pushs");
    assert(hoc_pushs);

    hoc_pxpop = (dptrv_function) dlsym(handle, "hoc_pxpop");
    assert(hoc_pxpop);

    auto nrn_rangepointer = (dptrsecptrsptrd_function) dlsym(handle, "_Z16nrn_rangepointerP7SectionP6Symbold");
    assert(nrn_rangepointer);


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

    // vec = h.Vector()
    sym = hoc_lookup("Vector");
    auto vec = hoc_newobj1(sym, 0);

    // ns = h.NetStim()
    sym = hoc_lookup("NetStim");
    auto ns = hoc_newobj1(sym, 0);
    pp_property(ns, "start") = 5;     // 5 ms
    pp_property(ns, "noise") = 1;     // use a Poisson process
    pp_property(ns, "interval") = 5;  // ms on average
    pp_property(ns, "number") = 10;

    // soma = h.Section(name="soma")
    auto soma = new_section("soma");

    // soma.insert("hh")
    insert_mechanism(soma, "hh");


    // syn = h.ExpSyn(soma(0.5))
    hoc_pushx(0.5);
    auto syn = hoc_newobj1(hoc_lookup("ExpSyn"), 1);
    pp_property(syn, "tau") = 3;  // 3 ms timeconstant
    pp_property(syn, "e") = 0;    // 0 mV reversal potential (excitatory synapse)

    // nc = h.NetCon(ns, syn)
    hoc_push_object(ns);
    hoc_push_object(syn);
    auto nc = hoc_newobj1(hoc_lookup("NetCon"), 2);

    // nc.record(vec)
    hoc_push_object(vec);
    sym2 = hoc_table_lookup("record", nc->ctemplate->symtable);
    assert(sym2);
    call_ob_proc(nc, sym2, 1);

    // nc.weight[0] = 0.5
    steered_property(nc, "weight") = 0.5;
    // nc.delay = 0
    steered_property(nc, "delay") = 0;


    // record time and soma(0.5).v
    auto t_vec = new_vector_record(hoc_lookup("t")->u.pval);
    auto v_vec = new_vector_record(nrn_rangepointer(soma, hoc_lookup("v"), 0.5));


    finitialize(-65);

    /***************************
     * run until t = 100 ms
     **************************/
    auto t_ptr = hoc_lookup("t")->u.pval;
    // we can look up fadvance once instead of at each time
    auto fadvance = hoc_lookup("fadvance");
    while (*t_ptr < 100) {
        hoc_call_func(fadvance, 0);
    }

    cout << "random times from NetStim: ";
    double* vec_ptr = vector_vec(vec->u.this_pointer);
    for (auto i = 0; i < vector_capacity(vec->u.this_pointer); i++) {
        cout << vec_ptr[i] << "  ";
    }
    cout << endl;


    double* t_vec_ptr = vector_vec(t_vec->u.this_pointer);
    double* v_vec_ptr = vector_vec(v_vec->u.this_pointer);
    ofstream out_file;
    out_file.open("netcon.csv");
    out_file << "t,v" << endl;
    for (auto i = 0; i < vector_capacity(t_vec->u.this_pointer); i++) {
        out_file << t_vec_ptr[i] << "," << v_vec_ptr[i] << endl;
    }
    out_file.close();

    cout << "results saved to netcon.csv" << endl;
    cout << "visualize via e.g. python plot_it.py" << endl; 
}
