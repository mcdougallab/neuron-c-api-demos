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


void finitialize(double v0) {
    hoc_pushx(v0);
    hoc_call_func(hoc_lookup("finitialize"), 1);   
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


void set_pp_property(Object* pp, const char* name, double value) {
    int index = hoc_table_lookup(name, pp->ctemplate->symtable)->u.rng.index;
    ob2pntproc_0(pp)->prop->param[index] = value;
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

    hoc_table_lookup = (scptrslptr_function) dlsym(handle, "hoc_table_lookup");
    assert(hoc_table_lookup);

    hoc_call_func = (dvptrint_function) dlsym(handle, "hoc_call_func");
    assert(hoc_call_func);

    auto hoc_oc = (icptr_function) dlsym(handle, "hoc_oc");
    assert(hoc_oc);

    hoc_pushx = (vd_function) dlsym(handle, "hoc_pushx");
    assert(hoc_pushx);

    hoc_pushpx = (vdptr_function) dlsym(handle, "hoc_pushpx");
    assert(hoc_pushpx);

    auto nrnmpi_stubs = (vv_function) dlsym(handle, "_Z12nrnmpi_stubsv");
    // no assert because may not exist if no dynamic MPI compiled in

    hoc_newobj1 = (optrsptri_function) dlsym(handle, "_Z11hoc_newobj1P6Symboli");
    assert(hoc_newobj1);

    auto vector_capacity = (ivptr_function) dlsym(handle, "vector_capacity");
    assert(vector_capacity);

    auto vector_vec = (dptrvptr_function) dlsym(handle, "vector_vec");
    assert(vector_vec);

    call_ob_proc = (voptrsptri_function) dlsym(handle, "hoc_call_ob_proc");
    assert(call_ob_proc);

    hoc_install_object_data_index = (vsptr_function) dlsym(handle, "hoc_install_object_data_index");
    assert(hoc_install_object_data_index);

    new_sections = (voptrsptritemptrptri_function) dlsym(handle, "_Z12new_sectionsP6ObjectP6SymbolPP8hoc_Itemi");
    assert(new_sections);

    auto nrn_rangepointer = (dptrsecptrsptrd_function) dlsym(handle, "_Z16nrn_rangepointerP7SectionP6Symbold");
    assert(nrn_rangepointer);

    mech_insert1 = (vsecptri_function) dlsym(handle, "_Z12mech_insert1P7Sectioni");
    assert(mech_insert1);

    ob2pntproc_0 = (ppoptr_function) dlsym(handle, "ob2pntproc_0");
    assert(ob2pntproc_0);


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
     * creating axon Section
     **************************/
    auto axon = new_section("axon");


    /***************************
     * add hh mechanism to axon
     **************************/
    insert_mechanism(axon, "hh");

    // record time and axon(0.5).v
    auto t_vec = new_vector_record(hoc_lookup("t")->u.pval);
    auto v_vec = new_vector_record(nrn_rangepointer(axon, hoc_lookup("v"), 0.5));


    /***************************
     * adding a stimulus at axon(0.5)
     **************************/
    // it's at 0.5 because of the pushx; on the axon because that's the currently accessed section
    hoc_pushx(0.5);
    auto iclamp = hoc_newobj1(hoc_lookup("IClamp"), 1);
    set_pp_property(iclamp, "del", 1);
    set_pp_property(iclamp, "dur", 1);
    set_pp_property(iclamp, "amp", 100);

    /***************************
     * finitialize(-65)
     **************************/
    finitialize(-65);

    /***************************
     * run until t = 10 ms
     **************************/
    auto t_ptr = hoc_lookup("t")->u.pval;
    // we can look up fadvance once instead of at each time
    auto fadvance = hoc_lookup("fadvance");
    while (*t_ptr < 10) {
        hoc_call_func(fadvance, 0);
    }

    /***************************
     * save results to demo2.csv
     **************************/
    double* t_vec_ptr = vector_vec(t_vec->u.this_pointer);
    double* v_vec_ptr = vector_vec(v_vec->u.this_pointer);
    ofstream out_file;
    out_file.open("demo2.csv");
    for (auto i = 0; i < vector_capacity(t_vec->u.this_pointer); i++) {
        out_file << t_vec_ptr[i] << "," << v_vec_ptr[i] << endl;
    }
    out_file.close();

    cout << "results saved to demo2.csv" << endl;
}
