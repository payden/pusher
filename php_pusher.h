#ifndef PHP_PUSHER_H
#define PHP_PUSHER_H 1

#define PHP_PUSHER_VERSION NO_VERSION_YET
#define PHP_PUSHER_EXTNAME "pusher"

//PHP Function prototypes
PHP_METHOD(pusher, __construct);
PHP_METHOD(pusher, getVal);
PHP_MINIT_FUNCTION(pusher);
PHP_MSHUTDOWN_FUNCTION(pusher);

//Typedefs

typedef struct {
	zend_object zo;
	int *val;
} pusher_object;

static zend_class_entry *pusher_ce;
void register_pusher_class(TSRMLS_D);
static void pusher_objects_dtor(void *object, zend_object_handle handle TSRMLS_DC);

extern zend_module_entry pusher_module_entry;

#define phpext_pusher_ptr &pusher_module_entry

#endif
