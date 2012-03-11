#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pusher.h"
#include "md5.h"
#include "sha2.h"
#include "hmac_sha2.h"
#include <curl/curl.h>

extern zend_class_entry *pusher_ce;

static zend_function_entry pusher_methods[] = {
	PHP_ME(pusher, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(pusher, getKey, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(pusher, trigger, NULL, ZEND_ACC_PUBLIC)
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


PHP_METHOD(pusher, trigger) {
	zval *this;
	pusher_object *obj;
	char strtime[32]; //32 seems like a nice round number.
	char *channel, *event, *payload;
	unsigned long int channel_len, event_len, payload_len, signature_len, i;
	CURL *curl;
	CURLcode res;
	const char *host = "http://api.pusherapp.com";
	char uri[512]; //TODO: revisit this and dynamically allocate just enough memory.
	char query_string[512]; //TODO: revisit this and dynamically allocate just enough memory.
	char url[1024]; //TODO: revisit this and dynamically allocate just enough memory.
	unsigned char signature[2 * SHA256_DIGEST_SIZE + 1]; //sha256 digest size is 32, but displaying it as ascii takes two bytes per digest byte + '\0'
	unsigned char mac[SHA256_DIGEST_SIZE];
	char *body_md5, *sign_this;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &channel, &channel_len, &event, &event_len, &payload, &payload_len) == FAILURE) {
		RETURN_FALSE;
	}

	this = getThis();
	obj = (pusher_object *)zend_object_store_get_object(this TSRMLS_CC);
	
	snprintf(uri, 2000, "/apps/%d/channels/%s/events", obj->app_id, channel);

	body_md5 = md5_hash(payload);
	snprintf(strtime, 32, "%d", time(NULL));

	//generating a signature is a bitch.  I'll try and annotate.
	signature_len = 5; // 'POST\n'
	signature_len += strlen(uri) + 1; // 'uri\n'
	signature_len += 9 + strlen(obj->key); // 'auth_key=<auth_key>'
	signature_len += 17 + strlen(strtime); // '&auth_timestamp=<timestamp>'
	signature_len += strlen("&auth_version=1.0"); // <--
	signature_len += 10 + strlen(body_md5); //&body_md5=<body_md5>
	signature_len += 6 + strlen(event); //&name=<event_name>

	sign_this = (char *)emalloc(signature_len + 1);
	memset(sign_this, '\0', signature_len + 1);
	snprintf(sign_this, signature_len, "POST\n%s\nauth_key=%s&auth_timestamp=%s&auth_version=1.0&body_md5=%s&name=%s", uri, obj->key, strtime, body_md5, event);

	hmac_sha256((unsigned char *)obj->secret, strlen(obj->secret), (unsigned char *)sign_this, strlen(sign_this), mac, SHA256_DIGEST_SIZE);

	for(i=0;i<SHA256_DIGEST_SIZE;i++)
		sprintf((char *)signature + i*2, "%02x", mac[i]);

	signature[2 * SHA256_DIGEST_SIZE] = '\0';

	efree(sign_this);

	snprintf(query_string, 512, "?auth_key=%s&auth_timestamp=%s&auth_version=1.0&body_md5=%s&name=%s&auth_signature=%s", obj->key, strtime, body_md5, event, signature);
	snprintf(url, 1024, "%s%s%s", host, uri, query_string);

	efree(body_md5);
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)payload);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	else {
		RETURN_FALSE;
	}
	RETURN_LONG(res);

}



PHP_METHOD(pusher, getKey) {
	zval *obj;
	obj = getThis();
	pusher_object *pusher_obj = (pusher_object *)zend_object_store_get_object(obj TSRMLS_CC);
	RETURN_STRING(pusher_obj->key, 1);
}

static size_t curl_write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
	return size * nmemb;
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


