PHP_ARG_ENABLE(pni, whether to enable PNI support,[ --enable-pni Enable PNI support])
if test "$PHP_PNI" = "yes"; then
AC_DEFINE(HAVE_PNI, 1, [Whether you have PNI])
PHP_NEW_EXTENSION(pni, pni.c, $ext_shared)
fi
