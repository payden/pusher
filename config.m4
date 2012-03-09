PHP_ARG_ENABLE(pusher, whether to enable pusher support,
		[ --enable-pusher	Enable pusher support])

if test "$PHP_PUSHER" = "yes"; then
	AC_DEFINE(HAVE_PUSHER, 1, [Whether you have pusher])
	PHP_NEW_EXTENSION(pusher,pusher.c php_pusher.c, $ext_shared)
fi
