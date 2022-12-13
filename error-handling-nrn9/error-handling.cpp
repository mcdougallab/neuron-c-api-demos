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

typedef void (*hoc_oop_ss)(Object**, Objectdata**, int*, Symlist**);
typedef void (*code_ss)(Inst**, Inst**, std::size_t&, void**, int*, int*, Inst**, void**, std::size_t&, Symlist**, Inst**, int*);
typedef void (*input_info_ss)(const char**, int*, int*, void**);
typedef void (*input_info_rs)(const char*, int, int, void*);
typedef void (*cabcode_ss)(int*, int*);

hoc_oop_ss oc_save_hoc_oop, oc_restore_hoc_oop;
code_ss oc_save_code, oc_restore_code;
input_info_ss oc_save_input_info;
input_info_rs oc_restore_input_info;
cabcode_ss oc_save_cabcode, oc_restore_cabcode;

// adapted from ocjump.cpp
class SavedState {
    public:
        SavedState() {
            // not complete but it is good for expressions and it can be improved
            oc_save_hoc_oop(&o1, &o2, &o4, &o5);
            oc_save_code(&c1, &c2, c3, &c4, &c5, &c6, &c7, &c8, c9, &c10, &c11, &c12);
            oc_save_input_info(&i1, &i2, &i3, &i4);
            oc_save_cabcode(&cc1, &cc2);
        }

        void restore() {
            oc_restore_hoc_oop(&o1, &o2, &o4, &o5);
            oc_restore_code(&c1, &c2, c3, &c4, &c5, &c6, &c7, &c8, c9, &c10, &c11, &c12);
            oc_restore_input_info(i1, i2, i3, i4);
            oc_restore_cabcode(&cc1, &cc2);
        }

    private:
        // hoc_oop
        Object* o1;
        Objectdata* o2;
        int o4;
        Symlist* o5;

        // code
        Inst* c1;
        Inst* c2;
        std::size_t c3;
        void* c4;
        int c5;
        int c6;
        Inst* c7;
        void* c8;
        std::size_t c9;
        Symlist* c10;
        Inst* c11;
        int c12;

        // input_info
        const char* i1;
        int i2;
        int i3;
        void* i4;

        // cabcode
        int cc1;
        int cc2;
};

