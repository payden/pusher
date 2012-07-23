/*
  +----------------------------------------------------------------------+
  | Pusher Extension                                                     |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Payden Sutherland <payden@paydensutherland.com>              |
  +----------------------------------------------------------------------+
*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pusher.h"

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
	register_pusher_class(TSRMLS_C);
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(pusher)
{
	return SUCCESS;
}
