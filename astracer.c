/* astracer extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_astracer.h"

#define ASTRACER_G(e) php_astracer_globals.e

typedef struct _php_astracer_globals_t {
    HashTable *ast_to_clean;
} php_astracer_globals_t;

ZEND_TLS php_astracer_globals_t php_astracer_globals;

ZEND_API void (*original_ast_process_function)(zend_ast *ast);

static zend_ast *create_ast_call(const char *name)
{
    zend_ast *call;
    zend_ast_zval *name_var;
    zend_ast_list *arg_list;

    name_var = emalloc(sizeof(zend_ast_zval));
    name_var->kind = ZEND_AST_ZVAL;
    ZVAL_STRING(&name_var->val, name);
    name_var->val.u2.lineno = 0;
    zend_hash_next_index_insert_ptr(ASTRACER_G(ast_to_clean), name_var);

    arg_list = emalloc(sizeof(zend_ast_list) + sizeof(zend_ast*));
    arg_list->kind = ZEND_AST_ARG_LIST;
    arg_list->lineno = 0;
    arg_list->children = 0;
    zend_hash_next_index_insert_ptr(ASTRACER_G(ast_to_clean), arg_list);

    call = emalloc(sizeof(zend_ast) + sizeof(zend_ast*));
    call->kind = ZEND_AST_CALL;
    call->lineno = 0;
    call->child[0] = (zend_ast*)name_var;
    call->child[1] = (zend_ast*)arg_list;
    zend_hash_next_index_insert_ptr(ASTRACER_G(ast_to_clean), call);

    return call;
}

static zend_ast_list *inject_ast(zend_ast *original)
{
    zend_ast *try;
    zend_ast_list *new_list, *finally, *empty_catches;

    empty_catches = emalloc(sizeof(zend_ast_list) + sizeof(zend_ast*));
    empty_catches->kind = ZEND_AST_ARG_LIST;
    empty_catches->lineno = 0;
    empty_catches->children = 0;
    zend_hash_next_index_insert_ptr(ASTRACER_G(ast_to_clean), empty_catches);

    finally = emalloc(sizeof(zend_ast_list) + sizeof(zend_ast*));
    finally->kind = ZEND_AST_ARG_LIST;
    finally->lineno = 0;
    finally->children = 1;
    finally->child[0] = create_ast_call("astracer_end");
    zend_hash_next_index_insert_ptr(ASTRACER_G(ast_to_clean), finally);

    try = emalloc(sizeof(zend_ast) + sizeof(zend_ast*)*2);
    try->kind = ZEND_AST_TRY;
    try->lineno = 0;
    try->child[0] = original;
    try->child[1] = (zend_ast*)empty_catches;
    try->child[2] = (zend_ast*)finally;
    zend_hash_next_index_insert_ptr(ASTRACER_G(ast_to_clean), try);

    /* create a new statement list */
    new_list = emalloc(sizeof(zend_ast_list) + sizeof(zend_ast*));
    new_list->kind = ZEND_AST_STMT_LIST;
    new_list->lineno = 0;
    new_list->children = 2;
    new_list->child[0] = create_ast_call("astracer_begin");
    new_list->child[1] = try;
    zend_hash_next_index_insert_ptr(ASTRACER_G(ast_to_clean), new_list);

    return new_list;
}

void astracer_ast_process_method(zend_ast *ast, zend_string *namespace, zend_string *class_name) {
    zend_ast_decl *decl;
    zend_ast *child;
    zend_string *method_name;

    if (ast->kind != ZEND_AST_METHOD) {
        return;
    }

    decl = (zend_ast_decl *)ast;

    method_name = decl->name;

    // TODO: Add dynamic check here
    if (1) {
        decl->child[2] = inject_ast(decl->child[2]);
    }
}

void astracer_ast_process_class(zend_ast *ast, zend_string *namespace) {
    zend_string *class_name;
    zend_ast_list *list;
    zend_ast *child;
    zend_ast_decl *decl;
    int i = 0;

    if (ast->kind != ZEND_AST_CLASS) {
        return;
    }

    decl = (zend_ast_decl *)ast;

    class_name = decl->name;

    // TODO: Add dynamic check here
    if (1) {
        // continue with class processing, it has at least one method that is hooked
        list = zend_ast_get_list(decl->child[2]);

        for (i = 0; i < list->children; i++) {
            child = list->child[i];

            // check for methods
            if (child->kind == ZEND_AST_METHOD) {
                astracer_ast_process_method(child, namespace, class_name);
            }
        }
    }
}

void astracer_ast_process_file(zend_ast *ast) {
    int i = 0;
    zend_ast_list *list;
    zend_ast *child;
    zend_string *namespace;

    if (ast->kind != ZEND_AST_STMT_LIST) {
        return;
    }

    list = zend_ast_get_list(ast);

    for (i = 0; i < list->children; i++) {
        child = list->child[i];

        switch (child->kind) {
            case ZEND_AST_NAMESPACE:
                namespace = zend_ast_get_str(child->child[0]);
                break;

            case ZEND_AST_CLASS:
                astracer_ast_process_class(child, namespace);
                break;

            case ZEND_AST_FUNC_DECL:
                //astracer_ast_process_function(child, namespace);
                break;
        }
    }

    php_printf("%s\n", ZSTR_VAL(zend_ast_export("", ast, "")));
}

void astracer_ast_process(zend_ast *ast)
{
    astracer_ast_process_file(ast);

    /* call the original zend_ast_process function if one was set */
    if (original_ast_process_function) {
        original_ast_process_function(ast);
    }
}

PHP_MINIT_FUNCTION(astracer)
{
    original_ast_process_function = zend_ast_process;
    zend_ast_process = astracer_ast_process;

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(astracer)
{
    return SUCCESS;
}

static void ast_to_clean_dtor(zval *zv)
{
    zend_ast *ast = (zend_ast *)Z_PTR_P(zv);
    efree(ast);
}

PHP_RINIT_FUNCTION(astracer)
{
#if defined(ZTS) && defined(COMPILE_DL_ASTRACER)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif

    ALLOC_HASHTABLE(ASTRACER_G(ast_to_clean));
    zend_hash_init(ASTRACER_G(ast_to_clean), 8, NULL, ast_to_clean_dtor, 1);

    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(astracer)
{
    zend_hash_destroy(ASTRACER_G(ast_to_clean));
    FREE_HASHTABLE(ASTRACER_G(ast_to_clean));

    return SUCCESS;
}

PHP_MINFO_FUNCTION(astracer)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "astracer support", "enabled");
    php_info_print_table_end();
}

zend_function_entry php_astracer_functions[] = {
    PHP_FE_END
};

zend_module_entry astracer_module_entry = {
    STANDARD_MODULE_HEADER,
    "astracer",
    php_astracer_functions,
    PHP_MINIT(astracer),
    PHP_MSHUTDOWN(astracer),
    PHP_RINIT(astracer),
    PHP_RSHUTDOWN(astracer),
    PHP_MINFO(astracer),
    PHP_ASTRACER_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_ASTRACER
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(astracer)
#endif