int main(void) {
    Symbol* sym;
    char* error;
    int oboff;
    void* handle = dlopen("libnrniv.dylib", RTLD_NOW | RTLD_LOCAL); 
    SavedState* state;
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

    oc_save_hoc_oop = (hoc_oop_ss) dlsym(handle, "_Z15oc_save_hoc_oopPP6ObjectPP10ObjectdataPiPP7Symlist");
    assert(oc_save_hoc_oop);

    oc_restore_hoc_oop = (hoc_oop_ss) dlsym(handle, "_Z18oc_restore_hoc_oopPP6ObjectPP10ObjectdataPiPP7Symlist");
    assert(oc_restore_hoc_oop);

    oc_save_code = (code_ss) dlsym(handle, "_Z12oc_save_codePP4InstS1_RmPPN3nrn2oc5frameEPiS8_S1_S7_S2_PP7SymlistS1_S8_");
    assert(oc_save_code);

    oc_restore_code = (code_ss) dlsym(handle, "_Z15oc_restore_codePP4InstS1_RmPPN3nrn2oc5frameEPiS8_S1_S7_S2_PP7SymlistS1_S8_");
    assert(oc_restore_code);

    oc_save_input_info = (input_info_ss) dlsym(handle, "_Z18oc_save_input_infoPPKcPiS2_PP7__sFILE");
    assert(oc_save_input_info);

    oc_restore_input_info = (input_info_rs) dlsym(handle, "_Z21oc_restore_input_infoPKciiP7__sFILE");
    assert(oc_restore_input_info);

    oc_save_cabcode = (cabcode_ss) dlsym(handle, "_Z15oc_save_cabcodePiS_");
    assert(oc_save_cabcode);

    oc_restore_cabcode = (cabcode_ss) dlsym(handle, "_Z18oc_restore_cabcodePiS_");
    assert(oc_restore_cabcode);


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

    cout << "vec->refcount: " << vec->refcount << endl;

    /***************************
     * call vec.indgen()... leaves vec: 0, 1, 2, 3, 4
     **************************/
    auto indgen = hoc_table_lookup("indgen", vec->ctemplate->symtable);
    cout << "vec.indgen()" << endl;
    call_ob_proc(vec, indgen, 0);
    cout << "vec->refcount: " << vec->refcount << " <-- question: why did the refcount increase here but not below?" << endl;

    /***************************
     * correctly call contains
     **************************/
    auto contains = hoc_table_lookup("contains", vec->ctemplate->symtable);
    for (auto i=0; i < 10; i++) {
        hoc_pushx(i);
        call_ob_proc(vec, contains, 1);
        cout << "vec.contains(" << i << ") = " << hoc_xpop() << endl;
    }
    cout << "vec->refcount: " << vec->refcount << endl;

    /***************************
     * incorrectly call contains (not enough args)
     **************************/
    (*nrn_try_catch_nest_depth)++;
    cout << "vec.contains();  [nrn_try_catch_nest_depth = "<< *nrn_try_catch_nest_depth << "]" << endl;
    state = new SavedState();
    try {
        call_ob_proc(vec, contains, 0);
    } catch (...) {
        state->restore();
        cout << "Uh oh. An error occurred." << endl;
        cout << "vec->refcount: " << vec->refcount << endl;
    }
    delete state;

    (*nrn_try_catch_nest_depth)--;


    /***************************
     * incorrectly call contains (not enough args)
     **************************/
    (*nrn_try_catch_nest_depth)++;
    cout << "vec.contains();  [nrn_try_catch_nest_depth = "<< *nrn_try_catch_nest_depth << "]" << endl;
    state = new SavedState();
    try {
        call_ob_proc(vec, contains, 0);
    } catch (...) {
        state->restore();
        cout << "Uh oh. An error occurred." << endl;
        cout << "vec->refcount: " << vec->refcount << endl;
    }
    delete state;
    (*nrn_try_catch_nest_depth)--;

    /***************************
     * correctly call contains
     **************************/
    hoc_pushx(3);
    call_ob_proc(vec, contains, 1);
    cout << "vec.contains(3) = " << hoc_xpop() << endl;
    cout << "vec->refcount: " << vec->refcount << endl;

    /***************************
     * create new Vector and correctly call contains
     **************************/

    sym = hoc_lookup("Vector");
    cout << "vec2 = Vector()" << endl;
    auto vec2 = hoc_newobj1(sym, 0);

    hoc_pushx(3);
    call_ob_proc(vec2, contains, 1);
    cout << "vec2.contains(3) = " << hoc_xpop() << endl;

    state = new SavedState();
    try {
        cout << "vec2.contains()" << endl;
        call_ob_proc(vec2, contains, 0);
    } catch (...) {
        state->restore();
        cout << "Uh oh. An error occurred." << endl;
    }
    delete state;

    hoc_oc(
        "objref veclist\n"
        "veclist = new List(\"Vector\")\n"
        "print \"Number of Vectors: \", veclist.count()\n"
    );

    cout << "vec->refcount: " << vec->refcount << endl;

    cout << "hoc_obj_unref(vec)" << endl;
    hoc_obj_unref(vec);
    cout << "vec->refcount: " << vec->refcount << endl;


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
    cout << "vec2.contains();  [nrn_try_catch_nest_depth = "<< *nrn_try_catch_nest_depth << "]" << endl;
    state = new SavedState();
    try {
        call_ob_proc(vec2, contains, 0);
    } catch (...) {
        state->restore();
        cout << "Uh oh. An error occurred." << endl;
    }
    delete state;
    (*nrn_try_catch_nest_depth)--;
}
