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
using std::string;

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
nptrsecptrd_function node_exact;
dptrsecptrsptrd_function nrn_rangepointer;
cptrsecptr_function secname;


Section* my_section;

Section* new_section(const string name) {
    Symbol* symbol = new Symbol;
    auto pitm = new hoc_Item*;
    char* name_ptr = new char[name.length() + 1];
    strcpy(name_ptr, name.c_str());
    symbol->name = name_ptr;
    symbol->type = 1;
    symbol->u.oboff = 0;
    symbol->arayinfo = 0;
    hoc_install_object_data_index(symbol);
    new_sections(nullptr, symbol, pitm, 1);
    return (*pitm)->element.sec;
}


void finitialize(const double v0) {
    hoc_pushx(v0);
    hoc_call_func(hoc_lookup("finitialize"), 1);   
}

int nseg(Section const * const sec) {
    // always one more node than nseg
    return sec->nnode - 1;
}


void test_callback(void) {
    cout << "Hello from C++" << endl;
    // must call hoc_ret... to tell the stack we're returning a value
    hoc_ret();
    // return value must be pushed onto the stack
    hoc_pushx(42);
}

void new_finitialize_rule(void) {
    Symbol* const v = hoc_lookup("v");
    // set my_section(0.0.833333).v = 47
    *nrn_rangepointer(my_section, v, 0.833333) = 47;
    hoc_ret();
    hoc_pushx(0);
}


void print_seg_v(Section* const sec) {
    double const my_nseg = nseg(sec);
    Symbol* const v = hoc_lookup("v");
    for (auto i = 0; i < my_nseg; i++) {
        const double x = (i + 0.5) / my_nseg;
        Node* node = node_exact(sec, x);
        cout << "    " << secname(sec) << "(" << x << ").v = " << *nrn_rangepointer(sec, v, x) << endl;
    }
    cout << endl;
}



int main(void) {
    string my_generated_name = "new_finitialize_rule";

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

    nrn_rangepointer = (dptrsecptrsptrd_function) dlsym(handle, "_Z16nrn_rangepointerP7SectionP6Symbold");
    assert(nrn_rangepointer);

    secname = (cptrsecptr_function) dlsym(handle, "_Z7secnameP7Section");
    assert(secname);

    hoc_install_object_data_index = (vsptr_function) dlsym(handle, "hoc_install_object_data_index");
    if (!hoc_install_object_data_index) {
        hoc_install_object_data_index = (vsptr_function) dlsym(handle, "_Z29hoc_install_object_data_indexP6Symbol");
    }
    assert(hoc_install_object_data_index);

    auto nrn_change_nseg = (vsecptri_function) dlsym(handle, "_Z15nrn_change_nsegP7Sectioni");
    assert(nrn_change_nseg);

    hoc_call_func = (dvptrint_function) dlsym(handle, "hoc_call_func");
    if (!hoc_call_func) {
        hoc_call_func = (dvptrint_function) dlsym(handle, "_Z13hoc_call_funcP6Symboli");

    }
    assert(hoc_call_func);

    node_exact = (nptrsecptrd_function) dlsym(handle, "node_exact");
    if (!node_exact) {
        node_exact = (nptrsecptrd_function) dlsym(handle, "_Z10node_exactP7Sectiond");
    }
    assert(node_exact);

    auto hoc_pushstr = (vcptrptr_function) dlsym(handle, "hoc_pushstr");
    if (!hoc_pushstr) {
        hoc_pushstr = (vcptrptr_function) dlsym(handle, "_Z11hoc_pushstrPPc");
    }
    assert(hoc_pushstr);

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


    // register the function test_callback with NEURON
    sym = hoc_install("test_callback", FUNCTION_TYPE, 0, hoc_top_level_symlist);
    cout << "populating the symbol data" << endl;
    sym->u.u_proc->defn.pf = test_callback;
    sym->u.u_proc->nauto = 0;  // total number of local variables; always 0 for C++ function
    sym->u.u_proc->nobjauto = 0;

    // register the function new_finitialize_rule with NEURON
    char* my_generated_name_char = const_cast<char*>(my_generated_name.c_str());
    sym = hoc_install(my_generated_name_char, FUNCTION_TYPE, 0, hoc_top_level_symlist);
    sym->u.u_proc->defn.pf = new_finitialize_rule;
    sym->u.u_proc->nauto = 0;  // total number of local variables; always 0 for C++ function
    sym->u.u_proc->nobjauto = 0;



    cout << "attempting to have HOC call our function" << endl;

    hoc_oc("{value=test_callback()}");
    hoc_oc("print \"received value: \", value");

    /***************************
     * creating Section
     **************************/
    my_section = new_section("my_section");
    nrn_change_nseg(my_section, 3);
    
    cout << "Normal finitialize(-65) behavior:" << endl;
    // show the normal finitialize behavior
    finitialize(-65);
    print_seg_v(my_section);

    cout << "Creating an FInitializeHandler" << endl;
    // in Python (calling a HOC function): fih = h.FInitializeHandler("new_finitialize_rule()")
    // there is surely a better way to get the necessary char**
    string command = my_generated_name;
    command += "()";
    char* command_c = const_cast<char*>(command.c_str());
    hoc_pushstr(&command_c);
    auto ps = hoc_newobj1(hoc_lookup("FInitializeHandler"), 1);  // 1 for 1 argument

    cout << "At this point, membrane potentials are unchanged:" << endl;
    print_seg_v(my_section);


    cout << "Now running finitialize(-65) and printing:" << endl;
    finitialize(-65);
    print_seg_v(my_section);

    cout << "(The difference is that new_finitialize_rule set the last segment's value to something else.)" << endl;

}
