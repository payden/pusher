#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pusher.h"


extern zend_class_entry *pusher_ce;

static zend_function_entry pusher_methods[] = {
	PHP_ME(pusher, __construct, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

PHP_METHOD(pusher, __construct) {
	zval *pusher_settings;
	ALLOC_INIT_ZVAL(pusher_settings);
	array_init(pusher_settings);
	zend_update_property(pusher_ce, getThis(), "settings", strlen("settings"), pusher_settings TSRMLS_CC);
	RETURN_TRUE;
}

PHP_METHOD(pusher, __destruct) {
	zval *pusher_settings, *obj;
	obj = getThis();
	zend_read_property(Z_OBJCE_P(obj), obj, "settings", strlen("settings"), 1 TSRMLS_CC);
	zval_dtor(pusher_settings);
	RETURN_TRUE;
}


void register_pusher_class(TSRMLS_D) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "Pusher", pusher_methods);
	pusher_ce = zend_register_internal_class(&ce TSRMLS_CC);
	zend_declare_property_null(pusher_ce, "settings", strlen("settings"), ZEND_ACC_PROTECTED TSRMLS_CC);
}

