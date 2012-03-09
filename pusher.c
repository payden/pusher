#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pusher.h"

static zend_class_entry *pusher_consumer_ce;

static function_entry pusher_functions[] = {
	PHP_FE(pusher, NULL)
	{NULL, NULL, NULL}
};

zend_module_entry pusher_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_PUSHER_EXTNAME,
	pusher_functions,
	PHP_MINIT(pusher),
	NULL,
	NULL,
	NULL,
	NULL,
#if ZEND_MODULE_API_NO >= 20010901
	PHP_PUSHER_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PUSHER
ZEND_GET_MODULE(pusher)
#endif


PHP_MINIT_FUNCTION(pusher)
{
	zend_class_entry consumer_ce;
	INIT_CLASS_ENTRY(consumer_ce, "pusher_Consumer", NULL);
	pusher_consumer_ce = zend_register_internal_class(&consumer_ce TSRMLS_CC);
}


PHP_FUNCTION(pusher)
{
	char *str;
	str = estrdup("Hello World!");
	RETURN_STRING(str, 0);
}
