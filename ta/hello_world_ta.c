

#define STR_TRACE_USER_TA "HELLO_WORLD"

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hello_world_ta.h"
static TEE_OperationHandle cipher = TEE_HANDLE_NULL;
static TEE_OperationHandle decipher = TEE_HANDLE_NULL;
static char key[16]={0};
static char iv[16]={0};
static char objID[] = "96c5d1b260aa4de30fedaf67e5b9227613abebff172a2b4e949994b8e561e2fb";
static size_t objID_len = 64;


char* readKeyObj (size_t strLen);
int createKeyObj(char buffer[16], size_t strLen );


/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");

	return TEE_SUCCESS;
}
static int myStrlen(const char* m)
{
	int c;
	for (c = 0; *m; ++c, ++m);
	return c;
}
static void myMemcpy(void* dest, void* src, unsigned len)
{
	for (unsigned i = 0; i < len; ++i)
		((char*)dest)[i] = ((char*)src)[i];
}
/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Hello World!\n");

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}

static TEE_Result inc_value(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("Got value: %u from NW", params[0].value.a);
	params[0].value.a++;
	IMSG("Increase value to: %u", params[0].value.a);

	return TEE_SUCCESS;
}

static void myMemset(void* dest, char b, unsigned len)
{
	for (unsigned i = 0; i < len; ++i)
		((char*)dest)[i] = b;
}

char* readKeyObj (size_t strLen){
	uint32_t flags = TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE;
	TEE_ObjectHandle object = (TEE_ObjectHandle)NULL;
	TEE_ObjectInfo info;
	TEE_Result ret;
	void* p = NULL;
	uint32_t read_bytes = 0;


	ret = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
										 (void *)objID, objID_len, flags, &object);
	if (ret != TEE_SUCCESS)
	{
		EMSG("TEE_OpenPersistentObject failed.\n");
		goto done;
	}
	TEE_GetObjectInfo(object, &info);
	DMSG("Object data size : %"PRIu32" and position : %" PRIu32"", info.dataSize, info.dataPosition);
	p = TEE_Malloc(strLen + 1, 0);
	if (p == NULL){
		EMSG("NO p :(");
		goto done;
	}

	myMemset(p, 0, strLen);
	ret = TEE_ReadObjectData(object, p, strLen, &read_bytes);
	if (ret != TEE_SUCCESS)
	{
		EMSG("TEE_ReadObjectData failed.\n");
		goto done;
	}
	IMSG("Got %d bytes of data:%s\n", read_bytes, (char*)p);
done:
	TEE_CloseObject(object);
	return (char*) p;
}


int createKeyObj(char buffer[16], size_t strLen ){
	TEE_Result ret;
	TEE_ObjectHandle object = (TEE_ObjectHandle)NULL;
	uint32_t flags = TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE;
	uint32_t read_bytes = 0;
	TEE_ObjectInfo info;
	int status = 1;
	void* p;

	IMSG ("%s\n", buffer);
	IMSG("Pre TEE_CreatePersistentObject\n");
	ret = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, (void *)objID, objID_len,
									TEE_DATA_FLAG_ACCESS_WRITE_META,
									(TEE_ObjectHandle)NULL, buffer, strLen,
									(TEE_ObjectHandle *)NULL);
	if (ret != TEE_SUCCESS)
	{
		IMSG("********************\n");
		EMSG("TEE_CreatePersistentObject failed.\n");
		goto done;
	}
	ret = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
							       (void *)objID, objID_len, flags, &object);
	if (ret != TEE_SUCCESS)
	{
		EMSG("TEE_OpenPersistentObject failed.\n");
		goto done;
	}
	TEE_GetObjectInfo(object, &info);
	DMSG("Object data size : %"PRIu32" and position : %" PRIu32"", info.dataSize, info.dataPosition);
	p = TEE_Malloc(strLen + 1, 0);
	if (p == NULL){
		EMSG("NO p :(");
		goto done;
	}

	myMemset(p, 0, strLen);
	ret = TEE_ReadObjectData(object, p, strLen, &read_bytes);
	if (ret != TEE_SUCCESS)
	{
		EMSG("TEE_ReadObjectData failed.\n");
		goto done;
	}
	IMSG("Got %d bytes of data:%s\n", read_bytes, (char*)p);
	status = 0;
