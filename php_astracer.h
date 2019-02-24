/* astracer extension for PHP */

#ifndef PHP_ASTRACER_H
# define PHP_ASTRACER_H

extern zend_module_entry astracer_module_entry;
# define phpext_astracer_ptr &astracer_module_entry

# define PHP_ASTRACER_VERSION "0.0.1"

# if defined(ZTS) && defined(COMPILE_DL_ASTRACER)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_ASTRACER_H */
