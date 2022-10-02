#include <dlfcn.h>
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include "../neuron_api_headers.h"

using std::cout;
using std::endl;
using std::exit;

typedef void (*initer_function)(int, const char**, const char**, int64_t);
typedef void (*vd_function)(double);
typedef void (*vv_function)(void);
typedef int64_t (*icptr_function)(const char*);
typedef void* (*vcptr_function)(const char*);
typedef Symbol* (*scptr_function)(const char*);
typedef double (*dvptrint_function) (void*, int64_t);
typedef Symbol* (*scptroptr_function) (char*, Object*);
typedef double (*dsio_function) (Symbol*, int64_t, Object*);
typedef Symbol* (*scptrslptr_function) (const char*, Symlist*);
typedef Object* (*optrsptri_function) (Symbol*, int);
typedef void (*voptr_function) (Object*);
typedef void (*vf2icif_function)(int (*)(int, char*), int(*)());


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

    auto hoc_newobj1 = (optrsptri_function) dlsym(handle, "hoc_newobj1");
    assert("hoc_newobj1");

    auto hoc_obj_ref = (voptr_function) dlsym(handle, "hoc_obj_ref");
    assert(hoc_obj_ref);


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

}