done:
	TEE_CloseObject(object);
	return status;
}

static TEE_Result Cps_init(uint32_t param_types,
	TEE_Param params[4])
	{
		char* src=NULL;
		char h[20]={0}, H[40]={0};
		uint32_t i=0, j=0,f=0;
		TEE_OperationHandle handle = TEE_HANDLE_NULL;
		TEE_Result res;
		uint32_t sz = params[0].memref.size;
		uint32_t hSize = 20;
		int response;

		//size_t HSize = 40;
		IMSG("***CPS_ENCRYPT***\n");
		param_types = param_types;
		src = TEE_Malloc(params[0].memref.size, 0);
		TEE_MemMove(src, params[0].memref.buffer, params[0].memref.size);
		IMSG("Alloc op\n");
		res = TEE_AllocateOperation(&handle, TEE_ALG_SHA1, TEE_MODE_DIGEST, 0);
		IMSG("Alloc object %x\n",res);
		TEE_DigestUpdate(handle, src, sz);
		IMSG("digest update");
		IMSG("not hashed: %s\n",h);
		res = TEE_DigestDoFinal(handle, src, sz, h, &hSize);
		IMSG("\n");
		IMSG("hashed: %s\n",h);

		for (i=0;i<2;i++){
			for (j=0;j<hSize;j++){
				H[f]=h[j];
				f++;
			}
		}
		IMSG("H is double hashed: %s", H);
		f=0;
		for (j=0;j<16;j++){
			f++;
		}
		for (j=0;j<16;j++){
			key[j]=H[f];
			f++;
		}



		IMSG("key %s", key);
		response = createKeyObj(key, (size_t) myStrlen(key));
		if (!response){
			IMSG ("key file is stored.");
			return TEE_SUCCESS;
		}
		else{
			EMSG("Cannot create key file");
		}


		return TEE_SUCCESS;
	}

