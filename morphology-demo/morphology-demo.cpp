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
optrsptri_function hoc_newobj1;
scptrslptr_function hoc_table_lookup;
vd_function hoc_pushx;
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
dptrsecptrsptrd_function nrn_rangepointer;
int* diam_changed;
vsecptrd_function nrn_length_change;
nptrsecptrd_function node_exact;



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

void set_length(Section* sec, double length) {
    // in NEURON code, there's also a check for can_change_morph(sec)... that checks pt3dconst_
    sec->prop->dparam[2].val = length;
    // dparam[7].val is for Ra
    // nrn_length_change updates 3D points if needed
    nrn_length_change(sec, length);
    *diam_changed = 1;
    sec->recalc_area_ = 1;
}

void set_node_diam(Node* node, double diam) {
    // TODO: this is fine if no 3D points; does it work if there are 3D points?
    for (auto prop = node->prop; prop; prop=prop->next) {
        if (prop->_type == MORPHOLOGY) {
            prop->param[0] = diam;
            *diam_changed = 1;
            node->sec->recalc_area_ = 1;
            break;
        }
    }
}

void set_diameter(Section* sec, double diam) {
    double my_nseg = nseg(sec);
    // grab each node (segment), then set the diam there
    for (auto i = 0; i < my_nseg; i++) {
        double x = (i + 0.5) / my_nseg;
        Node* node = node_exact(sec, x);
        set_node_diam(node, diam);
    }
}


void print_3d_points_and_segs(Section* sec) {
    double my_nseg = nseg(sec);
    Symbol* v = hoc_lookup("v");
    cout << secname(sec) << " has " << nseg(sec) << " segments and " << n3d(sec) << " 3d points:" << endl;
    // print out 3D points
    for (auto i = 0; i < n3d(sec); i++) {
        cout << "    (" << x3d(sec, i) << ", " << y3d(sec, i) << ", "<< z3d(sec, i) 
             << "; "<< diam3d(sec, i) << ")" << endl;
    }
    // print out membrane potential for each segment
    // grab each node (segment), then set the diam there
    for (auto i = 0; i < my_nseg; i++) {
        double x = (i + 0.5) / my_nseg;
        Node* node = node_exact(sec, x);
        cout << "    " << secname(sec) << "(" << x << ").v = " << *nrn_rangepointer(sec, v, x) << endl;
    }
    cout << endl;
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

    auto nrnmpi_stubs = (vv_function) dlsym(handle, "_Z12nrnmpi_stubsv");
    // no assert because may not exist if no dynamic MPI compiled in

    hoc_newobj1 = (optrsptri_function) dlsym(handle, "_Z11hoc_newobj1P6Symboli");
    assert(hoc_newobj1);

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
    
    diam_changed = (int*) dlsym(handle, "diam_changed");
    assert(diam_changed);

    nrn_length_change = (vsecptrd_function) dlsym(handle, "_Z17nrn_length_changeP7Sectiond");
    assert(nrn_length_change);

    node_exact = (nptrsecptrd_function) dlsym(handle, "node_exact");
    assert(node_exact);



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

    // creating sections
    auto main = new_section("main");
    auto branch1 = new_section("branch1");
    auto branch2 = new_section("branch2");
    vector<Section*> all_sec;
    all_sec.push_back(main);
    all_sec.push_back(branch1);
    all_sec.push_back(branch2);
    

    cout << "Initial topology:" << endl;
    hoc_call_func(hoc_lookup("topology"), 0);

    nrn_change_nseg(main, 3);
    cout << endl << "Topology after splitting main into 3 segments" << endl;
    hoc_call_func(hoc_lookup("topology"), 0);

    for (auto sec: all_sec) {
        cout << secname(sec) << ".nseg = " << nseg(sec) << endl;
    }

    // connect the beginning of each branch to the end of main
    connect(branch1, 0, main, 1);
    connect(branch2, 0, main, 1);

    cout << endl << "Topology after connecting:" << endl;
    hoc_call_func(hoc_lookup("topology"), 0);

    // add some 3D points to main (note: not cylinders, not related to nseg)
    pt3dadd(main, 1, 2, 3, 4);
    pt3dadd(main, 201, 2, 3, 1);

    // set abstract morphology info for branches (i.e. we're just setting L and
    // diam but not specifying the 3D points
    set_length(branch1, 100);
    set_diameter(branch1, 1);

    set_length(branch2, 150);
    set_diameter(branch2, 0.9);

    // constructing 3D points for everything remaining (the branches)
    hoc_call_func(hoc_lookup("define_shape"), 0);

    cout << endl << "Topology after setting morphology (should be unchanged):" << endl;
    hoc_call_func(hoc_lookup("topology"), 0); 

    // setting up a simple simulation... 
    // passive conductance everywhere, inject current at main(0)
    insert_mechanism(main, "pas");
    // stimulus at main(0)
    nrn_pushsec(main);
    hoc_pushx(0);
    auto iclamp = hoc_newobj1(hoc_lookup("IClamp"), 1);
    nrn_sec_pop();
    set_pp_property(iclamp, "del", 0);
    set_pp_property(iclamp, "dur", 10000);
    set_pp_property(iclamp, "amp", 1);

    // init and run for 10 steps
    finitialize(-65);
    auto fadvance = hoc_lookup("fadvance");
    for (auto i=0; i < 10; i++) {
        hoc_call_func(fadvance, 0);    
    }

    for (auto sec: all_sec) {
        print_3d_points_and_segs(sec);
    }
}
