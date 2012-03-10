#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pusher.h"


zend_object_handlers pusher_object_handlers;
extern zend_class_entry *pusher_ce;

static zend_function_entry pusher_methods[] = {
	PHP_ME(pusher, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(pusher, getVal, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

PHP_METHOD(pusher, __construct) {
	zval *obj;

	obj = getThis();
	pusher_object *pusher_obj = (pusher_object *)zend_object_store_get_object(obj TSRMLS_CC);
	pusher_obj->val = (int *)emalloc(sizeof(int));
	*(pusher_obj->val) = 10;

	RETURN_TRUE;
}

PHP_METHOD(pusher, getVal) {
	zval *obj;
	obj = getThis();
	pusher_object *pusher_obj = (pusher_object *)zend_object_store_get_object(obj TSRMLS_CC);
	RETURN_LONG(*(pusher_obj->val));
}


static void pusher_free_storage(void *object TSRMLS_DC) {
	pusher_object *intern = (pusher_object *)object;
	int *var = intern->val;
	efree(var);
	zend_hash_destroy(intern->zo.properties);
	FREE_HASHTABLE(intern->zo.properties);
	efree(intern);
}


zend_object_value pusher_object_create(zend_class_entry *class_type TSRMLS_DC) {
	zend_object_value retval;
	zval *tmp;
	pusher_object *intern;

	intern = (pusher_object *) emalloc(sizeof(pusher_object));
	memset(intern, 0, sizeof(pusher_object));
	intern->zo.ce = class_type;
	ALLOC_HASHTABLE(intern->zo.properties);
	zend_hash_init(intern->zo.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(intern->zo.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	retval.handle = zend_objects_store_put(intern, NULL, pusher_free_storage, NULL TSRMLS_CC);
	retval.handlers = (zend_object_handlers *) &pusher_object_handlers;
	return retval;
}

void register_pusher_class(TSRMLS_D) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "Pusher", pusher_methods);
	ce.create_object = pusher_object_create;
	memcpy(&pusher_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	pusher_object_handlers.clone_obj = NULL;
	pusher_ce = zend_register_internal_class(&ce TSRMLS_CC);
}


