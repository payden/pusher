#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pusher.h"
#include "md5.h"

extern zend_class_entry *pusher_ce;

static zend_function_entry pusher_methods[] = {
	PHP_ME(pusher, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(pusher, getKey, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

PHP_METHOD(pusher, __construct) {
	zval *this;
	char *passed_key;
	char *passed_secret;
	unsigned long int passed_app_id;
	unsigned long int passed_key_len;
	unsigned long int passed_secret_len;
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssl", &passed_key, &passed_key_len, &passed_secret, &passed_secret_len, &passed_app_id) == FAILURE) {
		RETURN_FALSE;
	}

	this = getThis();
	pusher_object *pusher_obj = (pusher_object *)zend_object_store_get_object(this TSRMLS_CC);


	pusher_obj->key = (char *)emalloc(passed_key_len+1);
	memset(pusher_obj->key, '\0', passed_key_len+1);
	strncpy(pusher_obj->key, passed_key, passed_key_len);

	pusher_obj->secret = (char *)emalloc(passed_secret_len+1);
	memset(pusher_obj->secret, '\0', passed_secret_len+1);
	strncpy(pusher_obj->secret, passed_secret, passed_secret_len);

	pusher_obj->app_id = passed_app_id;



	RETURN_TRUE;
}


PHP_METHOD(pusher, getKey) {
	zval *obj;
	obj = getThis();
	pusher_object *pusher_obj = (pusher_object *)zend_object_store_get_object(obj TSRMLS_CC);
	RETURN_STRING(pusher_obj->key, 1);
}

static char *md5_hash(const char *str) {
	md5_state_t *state;
	md5_byte_t digest[16];
	char *md5_str, *md5_str_ptr;
	int i, x;
	state = (md5_state_t *)emalloc(sizeof(md5_state_t));
	md5_str = md5_str_ptr = (char *)emalloc(33); //always 32 bytes long + '\0'
	memset(md5_str, '\0', 33);
	md5_init(state);
	md5_append(state, (const md5_byte_t *)str, strlen(str));
	md5_finish(state, digest);
	for(i=0,x=0;i<16;i++,x+=2) {
		sprintf(md5_str+x, "%02x", digest[i]);
	}
	efree(state);
	return md5_str_ptr;
}



static void pusher_free_storage(void *object TSRMLS_DC) {
	pusher_object *intern = (pusher_object *)object;
	char *key = (char *)intern->key;
	char *secret = (char *)intern->secret;
	if(key) { efree(key); }
	if(secret) { efree(secret); }
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


