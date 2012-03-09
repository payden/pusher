#ifndef PHP_OAUTH2_H
#define PHP_OAUTH2_H 1

#define PHP_PUSHER_VERSION NO_VERSION_YET
#define PHP_PUSHER_EXTNAME "pusher"

PHP_FUNCTION(pusher);
PHP_MINIT_FUNCTION(pusher);

extern zend_module_entry pusher_module_entry;
#define phpext_pusher_ptr &pusher_module_entry

#endif
