#include <dlfcn.h>
#include <iostream>
#include <cstdlib>

// NOTE: we explicitly do not include neuron_api.h; this is the file that defines everything

using std::cout;
using std::endl;
using std::exit;

struct Symbol;
struct Object;
struct Section;
struct SectionListIterator;
struct hoc_Item;
struct SymbolTableIterator;
struct Symlist;

// TODO: should this be here or part of every program instead?
extern "C" void modl_reg() {};


/****************************************
 * Initialization
 ****************************************/
int (*nrn_init)(int argc, const char **argv);
void (*nrn_redirect_stdout)(int (*myprint)(int, char *));

/****************************************
 * Sections
 ****************************************/
Section *(*nrn_new_section)(char const *const name);
void (*nrn_connect_sections)(Section *child_sec, double child_x,
                            Section *parent_sec, double parent_x);
void (*nrn_set_section_length)(Section *sec, double length);
double (*nrn_get_section_length)(Section *sec);
double (*nrn_get_section_Ra)(Section* sec);
void (*nrn_set_section_Ra)(Section* sec, double val);
char const *(*nrn_secname)(Section *sec);
void (*nrn_push_section)(Section *sec);
void (*nrn_pop_section)(void);
void (*nrn_insert_mechanism)(Section *sec, Symbol *mechanism);
hoc_Item* (*nrn_get_allsec)(void);
hoc_Item* (*nrn_get_sectionlist_data)(Object* obj);

/****************************************
 * Segments
 ****************************************/
int (*nrn_get_nseg)(Section const * const sec);
void (*nrn_set_nseg)(Section* const sec, const int nseg);
void (*nrn_set_segment_diam)(Section* const sec, const double x, const double diam);
double* (*nrn_get_rangevar_ptr)(Section* const sec, Symbol* const sym, double const x);

/****************************************
 * Functions, objects, and the stack
 ****************************************/
Symbol *(*nrn_get_symbol)(char const *const name);
int (*nrn_get_symbol_type)(Symbol *sym);
double *(*nrn_get_symbol_ptr)(Symbol *sym);
void (*nrn_push_double)(double val);
double (*nrn_pop_double)(void);
void (*nrn_push_double_ptr)(double *addr);
double *(*nrn_pop_double_ptr)(void);
void (*nrn_push_str)(char **str);
char **(*nrn_pop_str)(void);
void (*nrn_push_int)(int i);
int (*nrn_pop_int)(void);
Object *(*nrn_pop_object)(void);
int (*nrn_stack_type)(void);
char const *const (*nrn_stack_type_name)(int id);
Object *(*nrn_new_object)(Symbol *sym, int narg);
Symbol *(*nrn_get_method_symbol)(Object *obj, char const *const name);
void (*nrn_call_method)(Object *obj, Symbol *method_sym, int narg);
void (*nrn_call_function)(Symbol *sym, int narg);
void (*nrn_unref_object)(Object *obj);
char const * (*nrn_get_class_name)(Object* obj);

/****************************************
 * Miscellaneous
 ****************************************/
int (*nrn_call_hoc)(char const *const command);
SectionListIterator *(*nrn_new_sectionlist_iterator)(hoc_Item *my_sectionlist);
void (*nrn_free_sectionlist_iterator)(SectionListIterator *sl);
Section *(*nrn_sectionlist_iterator_next)(SectionListIterator* sl);
int (*nrn_sectionlist_iterator_done)(SectionListIterator* sl);
SymbolTableIterator *(*nrn_new_symbol_table_iterator)(Symlist *my_symbol_table);
void (*nrn_free_symbol_table_iterator)(SymbolTableIterator *st);
char const *(*nrn_symbol_table_iterator_next)(SymbolTableIterator *st);
int (*nrn_symbol_table_iterator_done)(SymbolTableIterator *st);
int (*nrn_vector_capacity)(Object *vec);
double *(*nrn_vector_data_ptr)(Object *vec);
double* (*nrn_get_pp_property_ptr)(Object* pp, const char* name);
double* (*nrn_get_steered_property_ptr)(Object* obj, const char* name);
char const * (*nrn_get_symbol_name)(Symbol* sym);
Symlist * (*nrn_get_symbol_table)(Symbol* sym);
Symlist * (*nrn_get_global_symbol_table)(void);