static TEE_Result Cps_encrypt(uint32_t param_types,
	TEE_Param params[4])
	{
		char* src=NULL, *tmp = NULL;
		char out[1024] = {0};
		//TEE_Attribute attr = {0};
		TEE_Result res;
		uint32_t sz,srcSz;
		TEE_ObjectHandle outcome;
		TEE_Attribute attr;
		TEE_ObjectInfo KeyInfo = {0};
		TEE_OperationInfo operationInfo = {0};
		param_types = param_types;
		tmp = readKeyObj(myStrlen(key));
		IMSG("***CPS_INIT***\n");
		if (tmp == NULL){
			EMSG("You did not init the key");
			return TEE_SUCCESS;
		}
		myMemcpy(key,tmp,myStrlen(key));
		res = TEE_AllocateTransientObject(TEE_TYPE_AES, 128, &outcome);
		IMSG("alloc Transient object %x\n",res);
		TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, key, 16);
		IMSG("TEE_InitRefAttribute %x\n",res);
		res = TEE_PopulateTransientObject(outcome, &attr, 1);
		IMSG("populate transient object %x\n",res);
		TEE_GetObjectInfo(outcome, &KeyInfo);
		DMSG("%s: maxObjectSize = (%08x) objectSize = (%08x).",__func__, KeyInfo.maxObjectSize, KeyInfo.objectSize);
		res = TEE_AllocateOperation(&cipher, TEE_ALG_AES_CBC_NOPAD, TEE_MODE_ENCRYPT, 128);
		TEE_GetOperationInfo(cipher, &operationInfo);
		IMSG("Alloc object cipher %x\n",res);
		res = TEE_SetOperationKey(cipher, outcome);
		DMSG("%s: maxObjectSize = (%08x) objectSize = (%08x).",__func__, operationInfo.maxKeySize, operationInfo.keySize);
		IMSG("Cipher TEE_SetOperationKey %x\n",res);
		TEE_CipherInit(cipher, iv, 16);
		src = TEE_Malloc(params[0].memref.size, 0);
		srcSz = params[0].memref.size;
		TEE_MemMove(src, params[0].memref.buffer, params[0].memref.size);
		DMSG ("I will handle the message");
		IMSG ("%s\n",src);
		sz = 1024	;
		res = TEE_CipherDoFinal(cipher,src, srcSz, out, &sz);
		IMSG("sz: %d",sz);
		IMSG("crypted final: %s", out);
		IMSG("res: %x\n",res);

		TEE_MemMove(params[0].memref.buffer, out, myStrlen(out)+1);
		params[0].memref.size= myStrlen((char*)out)+1;
		return TEE_SUCCESS;
	}

	static TEE_Result Cps_decrypt(uint32_t param_types,
		TEE_Param params[4])
	{
		char* src = NULL, *tmp = NULL;
		char out[1024] = {0};
		uint32_t sz,srcSz;
		TEE_ObjectHandle outcome;
		TEE_Attribute attr;
		TEE_ObjectInfo KeyInfo;
		TEE_OperationInfo operationInfo;
		TEE_Result res;
		params = params;
		param_types = param_types;

		IMSG("Cps_decrypt\n");
		//TEE_Attribute attr = {0};
		tmp = readKeyObj(myStrlen(key));
		if (tmp == NULL){
			EMSG("You did not init the key");
			return TEE_SUCCESS;
		}
		myMemcpy(key,tmp,myStrlen(key));

		res = TEE_AllocateOperation(&decipher, TEE_ALG_AES_CBC_NOPAD, TEE_MODE_DECRYPT, 128);
		IMSG("Alloc object cipher %x\n",res);
		IMSG("iv: %s", iv);
		IMSG("key %s", key);
		res = TEE_AllocateTransientObject(TEE_TYPE_AES, 128, &outcome);
		IMSG("alloc Transient object %x\n",res);
		TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, key, 16);
		IMSG("TEE_InitRefAttribute %x\n",res);
		res = TEE_PopulateTransientObject(outcome, &attr, 1);
		IMSG("populate transient object %x\n",res);
		TEE_GetObjectInfo(outcome, &KeyInfo);
		DMSG("%s: maxObjectSize = (%08x) objectSize = (%08x).",__func__, KeyInfo.maxObjectSize, KeyInfo.objectSize);
		TEE_GetOperationInfo(decipher, &operationInfo);
		IMSG("Alloc object cipher %x\n",res);
		res = TEE_SetOperationKey(decipher, outcome);
		DMSG("%s: maxObjectSize = (%08x) objectSize = (%08x).",__func__, operationInfo.maxKeySize, operationInfo.keySize);
		IMSG("Decipher TEE_SetOperationKey %x\n",res);
		TEE_CipherInit(decipher, iv, 16);
		IMSG("decipher is ready");


		src = TEE_Malloc(params[0].memref.size, 0);
		srcSz = params[0].memref.size;
		srcSz = srcSz;
		TEE_MemMove(src, params[0].memref.buffer, params[0].memref.size);
		DMSG ("I will handle the message");
		IMSG ("%s\n",src);
		sz = 1024	;
		res = TEE_CipherDoFinal(decipher,src, srcSz, out, &sz);
		IMSG("sz: %d",sz);
		IMSG("decrypted final: %s", out);
		IMSG("res: %x\n",res);

		TEE_MemMove(params[0].memref.buffer, out, myStrlen(out)+1);
		params[0].memref.size= myStrlen((char*)out)+1;

		return TEE_SUCCESS;

	}




static TEE_Result dec_value(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("Got value: %u from NW", params[0].value.a);
	params[0].value.a--;
	IMSG("Decrease value to: %u", params[0].value.a);

	return TEE_SUCCESS;
}
/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	IMSG("Got Command id with value %d\n", cmd_id);
	switch (cmd_id) {
	case TA_HELLO_WORLD_CMD_INC_VALUE:
		return inc_value(param_types, params);
	case TA_HELLO_WORLD_CMD_DEC_VALUE:
		return dec_value(param_types, params);
	case CPS_INIT:
		return Cps_init(param_types, params);
	case CPS_PROTECT:
		return Cps_encrypt(param_types, params);
	case CPS_VIEW:
		return Cps_decrypt(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
