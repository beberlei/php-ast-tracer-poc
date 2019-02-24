dnl config.m4 for extension astracer

PHP_ARG_ENABLE(astracer, whether to enable astracer support,
[  --enable-astracer          Enable astracer support], no)

if test "$PHP_astracer" != "no"; then
  PHP_NEW_EXTENSION(astracer, astracer.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