void setup_neuron_api(void) {
    void* handle = dlopen("libnrniv.dylib", RTLD_NOW | RTLD_LOCAL); 
    if (!handle) {
        cout << "Couldn't open dylib." << endl << dlerror() << endl;
        exit(-1);
    }

    /****************************************
     * Initialization
     ****************************************/
    nrn_init = reinterpret_cast<decltype(nrn_init)>(dlsym(handle, "nrn_init")); 
    assert(nrn_init);
    nrn_redirect_stdout = reinterpret_cast<decltype(nrn_redirect_stdout)>(dlsym(handle, "nrn_redirect_stdout")); 
    assert(nrn_redirect_stdout);

    /****************************************
     * Sections
     ****************************************/
    nrn_new_section = reinterpret_cast<decltype(nrn_new_section)>(dlsym(handle, "nrn_new_section")); 
    assert(nrn_new_section);
    nrn_connect_sections = reinterpret_cast<decltype(nrn_connect_sections)>(dlsym(handle, "nrn_connect_sections")); 
    assert(nrn_connect_sections);
    nrn_set_section_length = reinterpret_cast<decltype(nrn_set_section_length)>(dlsym(handle, "nrn_set_section_length")); 
    assert(nrn_set_section_length);
    nrn_get_section_length = reinterpret_cast<decltype(nrn_get_section_length)>(dlsym(handle, "nrn_get_section_length")); 
    assert(nrn_get_section_length);
    nrn_get_section_Ra = reinterpret_cast<decltype(nrn_get_section_Ra)>(dlsym(handle, "nrn_get_section_Ra")); 
    assert(nrn_get_section_Ra);
    nrn_set_section_Ra = reinterpret_cast<decltype(nrn_set_section_Ra)>(dlsym(handle, "nrn_set_section_Ra")); 
    assert(nrn_set_section_Ra);
    nrn_secname = reinterpret_cast<decltype(nrn_secname)>(dlsym(handle, "nrn_secname")); 
    assert(nrn_secname);
    nrn_push_section = reinterpret_cast<decltype(nrn_push_section)>(dlsym(handle, "nrn_push_section")); 
    assert(nrn_push_section);
    nrn_pop_section = reinterpret_cast<decltype(nrn_pop_section)>(dlsym(handle, "nrn_pop_section")); 
    assert(nrn_pop_section);
    nrn_insert_mechanism = reinterpret_cast<decltype(nrn_insert_mechanism)>(dlsym(handle, "nrn_insert_mechanism")); 
    assert(nrn_insert_mechanism);
    nrn_get_allsec = reinterpret_cast<decltype(nrn_get_allsec)>(dlsym(handle, "nrn_get_allsec")); 
    assert(nrn_get_allsec);
    nrn_get_sectionlist_data = reinterpret_cast<decltype(nrn_get_sectionlist_data)>(dlsym(handle, "nrn_get_sectionlist_data")); 
    assert(nrn_get_sectionlist_data);


    /****************************************
     * Segments
     ****************************************/
    nrn_get_nseg = reinterpret_cast<decltype(nrn_get_nseg)>(dlsym(handle, "nrn_get_nseg")); 
    assert(nrn_get_nseg);
    nrn_set_nseg = reinterpret_cast<decltype(nrn_set_nseg)>(dlsym(handle, "nrn_set_nseg")); 
    assert(nrn_set_nseg);
    nrn_set_segment_diam = reinterpret_cast<decltype(nrn_set_segment_diam)>(dlsym(handle, "nrn_set_segment_diam")); 
    assert(nrn_set_segment_diam);
    nrn_get_rangevar_ptr = reinterpret_cast<decltype(nrn_get_rangevar_ptr)>(dlsym(handle, "nrn_get_rangevar_ptr")); 
    assert(nrn_get_rangevar_ptr);


    /****************************************
     * Functions, objects, and the stack
     ****************************************/
    nrn_get_symbol = reinterpret_cast<decltype(nrn_get_symbol)>(dlsym(handle, "nrn_get_symbol")); 
    assert(nrn_get_symbol);
    nrn_get_symbol_type = reinterpret_cast<decltype(nrn_get_symbol_type)>(dlsym(handle, "nrn_get_symbol_type")); 
    assert(nrn_get_symbol_type);
    nrn_get_symbol_ptr = reinterpret_cast<decltype(nrn_get_symbol_ptr)>(dlsym(handle, "nrn_get_symbol_ptr")); 
    assert(nrn_get_symbol_ptr);
    nrn_push_double = reinterpret_cast<decltype(nrn_push_double)>(dlsym(handle, "nrn_push_double")); 
    assert(nrn_push_double);
    nrn_pop_double = reinterpret_cast<decltype(nrn_pop_double)>(dlsym(handle, "nrn_pop_double")); 
    assert(nrn_pop_double);
    nrn_push_double_ptr = reinterpret_cast<decltype(nrn_push_double_ptr)>(dlsym(handle, "nrn_push_double_ptr")); 
    assert(nrn_push_double_ptr);
    nrn_pop_double_ptr = reinterpret_cast<decltype(nrn_pop_double_ptr)>(dlsym(handle, "nrn_pop_double_ptr")); 
    assert(nrn_pop_double_ptr);
    nrn_push_str = reinterpret_cast<decltype(nrn_push_str)>(dlsym(handle, "nrn_push_str")); 
    assert(nrn_push_str);
    nrn_pop_str = reinterpret_cast<decltype(nrn_pop_str)>(dlsym(handle, "nrn_pop_str")); 
    assert(nrn_pop_str);
    nrn_push_int = reinterpret_cast<decltype(nrn_push_int)>(dlsym(handle, "nrn_push_int")); 
    assert(nrn_push_int);
    nrn_pop_int = reinterpret_cast<decltype(nrn_pop_int)>(dlsym(handle, "nrn_pop_int")); 
    assert(nrn_pop_int);
    nrn_pop_object = reinterpret_cast<decltype(nrn_pop_object)>(dlsym(handle, "nrn_pop_object")); 
    assert(nrn_pop_object);
    nrn_stack_type = reinterpret_cast<decltype(nrn_stack_type)>(dlsym(handle, "nrn_stack_type")); 
    assert(nrn_stack_type);
    nrn_stack_type_name = reinterpret_cast<decltype(nrn_stack_type_name)>(dlsym(handle, "nrn_stack_type_name")); 
    assert(nrn_stack_type_name);
    nrn_new_object = reinterpret_cast<decltype(nrn_new_object)>(dlsym(handle, "nrn_new_object")); 
    assert(nrn_new_object);
    nrn_get_method_symbol = reinterpret_cast<decltype(nrn_get_method_symbol)>(dlsym(handle, "nrn_get_method_symbol")); 
    assert(nrn_get_method_symbol);
    nrn_call_method = reinterpret_cast<decltype(nrn_call_method)>(dlsym(handle, "nrn_call_method")); 
    assert(nrn_call_method);
    nrn_call_function = reinterpret_cast<decltype(nrn_call_function)>(dlsym(handle, "nrn_call_function")); 
    assert(nrn_call_function);
    nrn_unref_object = reinterpret_cast<decltype(nrn_unref_object)>(dlsym(handle, "nrn_unref_object")); 
    assert(nrn_unref_object);
    nrn_get_class_name = reinterpret_cast<decltype(nrn_get_class_name)>(dlsym(handle, "nrn_get_class_name")); 
    assert(nrn_get_class_name);

    /****************************************
     * Miscellaneous
     ****************************************/
    nrn_call_hoc = reinterpret_cast<decltype(nrn_call_hoc)>(dlsym(handle, "nrn_call_hoc"));     
    assert(nrn_call_hoc);
    nrn_new_sectionlist_iterator = reinterpret_cast<decltype(nrn_new_sectionlist_iterator)>(dlsym(handle, "nrn_new_sectionlist_iterator"));     
    assert(nrn_new_sectionlist_iterator);
    nrn_free_sectionlist_iterator = reinterpret_cast<decltype(nrn_free_sectionlist_iterator)>(dlsym(handle, "nrn_free_sectionlist_iterator"));     
    assert(nrn_free_sectionlist_iterator);
    nrn_sectionlist_iterator_next = reinterpret_cast<decltype(nrn_sectionlist_iterator_next)>(dlsym(handle, "nrn_sectionlist_iterator_next"));     
    assert(nrn_sectionlist_iterator_next);
    nrn_sectionlist_iterator_done = reinterpret_cast<decltype(nrn_sectionlist_iterator_done)>(dlsym(handle, "nrn_sectionlist_iterator_done"));     
    assert(nrn_sectionlist_iterator_done);
    nrn_new_symbol_table_iterator = reinterpret_cast<decltype(nrn_new_symbol_table_iterator)>(dlsym(handle, "nrn_new_symbol_table_iterator"));     
    assert(nrn_new_symbol_table_iterator);
    nrn_free_symbol_table_iterator = reinterpret_cast<decltype(nrn_free_symbol_table_iterator)>(dlsym(handle, "nrn_free_symbol_table_iterator"));     
    assert(nrn_free_symbol_table_iterator);
    nrn_symbol_table_iterator_next = reinterpret_cast<decltype(nrn_symbol_table_iterator_next)>(dlsym(handle, "nrn_symbol_table_iterator_next"));
    assert(nrn_symbol_table_iterator_next);
    nrn_symbol_table_iterator_done = reinterpret_cast<decltype(nrn_symbol_table_iterator_done)>(dlsym(handle, "nrn_symbol_table_iterator_done"));
    assert(nrn_symbol_table_iterator_done);
    nrn_vector_capacity = reinterpret_cast<decltype(nrn_vector_capacity)>(dlsym(handle, "nrn_vector_capacity"));     
    assert(nrn_vector_capacity);
    nrn_vector_data_ptr = reinterpret_cast<decltype(nrn_vector_data_ptr)>(dlsym(handle, "nrn_vector_data_ptr"));     
    assert(nrn_vector_data_ptr);
    nrn_get_pp_property_ptr = reinterpret_cast<decltype(nrn_get_pp_property_ptr)>(dlsym(handle, "nrn_get_pp_property_ptr"));     
    assert(nrn_get_pp_property_ptr);
    nrn_get_steered_property_ptr = reinterpret_cast<decltype(nrn_get_steered_property_ptr)>(dlsym(handle, "nrn_get_steered_property_ptr"));     
    assert(nrn_get_steered_property_ptr);
    nrn_get_symbol_name = reinterpret_cast<decltype(nrn_get_symbol_name)>(dlsym(handle, "nrn_get_symbol_name"));     
    assert(nrn_get_symbol_name);
    nrn_get_symbol_table = reinterpret_cast<decltype(nrn_get_symbol_table)>(dlsym(handle, "nrn_get_symbol_table"));     
    assert(nrn_get_symbol_table);
    nrn_get_global_symbol_table = reinterpret_cast<decltype(nrn_get_global_symbol_table)>(dlsym(handle, "nrn_get_global_symbol_table"));
    assert(nrn_get_global_symbol_table);
}