#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_pusher.h"
#include "ext/standard/php_smart_str.h"
#include "md5.h"
#include "sha2.h"
#include "hmac_sha2.h"
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>


extern zend_class_entry *pusher_ce;



static zend_function_entry pusher_methods[] = {
	PHP_ME(pusher, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(pusher, trigger, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(pusher, socket_auth, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(pusher, presence_auth, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

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

static int pusher_post(char *uri, char *payload) {
	char *host = "api.pusherapp.com";
	char *port = "80";
	char *request;
	struct addrinfo *servinfo, *p, hints;
	//pad_len is roughly how many static chars there are in the request
	//This is used to calculate how much memory to allocate to construct the request
	int sock, result, content_len, request_len, pad_len = 133;

	request_len = strlen(uri) + strlen(host) + strlen(payload) + pad_len;
	content_len = strlen(payload);
        request = (char *)emalloc(request_len);

        snprintf(request, request_len, "POST %s HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s\r\n", uri, host, content_len, payload);	

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if(getaddrinfo(host, port, &hints, &servinfo) != 0) {
		return -1;
	}
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			continue;
		}
		if(connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
			close(sock);
			continue;
		}

		break; //only hit if we succeeded in getting a socket and connect'ing
	}

	if(p == NULL) {
		return -2;
	}

	freeaddrinfo(servinfo);
	p = servinfo = NULL;

	result = send(sock, request, strlen(request), 0);
	close(sock);
	efree(request);
	efree(uri);
	efree(payload);
	return result;
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

static char *socket_auth_str(pusher_object *obj, char *channel, int channel_len, char *socket_id, int socket_id_len, char *custom_data, int custom_data_len) {
	zval *ret_arr;
	char *sign_this, *signature, *auth_str, *ret_str;
	int sign_this_len, i;
	smart_str *sstr;
	unsigned char mac[SHA256_DIGEST_SIZE];
	if(custom_data) {
		sign_this_len = channel_len + socket_id_len + custom_data_len + 3;
		sign_this = (char *)emalloc(sign_this_len);
		snprintf(sign_this, sign_this_len, "%s:%s:%s", socket_id, channel, custom_data);
	}
	else {
		sign_this_len = channel_len + socket_id_len + 2;
		sign_this = (char *)emalloc(sign_this_len);
		snprintf(sign_this, sign_this_len, "%s:%s", socket_id, channel);
	}
	hmac_sha256((unsigned char *)obj->secret, strlen(obj->secret), (unsigned char *)sign_this, strlen(sign_this), mac, SHA256_DIGEST_SIZE);
	signature = (char *)emalloc(SHA256_DIGEST_SIZE * 2 + 1);
	memset(signature, 0, SHA256_DIGEST_SIZE * 2 + 1);
	for(i=0;i<SHA256_DIGEST_SIZE;i++)
		sprintf((char *)signature + i*2, "%02x", mac[i]);
	efree(sign_this);
	i = strlen(obj->key) + strlen(signature);
	auth_str = (char *)emalloc(i);
	snprintf(auth_str, i, "%s:%s", obj->key, signature);
	efree(signature);
	ALLOC_INIT_ZVAL(ret_arr);
	array_init(ret_arr);
	add_assoc_string(ret_arr, "auth", auth_str, 1);
	efree(auth_str);
	if(custom_data) {
		add_assoc_string(ret_arr, "channel_data", custom_data, 1);
	}

	sstr = emalloc(sizeof(smart_str));
	smart_str_sets(sstr, estrdup(""));
	php_json_encode(sstr, ret_arr, 0);
	smart_str_0(sstr);
	ret_str = estrdup(sstr->c);
	smart_str_free(sstr);
	return ret_str;
}

static zend_object_value pusher_object_create(zend_class_entry *class_type TSRMLS_DC) {
        zend_object_value retval;
        pusher_object *intern;
        intern = (pusher_object *) emalloc(sizeof(pusher_object));
        memset(intern, 0, sizeof(pusher_object));
        intern->zo.ce = class_type;
        ALLOC_HASHTABLE(intern->zo.properties);
        zend_hash_init(intern->zo.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
#if ZEND_MODULE_API_NO >= 20100525
        object_properties_init(&(intern->zo), class_type);
#else
        zval *tmp;
        zend_hash_copy(intern->zo.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
#endif

        retval.handle = zend_objects_store_put(intern, NULL, pusher_free_storage, NULL TSRMLS_CC);
        retval.handlers = (zend_object_handlers *) &pusher_object_handlers;
        return retval;
}


PHP_METHOD(pusher, __construct) {
	zval *this;
	char *passed_key;
	char *passed_secret;
	long int passed_app_id;
	int passed_key_len;
	int passed_secret_len;
	
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
	char *channel, *event, *payload, *socket_id, *body_md5, *sign_this, *post_payload, *post_uri;
	unsigned long int channel_len, event_len, payload_len, socket_id_len, signature_len, i, result;
	const char *host = "http://api.pusherapp.com";
	char uri[512]; //TODO: revisit this and dynamically allocate just enough memory.
	char query_string[512]; //TODO: revisit this and dynamically allocate just enough memory.
	char full_uri[1024]; //TODO: revisit this and dynamically allocate just enough memory.
	char strtime[32]; //32 seems like a nice round number.
	unsigned char signature[2 * SHA256_DIGEST_SIZE + 1]; //sha256 digest size is 32, but displaying it as ascii takes two bytes per digest byte + '\0'
	unsigned char mac[SHA256_DIGEST_SIZE];
	

	channel = event = payload = socket_id = body_md5 = sign_this = post_payload = post_uri = NULL;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|s", &channel, &channel_len, &event, &event_len, &payload, &payload_len, &socket_id, &socket_id_len) == FAILURE) {
		RETURN_FALSE;
	}

	this = getThis();
	obj = (pusher_object *)zend_object_store_get_object(this TSRMLS_CC);
	
	snprintf(uri, 512, "/apps/%d/channels/%s/events", obj->app_id, channel);

	body_md5 = md5_hash(payload);
	snprintf(strtime, 32, "%d", time(NULL));

	//generating a signature is a *pain*.  I'll try and annotate.
	signature_len = 5; // 'POST\n'
	signature_len += strlen(uri) + 1; // 'uri\n'
	signature_len += 9 + strlen(obj->key); // 'auth_key=<auth_key>'
	signature_len += 17 + strlen(strtime); // '&auth_timestamp=<timestamp>'
	signature_len += strlen("&auth_version=1.0"); // <--
	signature_len += 10 + strlen(body_md5); //&body_md5=<body_md5>
	signature_len += 6 + strlen(event); //&name=<event_name>
	if(socket_id) { signature_len += 12 + strlen(socket_id); }//&socket_id=<socket_id>

	sign_this = (char *)emalloc(signature_len + 1);
	memset(sign_this, '\0', signature_len + 1);
	snprintf(sign_this, signature_len, "POST\n%s\nauth_key=%s&auth_timestamp=%s&auth_version=1.0&body_md5=%s&name=%s", uri, obj->key, strtime, body_md5, event);
	
	if(socket_id) {
		i = strlen(sign_this);
		snprintf(sign_this+i, signature_len-i, "&socket_id=%s", socket_id);

	}


	hmac_sha256((unsigned char *)obj->secret, strlen(obj->secret), (unsigned char *)sign_this, strlen(sign_this), mac, SHA256_DIGEST_SIZE);

	for(i=0;i<SHA256_DIGEST_SIZE;i++)
		sprintf((char *)signature + i*2, "%02x", mac[i]);

	signature[2 * SHA256_DIGEST_SIZE] = '\0';

	efree(sign_this);

	snprintf(query_string, 512, "?auth_key=%s&auth_timestamp=%s&auth_version=1.0&body_md5=%s&name=%s&auth_signature=%s", obj->key, strtime, body_md5, event, signature);
	snprintf(full_uri, 1024, "%s%s", uri, query_string);

	efree(body_md5);
	
	post_payload = (char *)estrdup(payload);
	post_uri = (char *)estrdup(full_uri);

	result = pusher_post(post_uri, post_payload);
	if(result >= 0) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}


PHP_METHOD(pusher, socket_auth) {
        zval *this;
        char *channel, *socket_id, *custom_data, *ret_str;
        int channel_len, socket_id_len, custom_data_len;
        pusher_object *obj;

        channel = socket_id = custom_data = ret_str = NULL;
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|s", &channel, &channel_len, &socket_id, &socket_id_len, &custom_data, &custom_data_len) == FAILURE) {
                RETURN_FALSE;
        }


        this = getThis();
        obj = (pusher_object *)zend_object_store_get_object(this TSRMLS_CC);
        ret_str = socket_auth_str(obj, channel, channel_len, socket_id, socket_id_len, custom_data, custom_data_len);
        RETURN_STRING(ret_str, 0);
}

PHP_METHOD(pusher, presence_auth) {
        zval *this, *ret_arr, *user_info;
        smart_str *sstr;
        char *channel, *socket_id, *user_data, *ret_str;
	long user_id;
        int channel_len, socket_id_len, i;
        pusher_object *obj;

        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssl|z", &channel, &channel_len, &socket_id, &socket_id_len, &user_id, &user_info) == FAILURE) {
                RETURN_FALSE;
        }

	this = getThis();
	obj = (pusher_object *)zend_object_store_get_object(this TSRMLS_CC);
	ALLOC_INIT_ZVAL(ret_arr);
	array_init(ret_arr);
	add_assoc_long(ret_arr, "user_id", user_id);
	if(user_info != NULL) {
		add_assoc_zval(ret_arr, "user_info", user_info);
	}
	sstr = emalloc(sizeof(smart_str));
	smart_str_sets(sstr, estrdup(""));
	php_json_encode(sstr, ret_arr, 0);
	smart_str_0(sstr);
	user_data = estrdup(sstr->c);
	smart_str_free(sstr);
	ret_str = socket_auth_str(obj, channel, channel_len, socket_id, socket_id_len, user_data, strlen(user_data));
	efree(user_data);
	RETURN_STRING(ret_str, 0);
}


void register_pusher_class(TSRMLS_D) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "Pusher", pusher_methods);
	ce.create_object = pusher_object_create;
	memcpy(&pusher_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	pusher_object_handlers.clone_obj = NULL;
	pusher_ce = zend_register_internal_class(&ce TSRMLS_CC);
}


