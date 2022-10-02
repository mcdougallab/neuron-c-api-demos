#include <dlfcn.h>
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include "../neuron_api_headers.h"

using std::cout;
using std::endl;
using std::exit;

typedef void (*initer_function)(int, const char**, const char**, int);
typedef void (*vd_function)(double);
typedef void (*vv_function)(void);
typedef int64_t (*icptr_function)(const char*);
typedef void* (*vcptr_function)(const char*);
typedef Symbol* (*scptr_function)(const char*);
typedef double (*dvptrint_function) (void*, int);
typedef Symbol* (*scptroptr_function) (char*, Object*);
typedef double (*dsio_function) (Symbol*, int, Object*);
typedef Symbol* (*scptrslptr_function) (const char*, Symlist*);
typedef Object* (*optrsptri_function) (Symbol*, int);
typedef void (*voptr_function) (Object*);
typedef void (*vf2icif_function)(int (*)(int, char*), int(*)());
typedef int (*ivptr_function)(void*);
typedef double* (*dptrvptr_function)(void*);
typedef double (*dv_function)(void);
typedef void (*voptrsptri_function)(Object*, Symbol*, int);
typedef void (*vcptrptr_function)(char**);

static const char* argv[] = {"nrn_test", "-nogui", "-nopython", NULL};

scptr_function hoc_lookup;

extern "C" void modl_reg() {};



void print_symbol_table(Symlist* table) {
    for (Symbol* sp = table->first; sp != NULL; sp = sp->next) {
        // type distinguishes methods from properties, return type
        cout << sp->name << " (" << sp->type << ")";
        if (sp->next != NULL) {
            cout << ", ";
        }
    }
    cout << endl;
}



void print_class_methods(const char* class_name) {
    auto sym = hoc_lookup(class_name);
    assert(sym);
    cout << sym->name << " properties and methods:" << endl;
    print_symbol_table(sym->u.ctemplate->symtable);
}


int myprint(int stream, char* msg) {
    // stream = 1 for stdout; otherwise stderr
    // here we're just prepending an ">" to indicate that it's from us
    cout << "> " << msg;
    return 0;
}

void register_print_function(void* handle, int (*print_function)(int, char*)) {
    // the print_function will be called for all output from NEURON
    // note: this function may be called multiple times per line

    vf2icif_function nrnpy_set_pr_etal = (vf2icif_function) dlsym(handle, "nrnpy_set_pr_etal");
    assert(nrnpy_set_pr_etal);

    int old_python_flag = *((int*) dlsym(handle, "nrn_is_python_extension"));
    *((int*) dlsym(handle, "nrn_is_python_extension")) = 1;
    nrnpy_set_pr_etal(myprint, NULL);
    *((int*) dlsym(handle, "nrn_is_python_extension")) = old_python_flag;
}

