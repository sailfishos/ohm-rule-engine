#ifndef __LIBPROLOG_H__
#define __LIBPROLOG_H__

/*
 * prolog glue path
 */

#define LIBPROLOG_SO "libprolog.so"

#ifndef LIBPROLOG_HELPER
#  define LIBPROLOG_HELPER "/usr/share/libprolog/libprolog.pl"
#endif



/*
 * prolog shell commands
 */

#define SHELL_QUIT_COMMAND "quit"
#define SHELL_MORE_COMMAND ';'


/*
 * predicate tracing
 */

#define COMMAND_ENABLE     "enable"
#define COMMAND_DISABLE    "disable"
#define COMMAND_INDENT     "indent"
#define COMMAND_DEFAULTS   "defaults"
#define COMMAND_RESET      "reset"
#define COMMAND_CLEAR      "clear"
#define COMMAND_SHOW       "show"
#define COMMAND_ON         "on"
#define COMMAND_OFF        "off"
#define COMMAND_TRANSITIVE "transitive"
#define COMMAND_SUPPRESS   "suppress"
#define COMMAND_DETAILED   "detailed"
#define COMMAND_SHORT      "short"
#define PORT_CALL          "call"
#define PORT_REDO          "redo"
#define PORT_PROVEN        "proven"
#define PORT_FAILED        "failed"
#define PORT_EXIT          "exit"
#define PORT_FAIL          "fail"
#define PORT_ALL           "all"
#define PRED_DEFAULT       "default"
#define WILDCARD_ANY       "*"
#define WILDCARD_ALL       "%"


/*
 * prolog query flags for normal and tracing evaluation
 */

#define NORMAL_QUERY_FLAGS (PL_Q_NORMAL | PL_Q_NODEBUG | PL_Q_CATCH_EXCEPTION)
#define TRACE_QUERY_FLAGS  (PL_Q_NORMAL | PL_Q_CATCH_EXCEPTION)


/*
 * predicates to export rules
 */

#define PRED_EXPORTED "exported"                      /* old interface */
#define PRED_RULES    "rules"                         /* new interface */


/*
 * prolog predicate result types
 */

enum {
    RESULT_UNKNOWN = 0,
    RESULT_OBJECTS,
    RESULT_EXCEPTION,
};



/*
 * macros for emitting messages from within the library
 */

#define PROLOG_ERROR(fmt, args...) do {                 \
        fprintf(stderr, "[ERROR] "fmt"\n", ## args);    \
    } while (0)

#define PROLOG_WARNING(fmt, args...) do {               \
        fprintf(stderr, "[WARNING] "fmt"\n", ## args);  \
    } while (0)

#define PROLOG_INFO(fmt, args...) do {                  \
        fprintf(stdout, "[INFO] "fmt"\n", ## args);	\
    } while (0)



/*
 * macros for memory allocation
 */

#define ALLOC(type) ({                            \
            type   *__ptr;                        \
            size_t  __size = sizeof(type);        \
                                                  \
            if ((__ptr = malloc(__size)) != NULL) \
                memset(__ptr, 0, __size);         \
            __ptr; })

#define ALLOC_OBJ(ptr) ((ptr) = ALLOC(typeof(*ptr)))

#define ALLOC_ARRAY(type, n) ({			  \
            type   *__ptr;                        \
            size_t   __size = (n) * sizeof(type); \
                                                  \
            if ((__ptr = malloc(__size)) != NULL) \
                memset(__ptr, 0, __size);         \
            __ptr; })

#define REALLOC_ARRAY(ptr, o, n) ({					\
            typeof(ptr) __ptr;                                          \
            size_t      __size = sizeof(*ptr) * (n);                    \
                                                                        \
            if ((ptr) == NULL) {                                        \
	      (__ptr) = ALLOC_ARRAY(typeof(*ptr), n);                   \
                ptr = __ptr;                                            \
            }                                                           \
            else if ((__ptr = realloc(ptr, __size)) != NULL) {          \
                if ((n) > (o))                                          \
                    memset(__ptr + (o), 0, ((n)-(o)) * sizeof(*ptr));   \
                ptr = __ptr;                                            \
            }                                                           \
            __ptr; })
                
#define FREE(obj) do { if (obj) free(obj); } while (0)

#define STRDUP(s) ({                                    \
            char *__s = (char*) s;                      \
            __s = ((s) ? strdup(s) : strdup(""));       \
            __s; })



/*
 * visibility-hiding
 *
 * If we have visibility-hiding support hide global symbols by default and
 * export only the ones tagged with PROLOG_API.
 */

#if __GNUC__
#  if HAVE_VISIBILITY_SUPPORT
#    pragma GCC visibility push(hidden)
#    define PROLOG_API __attribute__ ((visibility("default")))
#  else
#    define PROLOG_API
#  endif
#else
#  define PROLOG_API
#endif



/*
 * prototypes
 */


/* prolog-lib.c */
int libprolog_initialized(void);


/* prolog-trace.c */
int  libprolog_trace_init(void);
void libprolog_trace_exit(void);
int  libprolog_tracing   (void);

foreign_t libpl_trace_pred  (term_t pl_args, int arity, void *context);
foreign_t libpl_trace_config(term_t pl_args, int arity, void *context);


/* prolog-loader.c */
int  libprolog_load_file(char *path, int extension);
void libprolog_clear_errors(void);
int  libprolog_has_errors(void);
void libprolog_mark_error(void);

foreign_t libpl_loading(term_t noargs, int arity, void *context);
foreign_t libpl_mark_error(term_t noargs, int arity, void *context);
foreign_t libpl_clear_errors(term_t noargs, int arity, void *context);
foreign_t libpl_has_errors(term_t noargs, int arity, void *context);


/* prolog-object.c */
int libprolog_collect_result(qid_t qid, term_t pl_retval, void *retval);
int libprolog_collect_exception(qid_t qid, void *retval);


/* prolog-utils.c */
int    swi_list_length(term_t pl_list);
term_t swi_list_new(char **items, int n, term_t result);
term_t swi_list_prepend(term_t list, term_t item);
int    swi_list_walk(term_t list,
		     int (*callback)(term_t item, int i, void *data),
		     void *data);

int swi_set_trace(int state);



#endif /* __LIBPROLOG_H__ */
