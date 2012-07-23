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


#ifndef PHP_PUSHER_H
#define PHP_PUSHER_H 1

#define PHP_PUSHER_VERSION NO_VERSION_YET
#define PHP_PUSHER_EXTNAME "pusher"

//PHP Function prototypes
PHP_METHOD(pusher, __construct);
PHP_METHOD(pusher, trigger);
PHP_METHOD(pusher, socket_auth);
PHP_METHOD(pusher, presence_auth);

PHP_MINIT_FUNCTION(pusher);
PHP_MSHUTDOWN_FUNCTION(pusher);

//Typedefs

typedef struct {
	zend_object zo;
	char *key;
	char *secret;
	unsigned long app_id;
} pusher_object;

//global symbols
static zend_class_entry *pusher_ce;
zend_object_handlers pusher_object_handlers;

//function prototypes
void register_pusher_class(TSRMLS_D);

extern zend_module_entry pusher_module_entry;

#define phpext_pusher_ptr &pusher_module_entry

#endif
