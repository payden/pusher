dnl
dnl $Id: config.m4 242949 2007-09-26 15:44:16Z cvs2svn $
dnl

PHP_ARG_ENABLE(pusher, for pusher support,
[  --enable-pusher        Enable pusher support])

if test "$PHP_PUSHER" != "no"; then
  PHP_NEW_EXTENSION(pusher, pusher.c php_pusher.c md5.c hmac_sha2.c sha2.c, $ext_shared)
fi