int main(void) {
    char* error;
    Symbol* sym;
    Symbol* sym2;
    void* handle = dlopen("libnrniv.dylib", RTLD_NOW | RTLD_LOCAL); 
    if (!handle) {
        cout << "Couldn't open dylib." << endl << dlerror() << endl;
        exit(-1);
    } else {
        cout << "Opened dylib" << endl;
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

    auto hoc_table_lookup = (scptrslptr_function) dlsym(handle, "hoc_table_lookup");
    assert(hoc_table_lookup);

    auto hoc_call_func = (dvptrint_function) dlsym(handle, "hoc_call_func");
    assert(hoc_call_func);

    auto hoc_oc = (icptr_function) dlsym(handle, "hoc_oc");
    assert(hoc_oc);

    auto hoc_pushx = (vd_function) dlsym(handle, "hoc_pushx");
    assert(hoc_pushx);

    auto hoc_ret = (vv_function) dlsym(handle, "hoc_ret");
    assert(hoc_ret);

    auto hoc_call_objfunc = (dsio_function) dlsym(handle, "hoc_call_objfunc");
    assert(hoc_call_objfunc);

    auto hoc_objectdata = (Objectdata*) dlsym(handle, "hoc_objectdata");
    assert(hoc_objectdata);

    auto nrnmpi_stubs = (vv_function) dlsym(handle, "_Z12nrnmpi_stubsv");
    // no assert because may not exist if no dynamic MPI compiled in

    auto hoc_newobj1 = (optrsptri_function) dlsym(handle, "_Z11hoc_newobj1P6Symboli");
    assert(hoc_newobj1);

    auto hoc_obj_ref = (voptr_function) dlsym(handle, "hoc_obj_ref");
    assert(hoc_obj_ref);

    auto vector_capacity = (ivptr_function) dlsym(handle, "vector_capacity");
    assert(vector_capacity);

    auto vector_vec = (dptrvptr_function) dlsym(handle, "vector_vec");
    assert(vector_vec);

    auto hoc_pushstr = (vcptrptr_function) dlsym(handle, "hoc_pushstr");
    assert(hoc_pushstr);

    auto call_ob_proc = (voptrsptri_function) dlsym(handle, "hoc_call_ob_proc");
    assert(call_ob_proc);

    auto hoc_xpop = (dv_function) dlsym(handle, "hoc_xpop");
    assert(hoc_xpop);

    /***************************
     * 
     * Miscellaneous initialization
     * 
     **************************/

    // commenting out the following line shows the banner
    *((int64_t*) dlsym(handle, "nrn_nobanner_")) = 1;


    // need this to support dynamic MPI, but might not exist without that enabled
    if (nrnmpi_stubs) {
        nrnmpi_stubs();
    }

    ivocmain(3, argv, NULL, 0);


    //cout << "Built-in symbols:" << endl;
    //print_symbol_table(hoc_built_in_symlist);

    auto hoc_built_in_symlist = (Symlist*) dlsym(handle, "hoc_built_in_symlist");
    assert(hoc_built_in_symlist);

    auto hoc_top_level_symlist = (Symlist*) dlsym(handle, "hoc_top_level_symlist");
    assert(hoc_top_level_symlist);

    // register redirected print
    register_print_function(handle, myprint);


    /***************************
     * run HOC code
     **************************/
    hoc_oc(
        "create soma\n"
    );

    cout << "created the soma; now lets look at topology:" << endl;    

    /***************************
     * lookup a symbol and call the corresponding function with 0 arguments
     **************************/
    sym = hoc_lookup("topology");
    hoc_call_func(sym, 0);  // the 0 is the number of args; returns the return of the function (1)


    /***************************
     * call finitialize, pass in an initial voltage (so 1 argument instead of 0)
     **************************/
    sym = hoc_lookup("finitialize");
    hoc_pushx(3.14);
    hoc_call_func(sym, 1);

    /***************************
     * lookup a top-level symbol and set the value (equivalent to t=1.23)
     **************************/
    sym = hoc_lookup("t");
    *(sym->u.pval) = 1.23;


    /***************************
     * call fadvance
     **************************/
    sym = hoc_lookup("fadvance");
    assert(sym);
    hoc_call_func(sym, 0);

    /***************************
     * print out the time and the voltage (print, alas, is a statement not a function)
     **************************/
    cout << "time and voltage:" << endl;
    hoc_oc("print t, v\n");

    /***************************
     * Vectors
     **************************/
    print_class_methods("Vector");
    sym = hoc_lookup("Vector");
    hoc_pushx(5);
    Object* my_vec = hoc_newobj1(sym, 1);
    cout << "my_vec refcount: " << my_vec->refcount << endl;
    cout << "vector_capacity(my_vec) = " << vector_capacity(my_vec->u.this_pointer) << endl;
    double* vec_data = vector_vec(my_vec->u.this_pointer);
    for (auto i = 0; i < vector_capacity(my_vec->u.this_pointer); i++) {
        vec_data[i] = i * i;
    }

    // calling: my_vec.printf()
    sym2 = hoc_table_lookup("printf", my_vec->ctemplate->symtable);
    assert(sym2);
    call_ob_proc(my_vec, sym2, 0);  // last argument is narg

    // calling: my_vec.apply("sin") -- where "sin" is a built-in function
    sym2 = hoc_table_lookup("apply", my_vec->ctemplate->symtable);
    assert(sym2);
    char* fname_ptr = new char[4];
    strcpy(fname_ptr, "sin");
    hoc_pushstr(&fname_ptr);
    call_ob_proc(my_vec, sym2, 1);  // last argument is narg
    delete[] fname_ptr;

    // calling: my_vec.printf(), to show the sines of all the previous values
    sym2 = hoc_table_lookup("printf", my_vec->ctemplate->symtable);
    assert(sym2);
    call_ob_proc(my_vec, sym2, 0);  // last argument is narg

    // calling: my_vec.mean()
    sym2 = hoc_table_lookup("mean", my_vec->ctemplate->symtable);
    assert(sym2);
    call_ob_proc(my_vec, sym2, 0);  // last argument is narg
    cout << "my_vec.mean() = " << hoc_xpop() << endl;
    // I don't understand why this has to be in two steps instead of just
    // hoc_call_objfunc(sym2, 0, my_vec)
}