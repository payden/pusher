#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pusher.h"
#include <curl/curl.h>

static zend_function_entry pusher_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry pusher_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_PUSHER_EXTNAME,
	pusher_functions,
	PHP_MINIT(pusher),
	PHP_MSHUTDOWN(pusher),
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
	curl_global_init(CURL_GLOBAL_ALL);
	register_pusher_class(TSRMLS_C);
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(pusher)
{
	curl_global_cleanup();
	return SUCCESS;
}
