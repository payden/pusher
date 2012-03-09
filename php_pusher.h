#ifndef PHP_PUSHER_H
#define PHP_PUSHER_H 1

#define PHP_PUSHER_VERSION NO_VERSION_YET
#define PHP_PUSHER_EXTNAME "pusher"

PHP_METHOD(pusher, __construct);
PHP_MINIT_FUNCTION(pusher);


static zend_class_entry *pusher_ce;
void register_pusher_class(TSRMLS_D);
extern zend_module_entry pusher_module_entry;

#define phpext_pusher_ptr &pusher_module_entry

#endif
