#ifndef PHP_PUSHER_H
#define PHP_PUSHER_H 1

#define PHP_PUSHER_VERSION NO_VERSION_YET
#define PHP_PUSHER_EXTNAME "pusher"

//PHP Function prototypes
PHP_METHOD(pusher, __construct);
PHP_METHOD(pusher, getKey);
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
static char *md5_hash(const char *str);

extern zend_module_entry pusher_module_entry;

#define phpext_pusher_ptr &pusher_module_entry

#endif
