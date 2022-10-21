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
vv_function simpleconnectsection;
vsecptr_function nrn_pushsec;
secptrv_function nrn_sec_pop;
cptrsecptr_function secname;
dv_function hoc_xpop;
iv_function hoc_ipop;
dptrsecptrsptrd_function nrn_rangepointer;

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
    hoc_install_object_data_index(symbol);
    new_sections(nullptr, symbol, pitm, 1);
    return (*pitm)->element.sec;
}


void set_pp_property(Object* pp, const char* name, double value) {
    int index = hoc_table_lookup(name, pp->ctemplate->symtable)->u.rng.index;
    ob2pntproc_0(pp)->prop->param[index] = value;
}

void pt3dadd(Section* sec, double x, double y, double z, double diam) {
    nrn_pushsec(sec);
    hoc_pushx(x);
    hoc_pushx(y);
    hoc_pushx(z);
    hoc_pushx(diam);
    hoc_call_func(hoc_lookup("pt3dadd"), 4);
    nrn_sec_pop();
}

int nseg(Section* sec) {
    // always one more node than nseg
    return sec->nnode - 1;
}

void connect(Section* child_sec, double child_x, Section* parent_sec, double parent_x) {
    nrn_pushsec(child_sec);
    hoc_pushx(child_x);
    nrn_pushsec(parent_sec);
    hoc_pushx(parent_x);
    simpleconnectsection();
}

int n3d(Section* sec) {
    // would prefer to do this through the regular stack interface, but got a
    // stack underflow when I tried
    return sec->npt3d;
}

double x3d(Section* sec, int i) {
    // return the x coordinate of the ith 3d point
    // would prefer to do this through the regular stack interface, but got a
    // stack underflow when I tried
    return sec->pt3d[i].x;
}

double y3d(Section* sec, int i) {
    // return the x coordinate of the ith 3d point
    // would prefer to do this through the regular stack interface, but got a
    // stack underflow when I tried
    return sec->pt3d[i].y;
}
double z3d(Section* sec, int i) {
    // return the x coordinate of the ith 3d point
    // would prefer to do this through the regular stack interface, but got a
    // stack underflow when I tried
    return sec->pt3d[i].z;
}
double diam3d(Section* sec, int i) {
    // return the x coordinate of the ith 3d point
    // would prefer to do this through the regular stack interface, but got a
    // stack underflow when I tried
    return sec->pt3d[i].d;
}


void print_3d_points(Section* sec) {
    cout << secname(sec) << " has " << nseg(sec) << " segments and " << n3d(sec) << " 3d points:" << endl;
    for (auto i = 0; i < n3d(sec); i++) {
        cout << "    (" << x3d(sec, i) << ", " << y3d(sec, i) << ", "<< z3d(sec, i) 
             << "; "<< diam3d(sec, i) << ")" << endl;
    }
    // TODO: demo using e.g. nrn_rangepointer(axon, hoc_lookup("v"), 0.5) to grab each segments voltages
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

    nrn_rangepointer = (dptrsecptrsptrd_function) dlsym(handle, "_Z16nrn_rangepointerP7SectionP6Symbold");
    assert(nrn_rangepointer);

    mech_insert1 = (vsecptri_function) dlsym(handle, "_Z12mech_insert1P7Sectioni");
    assert(mech_insert1);

    ob2pntproc_0 = (ppoptr_function) dlsym(handle, "ob2pntproc_0");
    assert(ob2pntproc_0);

    simpleconnectsection = (vv_function) dlsym(handle, "_Z20simpleconnectsectionv");
    assert(simpleconnectsection);

    nrn_pushsec = (vsecptr_function) dlsym(handle, "nrn_pushsec");
    assert(nrn_pushsec);

    nrn_sec_pop = (secptrv_function) dlsym(handle, "_Z11nrn_sec_popv");
    assert(nrn_sec_pop);

    auto nrn_change_nseg = (vsecptri_function) dlsym(handle, "_Z15nrn_change_nsegP7Sectioni");
    assert(nrn_change_nseg);

    secname = (cptrsecptr_function) dlsym(handle, "_Z7secnameP7Section");
    assert(secname);

    hoc_xpop = (dv_function) dlsym(handle, "hoc_xpop");
    assert(hoc_xpop);
    
    hoc_ipop = (iv_function) dlsym(handle, "hoc_ipop");
    assert(hoc_ipop);

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
     * creating Sections
     **************************/
    auto main = new_section("main");
    auto branch1 = new_section("branch1");
    auto branch2 = new_section("branch2");

    cout << "Initial topology:" << endl;
    hoc_call_func(hoc_lookup("topology"), 0);

    nrn_change_nseg(main, 3);
    cout << endl << "Topology after splitting main into 3 segments" << endl;
    hoc_call_func(hoc_lookup("topology"), 0);

    cout << "main.nseg(): " << nseg(main) << endl;
    cout << "branch1.nseg(): " << nseg(branch1) << endl;
    cout << "branch2.nseg(): " << nseg(branch2) << endl;

    // connect the beginning of each branch to the end of main
    connect(branch1, 0, main, 1);
    connect(branch2, 0, main, 1);

    cout << endl << "Topology after connecting:" << endl;
    hoc_call_func(hoc_lookup("topology"), 0);

    // add some 3D points to main (note: not cylinders, not related to nseg)
    pt3dadd(main, 1, 2, 3, 4);
    pt3dadd(main, 21, 2, 3, 1);

    // using HOC to set abstract morphology info for branches
    nrn_pushsec(branch1);
    hoc_oc("L = 10");
    hoc_oc("diam = 1");
    nrn_sec_pop();

    nrn_pushsec(branch2);
    hoc_oc("L = 10");
    hoc_oc("diam = 1");
    nrn_sec_pop();

    // constructing 3D points for everything remaining (the branches)
    hoc_call_func(hoc_lookup("define_shape"), 0);

    cout << endl << "Topology after setting morphology (should be unchanged):" << endl;
    hoc_call_func(hoc_lookup("topology"), 0); 

    print_3d_points(main);
    print_3d_points(branch1);
    print_3d_points(branch2);
}