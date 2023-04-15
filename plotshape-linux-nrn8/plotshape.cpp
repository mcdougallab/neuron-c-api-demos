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
static vsecptr_function section_unref;
static vitemptr_function hoc_l_delete;
static hoc_Item** section_list;


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

int nseg(Section* sec) {
    // always one more node than nseg
    return sec->nnode - 1;
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



void print_3d_points_and_segs(Section* sec, const char* varname) {
    double my_nseg = nseg(sec);
    Symbol* v = hoc_lookup(varname);
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
        cout << "    " << secname(sec) << "(" << x << ")." << varname << " = " << *nrn_rangepointer(sec, v, x) << endl;
    }
    cout << endl;
}


void display_info(hoc_Item** my_section_list, const char* varname) {
    // note: we have a circularly linked list with a special node at start/stop
    for (auto list_ptr = (*my_section_list)->next; list_ptr != *my_section_list; list_ptr=list_ptr->next) {
        Section* sec = list_ptr->element.sec;
        // check to make sure the section has properties
        // missing properties means that the section has been invalidated
        // but not removed
        if (sec->prop) {
            // we've found a real section, now let's do something
            print_3d_points_and_segs(sec, varname);
        } else {
            // invalidated section; we can remove it from the section_list and unref it
            // it would be mostly harmless to skip this, but NEURON does not routinely
            // search for and remove sections
            hoc_l_delete(list_ptr);
            section_unref(sec);
        }
    }
}

void display_plotshape_info(Object* ps) {
    ShapePlotInterface* spi;
    hoc_Item** my_section_list;
    spi = ((ShapePlotInterface*) ps->u.this_pointer);
    cout << "ps.low = " << spi->low() << endl;
    cout << "ps.high = " << spi->high() << endl;
    cout << "ps.varname = \"" << spi->varname() << "\"" << endl;
    Object* sl = spi->neuron_section_list();
    if (sl) {
        my_section_list = (hoc_Item**) &sl->u.this_pointer;
    } else {
        // no section list specified so use the global all sections list
        my_section_list = section_list;
    }
    display_info(my_section_list, spi->varname());
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

    section_unref = (vsecptr_function) dlsym(handle, "_Z13section_unrefP7Section");
    assert(section_unref);

    hoc_l_delete = (vitemptr_function) dlsym(handle, "_Z12hoc_l_deleteP8hoc_Item");
    assert(hoc_l_delete);

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

    auto call_ob_proc = (voptrsptri_function) dlsym(handle, "hoc_call_ob_proc");
    assert(call_ob_proc);

    auto hoc_pushstr = (vcptrptr_function) dlsym(handle, "hoc_pushstr");
    assert(hoc_pushstr);

    auto hoc_push_object = (voptr_function) dlsym(handle, "hoc_push_object");
    assert(hoc_push_object);

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

    // creating sections -- here we'll define our shape topologically and let 
    // NEURON pick consistent 3d coordinates
    auto main = new_section("main");
    auto branch1 = new_section("branch1");
    auto branch2 = new_section("branch2");

    set_diameter(main, 5);
    set_diameter(branch1, 2);
    set_diameter(branch2, 2);
    set_length(main, 30);
    set_length(branch1, 10);
    set_length(branch2, 20);

    // connect the beginning of each branch to the end of main
    connect(branch1, 0, main, 1);
    connect(branch2, 0, main, 1);    

    // temporarily split main into 2 segments
    nrn_change_nseg(main, 2);

    // let's have it make a consistent morphology
    hoc_call_func(hoc_lookup("define_shape"), 0);

    // split main into 5 segments
    nrn_change_nseg(main, 5);


    cout << "Topology:" << endl;
    hoc_call_func(hoc_lookup("topology"), 0);
 
    cout << "*** PlotShape experiment 1 (no SectionList specified; so all... plotting diam from -1 to 6) ***" << endl;

    // create a PlotShape with simply a flag not to show anything
    // in Python: ps = h.PlotShape(False)
    hoc_pushx(0);
    auto ps = hoc_newobj1(hoc_lookup("PlotShape"), 1);

    // ps.variable("diam")
    sym2 = hoc_table_lookup("variable", ps->ctemplate->symtable);
    assert(sym2);
    char* varname_ptr = new char[5];
    strcpy(varname_ptr, "diam");
    hoc_pushstr(&varname_ptr);    
    call_ob_proc(ps, sym2, 1);  // last argument is narg

    // in Python: ps.scale(-1, 6)
    sym2 = hoc_table_lookup("scale", ps->ctemplate->symtable);
    assert(sym2);
    hoc_pushx(-1);
    hoc_pushx(6);
    call_ob_proc(ps, sym2, 2);  // last argument is narg


    display_plotshape_info(ps);

    // create a section list called my_sec_list with main and branch2 (but not branch1)
    auto my_sec_list = hoc_newobj1(hoc_lookup("SectionList"), 0);
    nrn_pushsec(main);
    sym2 = hoc_table_lookup("append", my_sec_list->ctemplate->symtable);
    call_ob_proc(my_sec_list, sym2, 0);
    nrn_sec_pop();
    nrn_pushsec(branch2);
    call_ob_proc(my_sec_list, sym2, 0);
    nrn_sec_pop();

    cout << "*** PlotShape experiment 2 (smaller SectionList, default plotting v from 0 to 0) ***" << endl;

    // ps2 = h.PlotShape(my_sec_list, False)
    hoc_push_object(my_sec_list);
    hoc_pushx(0);
    auto ps2 = hoc_newobj1(hoc_lookup("PlotShape"), 2);

    display_plotshape_info(ps2);

}
